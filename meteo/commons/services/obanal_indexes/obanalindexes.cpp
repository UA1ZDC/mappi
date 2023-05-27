#include "obanalindexes.h"

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/services/obanal/tobanalservice.h>
#include <meteo/commons/services/obanal/tobanaldb.h>

#include <commons/obanal/tfield.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/meteo_data/meteo_data.h>
#include <cross-commons/debug/tlog.h>

#include <qdatetime.h>

using namespace meteo;

ObanalIndexes::ObanalIndexes()
{
  _oservice = new meteo::TObanalService;
  _db = new TObanalDb;
}


ObanalIndexes::~ObanalIndexes()
{
  delete _oservice;
  _oservice = nullptr;

  delete _db;
  _db = nullptr;
}

bool ObanalIndexes::run(const QDateTime& dt, const QStringList& descrname)
{
  bool ok = false;
  for (auto descr : descrname) {
    ok |= run(dt, descr);
  }

  return ok;
}

bool ObanalIndexes::run(const QDateTime& dt, const QString& descrname)
{
  meteo::surf::DataRequest request;
  
  request.set_date_start( dt.toString(Qt::ISODate).toStdString() );  
  request.set_type_level(meteodescr::kSurfaceLevel);
  request.set_level_p(0);
  request.add_type(meteo::surf::kAeroFix);
  request.add_meteo_descrname(descrname.toStdString());
  request.add_meteo_descr(TMeteoDescriptor::instance()->descriptor(descrname));

  auto ctrl = std::unique_ptr<meteo::rpc::Channel>(meteo::global::serviceChannel( meteo::settings::proto::kSrcData ));
  if (nullptr == ctrl) {
    error_log << QObject::tr("Ошибка подключения к сервису доступа к раскодированным данным ");
    return false;
  }
    
  meteo::surf::DataReply* reply = ctrl->remoteCall( &meteo::surf::SurfaceService::GetMeteoData, request, 300000 );
  if (nullptr == reply) {
    error_log << QObject::tr("При попытке выполнить расчет индексов неустойчивости ответ от сервиса данных не получен");
    return false;
  }
  //  var(reply->meteodata_size());
  bool ok = _oservice->haveValueResp(request, reply);
  //var(ok);
  delete reply; reply = nullptr;
  
  return ok;
}

bool ObanalIndexes::runField(const QDateTime& dt)
{
  QMap<FieldsKey, FieldsId> fileid;
  
  if(! getAvailable(dt, &fileid)) {
    info_log << QObject::tr("Ошибка получения данных");
    return false;
  }

  int cnt = 0;
  QMapIterator<FieldsKey, FieldsId> it(fileid);
  while (it.hasNext()) {
    it.next();
    cnt++;
    
    meteo::field::DataDesc fdescr;

    obanal::TField* t500  = new obanal::TField;
    bool ok = false;
    if (!it.value().t500.empty()) {
      //debug_log << "t500";
      ok = getField(it.value().t500, t500, &fdescr);
      fdescr.set_level_type(meteodescr::kSurfaceLevel);
      fdescr.set_level(0);
    }
    if (!ok) {
      delete t500; t500 = nullptr;
      continue;
    }
    //debug_log << cnt << it.key().hour << it.key().model << it.key().center;
    
    obanal::TField* t850  = new obanal::TField;   
    obanal::TField* vt = nullptr;
    ok = true;
    if (!it.value().t850.empty()) {
      ok &= getField(it.value().t850, t850);
      if (ok) {
	t850->minus(t500, &vt);
      	fdescr.set_meteodescr(TMeteoDescriptor::instance()->descriptor("vti"));
      	saveField(vt, fdescr);
      }
    }

    obanal::TField* D850  = new obanal::TField;
    obanal::TField* td850 = nullptr;
    ok = true;
    if ( !t850->isEmpty() && 
	 !it.value().D850.empty()) {
      ok &= getField(it.value().D850, D850);
      if (ok) {
      	t850->minus(D850, &td850);
      }
    }
    
    obanal::TField* ct = nullptr;
    if (nullptr != td850 &&
	!t500->isEmpty() && !td850->isEmpty()) {
      td850->minus(t500, &ct);
      fdescr.set_meteodescr(TMeteoDescriptor::instance()->descriptor("cti"));
      saveField(ct, fdescr);
    }
    
    obanal::TField* tt = nullptr;
    if (nullptr != vt && nullptr != ct &&
	!vt->isEmpty() && !ct->isEmpty()) {
      //var("tti");
      vt->plus(ct, &tt);
      fdescr.set_meteodescr(TMeteoDescriptor::instance()->descriptor("tti"));
      saveField(tt, fdescr);
    }
    delete ct;
    delete t850; 
    delete t500;
     
    if (nullptr != tt && nullptr != td850) {
      obanal::TField* sweat = calcSweat(*td850, *tt, it.value().f850, it.value().f500, it.value().d850, it.value().d500);
      if (nullptr != sweat && !sweat->isEmpty()) {
    	//debug_log << "sweat";
    	fdescr.set_meteodescr(TMeteoDescriptor::instance()->descriptor("sweat"));
    	saveField(sweat, fdescr);
      }
      delete sweat;
      delete tt;
    }

    obanal::TField* D700  = new obanal::TField;    
    obanal::TField* ki = nullptr;
    if (nullptr != vt && nullptr != td850 &&
	!vt->isEmpty() && !td850->isEmpty() &&
    	!it.value().D700.empty()) {
      //debug_log << "D700";
      ok = getField(it.value().D700, D700);
      if (ok) {
	//vt + td850 - D700;
	QList<const obanal::TField*> f;
	f << vt << td850 << D700;
	QList<float> k = QList<float>() << 1 << 1 << -1;
	QList<float> b = QList<float>() << 0 << 0 << 0;
	obanal::TField::sumLinear(f, k, b, &ki);
    	fdescr.set_meteodescr(TMeteoDescriptor::instance()->descriptor("ki"));
    	saveField(ki, fdescr);    
      }
    }
    delete vt;
    delete ki;
    delete td850;
    delete D700;     
  }
  
  return true;
}

 //SWEAT = 12⋅Td850 + 20⋅(TT- 49) + 3.888⋅F850 + 1.944⋅F500 + (125⋅[sin(D500 - D850)+0.2])
