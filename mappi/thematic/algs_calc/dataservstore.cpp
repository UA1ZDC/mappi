#include "dataservstore.h"

#include <mappi/proto/sessiondataservice.pb.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>

using namespace mappi;
using namespace to;

DataServiceStore::DataServiceStore()
{
}

DataServiceStore::~DataServiceStore()
{
}

bool DataServiceStore::getFiles(const QDateTime& start, const QString& satname, mappi::conf::InstrumentType itype,
        const QList<std::string>& chAlias, QMap<std::string, QString>* chfiles)
{
  if (nullptr == chfiles) {
    return false;
  }

  auto* ch = meteo::global::serviceChannel(meteo::settings::proto::kDataControl);
  if (nullptr == ch) {
    return false;
  }

  mappi::proto::Pretreatment req; //TODO может отдельный для запроса
  req.set_date_start(start.toString(Qt::ISODate).toStdString());
  req.set_sat_name(satname.toStdString());
  req.set_instrument_type(itype);
  for (const auto &alias : chAlias) {
    req.add_channel_alias(alias);
  }
  
  proto::PretreatmentList* response = ch->remoteCall(&mappi::proto::SessionDataService::GetAvailablePretreatment, req, 60 * 1000, true);
  delete ch;

  //var(response->Utf8DebugString());

  if(nullptr == response) {
    error_log << QObject::tr("Ответ от сервиса данных не получен");
    return false;
  }

  if(!response->result()){
      error_log << response->comment();
      delete response;
      return false;
  }

  if (0 == response->pretreatments_size()) {
    //error_log << QObject::tr("Нет данных");
  } else {
    for(const auto &channel : response->pretreatments()) {
      //var(channel.channel_alias(0));
      chfiles->insert(channel.channel_alias(0),
          QString::fromStdString(channel.path()));
    }
  }
  
  delete response;
    
  return true;  
}

bool DataServiceStore::getFiles(const QDateTime& start, const QString& satname, mappi::conf::InstrumentType itype,
        const QList<uint8_t>& chNum, QMap<uint8_t, QString>* chfiles)
{
  if (nullptr == chfiles) {
    return false;
  }

  auto* ch = meteo::global::serviceChannel(meteo::settings::proto::kDataControl);
  if (nullptr == ch) {
    return false;
  }

  mappi::proto::Pretreatment req; //TODO может отдельный для запроса
  req.set_date_start(start.toString(Qt::ISODate).toStdString());
  req.set_sat_name(satname.toStdString());
  req.set_instrument_type(itype);
  for (auto number : chNum) {  
    req.add_channel(number);
  }
  
  proto::PretreatmentList* response = ch->remoteCall(&mappi::proto::SessionDataService::GetAvailablePretreatment, req, 60 * 1000, true);
  delete ch;

  //var(response->Utf8DebugString());

  if(nullptr == response) {
    error_log << QObject::tr("Ответ от сервиса данных не получен");
    return false;
  }

  if (0 == response->pretreatments_size()) {
    error_log << QObject::tr("Нет данных");
  } else {
    for(const auto &channel : response->pretreatments()) {
      var(channel.channel(0));
      chfiles->insert(channel.channel(0),
          QString::fromStdString(channel.path()));
    }
  }
  
  delete response;
    
  return true;  
}

  
// bool DataServiceStore::saveThematic(const QDateTime& start, mappi::conf::ThemType type)
// {
//   Q_UNUSED(start);
//   Q_UNUSED(type);
  
//   return false;
// }


bool DataServiceStore::save(const struct meteo::global::PoHeader& header,
                            conf::ThemType type, const std::string &themname, const QString& fileName,const std::string &format)
{
  bool ok = false;
  auto* channel = meteo::global::serviceChannel(meteo::settings::proto::kDataControl);

  if (nullptr == channel) {
    return false;
  }

  proto::ThematicData req;
  req.set_satellite_name(header.satellite.toStdString());
  req.set_type(type);
  req.set_instrument_type(header.instr);
  req.set_date_start(header.start.toString(Qt::ISODate).toStdString());
  req.set_date_end(header.stop.toString(Qt::ISODate).toStdString());
  req.set_path(fileName.toStdString());
  req.set_name(themname);
  req.set_format(format);
  // req.set_channel_number(ch_number);
  
  auto* resp = channel->remoteCall(&mappi::proto::SessionDataService::SaveThematic, req, 10000, true);
  if(nullptr != resp) {
    ok = (resp->has_result() ? resp->result() : false);
    if(false == ok && resp->has_comment()) {
      error_log << QString::fromStdString(resp->comment());
    }
    delete resp;
  }
  
  delete channel;
    
  return ok;
}
