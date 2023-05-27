#include "fieldanalysewrap.h"

#include <qelapsedtimer.h>
#include <qcoreapplication.h>
#include <qdir.h>
#include <qmap.h>
#include <qprocess.h>
#include <qstring.h>
#include <qvector.h>
#include <qmath.h>

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/textproto/pbtools.h>

#include <sql/dbi/gridfs.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/zond/zond.h>
#include <meteo/commons/services/obanal/tobanal.h>

constexpr int  kMaxForecastTime = 6 * 3600; //!< максимальное время действия прогноза
const int  kMinAdvStep = 1800; //!< минимальный шаг по времени для адвекции

namespace meteo {
namespace field {

TFieldAnalyseWrap::TFieldAnalyseWrap(TFieldDataService* service)
  :MethodWrap(service)
{
}

TFieldAnalyseWrap::~TFieldAnalyseWrap()
{
}

/*
bool meteo::field::TFieldDataService::runMethod<meteo::field::TFieldAnalyseWrap, meteo::field::DataAnalyseRequest const*,
meteo::field::SimpleDataReply*>(google::protobuf::RpcController*, meteo::field::TFieldAnalyseWrap*,
                    void (meteo::field::TFieldAnalyseWrap::*)
                    (meteo::field::DataAnalyseRequest const*, meteo::field::SimpleDataReply*),
                    meteo::field::DataAnalyseRequest const*, meteo::field::SimpleDataReply*)'
*/

void TFieldAnalyseWrap::getSynSit(const DataRequest* req, ValueDataReply* resp)
{
  QTime ttt; ttt.start();

  QString error;
  DataDescResponse response;

  if ( false == getAvailableForecastData(req, &response ) ) {
      info_log << ( error = msglog::kDataServiceNoData );
      resp->set_error(error.toStdString());
      return;
    }

  for(const DataDesc& descr : response.descr()) {
      QString field_id = QString::fromStdString(descr.id()); // TODO change int32 to std::string in proto
      if ( getOnePointSynSit(field_id, req, resp)) {
          continue;
        }
    }

  int cur =  ttt.elapsed();
  if (1000 < cur) {
      debug_log << msglog::kServiceRequestTime.arg("getSynSit").arg(cur);
    }

  if ( 0 < resp->data_size() ) {
      resp->set_error(msglog::kServerAnswerOK.arg("getSynSit").toStdString());
      return;
    }

  info_log << ( error = msglog::kDataServiceNoData );
  resp->set_error(error.toStdString());
}

void TFieldAnalyseWrap::GetAdvectObject( const AdvectObjectRequest* req, AdvectObjectReply* resp)
{
  resp->set_total_adv_time("");
  if((1 > req->skelet_size() || !req->has_adv_time() || !req->has_date())) {
      resp->set_error(msglog::kServiceRequestFailedErr.toStdString());
      return;
    }
  int small_step = kMinAdvStep;
  QDateTime start_time = QDateTime::fromString(QString::fromStdString(req->date()), Qt::ISODate);
  if(false == (start_time.isValid() )) {
      resp->set_error(msglog::kServiceRequestFailedErr.toStdString());
      return;
    }
  QVector<DataDesc> fdescrlist;
  QMap<QString,QVector<meteo::GeoVector>> result_dots;
  QMap<QDateTime, QString> fields_for_advect;
  getFieldsIdForAdvect(req, &fields_for_advect);
  if(fields_for_advect.empty()) {
      resp->set_error(msglog::kDataServiceNoData.toStdString());
      return;
    }
  if(req->adv_time() < 0){
      small_step = -1*small_step;
    }
  advect(req,fields_for_advect,&result_dots,&fdescrlist);

  if(result_dots.size()<1){
      return;
    }
  int astep = 0;

  foreach (meteo::field::Skelet cursc, req->skelet()) {
    meteo::field::Skelet *skel = resp->add_skelets();
    QString uuid = QString::fromStdString(cursc.uuid());
    skel->CopyFrom(cursc);

    int step = req->has_time_step() ? req->time_step() : small_step;
    if(step < small_step) step = small_step;
    int st = 0;
    if(!req->has_need_prom_points() || !req->need_prom_points()) {
        st = result_dots.first().size()-1;
      }
    const QVector<meteo::GeoVector>& curdot = result_dots.value(uuid);

    for(int i = 0; i < curdot.size();++i ){//последовательно по точкам скелета
        const GeoVector &gv = curdot.at(i);
        if(skel->dots_size() <= i) break;
        meteo::field::Skelet_Dot *dot = skel->mutable_dots(i);
        GeoPoint gp;
        astep = 0;
        for(int j = st; j<gv.size();++j ){//последовательно по точкам адвекции
            astep += small_step;
            if(astep%step!=0 && j!=gv.size()-1)continue;
            gp = gv.at(j);
            meteo::surf::Point *p = dot->add_coord();
            p->set_fi(gp.fi());
            p->set_la(gp.la());
            p->set_height(gp.alt());
            p->set_date_time(start_time.addSecs(astep).toString(Qt::ISODate).toStdString());
          }
        if(req->has_need_length() && true == req->need_length() && 1<gv.size()){
            float length = 0;
            int sz = gv.size()-1;
            for(int j = 0; j<sz;++j ){//последовательно по точкам адвекции
                length+=gv.at(j).calcDistance(gv.at(j+1));
             }
            dot->set_length(length);
          }
      }
    }

  for(int i=0; i < fdescrlist.size(); ++i) {
      resp->add_descr()->CopyFrom(fdescrlist.at(i));
    }
  resp->set_total_adv_time(start_time.addSecs(astep).toString(Qt::ISODate).toStdString());
}

int findNearestDt(const QList<QDateTime>&dtlist, const QDateTime &dt){
  if(dtlist.size()<1) return -1;
  int minSeks = abs(dtlist.at(0).secsTo(dt));
  int minnum=0;
  for(int i=0;i< dtlist.size();++i){
      int cur_sek = abs(dtlist.at(i).secsTo(dt));
      if(cur_sek < minSeks){
         minnum = i;
         minSeks =cur_sek;
        }
    }
  return minnum;
}

void skeletToGVector(const meteo::field::Skelet& sc, int num, GeoVector* gv){
  foreach (meteo::field::Skelet::Dot dot, sc.dots()) {
      if(dot.coord().size() > num){
          gv->append(meteo::GeoPoint(dot.coord(num).fi(),dot.coord(num).la(),dot.coord(num).height()));
        }
    }
}

void TFieldAnalyseWrap::advect(const AdvectObjectRequest* req,
                                    const QMap<QDateTime, QString> &fields_for_advect,
                                    QMap<QString,QVector<meteo::GeoVector>> *result_dots,
                                    QVector<DataDesc> *fdescrlist)
{
  float koef = req->has_koef() ? req->koef() : 0.8;
  QMap<QString,meteo::GeoVector> gskelets;
  meteo::GeoVector gskelet;
  QVector<meteo::GeoVector> resdot;
  foreach (const meteo::field::Skelet& sc, req->skelet()) {
      gskelet.clear();
      resdot.clear();
      QString uuid = QString::fromStdString(sc.uuid());
      skeletToGVector(sc,0, &gskelet);
      resdot.resize(sc.dots_size());
      result_dots->insertMulti(uuid,resdot);
      gskelets.insertMulti(uuid,gskelet);
    }


  int real_adv_time = 0;
  int napr = 1;
  int ostatok_time = req->adv_time();
  if(req->adv_time() < 0){
      ostatok_time = abs(ostatok_time);
      napr = -1;
      koef *=-1.;
    }
  QDateTime cur_time = QDateTime::fromString(QString::fromStdString(req->date()), Qt::ISODate);
  int cur_secs_adv = ostatok_time; //на сколько считать адвекцию
  int small_step = kMinAdvStep;//
  int fisz = fields_for_advect.size();
  int cfi = findNearestDt(fields_for_advect.keys(),cur_time);
  if(cfi < 0) return;
  for(; cfi<fisz; cfi=cfi+napr) // по срокам
    {
      if( ((napr < 0) && (cfi >0)) ||
          ((napr > 0) && (cfi <fisz-1)) ){//если полей больше одного, то считаем сколько до следующего
          cur_secs_adv = napr*cur_time.secsTo(fields_for_advect.keys().at(cfi+napr)); //
        } else cur_secs_adv = ostatok_time;
      if(0 >= cur_secs_adv) continue;
      if(cur_secs_adv%small_step != 0) cur_secs_adv += (small_step-cur_secs_adv%small_step);
      if(cur_secs_adv > ostatok_time) {
          cur_secs_adv = ostatok_time;
        }

      QString cur_field_id = fields_for_advect.values().at(cfi);
      DataReply fresp;
      if(false == GetFieldPoID(cur_field_id, &fresp)) { break; }

      ::obanal::TField fd;
      QByteArray arr(fresp.fielddata().data(), fresp.fielddata().size());
      if(false == fd.fromBuffer(&arr)) {
          info_log <<  msglog::kDataServiceNoData;
          break;
        }
      QMap<QString,meteo::GeoVector>::iterator bi = gskelets.begin();
      QMap<QString,meteo::GeoVector>::iterator ei = gskelets.end();
      QMap<QString,meteo::GeoVector>::iterator it = bi;
      int ret_val =ERR_NOERR;
      for(; it!=ei; ++it) {
        QVector<meteo::GeoVector> g;// здесь будут координаты точек адвекции для через cur_step секунд
        // каждый meteo::GeoVector содержит последовательность точек адвекции для одной из точек скелета
        meteo::GeoVector& gskelet = it.value();
        QString uuid = it.key();
        ret_val = fd.prognTrackObj(gskelet, &g, cur_secs_adv,small_step, &real_adv_time,  koef);
        if(g.count() < 1 ) {
            break;
          }
        gskelet.clear();
        for(int i = 0; i<g.size();++i ){//последовательно по точкам скелета
            const GeoVector &gv = g.at(i);
             int her = 0;
            for(int j = 0; j<gv.size();++j ){//последовательно по точкам адвекции
                her+=small_step;
                (*result_dots)[uuid][i].append(gv.at(j));
              }
            gskelet.append((*result_dots)[uuid][i].last());
          }
        }
      if(req->has_need_field_descr() && req->need_field_descr()) {
         fdescrlist->append(fresp.descr());
        }
      cur_time = cur_time.addSecs(napr*real_adv_time);
      if(ERR_NOERR != ret_val || cur_secs_adv > real_adv_time || 0 >= ostatok_time) {
          break;
        }
      ostatok_time -= real_adv_time;
    }
}

void TFieldAnalyseWrap::CalcForecastOpr(const DataAnalyseRequest* req, SimpleDataReply* )
{
  QList<forecastDelta>resps;
  int delta_day = -10;
  int delta_sec = -3600*6;
  if(true == req->has_days_interval()){
      delta_day = req->days_interval();
    }
  if(true == req->has_step()){
      delta_sec = req->step();
    }


  if( false == getForecastsDeltas(&resps)) {
      return ;
    }
  for(int center: req->centers() ){
      for(forecastDelta req: resps ){
          QDateTime start_dt = QDateTime::currentDateTime();
          start_dt.setTime(QTime(0,0,0));
          QDateTime end_dt = start_dt.addDays(delta_day);
          while (start_dt > end_dt) {
              analyseForecast(start_dt,req,center);
              start_dt = start_dt.addSecs(delta_sec);
            }
        }
    }

}


bool TFieldAnalyseWrap::analyseForecast(const QDateTime &analyse_dt, const forecastDelta& deltas, int center)
{
  int count_field = 0;
  QElapsedTimer ttt; ttt.start();
  ::obanal::TField* fd = nullptr;
  ::obanal::TField* abs_field = nullptr;
  ::obanal::TField* porog_field = nullptr;
  ::obanal::TField* delta_field = nullptr;
  ::obanal::TField* analyse_field = nullptr;
  TObanalDb *obanal_db = nullptr;

  auto cont_func = [&fd,&delta_field,&abs_field,&porog_field]() {
      delete fd;fd = nullptr;
      delete abs_field;abs_field = nullptr;
      delete delta_field;delta_field = nullptr;
      delete porog_field;porog_field = nullptr;
    };
  auto return_func = [&analyse_field,&obanal_db,&cont_func,&ttt]() {
      cont_func();
      delete analyse_field;analyse_field = nullptr;
      delete obanal_db; obanal_db = nullptr;
      int cur = ttt.restart();
      // if(1000 < cur) {
      debug_log << msglog::kServiceRequestTime.arg("AnalyseField.loadFields").arg(cur);
      // }
    };
  ::std::string analyse_dt_s = analyse_dt.toString(Qt::ISODate).toStdString();
  DataRequest req;
  req.add_center(center);
  req.add_hour(0);
  req.add_level(deltas.level);
  req.add_type_level(deltas.type_level);
  req.add_meteo_descr(deltas.descriptor);
  req.set_date_start(analyse_dt_s);
  req.set_date_end(analyse_dt_s);
  info_log << deltas.descriptor<<deltas.level<<analyse_dt_s<<center;
  //определяем поля анализа
  ManyDataReply mresp;
  if(false == loadFields(&req, &mresp)
     ||0 == mresp.data_size()) {
      return_func();
      return false;
    }
  if(1 < mresp.data_size()){
      debug_log << QObject::tr("Дубликаты полей в базе")<<mresp.descriptor()->DebugString();
    }
  DataReply drep = mresp.data(0);
  analyse_field = new ::obanal::TField();
  if( !analyse_field->fromStdString(drep.fielddata())){
      debug_log << QObject::tr("Ошибка получения данных");
      delete analyse_field;
      return_func();
      return false;
    };

  const int delta_descr       = 9000000;
  const int delta_descr_porog = 8000000;
  obanal_db = new TObanalDb();

  for(hdelta delta: deltas.deltas){
      req.clear_hour();           req.add_hour(delta.hour);
      req.clear_meteo_descr();    req.add_meteo_descr(deltas.descriptor );
      QList<int> models;
      getFieldModels(req, &models);
      for(int model: models){
          // азполняем запрос
          req.set_model(model);
          req.clear_meteo_descr();
          req.add_meteo_descr(deltas.descriptor + delta_descr);
          req.clear_date_start();
          req.clear_date_end();

          DataReply drep_porog;
          DataRequest req_porog;

          req_porog.CopyFrom(req);
          req_porog.clear_meteo_descr();
          req_porog.add_meteo_descr(deltas.descriptor + delta_descr_porog);

          //читаем поле ошибок
          drep.mutable_descr()->clear_dates();
          if(true == hasDates(req,analyse_dt_s)){
              cont_func();
              continue;
            }

          // если загрузили поля
          // с дельтами и порогом
          if(
             true == loadField(&req, &drep)
             && true == loadField(&req_porog, &drep_porog)
             ) {
              // даты начала и конца
              QDateTime dt1_e = QDateTime::fromString(QString::fromStdString(drep.descr().dt1()), Qt::ISODate);
              QDateTime dt2_e = QDateTime::fromString(QString::fromStdString(drep.descr().dt2()), Qt::ISODate);
              // количество данных
              count_field = drep.descr().count_point();
              // поля для данных
              delta_field = new ::obanal::TField();
              porog_field = new ::obanal::TField();

              // загружаем поле
              if( !delta_field->fromStdString(drep.fielddata())
                  ||
                  !porog_field->fromStdString(drep_porog.fielddata()) ) {
                  error_log <<QObject::tr("Ошибка получения данных");
                  cont_func();
                  continue;
                }

              if(analyse_dt > dt2_e){ drep.mutable_descr()->set_dt2(analyse_dt_s); }
              if(analyse_dt < dt1_e){ drep.mutable_descr()->set_dt1(analyse_dt_s); }

            } else { // если нет полей заполняем описание и счетчик полей ставим на 1

              DataDesc* dd = drep.mutable_descr();
              dd->set_hour(delta.hour);
              dd->set_meteodescr(deltas.descriptor +delta_descr);
              dd->set_dt1(analyse_dt_s);
              dd->set_dt2(analyse_dt_s);
              dd->set_model(model);
              count_field = 1;
            }

          // время до прогноза в секундах
          ::std::string forecast_dt_s = analyse_dt.addSecs(-delta.hour).toString(Qt::ISODate).toStdString();
          req.clear_meteo_descr();
          // выставляем дескриптор
          req.add_meteo_descr(deltas.descriptor);
          req.set_date_start(forecast_dt_s);
          req.set_date_end(forecast_dt_s);

          // прогностическое поле
          DataReply freply;
          // загружаем в датарепли
          if(false == loadField(&req, &freply)) {
              cont_func();
              continue;
            }

          fd = new ::obanal::TField();
          // новое чистое поле
          abs_field = new ::obanal::TField();
          // загружаем поле прогнозов
          if(   false == fd->fromStdString(freply.fielddata())
                // сравниваем поле фактическое с полем прогностическим
                // получаем дельту
                || (false == analyse_field->delta_fabs(fd,&abs_field)) ) {
              cont_func();
              continue;
            }
          //  в abs_field теперь дельты находятся
          //  считаем какие-то пороги
          abs_field->calcDeltaPorog(porog_field, &porog_field, delta.delta );
          // сохраняем количество полей
          drep.mutable_descr()->set_count_point(count_field);
          // есть ли уже посчитанные ранее дельты
          if( nullptr != delta_field){
              // пересчитываем дельты, которые были
              // т.е. замешиваем новые дельты со старыми
              if (false == delta_field->mnoz_plus_mnoz(abs_field,&abs_field, float(count_field), 1./float(count_field+1))){
                  error_log << "Ошибка вычисления дельты";
                  cont_func();
                  continue;
                }
              // сохраняем новое количество полей
              drep.mutable_descr()->set_count_point(count_field+1);
            }
          // добавляем даты
          drep.mutable_descr()->add_dates(analyse_dt_s);
          // сохраняем поле
          obanal_db->saveField(*abs_field, drep.descr());
          // устанавливаем дескриптор
          drep.mutable_descr()->set_meteodescr(deltas.descriptor +delta_descr_porog);
          // сохраняем поле порогов
          obanal_db->saveField(*porog_field, drep.descr());
          debug_log<<"saveField ok"<<count_field+1;//drep.descr().DebugString();
          cont_func();
        }
    }
  return_func();
  return true;
}


bool TFieldAnalyseWrap::getOnePointSynSit(const QString& oid, const DataRequest* req, ValueDataReply* resp)
{
  if(0 == req->coords_size()) { return false; }

  DataReply fresp;
  if(false == GetFieldDescrPoID(oid, &fresp)) {
      info_log << msglog::kDataServiceNoData;
      return false;
    }

  // GRIDFS
  GridFs gridfs;
  if ( !gridfs.connect(global::mongodbConfObanal() ) ) {
      resp->set_error(gridfs.lastError().toStdString());
      return false;
    }
  gridfs.use(global::mongodbConfObanal().name(), kFieldPrefix);
  GridFile file = gridfs.findOneById(QString::fromStdString(fresp.descr().file_id()));

  obanal::TFieldMongo fd;// = new obanal::TFieldMongo();
  if ( !fd.fromGridFile(file) ) {
      error_log << QObject::tr("Не удалось обработать файл %1").arg(file.fileName());
      return false;
    }
  // GRIDFS

  for(const meteo::surf::Point& coord : req->coords()) {
      meteo::GeoPoint point(coord.fi(), coord.la());
      //if(coord.has_number()) { point.setAlt(coord.number()); }
      if (coord.has_height()){
          point.setAlt(coord.height());
        }

      QString sit;
      QString fenom;
      int num_sit;

      if(false == fd.oprSynSit5(point, &sit, &fenom, &num_sit)) { continue; }
      OnePointData *op = resp->add_data();
      op->set_value(num_sit);
      op->set_text(sit.toStdString());
      op->mutable_coord()->CopyFrom(coord);

      if(req->has_need_field_descr() && req->need_field_descr()) {
          op->mutable_fdesc()->CopyFrom(fresp.descr());
        }
    }
  return true;
}

bool TFieldAnalyseWrap::getFieldsIdForAdvect(const AdvectObjectRequest* req, QMap<QDateTime, QString>* field_for_advect)
{
  if(false == req->has_date()) {
      error_log << msglog::kServiceRequestFailedErr;
      return false;
    }

  DataDescResponse response;
  DataRequest request;// = *req;

  QDateTime start_time = QDateTime::fromString(QString::fromStdString(req->date()), Qt::ISODate);
  QDateTime end_time   = start_time.addSecs(req->adv_time());

  if(false == start_time.isValid() || false == end_time.isValid()) {
      error_log << msglog::kServiceRequestFailedErr;
      return false;
    }

  if(start_time < end_time) {
      request.set_forecast_start(start_time.addSecs(-kMaxForecastTime).toString(Qt::ISODate).toStdString());
      request.set_forecast_end(end_time.toString(Qt::ISODate).toStdString());
    }
  else {
      request.set_forecast_start(end_time.addSecs(-kMaxForecastTime).toString(Qt::ISODate).toStdString());
      request.set_forecast_end(start_time.toString(Qt::ISODate).toStdString());
    }


  int level = 850;
  if(req->has_level()) { level = req->level(); }
  request.add_level(level);
  request.add_meteo_descr(10009);

 // QMap<int, int> centers;
  if(req->has_center()) { request.add_center(req->center()); }

  response.Clear();
  getAvailableForecastData(&request, &response);//FIXME убедиться, что нужно использовать именно эту функцию (см описание)
  int field_count = response.descr_size();
  if(0 == field_count) { return false; }
  QMap<QDateTime, DataDesc> fields;
  for(const DataDesc& descr : response.descr()) {
      QDateTime cur_hour = QDateTime::fromString(QString::fromStdString(descr.date()), Qt::ISODate);
      cur_hour = cur_hour.addSecs(descr.hour());
      if(false == cur_hour.isValid()) { continue; }
      int index_of = fields.keys().indexOf(cur_hour);
      if(-1 < index_of){ //уже есть поле на эту дату
          int ahour = fields.value(fields.keys().at(index_of)).hour();
          if(descr.hour() > ahour){
              continue;
            }
        }
      fields.insert(cur_hour,descr);
    }
  for(const DataDesc& descr : fields) {
      QString cur_id = QString::fromStdString(descr.id());
      QDateTime cur_hour = QDateTime::fromString(QString::fromStdString(descr.date()), Qt::ISODate);
      cur_hour = cur_hour.addSecs(descr.hour());
      if(false == cur_hour.isValid()) { continue; }
      field_for_advect->insertMulti(cur_hour, cur_id);
    }

  return true;
}


bool TFieldAnalyseWrap::getForecastsDeltas(QList<forecastDelta> *resps){
  if (nullptr == resps) {
      error_log<<"Неверные входные данные";
      return false;
    }

  std::unique_ptr<Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) {
      return false;
  }