obanal::TField* ObanalIndexes::calcSweat(const obanal::TField& td850, const obanal::TField& tt,
					 const std::string& if850, const std::string& if500,
					 const std::string& id850, const std::string& id500)
{
  if (td850.isEmpty() || tt.isEmpty() ||
      if850.empty() || if500.empty() ||
      id850.empty() || id500.empty()) {
    return nullptr;
  } 

  obanal::TField* f850 = new obanal::TField;
  obanal::TField* f500 = new obanal::TField;
  obanal::TField* d850 = new obanal::TField;
  obanal::TField* d500 = new obanal::TField;
  
  bool ok = getField(if850, f850);
  ok = getField(if500, f500);
  ok = getField(id850, d850);
  ok = getField(id500, d500);
  if (!ok) {
    delete f850;
    delete f500;
    delete d850;
    delete d500;
    return nullptr;
  }
  obanal::TField* sweat = nullptr;
  
  obanal::TField* val2  = nullptr;
  d500->sinDiff(d850, 125, 125*0.2, &val2);
  
  QList<const obanal::TField*> f;
  f << &td850 << &tt << f850 << f500 << val2;
  QList<float> k = QList<float>() << 12 << 20 << 3.888 << 1.944 << 1;
  QList<float> b = QList<float>() << 0 << 20*49 << 0 << 0 << 0;
  obanal::TField::sumLinear(f, k, b, &sweat);
  
  delete val2;
  delete f850;
  delete f500;
  delete d850;
  delete d500;
    
  return sweat;
}

bool ObanalIndexes::saveField(obanal::TField* fd, const meteo::field::DataDesc& fdescr)
{
  if ( nullptr == _db || nullptr == fd) {
    return false;
  }
  if (fd->isEmpty()) {
    return false;
  }

  fd->setValueType(fdescr.meteodescr(), fdescr.level(), fdescr.level_type());
  fd->setHour(fdescr.hour());
  fd->setModel(fdescr.model());
  fd->setCenter(fdescr.center());
  fd->setDate(QDateTime::fromString(QString::fromStdString(fdescr.date()), Qt::ISODate));
  if(fdescr.has_center_name()) {
    fd->setCenterName(QString::fromStdString(fdescr.center_name()));
  }
  
  bool ok = _db->saveField(*fd, fdescr);
  if (ok) {
    info_log << QObject::tr("Сохранение поля %1 срок %2 центр %3 модель %4").
      arg(fdescr.meteodescr()).arg(fdescr.hour()).arg(fdescr.center()).arg(fdescr.model());
  }
  return ok;
}

bool ObanalIndexes::getField(const std::string &id, obanal::TField *fd, meteo::field::DataDesc* fdescr /*= nullptr*/)
{
  if (nullptr == fd) {
    return false;
  }
  
  meteo::field::SimpleDataRequest request;
  request.add_id(id);

  auto ctrl = std::unique_ptr<meteo::rpc::Channel>(meteo::global::serviceChannel( meteo::settings::proto::kField ));
  if(nullptr == ctrl) {
    return false;
  }
  
  meteo::field::DataReply* reply = ctrl->remoteCall( &meteo::field::FieldService::GetFieldDataPoID, request,  30000);
  if ( nullptr == reply ) {
    error_log << QObject::tr("При попытке выполнить расчет индексов неустойчивости ответ от сервиса полей не получен");
    return false;
  }

  bool result = false;
  reply->fielddata();
  if(false ==  reply->has_fielddata()){
    error_log << QObject::tr("Нет данных") << request.DebugString();
    result = false;
  } else {
    //var(reply->fielddata().size());
    QByteArray arr( reply->fielddata().data(), reply->fielddata().size() );
    if( !fd->fromBuffer(&arr)){
      error_log << QObject::tr("Не удалось получить поле из сериализованного сообщения");
      result = false;
    } else {
      if (nullptr != fdescr) {
	fdescr->CopyFrom(reply->descr());	
      }
      result = true;
    }
  }

  delete reply;
  
  return result;
}

