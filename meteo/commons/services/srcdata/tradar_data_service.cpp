#include "tradar_data_service.h"

#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/global/global.h>

#include <qmap.h>
#include <qprocess.h>
#include <qfile.h>
#include <QString>

#include <sql/nosql/nosqlquery.h>
#include <sql/dbi/gridfs.h>

#include <qjsonobject.h>
#include <qjsondocument.h>


namespace meteo {
namespace surf {

static const QString kPrefix = "radarmap";

TRadarDataService::TRadarDataService( )

{
}

void TRadarDataService::getAllMrlData(const meteo::surf::DataRequest* req,
                                      meteo::surf::ManyMrlValueReply* res)
{
  auto return_func = [res](QString err)
  {
    error_log << err;
    res->set_result(false);
    res->set_comment(err.toStdString());
    return;
  };

  res->set_result(false);
  if(!req->has_date_start()|| !req->has_level_h() || !req->has_level_h2()
     || 1 != req->meteo_descr_size() ){
    return return_func(QObject::tr("Запрос заполнен не полностью"));
  }
  if ( 0 == req->type_size() ) {
    return return_func(QObject::tr("Запрос заполнен не полностью"));
  }
  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) {
    return return_func(QObject::tr("Не удается подключиться к в базе данных"));
  }
  auto query = db->queryptrByName("get_all_mrl_data");
  if( nullptr == query.get() ) {
    return return_func("");
  }

  int datatype = req->type(0);
  int descr = req->meteo_descr(0);
  query->arg("data_type",datatype);
  query->arg("h1",req->level_h());
  query->arg("h2",req->level_h2());
  query->arg("product",descr);

  QDateTime dt = QDateTime::fromString(QString::fromStdString(req->date_start()), Qt::ISODate);
  query->arg("dt",dt);

  GridFs gfs;
  if ( false == gfs.connect(global::mongodbConfMeteo() )) {
    return return_func(gfs.lastError());
  }
  gfs.use(global::mongodbConfMeteo().name(),kPrefix);

  QString error;
  if(false == query->execInit( &error)){
    return return_func(error);
  }

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    meteo::surf::OneMrlValueReply* one_res = res->add_values();
    bool ok = false;
    QString id = doc.valueOid("fileid", &ok);
    one_res->set_station( doc.valueString("station").toInt() );
    if ( !ok ) {
      error_log << QObject::tr("Не найден идентификатор файла в поле fileid");
      debug_log << doc.jsonExtendedString();
      continue;
    }
    GridFile file = gfs.findOneById(id);
    if ( !file.hasFile() ) {
      error_log << QObject::tr("Файл %1 не найден").arg(id);
      res->mutable_values()->RemoveLast();
      continue;
    }

    TMeteoData md;

    QByteArray ba = file.readAll(&ok);
    md << ba;
    if ( !ok ) {
      error_log << QObject::tr("Не удалось загрузить файл %1.").arg(id) << file.lastError();
      res->mutable_values()->RemoveLast();
      continue;
    }
    if ( !toProto(descr, md, one_res) ) {
      res->mutable_values()->RemoveLast();
    }
  }
  res->set_result(true);
}



bool TRadarDataService::getAvailableRadar( const meteo::surf::DataRequest* req,
                                           meteo::surf::DataDescResponse* res )
{
  auto return_func = [res](QString err)
  {
    error_log << err;
    res->set_result(false);
    res->set_comment(err.toStdString());
    return false;
  };

  res->set_result(false);
  if ( 0 == req->type_size() ) {
    return return_func(QObject::tr("Запрос заполнен не полностью"));
  }
  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) {
    return return_func(QObject::tr("Не удается подключиться к в базе данных"));
  }
  std::unique_ptr<DbiQuery> query = db->queryptrByName("get_available_radar");
  if(nullptr == query.get()) {return return_func("");}
  int datatype = req->type(0);
  query->arg("data_type",datatype);

  if ( true == req->has_level_h()){
    query->arg("h1",req->level_h());
  }
  if ( true == req->has_level_h2()){
    query->arg("h2",req->level_h2());
  }
  if ( req->meteo_descr_size() > 0 ) {
    query->arg("product", req->meteo_descr(0));
  }
  QDateTime dtBegin = req->has_date_start() ?
        QDateTime::fromString(QString::fromStdString(req->date_start()), Qt::ISODate) :
        QDateTime::fromMSecsSinceEpoch(0);

  QDateTime dtEnd = req->has_date_end() ?
        QDateTime::fromString(QString::fromStdString(req->date_end()), Qt::ISODate) :
        QDateTime::currentDateTime();
  query->arg("start_dt",dtBegin);
  query->arg("end_dt",dtEnd);

  QString error;
  if(false == query->execInit( &error)){
    return return_func(error);
  }

  QHash<QDateTime, QHash<qint32, QHash<qint32, QHash<qint32, ::meteo::surf::DataDesc*>>>> bigHash;

  while ( true == query->next() ) {
    const DbiEntry& doc = query->entry();
    QDateTime dt = doc.valueDt("dt");
    int h1 = (int) doc.valueDouble("h1");
    int h2 = (int) doc.valueDouble("h2");
    int meteodescr = (int) doc.valueDouble("product");
    QString station = doc.valueString("station");

    ::meteo::surf::DataDesc* data = bigHash[dt][h1][h2][meteodescr];
    if (nullptr == bigHash[dt][h1][h2][meteodescr]){
      data = res->add_descr();
      bigHash[dt][h1][h2][meteodescr] = data;
      data->set_count(0);
    }

    data->set_date(dt.toString(Qt::DateFormat::ISODate).toStdString());
    data->set_level(h1);
    data->set_h2(h2);
    data->set_meteodescr(meteodescr);
    data->set_count(data->count() + 1);
    data->set_station( station.toInt() );
  }
  res->set_result(true);
  return true;
}