  auto query = db->queryptrByName("load_forecasts_deltas");
  if(nullptr == query) {
      return false;
    }

  while (true == query->next()) {
      const DbiEntry& document = query->entry();
      forecastDelta req;
      req.level = document.valueInt32("level");
      req.type_level = document.valueInt32("type_level");
      req.descriptor =  document.valueInt32("descriptor");
      bool ok ;
      auto arr = document.valueArray("deltas",&ok);
      if(!ok){
          continue;
        }
      while ( true == arr.next() ) {
          Document doc_delta;
          int hour = 0;
          int delta = 0.;
          if(arr.valueDocument(&doc_delta)){
              hour =  doc_delta.valueInt32("hour");
            }
          if(arr.valueDocument(&doc_delta)){
              delta =  doc_delta.valueDouble("delta");
            }
          req.deltas.append(hdelta(hour*3600,delta));
        }
      resps->append(req);
    }
  return true;
}

bool TFieldAnalyseWrap::loadField(const DataRequest* req, DataReply* resp)
{
  if ((nullptr == req)
      ||(nullptr == resp)) {
      error_log<<"Неверные входные данные";
      resp->set_error(QObject::tr("Неверные входные данные").toStdString());
      resp->set_result(false);
      return false;
    }

  std::unique_ptr<Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) {
      return false;
  }