bool ObanalIndexes::getAvailable(const QDateTime& dt, QMap<FieldsKey, FieldsId>* fileid)
{
  if (fileid == nullptr) return false;
  
  meteo::field::DataRequest req;
  req.set_date_start(dt.toString(Qt::ISODate).toStdString());
  req.set_date_end(dt.toString(Qt::ISODate).toStdString());
  req.add_type_level(::meteodescr::kIsobarLevel);
  req.set_is_df(1);

  meteo::field::DataDescResponse* resp = requestAvailable(&req, "T", 850);
  if (nullptr == resp) return false;
  for (auto one: resp->descr()) {
    FieldsKey key(one.hour(), one.model(), one.center(), one.net_type());
    fileid->insert(key, FieldsId(one.id()));
  }  
  delete resp;

  resp = requestAvailable(&req, "T", 500);
  if (nullptr == resp) return false;
  for (auto one: resp->descr()) {
    FieldsKey key(one.hour(), one.model(), one.center(), one.net_type());
    if (fileid->contains(key)) {
      (*fileid)[key].t500 = one.id();
    }
  }
  delete resp;

  resp = requestAvailable(&req, "D", 850);
  if (nullptr == resp) return false;
  for (auto one: resp->descr()) {
    FieldsKey key(one.hour(), one.model(), one.center(), one.net_type());
    if (fileid->contains(key)) {
      (*fileid)[key].D850 = one.id();
    }
  }
  delete resp;

  resp = requestAvailable(&req, "D", 700);
  if (nullptr == resp) return false;
  for (auto one: resp->descr()) {
    FieldsKey key(one.hour(), one.model(), one.center(), one.net_type());
    if (fileid->contains(key)) {
      (*fileid)[key].D700 = one.id();
    }
  }
  delete resp;  

  resp = requestAvailable(&req, "ff", 850);
  if (nullptr == resp) return false;
  for (auto one: resp->descr()) {
    FieldsKey key(one.hour(), one.model(), one.center(), one.net_type());
    if (fileid->contains(key)) {
      (*fileid)[key].f850 = one.id();
    }
  }
  delete resp;  

  resp = requestAvailable(&req, "ff", 500);
  if (nullptr == resp) return false;
  for (auto one: resp->descr()) {
    FieldsKey key(one.hour(), one.model(), one.center(), one.net_type());
    if (fileid->contains(key)) {
      (*fileid)[key].f500 = one.id();
    }
  }
  delete resp;  
  
  resp = requestAvailable(&req, "dd", 850);
  if (nullptr == resp) return false;
  for (auto one: resp->descr()) {
    FieldsKey key(one.hour(), one.model(), one.center(), one.net_type());
    if (fileid->contains(key)) {
      (*fileid)[key].d850 = one.id();
    }
  }
  delete resp;  

  resp = requestAvailable(&req, "dd", 500);
  if (nullptr == resp) return false;
  for (auto one: resp->descr()) {
    FieldsKey key(one.hour(), one.model(), one.center(), one.net_type());
    if (fileid->contains(key)) {
      (*fileid)[key].d500 = one.id();
    }
  }
  delete resp;  

  return true;
}

meteo::field::DataDescResponse* ObanalIndexes::requestAvailable(meteo::field::DataRequest* req, const QString& descrname, int level)
{
  if (nullptr == req) return nullptr;
  
  auto ctrl = std::unique_ptr<meteo::rpc::Channel>(meteo::global::serviceChannel( meteo::settings::proto::kField ));
  if(nullptr == ctrl) {
    error_log << QObject::tr("Ошибка подключения к сервису доступа к результатам ОА");
    return nullptr;
  }

  req->clear_meteo_descr();
  req->clear_level();
  req->add_meteo_descr(TMeteoDescriptor::instance()->descriptor(descrname));
  req->add_level(level);

  meteo::field::DataDescResponse* resp = ctrl->remoteCall( &meteo::field::FieldService::GetAvailableData, *req, 300000 );
  if (nullptr == resp) {
    error_log << QObject::tr("При попытке выполнить расчет индексов неустойчивости ответ от сервиса полей не получен");
  }

  return resp;
}
