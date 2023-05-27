#include "dataloader.h"

#include <cross-commons/debug/tlog.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/meteo_data/complexmeteo.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/primarydb/ancdb.h>

namespace meteo {

DataLoader::DataLoader(QObject* parent)
  : QObject(parent)
{
}

void DataLoader::slotInit()
{
  timerId_ = startTimer(10000);
}

void DataLoader::timerEvent(QTimerEvent* event)
{
  Q_UNUSED( event );

  if ( address_.isEmpty() || queue_->size() == 0 ) { return; }

  killTimer(timerId_);

  if ( nullptr == channel_ || !channel_->isConnected() ) {
    delete channel_;
    channel_ = rpc::Channel::connect(address_);
  }

  while ( nullptr != channel_ && channel_->isConnected() && queue_->size() > 0 ) {
    LoaderCard c = queue_->takeFirst();
    c.request.set_use_parent(false);
    c.request.set_request_id(c.id.toStdString());

    switch ( c.method ) {
    case kGetMeteoDataOnStation:
    {
      if ( !channel_->remoteCall(&surf::SurfaceService::GetMeteoDataOnStation, c.request, this, &DataLoader::callbackGetMeteoData) ) {
        error_log << tr("Ошибка при выполнении запроса к сервису.");
        debug_log << "REQUEST:\n" << c.request.Utf8DebugString();
        queue_->append(c);
      }
      else {
        // FIXME: если ответ от сервиса не придёт, то в card_ будут накапливаться данные
        cards_.insert(c.id, c);
      }
    } break;
    case kGetMeteoDataMeteotablo:
    {
      if ( !channel_->remoteCall(&surf::SurfaceService::GetMeteoDataMeteotablo, c.request, this, &DataLoader::callbackGetMeteoData) ) {
        error_log << tr("Ошибка при выполнении запроса к сервису.");
        debug_log << "REQUEST:\n" << c.request.Utf8DebugString();
        queue_->append(c);
      }
      else {
        // FIXME: если ответ от сервиса не придёт, то в card_ будут накапливаться данные
        cards_.insert(c.id, c);
      }
    }
    case kGetMeteoData:
    {
      if ( !channel_->remoteCall(&surf::SurfaceService::GetMeteoData, c.request, this, &DataLoader::callbackGetMeteoData) ) {
        error_log << tr("Ошибка при выполнении запроса к сервису.");
        debug_log << "REQUEST:\n" << c.request.Utf8DebugString();
        queue_->append(c);
      }
      else {
        // FIXME: если ответ от сервиса не придёт, то в card_ будут накапливаться данные
        cards_.insert(c.id, c);
      }
    } break;
    case kGetGribMeteoData:
    {
      if ( !channel_->remoteCall(&surf::SurfaceService::GetMeteoData, c.request, this, &DataLoader::callbackGetMeteoData) ) {
        error_log << tr("Ошибка при выполнении запроса к сервису.");
        debug_log << "REQUEST:\n" << c.request.Utf8DebugString();
        queue_->append(c);
      }
      else {
        // FIXME: если ответ от сервиса не придёт, то в card_ будут накапливаться данные
        cards_.insert(c.id, c);
      }
    } break;
    case kGetMeteoDataByDay:
    {
      if ( !channel_->remoteCall(&surf::SurfaceService::GetMeteoDataByDay, c.request, this, &DataLoader::callbackGetMeteoData) ) {
        error_log << tr("Ошибка при выполнении запроса к сервису.");
        debug_log << "REQUEST:\n" << c.request.Utf8DebugString();
        queue_->append(c);
      }
      else {
        // FIXME: если ответ от сервиса не придёт, то в card_ будут накапливаться данные
        cards_.insert(c.id, c);
      }
    } break;
    case kGetDts:
    {
      if ( !channel_->remoteCall(&surf::SurfaceService::GetDtsForPeriod, c.request, this, &DataLoader::callbackGetMeteoData) ) {
        error_log << tr("Ошибка при выполнении запроса к сервису.");
        debug_log << "REQUEST:\n" << c.request.Utf8DebugString();
        queue_->append(c);
      }
      else {
        // FIXME: если ответ от сервиса не придёт, то в card_ будут накапливаться данные
        cards_.insert(c.id, c);
      }
    } break;
    }

  }

  timerId_ = startTimer(10000);
}

void DataLoader::callbackGetMeteoData(surf::DataReply* r)
{
  QString id = QString::fromStdString(r->request_id());
  if ( !cards_.contains(id ) ) {
    delete r;
    debug_log << tr("Карточка для запроса %1 не найдена").arg(id);
    return;
  }

  const LoaderCard& c = cards_[id];

  AncDb* db = meteo::ancdb();

  TMeteoData md;
  for ( int i = 0, isz = r->meteodata_size(); i < isz; ++i ) {
    md << QByteArray::fromRawData(r->meteodata(i).data(), r->meteodata(i).size());
    QDateTime dt = TMeteoDescriptor::instance()->dateTime(md);

    meteo::GeoPoint gp;
    if ( !TMeteoDescriptor::instance()->getCoord(md,&gp) ) {
      // FIXME: запрашивать координаты
      error_log << tr("Ошибка при сохранении данных: не удалось определить координаты.");
      //      md.printData();
      continue;
    }
    md.setCoord(gp.fi(),gp.la(),gp.alt());

    //    int station_type = md.getValue(TMeteoDescriptor::instance()->descriptor("station_type"), 0, false);
    int category = md.getValue(TMeteoDescriptor::instance()->descriptor("category"), -1, false);
    if ( -1 == category ) {
      error_log << tr("Ошибка при сохранении данных: не удалось определить тип данных.");
      //      md.printData();
      continue;
    }

    meteo::StationInfo info;
    db->fillStationInfo(&md, category, &info);
    // FIXME: как поступать, если некоторые (или все) данные не удалось сохранить в локальную БД?
    if ( !db->saveReport(md, category, dt, info, "meteoreport") ) {
      error_log << QObject::tr("Ошибка при сохранении данных");
      //      md.printData();
    }
  }
  emit finished(c.id, r->meteodata_size(), r->result(), QString::fromStdString(c.request.notify_message()), QString::fromStdString(c.request.notify_action()));

  delete r;
  cards_.remove(id);
}

} // meteo