  auto query = db->queryptrByName("load_field");
  if(nullptr == query) {
      resp->set_error(QObject::tr("Ошибка при загрузке запроса").toStdString());
      resp->set_result(false);
      return false;
    }

  if (true == req->has_date_start()) {
      if (true == req->has_interval()) {
          QDateTime dt = QDateTime::fromString(QString::fromStdString(req->date_start()), Qt::ISODate);
          QDateTime dts = dt.addSecs(req->interval() * 3600);
          QDateTime dte = dt.addSecs(req->interval() * -3600);
        query->arg("start_dt",dts);
        query->arg("end_dt",dte);

        }
      else if ( true == req->has_date_end() ) {
        query->argDt("start_dt",req->date_start());
        query->argDt("end_dt",req->date_end());
        }
      else {
        query->argDt("start_dt",req->date_start());
        query->argDt("end_dt",req->date_start());
        }
    }

  query->arg("descr",req->meteo_descr());
  query->arg("level_type",req->type_level());
  query->arg("level",req->level());
  query->arg("hour",req->hour());
  query->arg("center",req->center());

  if (true == req->has_model()) {
      query->arg("model",req->model());
    }

 QString err;
  bool queryRes = query->execInit(&err);
  if (false == queryRes) {
      resp->set_comment(err.toStdString());
      resp->set_result(false);
      return false;
    }
  if (true == query->next()) {
      const DbiEntry& document = query->entry();
      QDateTime dt = document.valueDt("dt");
      QDateTime dt1 = document.valueDt("forecast_start");
      QDateTime dt2 = document.valueDt("forecast_end");
      DataDesc* dataDesc = resp->mutable_descr();
      int descr = document.valueInt32("descr");
      int hour = document.valueInt32("hour");
      int level_type = document.valueInt32("level_type");
      int level = document.valueInt32("level");
      int model = document.valueInt32("model");
      int center = document.valueInt32("center");
      int net_type = document.valueInt32("net_type");
      int count_points = document.valueInt32("count_points");
      dataDesc->set_date(dt.toString(Qt::ISODate).toStdString());
      dataDesc->set_dt1(dt1.toString(Qt::ISODate).toStdString());
      dataDesc->set_dt2(dt2.toString(Qt::ISODate).toStdString());
      dataDesc->set_meteodescr(descr);
      dataDesc->set_hour(hour);
      dataDesc->set_level_type(level_type);
      dataDesc->set_level(level);
      dataDesc->set_model(model);
      dataDesc->set_center(center);
      dataDesc->set_net_type(net_type);
      dataDesc->set_count_point(count_points);
      if (document.hasField("dates")){
          bool ok;
          Array dates_arr = document.valueArray("dates",&ok);
          if(true == ok){
              while ( true == dates_arr.next() ) {
                  QDateTime date = dates_arr.valueDt(&ok);
                  if(false == ok){ continue; }
                  dataDesc->add_dates(date.toString(Qt::ISODate).toStdString());
                }
            }
        }

      QByteArray barr;
      QString fileName = document.valueString("path");

      if(getFieldFromFile(fileName, &barr)) {
          resp->set_fielddata(barr.data(), barr.size());
          QString id = document.valueOid("_id");
          dataDesc->set_id(id.toStdString());
        }
      else {
          error_log << msglog::kFileReadFailed.arg(fileName).arg(-1);
          resp->set_error(msglog::kFileReadFailed.arg(fileName).arg(-1).toStdString());
          resp->set_result(false);
        }
      return true;
    }
 return false;
}

