#ifndef METEO_COMMONS_MSGDATA_MSGUPDATER_H
#define METEO_COMMONS_MSGDATA_MSGUPDATER_H

#include <qobject.h>

#include <commons/container/ts_list.h>
#include <commons/container/ringvector.h>

#include <meteo/commons/global/appstatusthread.h>

#include "types.h"

class QTimer;

namespace meteo {

class MsgUpdater : public QObject
{
  Q_OBJECT

public:
  explicit MsgUpdater(QObject* parent = nullptr);

  void setRequestQueue(TSList<msgcenter::DecodeRequest>* queue) { queue_ = queue; }
  void setStatusSender(meteo::AppStatusThread* status);

public slots:
  void slotInit();
  void slotRun();

private:
  // данные
  TSList<msgcenter::DecodeRequest>* queue_ = nullptr;
  RingVector<TimeCountMetric> metric_;

  // служебные
  bool run_       = false;
  QTimer* timer_  = nullptr;
  meteo::AppStatusThread* status_ = nullptr;
};

} // meteo

#endif // METEO_COMMONS_MSGDATA_MSGUPDATER_H
