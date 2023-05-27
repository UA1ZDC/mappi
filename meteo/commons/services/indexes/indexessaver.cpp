#include "indexessaver.h"

#include <cross-commons/debug/tlog.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/global/global.h>

#include <meteo/commons/primarydb/ancdb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/rpc/rpc.h>

#include <meteo/commons/zond/zond.h>
#include <meteo/commons/zond/indexes.h>

#include <sql/nosql/nosqlquery.h>

const int kTimeout = 1000000;

using namespace meteo;

IndexesSaver::IndexesSaver()
{
}

IndexesSaver::~IndexesSaver()
{
}

bool IndexesSaver::calcVolatileIndexes(const QDateTime& dt)
{
  QTime ttt;
  ttt.start();

  surf::DataRequest request;
  request.set_date_start(QString("%1").arg(dt.toString(Qt::ISODate)).toStdString());
  request.add_type(meteo::surf::kAeroFix);

  rpc::Channel* ctrl = meteo::global::serviceChannel( meteo::settings::proto::kSrcData );
  if ( nullptr == ctrl ) {
      error_log << QObject::tr("Не удалось установить соединение со службой доступа к данным");
      return false;
    }
  surf::StationsDataReply* response = ctrl->remoteCall( &meteo::surf::SurfaceService::GetAvailableStations, request, 100000 );
  delete ctrl;

  if ( nullptr == response ) {
      return false;
    }

  //var(response->coords_size());

  int cnt = 0;

  bool rok = false;
  for (auto pnt: response->coords()) {
      ::zond::Zond zond;
      bool ok = getAero(pnt.index(), dt, &zond);
      if (!ok) { continue; }

      //var(pnt.index());

      TMeteoData md;
      md.setStation(std::stoi(pnt.index()));
      md.add(TMeteoDescriptor::instance()->descriptor("station_type"), "", pnt.type(), control::RIGHT);
      md.add(TMeteoDescriptor::instance()->descriptor("category"), "", meteo::surf::kAeroFix, control::RIGHT);
      md.setCoord(MnMath::rad2deg(pnt.fi()), MnMath::rad2deg(pnt.la()), pnt.height());
      md.add(TMeteoDescriptor::instance()->descriptor("level_type"), "", meteodescr::kSurfaceLevel, control::RIGHT);
      md.setDateTime(dt);

      if (calcIndexes(zond, &md)) {
          if (saveIndexes(&md, meteo::surf::kAeroFix, dt)) {
              rok = true;
              ++cnt;
            }
        } else {
          // debug_log << "no index" << pnt.index();
        }
    }

  debug_log << "indexes" << dt.toString(Qt::ISODate) << "cnt" << cnt << "time save=" << ttt.elapsed();

  delete response;

  return rok;
}

bool IndexesSaver::calcHydroLevels(const QDateTime &dt)
{
  QTime ttt;
  ttt.start();

  QString error;
  auto return_func = [&error]( bool fl ) {
      if ( false == fl ) {
          error_log << error;
        }
      else if ( 0 != error.size() ) {
          info_log << error;
        }
      return fl;
    };

  surf::DataRequest request;
  request.add_meteo_descrname("HHHH");
  getStationsWithHydroLevels();
  //  request.add_type(meteo::surf::kAeroFix);
  for(auto index : stations_.keys()) {
      request.add_station(QString::number(index).toStdString());
    }

  std::unique_ptr<Dbi> db(meteo::global::dbMeteo());
  if ( nullptr == db.get() ) { return_func(false); }
  auto query = db->queryptrByName("get_meteo_data");
  if(nullptr == query) {return return_func(false);}

  query->arg("start_dt", dt );
  query->arg("station", request.station());
  query->arg("level_type", 1);
  query->arg("level", 0);
  query->arg("descrname", request.meteo_descrname());

  if ( false == query->execInit(&error)) {
      return return_func(false);
    }

  int cnt = 0;
  while ( true == query->next() ) {
      TMeteoData md;
      const DbiEntry& doc = query->entry();
      int categ = doc.valueInt32("data_type");
      if ( false == global::json2meteodata( &doc, &md ) ) {
          error_log << QObject::tr("Не удалось получить метеоданные из документа");
          continue;
        }
      if ( false == md.hasParam("HHHH") ) {
          error_log << QObject::tr("Нет данных об уровне воды");
          continue;
        }
      auto hhhh = md.meteoParam("HHHH");
      int station_index = md.meteoParam("station_index").value();
      if ( false == stations_.keys().contains(station_index) ) {
          error_log << QObject::tr("Нет данных о порогах для станции");
          continue;
        }
      int hhhhl = 0;
      auto levels = stations_[station_index].hydro_levels();
      if ( true == levels.has_floodplain() && hhhh.value() >= levels.floodplain() - 50 ) {
          hhhhl = 1;
        }
      if ( true == levels.has_floodplain() && hhhh.value() >= levels.floodplain() ) {
          hhhhl = 2;
        }
      if ( true == levels.has_flooding() && hhhh.value() >= levels.flooding() ) {
          hhhhl = 3;
        }
      if ( true == levels.has_danger_level() && hhhh.value() >= levels.danger_level() ) {
          hhhhl = 4;
        }
      if ( 0 == hhhhl ) {
          continue;
        }
      TMeteoParam hydroLevel(QString::number(hhhhl), hhhhl, control::RIGHT);
      md.add("HHHHL", hydroLevel);
      saveIndexes(&md,categ,dt);
      ++cnt;
    }
  debug_log << "hydro" << dt.toString(Qt::ISODate) << "cnt" << cnt << "time save=" << ttt.elapsed();
  return true;
}