bool TFieldAnalyseWrap::loadFields(const DataRequest* req, ManyDataReply* resp)
{
  DataDescResponse descrresp;

  if(false == getAvailableForecastData(req,&descrresp)){//FIXME убедиться, что нужно использовать именно эту функцию (см описание)
      return false;
    }

  for( DataDesc descr : descrresp.descr()){
      DataReply* dr = resp->add_data();
      DataDesc* mdescr = dr->mutable_descr();
      mdescr->CopyFrom(descr);
      QString file_id = QString::fromStdString(descr.file_id());
      QByteArray barr;

      if(getFieldFromFile(file_id, &barr)) {
          dr->set_fielddata(barr.data(), barr.size());
        } else {
          error_log << msglog::kFileReadFailed.arg(file_id).arg(-1);
        }

    }
  return true;
}

bool TFieldAnalyseWrap::getFieldModels(const DataRequest& req, QList<int>* models){
  if (nullptr == models) {
      error_log<<"Неверные входные данные";
      return false;
    }
  std::unique_ptr<Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) {
      return false;
  }

  auto query = db->queryptrByName("get_field_models");
  if(nullptr == query) {
      return false;
    }

  query->arg("descr",req.meteo_descr());
  query->arg("level_type",req.type_level());
  query->arg("level",req.level());
  query->arg("hour",req.hour());
  query->arg("center",req.center());

  bool queryRes = query->exec( );
  QString error_str;

  if (false == queryRes) {
      error_str = QObject::tr("Запрос выполнен с ошибкой");
      error_log << error_str;
      return false;
    }
  const DbiEntry& document = query->result();
  bool ok;
  Array arr = document.valueArray("values", &ok);
  if(false == ok){
      return false;
    }
  while ( true == arr.next() ) {
      int model = arr.valueInt32(&ok);
      if(false == ok){ continue; }
      models->append(model);
    }
  //}
  if(0 < models->size()){
      return true;
    }
  return false;
}