void TRadarDataService::setMeteoDataData(meteo::surf::OneMrlValueReply* r_par) {

  meteo::GeoPoint fl_0, fl ;
  float dx = r_par->dx();
  float dy = r_par->dy();
  float rad_dx =dx/6371000.;
  float ddx = dx*r_par->kol_x()*0.5;
  float ddy = dy*r_par->kol_y()*0.5;
  float toconer = sqrt(ddx*ddx+ddy*ddy)/6371000.;
  fl_0 = GeoPoint::fromDegree(r_par->center().fi(),r_par->center().la());
  fl_0 = fl_0.findSecondCoord(toconer,M_PI*1.25);

  int kol_x = r_par->kol_x();
  int kol_y = r_par->kol_y();

  QVector <double> net_fi_(kol_y); //!координаты узлов сетки
  QVector <double> net_la_(kol_x); //!координаты узлов сетки

  //  net_fi_.resize(r_par->kol_y());
  //  net_la_.resize(r_par->kol_x());
  net_fi_.fill(0.);
  net_la_.fill(0.);

  fl = fl_0;
  for(int i = 0; i< kol_x ;++i){//по долготе
    net_la_[i] = fl.la();
    fl = fl.findSecondCoord(rad_dx,M_PI*0.5);
  }
  fl=fl_0;
  for(int j = 0; j< kol_y ;++j){//по широте
    net_fi_[j]  = fl.fi();
    fl = fl.findSecondCoord(rad_dx,0.);
  }
  int data_count = r_par->rdata_size();


  for (int h =0; h< data_count;h++){
    int num_kvadrat = r_par->rdata(h).kvadrat();
    int i = int(num_kvadrat/kol_x );
    int j = num_kvadrat - i*kol_x;
    if ( i >= kol_x || j >= kol_y ) {
      error_log << QObject::tr("Выход за границы массива. i = %1, j = %2. Дата = %3. Дескриптор = %4")
                   .arg( i )
                   .arg( j )
                   .arg( QString::fromStdString( r_par->date() ) )
                   .arg( r_par->meteo_descr() );
      continue;
    }
    ::meteo::surf::MeteoData* md = r_par->add_mdata();
    md->mutable_point()->set_fi(net_fi_[i]);
    md->mutable_point()->set_la(net_la_[j]);
    md->mutable_value()->set_value(r_par->rdata(h).value().value());
    md->mutable_value()->set_quality(true);
  }

}


bool TRadarDataService::toProto(int descr, const TMeteoData &md, meteo::surf::OneMrlValueReply* res)
{

  //md.printData();
  QList<int> keys = md.getParamList(KVADRATS_DSCR).keys();
  int cnt = keys.count();
  float fi, la;
  if(0 == cnt ) {
    return false;
  }
  if(! TMeteoDescriptor::instance()->getCoord(md, &fi, &la)){
    error_log << "не удалось определить координаты станции";
    return false;
  } else {
    res->mutable_center()->set_fi(fi);
    res->mutable_center()->set_la(la);
    res->mutable_center()->set_height(md.getParam(HEIGHT_DSCR).value());
  }
  res->set_date(TMeteoDescriptor::instance()->dateTime(md).toString().toStdString());

  QMap<int, TMeteoParam> levels =md.getParamList(level_DSCR);
  QList <int> keysl = levels.keys();
  for(int i=0; i< keysl.count(); i++){
    res->add_level(levels.value(keysl.at(i)).value());
  }
  res->set_meteo_descr(descr);
  res->set_dx(md.getParam(dX_DSCR).value());
  res->set_dy(md.getParam(dY_DSCR).value());
  res->set_kol_x(md.getParam(KOLX_DSCR).value());
  res->set_kol_y(md.getParam(KOLY_DSCR).value());
  res->set_is_abs_h(md.getParam(isabsh_DSCR).value());
  res->set_proj(md.getParam(proj_DSCR).value());
//  res->set_station(md.getParam(STATION_DSCR).value());
  for (int idx = 0 ; idx <cnt ; ++idx) {
    MrlData *ad = res->add_rdata();
    int akey = keys.at(idx);
    TMeteoParam p;
    p = md.getParam(descr, akey);
    int kv = md.getParam(KVADRATS_DSCR, akey).value();
    ad->set_kvadrat(kv);
    ad->mutable_value()->set_value(p.value());
    ad->mutable_value()->set_quality(p.quality());
    if ( true == md.hasParam(SYNOP_DIRECTION) ) {
      res->set_synopdirection( md.getParam(SYNOP_DIRECTION).value() );
    }
    if ( true == md.hasParam(SYNOP_DIRECTION) ) {
      res->set_synopspeed( md.getParam(SYNOP_SPEED).value() );
    }
  }
  setMeteoDataData(res);
  return true;
}
}
}