bool IndexesSaver::getAero(const std::string& station, const QDateTime& dt, zond::Zond* zond)
{
  surf::DataRequest req;
  req.add_station(station);
  req.add_type(surf::kAeroType);
  req.set_date_start(dt.toString(Qt::ISODate).toStdString());
  req.set_only_last(1);

  ::meteo::rpc::Channel* channel = ::meteo::global::serviceChannel(meteo::settings::proto::kSrcData);
  if ( nullptr == channel ) {
      error_log << QObject::tr("Не удалось установить соединение со службой доступа к данным");
      return false;
    }
  surf::TZondValueReply* aero = channel->remoteCall(&::meteo::surf::SurfaceService::GetTZondOnStation, req, kTimeout);
  delete channel;

  if (aero == nullptr) return false;

  QByteArray buf(aero->data(0).meteodata().data(), aero->data(0).meteodata().size());
  (*zond) << buf;

  zond->preobr();

  return true;
}


bool IndexesSaver::calcIndexes(const zond::Zond& zond, TMeteoData* md)
{
  float val;
  bool rok = false;
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

  bool ok = kiIndex(zond, &val);
  if (ok) {
      rok = true;
      md->add(d->descriptor("ki"), QString::number(val), val, control::RIGHT);
    }
  ok = verticalTotalsIndex(zond, &val);
  if (ok) {
      rok = true;
      md->add(d->descriptor("vti"), QString::number(val), val, control::RIGHT);
    }
  ok = crossTotalsIndex(zond, &val);
  if (ok) {
      rok = true;
      md->add(d->descriptor("cti"), QString::number(val), val, control::RIGHT);
    }
  ok = totalTotalsIndex(zond, &val);
  if (ok) {
      rok = true;
      md->add(d->descriptor("tti"), QString::number(val), val, control::RIGHT);
    }
  ok = sweatIndex(zond, &val);
  if (ok) {
      rok = true;
      md->add(d->descriptor("sweat"), QString::number(val), val, control::RIGHT);
    }

  return rok;
}


bool IndexesSaver::saveIndexes(TMeteoData* md, int categ, const QDateTime& dt)
{
  meteo::StationInfo info;
  ancdb()->setPtkppId("");
  ancdb()->fillStationInfo(md, categ, &info);

  // debug_log;
  // md->printData();
  // debug_log;

  bool ok = ancdb()->saveReport(*md, categ, dt, info, "meteoreport");

  return ok;
}

bool IndexesSaver::getStationsWithHydroLevels()
{
  sprinf::MultiStatementRequest req;
  rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kSprinf );
  if ( nullptr == ch ) {
      error_log << QObject::tr("Не удалось установить соединение со службой доступа к данным");
      return false;
    }

  sprinf::Stations* resp = ch->remoteCall( &meteo::sprinf::SprinfService::GetStations, req, 100000);
  delete ch;

  if ( nullptr == resp ) {
      return false;
    }

  for( int i = 0, sz = resp->station_size(); i < sz; ++i ) {
      if ( true == resp->station(i).has_hydro_levels() ) {
          stations_[resp->station(i).index()] = resp->station(i);
        }
    }

  return true;
}