bool TFieldAnalyseWrap::hasDates(const DataRequest& req,const::std::string& date)
{
  std::unique_ptr<Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) {
      return false;
  }

  auto query = db->queryptrByName("find_dates");
  if(nullptr == query) {
      return false;
    }
  query->arg("descr",req.meteo_descr());
  query->arg("level_type",req.type_level());
  query->arg("level",req.level());
  query->arg("hour",req.hour());
  query->arg("center",req.center());

  query->arg("dt",date);
  //debug_log<<query.query();

  QString err;
  bool queryRes = query->execInit(&err);
  if (false == queryRes) {
      //debug_log << err;
      return false;
    }
  if (true == query->next()) {
       return true;
    }
  return false;
}

void TFieldAnalyseWrap::GetExtremums( const DataRequest* req, meteo::field::ExtremumTrajReply* resp)
{

  QString error;

  ::meteo::field::ManyDataReply fields_reply;
  GetFieldsData(req, &fields_reply);

  if(fields_reply.has_error()) {
      error_log << msglog::kServerAnswerError.arg(meteo::global::serviceTitle(settings::proto::kField))
                   .arg(QString::fromStdString( fields_reply.error()));
      resp->set_error(msglog::kServiceRequestFailedErr.toStdString());
      return;
    }

  QMap<QDateTime, QVector<fieldExtremum>> extr_map;
  for(const ::meteo::field::DataReply& data : fields_reply.data()) {
      if(false == data.has_descr() || false == data.descr().has_date()) {
          continue;
        }

      obanal::TField* field = new ::obanal::TField;
      QByteArray arr(data.fielddata().data(), data.fielddata().size());
      if(false == field->fromBuffer(&arr)) {
          delete field;
          field = nullptr;
          continue;
        }

      field->smootchField(50);


      QVector<fieldExtremum> extrems = field->calcExtremum();
      if(extrems.count() < 1) {
          continue;
        }

      QDateTime dt = QDateTime::fromString(QString::fromStdString(data.descr().date()), Qt::ISODate);
      if(false == dt.isValid()) {
          continue;
        }
      extr_map.insert(dt, extrems);
    }

  if(extr_map.count() < 2) {
      return;
    }

  QList<QDateTime> tr_list = extr_map.uniqueKeys();
  qSort(tr_list);

  for(const QDateTime& tr : tr_list) {
      const QVector<fieldExtremum>& vs = extr_map.value(tr);
      auto traj = resp->add_traj();
      traj->set_date(tr.toString(Qt::ISODate).toStdString());
      for(const fieldExtremum& it : vs) {
          auto extr = traj->add_extremums();
          extr->set_date(tr.toString(Qt::ISODate).toStdString());
          extr->set_tip(it.tip);
          extr->set_value(it.znach);

          auto point = extr->mutable_pbcoord();
          point->set_lat_radian(it.koord.fi());
          point->set_lon_radian(it.koord.la());

//          ::meteo::surf::Point* point = extr->mutable_coord();
//          point->set_fi(it.koord.fi());
//          point->set_la(it.koord.la());
//          debug_log << "FI =" << it.koord.fi() << "LA =" << it.koord.la();
        }
    }

}

