#ifndef METEO_COMMONS_SERVICES_SRCDATA_DATALOADER_H
#define METEO_COMMONS_SERVICES_SRCDATA_DATALOADER_H

#include <qmap.h>
#include <qobject.h>

#include <google/protobuf/service.h>

#include <commons/container/ts_list.h>

#include <meteo/commons/proto/surface.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/rpc/rpc.h>

namespace meteo {

enum MethodType {
  kGetMeteoDataOnStation,
  kGetMeteoData,
  kGetMeteoDataByDay,
  kGetMeteoDataMeteotablo,
  kGetDts,
  kGetGribMeteoData
};

class LoaderCard
{
public:
  QString           id;
  surf::DataRequest request;
  MethodType        method;

public:
  LoaderCard(){}

  LoaderCard(const QString& aId, MethodType aMethod, const surf::DataRequest& aRequest)
    : id(aId), request(aRequest), method(aMethod)
  {}
};

//! Класс DataLoader обеспечивает получение данных от удалённых центров данных
//! и их сохранение в локальной БД.
class DataLoader : public QObject
{
  Q_OBJECT

public:
  DataLoader(QObject* parent = 0);

  void setQueue(TSList<LoaderCard>* queue) { queue_ = queue; }
  void setAddress(const QString& address) { address_ = address; }

signals:
  void finished(const QString& id, int dataCount, bool result, const QString& comment, const QString& button);

public slots:
  void slotInit();

protected:
  virtual void timerEvent(QTimerEvent* event);

private:
  void callbackGetMeteoData(surf::DataReply* r);

private:
  QString address_;
  TSList<LoaderCard>* queue_ = nullptr;
  QMap<QString,LoaderCard> cards_;
  rpc::Channel* channel_ = nullptr;

  int timerId_ = -1;
};

} // meteo

#endif // METEO_COMMONS_SERVICES_SRCDATA_DATALOADER_H
