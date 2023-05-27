#include "tfield_calc.h"

#include <meteo/commons/services/obanal/tobanaldb.h>
#include <meteo/commons/global/log.h>

static const int kTimeout = 30000;

namespace obanal {
  
TFieldCalc::TFieldCalc(ObanalDbPtr db)
  : db_(db)
{
}

TFieldCalc::~TFieldCalc()
{
}

bool TFieldCalc::calcOT500_1000( const obanal::TField& f, const meteo::field::DataDesc* afdescr )
{
  if ( nullptr == db_ ) {
    return false;
  }
  if ( false == (
        ( 500 == afdescr->level()
       || 1000 == afdescr->level())
       && 10009 == afdescr->meteodescr())) {
    return false;
  }

  QDateTime cdt = cdt.fromString(QString::fromStdString(afdescr->date()), Qt::ISODate);
  int aflevel = afdescr->level();

  meteo::field::DataDesc fdescr = *afdescr;
  TField f2;
  TField *ot5001000fd = nullptr;
  const TField* f500 = nullptr;
  const TField* f1000 = nullptr;
  if ( 500 == aflevel ) {
    fdescr.set_level(1000);
    f500 = &f;
    f1000 = &f2;
  }
  else {
    fdescr.set_level(500);
    f500 = &f2;
    f1000 = &f;
  }
  if ( false == getField( &fdescr, &f2 ) ) {
    return false;
  }
  if ( false == f500->minus( f1000, &ot5001000fd ) ) {
    return false ;
  }
  
  ot5001000fd->setValueType(10009, 15000, 15000);

  meteo::field::DataDesc ot5001000_fdescr;
  ot5001000_fdescr.set_date(afdescr->date());
  ot5001000_fdescr.set_center(afdescr->center());
  ot5001000_fdescr.set_level(15000);
  ot5001000_fdescr.set_hour(afdescr->hour());
  ot5001000_fdescr.set_level_type(15000);//TODO Где-то это надо описать
  ot5001000_fdescr.set_meteodescr(10009); 
  ot5001000_fdescr.set_net_type(f.typeNet());
  ot5001000_fdescr.set_model(afdescr->model());
  ot5001000_fdescr.set_dt1(afdescr->dt1());
  ot5001000_fdescr.set_dt2(afdescr->dt2());
  ot5001000_fdescr.set_time_range(afdescr->time_range());
  
  ot5001000fd->setHour(afdescr->hour());
  ot5001000fd->setModel(afdescr->model());
  ot5001000fd->setCenter(afdescr->center());
  ot5001000fd->setDate(QDateTime::fromString(QString::fromStdString(afdescr->date()), Qt::ISODate));
  ot5001000fd->setCenterName(QString::fromStdString(afdescr->center_name()));
  ot5001000fd->setLevelTypeName("ОТ500/1000");
  
  ot5001000_fdescr.set_count_point(ot5001000fd->getSrcPointCount());

  bool rv = db_->saveField(*ot5001000fd, ot5001000_fdescr);
  //  debug_log << "end calcOT500_1000" <<adt<< res; 
  delete ot5001000fd;
  ot5001000fd = nullptr;
 /* info_log
    << QObject::tr("Анализ данных OT500/1000 за %1 центр %2 срок  %3 ч.")
    .arg( QString::fromStdString(afdescr->date() ) )
    .arg( afdescr->center() )
    .arg( afdescr->hour()/3600 )
    << QObject::tr(" - успешно");*/
  return rv;
}

bool TFieldCalc::calcWind(const TField& f, const meteo::field::DataDesc* afdescr)
{
  if ( nullptr == db_ ) {
    return false;
  }
  if ( 11004 != afdescr->meteodescr() && 11003 != afdescr->meteodescr() ) {
    return false;
  }
  meteo::field::DataDesc fdescr = *afdescr;

  QDateTime cdt = cdt.fromString( QString::fromStdString(fdescr.date()), Qt::ISODate);
  int descr = fdescr.meteodescr();

  obanal::TField f2;
  fdescr.set_meteodescr(11003 == descr ? 11004 : 11003);
  if ( false == getField( &fdescr, &f2 ) ) {
    return false;
  }
  if ( f.getRegPar() != f2.getRegPar() || f.kolData() != f2.kolData()) {
    return false;
  }
  
  TField fd_dd;
  TField fd_v;
  QDateTime dt = QDateTime::fromString( QString::fromStdString(fdescr.date()), Qt::ISODate );
  fd_v.setNet(f.getRegPar(), f.typeNet());
  fd_v.setValueType(fdescr.meteodescr(), fdescr.level(), fdescr.level_type());
  fd_v.setHour(fdescr.hour());
  fd_v.setModel(fdescr.model());
  fd_v.setCenter(fdescr.center());
  fd_v.setDate(dt);
  fd_dd.setNet(f.getRegPar(), f.typeNet());
  fd_dd.setValueType(fdescr.meteodescr(), fdescr.level(), fdescr.level_type());
  fd_dd.setHour(fdescr.hour());
  fd_dd.setModel(fdescr.model());
  fd_dd.setCenter(fdescr.center());
  fd_dd.setDate(dt);
  if ( true == fdescr.has_center_name() ) {
    fd_v.setCenterName( QString::fromStdString(fdescr.center_name() ) );
    fd_dd.setCenterName(QString::fromStdString(fdescr.center_name())); 
  }
  if ( true == fdescr.has_level_type_name() ) {
    fd_v.setLevelTypeName( QString::fromStdString( fdescr.level_type_name() ) );
    fd_dd.setLevelTypeName( QString::fromStdString( fdescr.level_type_name() ) ); 
  }

  
  int dsize = f.kolData();
  int j=0;
  for(int i=0; i < dsize; ++i) {
    if(f.getMask(i) && f2.getMask(i)) {
      float dd = 0.0;
      float ff = 0.0;
      float u = 0.0;
      float v = 0.0;
      
      if(11003 == descr){
        u = f.getData(i);
        v = f2.getData(i);
      }
      else {
        u = f2.getData(i);
        v = f.getData(i);
      }

      MnMath::preobrUVtoDF(u, v, &dd, &ff);
     // debug_log<<"uu "<<u<<" vv "<<v<<" dd "<<dd<<" ff "<<ff;
      fd_dd.setData(i, dd, true);
      fd_v.setData(i, ff, true);
      ++j;
    }
    else  {
      fd_dd.setMasks(i, false);
      fd_v.setMasks(i, false);
    }
  }
  
  fd_dd.setSrcPointCount(j);
  fd_v.setSrcPointCount(j);
  
  fdescr.set_meteodescr(11001);
  fd_dd.setDescr(11001);
  fd_dd.setValueType(fdescr.meteodescr(),  fdescr.level(), fdescr.level_type());
  
  if ( false == db_->saveField( fd_dd, fdescr ) ) {
    return false;
  }
  
  fdescr.set_meteodescr(11002);
  fd_v.setDescr(11002);
  fd_v.setValueType(fdescr.meteodescr(), fdescr.level(), fdescr.level_type());
  if ( false == db_->saveField( fd_v, fdescr ) ) {
    return false;
  }
  fdescr.set_meteodescr(descr);

/*  info_log
    << QObject::tr("Пересчет данных о ветре за %1 центр %2 срок  %3 ч.")
    .arg(QString::fromStdString(fdescr.date()))
    .arg(fdescr.center())
    .arg(fdescr.hour()/3600)
    << QObject::tr(" - успешно");*/
  return true;
}

bool TFieldCalc::calcGeopotentialHeight(const TField& f, const meteo::field::DataDesc* afdescr)
{
  if ( nullptr == db_ ) {
    return false;
  }
  if ( 10008 != afdescr->meteodescr() ) {
    return false;
  }
  meteo::field::DataDesc fdescr = *afdescr;
  fdescr.set_meteodescr(10009);

  QDateTime cdt = cdt.fromString( QString::fromStdString(fdescr.date()), Qt::ISODate);
  
  TField fd_hh;
  QDateTime dt = QDateTime::fromString( QString::fromStdString(fdescr.date()), Qt::ISODate );
  fd_hh.setNet(f.getRegPar(), f.typeNet());
  fd_hh.setValueType(fdescr.meteodescr(), fdescr.level(), fdescr.level_type());
  fd_hh.setHour(fdescr.hour());
  fd_hh.setModel(fdescr.model());
  fd_hh.setCenter(fdescr.center());
  fd_hh.setDate(dt);
  if ( true == fdescr.has_center_name() ) {
    fd_hh.setCenterName(QString::fromStdString(fdescr.center_name())); 
  }
  if ( true == fdescr.has_level_type_name() ) {
    fd_hh.setLevelTypeName( QString::fromStdString( fdescr.level_type_name() ) ); 
  }

  
  int dsize = f.kolData();
  int j = 0;
  for(int i=0; i < dsize; ++i) {
    if ( true == f.getMask(i) ) {
      float hh = 0.0;
      float phi = 0.0;
      
      phi = f.getData(i);
      hh = phi/9.81;
      fd_hh.setData( i, hh, true );
      ++j;
    }
    else  {
      fd_hh.setMasks(i, false);
    }
  }
  
  fd_hh.setSrcPointCount(j);
  
  fdescr.set_meteodescr(10009);
  fd_hh.setDescr(10009);
  fd_hh.setValueType( fdescr.meteodescr(),  fdescr.level(), fdescr.level_type() );
  
  if ( false == db_->saveField( fd_hh, fdescr ) ) {
    return false;
  }

  return true;
}

bool TFieldCalc::getField(const meteo::field::DataDesc* req, TField* fd)
{
  meteo::field::DataRequest request;
  request.set_date_start(req->date());
  request.add_hour(req->hour());
  request.add_level(req->level());
  request.add_meteo_descr(req->meteodescr());
  request.add_center(req->center());
  request.set_model(req->model());
  request.set_net_type(req->net_type());
  request.add_type_level(req->level_type());
  request.set_is_df(false);
  
  meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if ( nullptr == ch ) {
    error_log << meteo::msglog::kServiceConnectFailedSimple.arg("FieldService");
    return false;
  }
  meteo::field::DataReply* reply = ch->remoteCall(&meteo::field::FieldService::GetFieldData, request,  kTimeout);
  delete ch; ch = nullptr;
  if ( nullptr == reply ) {
    error_log << meteo::msglog::kServiceAnalyzeAnswerFailed.arg("GRIB").arg("FieldService");
    return false;
  }

  if ( ( true == reply->has_result() && false == reply->result() ) || 0 == reply->fielddata().size()) {
    delete reply;
    return false;
  }
  
  QByteArray barr(reply->fielddata().data(), reply->fielddata().size());
//  QDataStream stream(barr);
//  stream >> (*fd);
  if ( false == fd->fromBuffer(&barr) ) {
    delete reply;
    return false;
  }
  delete reply;
  return true;
}
  
bool TFieldCalc::getField(const QDateTime& acur_dt, int level, int hour, int descr, int center, obanal::TField* fd)
{
  meteo::field::DataRequest request;
  request.set_date_start(acur_dt.toString(Qt::ISODate).toStdString());
  request.add_hour(hour);
  request.add_level(level);
  request.add_meteo_descr(descr);
  request.add_center(center);
  
  meteo::rpc::Channel* ch= meteo::global::serviceChannel( meteo::settings::proto::kField);
  if ( nullptr == ch ) {
    error_log << meteo::msglog::kServiceConnectFailedSimple.arg("FieldService");
    return false;
  }
  
  meteo::field::DataReply* reply = ch->remoteCall(&meteo::field::FieldService::GetFieldData, request,  kTimeout);
  delete ch; ch = nullptr;
  if(nullptr == reply) {
    error_log << meteo::msglog::kServiceAnalyzeAnswerFailed.arg("GRIB").arg("FieldService");
    return false;
  }

  //Code will never be executed
  if ( ( true == reply->has_result() && false == reply->result()) || 0 == reply->fielddata().size()) {
    return false;
    delete reply;
  }
  
  QByteArray barr(reply->fielddata().data(), reply->fielddata().size());
  if ( false == fd->fromBuffer(&barr) ) {
    delete reply;
    return false;
  }
  delete reply;
  return true;
}
  
} // obanal