void TFieldAnalyseWrap::GetExtremumsTraj(const SimpleDataRequest* req, meteo::field::ExtremumTrajReply* resp )
{

  QString error;
  QMap<QDateTime, QVector<fieldExtremum>> extr_map;
  float smootch = 50.;
  if(req->has_smootch()){
      smootch = req->smootch();
    }
  for(int i =0;i < req->id_size();++i){
      ::meteo::field::DataReply data;
      GetFieldDataPoID(req, &data,i);

      if(data.has_error()) {
          error_log << msglog::kServerAnswerError.arg(meteo::global::serviceTitle(settings::proto::kField))
                       .arg(QString::fromStdString(data.error()));
          resp->set_error(msglog::kServiceRequestFailedErr.toStdString());
          return;
        }

      if(false == data.has_descr() || false == data.descr().has_date()) {
          continue;
        }

      ::obanal::TField* field = new ::obanal::TField;
      QByteArray arr(data.fielddata().data(), data.fielddata().size());

      if(false == field->fromBuffer(&arr)) {
          delete field; field = nullptr;
          continue;
        }

      field->smootchField(smootch);

      QVector<fieldExtremum> extrems = field->calcExtremum();
      if(extrems.count() < 1) {
          continue;
        }

      QDateTime dt = QDateTime::fromString(QString::fromStdString(data.descr().date()), Qt::ISODate).addSecs(data.descr().hour());
      if(false == dt.isValid()) {
          continue;
        }
      // debug_log<<dt.toString()<<extrems.size();


      extr_map.insert(dt, extrems);
    }

  if(extr_map.count() < 2) {
      return;
    }

  QList<QDateTime> dt_list = extr_map.uniqueKeys();
  qSort(dt_list);

  QMap<int, QMap<QDateTime, fieldExtremum>> traj_extr;
  int i = 0;
  for(const fieldExtremum& extr : extr_map.value(dt_list.first())) {
      QMap<QDateTime, fieldExtremum> vs;
      vs.insert(dt_list.first(), extr);
      traj_extr.insert(i++, vs);
    }

  for(const QDateTime& dt : dt_list) {
      const QVector<fieldExtremum>& next_extr = extr_map.value(dt);
      calcNear(dt, next_extr, &traj_extr);
    }

  QList<int> tr_list = traj_extr.uniqueKeys();
  qSort(tr_list);

  for(const int tr : tr_list) {
      const QMap<QDateTime, fieldExtremum>& vs = traj_extr.value(tr);
      QList<QDateTime> vskeys = vs.uniqueKeys();
      auto traj = resp->add_traj();
      qSort(vskeys);

      for(const QDateTime& dt : vskeys) {
        const fieldExtremum& vs_tr = vs.value(dt);

        auto extr = traj->add_extremums();
        extr->set_date(dt.toString(Qt::ISODate).toStdString());
        extr->set_tip(vs_tr.tip);
        extr->set_value(vs_tr.znach);

        auto point = extr->mutable_pbcoord();
        point->set_lat_radian(vs_tr.koord.fi());
        point->set_lon_radian(vs_tr.koord.la());
      }
    }
}

void  TFieldAnalyseWrap::calcNear( const QDateTime& cdt, const QVector<fieldExtremum>& next_extr,
                                   QMap<int, QMap<QDateTime, fieldExtremum>>* traj_extr)
{
  QList<int> keys = traj_extr->uniqueKeys();
  qSort(keys);
  for(const int key : keys) {
      QMap<QDateTime, fieldExtremum>& values = (*traj_extr)[key];

      QList<QDateTime> dts = values.uniqueKeys();
      qSort(dts);

      const fieldExtremum& cextr = values.value(dts.last());

      const ::meteo::GeoPoint& coord_0 = cextr.koord;
      const fieldExtremum*     next_ex = nullptr;

      int   tip_0    = cextr.tip;
      float min_rast = 400;

      for(const fieldExtremum& next : next_extr) {
          const ::meteo::GeoPoint& coord_1 = next.koord;
          int tip_1 = next.tip;
          if(tip_0 != tip_1) {
              continue;
            }

          float rast = coord_0.calcDistance(coord_1);
          if(rast < min_rast) {
              min_rast = rast;
              next_ex = &next;
            }
        }

      if(nullptr != next_ex) {
          values.insert(cdt, *next_ex);
        }
    }
}



} // field
} // meteo


