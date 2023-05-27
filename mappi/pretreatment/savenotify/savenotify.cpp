#include <mappi/pretreatment/savenotify/savenotify.h>

#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>
#include <mappi/proto/sessiondataservice.pb.h>
#include <mappi/proto/thematic.pb.h>


using namespace mappi;

SaveNotify* CreateNotify::createEmptyNotify()
{
  return new SaveNotify();
}


ServiceSaveNotify* CreateNotify::createServiceNotify(Type nt)
{
  ServiceSaveNotify* notify = nullptr;
  
  switch(nt) {
  case ServiceNotify:
    notify = new ServiceSaveNotify();
    break;
  case StubServiceNotify:
    notify = new StubServiceSaveNotify();
    break;
  case FledgetNotify:
    notify = new FledgetSaveNotify();
    break;
  default: {}
  }

  return notify;
}

//---------

ServiceSaveNotify::ServiceSaveNotify()
{
}

ServiceSaveNotify::~ServiceSaveNotify()
{
}

void ServiceSaveNotify::clear()
{
  _satname = QString();
  _dtStart = QDateTime();
  _instrs.clear();
}

//! для сохранения в БД
bool ServiceSaveNotify::dataNotify(const meteo::global::PoHeader& header, const QString& file)
{
  //trc;
  meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kDataControl);
  if (nullptr == ch) {
    error_log << meteo::msglog::kServiceConnectFailedSimple.arg(meteo::settings::proto::kDataControl);
    return false;
  }

  bool ok = false;

  mappi::proto::Pretreatment request;
  // var(start);
  // var(end);
  request.set_date_start(pbtools::toString(header.start.toString(Qt::ISODate)));
  request.set_date_end(pbtools::toString(header.stop.toString(Qt::ISODate)));
  request.set_instrument_type(header.instr);
  request.add_channel(header.channel);
  request.set_calibration(header.procflag == 0x1);
  request.set_session_id(_sessionId);
  request.set_path(pbtools::toString(file));
  

  mappi::proto::Response* response = ch->remoteCall(&mappi::proto::SessionDataService::SavePretreatment,
						    request, 10000, true);

  delete ch;
  
  if (response == nullptr) {
    return false;
  }
  
  ok = response->result();  
  if (!ok) {
    var(response->comment());
  }
  
  delete response;

  if (ok) {
    _satname = header.satellite;
    _dtStart = header.start;
    if (!_instrs.contains(header.instr)) {
      _instrs.append(header.instr);
    }
  }

  return ok;
}

//! отправка сообщения в сервис ТО
bool ServiceSaveNotify::finish()
{
  trc;
  meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kThematic);
  if (nullptr == ch) {
    error_log << meteo::msglog::kServiceConnectFailedSimple.arg(meteo::global::serviceTitle(meteo::settings::proto::kThematic));
    return false;
  }

  mappi::conf::PerformThemRequest request;
  request.set_sat_name(_satname.toStdString());
  request.set_date_start(_dtStart.toString(Qt::ISODate).toStdString());
  for (auto itype : _instrs) {
    request.add_instr(itype);
  }

  var(request.Utf8DebugString());
  
  mappi::conf::ThemResponse* res =  ch->remoteCall(&mappi::conf::ThematicService::PerformThemProcess,
						  request, 10000, true);
  delete ch;
  if (res == nullptr) {
    return false;
  }

  debug_log << "add them task" << res->result() << res->comment();
  
  delete res;

  clear();
  
  return true;
}


//---------------------


//! сохр в БД сесии, в БД ПО

bool StubServiceSaveNotify::rawNotify(const meteo::global::StreamHeader& header, const QString& fileName)
{
  //trc;
  uint64_t sessionId = 0;   

  meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kDataControl);
  if (nullptr == ch) {
    error_log << meteo::msglog::kServiceConnectFailedSimple.arg(meteo::global::serviceTitle(meteo::settings::proto::kDataControl));
    return 0;
  }

  var(header.start);
  
  mappi::proto::SessionData request;
  request.set_sat_name(header.satellite.toStdString());
  request.set_date_start(header.start.toString(Qt::ISODate).toStdString());
  if (header.stop.isValid()) {
    request.set_date_end(header.stop.toString(Qt::ISODate).toStdString());
  } else {
    request.set_date_end(header.start.toString(Qt::ISODate).toStdString());
  }
  request.mutable_site()->set_name(pbtools::toString(header.site));
  request.mutable_site()->mutable_point()->set_lat_radian(header.siteCoord.lat());
  request.mutable_site()->mutable_point()->set_lon_radian(header.siteCoord.lon());
  request.mutable_site()->mutable_point()->set_height_meters(header.siteCoord.alt());
  request.set_server_path(pbtools::toString(fileName));

  QByteArray tle;
  QDataStream ds(&tle, QIODevice::WriteOnly);
  ds << header.tle;
  tle = tle.toBase64();
  request.set_tle(tle, tle.size());

  //var(request.Utf8DebugString());

  mappi::proto::Response* response = ch->remoteCall(&mappi::proto::SessionDataService::SaveSession,
  						    request, 10000, true);

  delete ch;

  if (response != nullptr && response->has_result()) {
    sessionId = response->index();
  }
  delete response;

  var(sessionId);
  
  ServiceSaveNotify::setSource(sessionId);
  
  return true;
}

//---------------------

bool FledgetSaveNotify::imageNotify(const meteo::global::PoHeader& header,
				    const QString& proj, const QString& format, const QString& file)
{
  trc;
  
  meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kDataControl);
  if (nullptr == ch) {
    error_log << meteo::msglog::kServiceConnectFailedSimple.arg(meteo::settings::proto::kDataControl);
    return false;
  }

  bool ok = false;

  mappi::proto::ThematicData request;
  // var(start);
  // var(end);
  request.set_session_id(sessionId());
  request.set_satellite_name(satname().toStdString());
  request.set_type(mappi::conf::kGrayScale);
  request.set_date_start(pbtools::toString(header.start.toString(Qt::ISODate)));
  request.set_date_end(pbtools::toString(header.stop.toString(Qt::ISODate)));
  request.set_instrument_type(header.instr);
  request.set_projection(proj.toStdString());
  request.set_format(format.toStdString());
  //  request.add_channel(header.channel);
  request.set_path(pbtools::toString(file));
  

  mappi::proto::Response* response = ch->remoteCall(&mappi::proto::SessionDataService::SaveThematic,
						    request, 10000, true);

  delete ch;
  
  if (response == nullptr) {
    return false;
  }
  
  ok = response->result();  
  if (!ok) {
    var(response->comment());
  }
  
  delete response;

  var(ok);

  return ok;
}
