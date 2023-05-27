#ifndef METEO_COMMONS_SERVICES_STATE_RPCTIMER_H
#define METEO_COMMONS_SERVICES_STATE_RPCTIMER_H

#include <QObject>

class QTimer;

#include <cross-commons/app/tsigslot.h>
#include <meteo/commons/services/decoders/tservicestat.h>
#include <meteo/commons/rpc/rpc.h>

namespace rpc {
  class Channel;
  class Address;
}

namespace meteo {
  namespace app {
    class OperationStatus;
  }
}

namespace meteo {
namespace state {

//! отправка статуса по таймеру через rpc
class RpcTimer :  public TServiceStatus
{
Q_OBJECT
public:
  RpcTimer(const QString& manager, uint reconnect_timeout = 5);
  ~RpcTimer();

  void setAppId(meteo::app::OperationStatus* status) const;
 
  bool isNeedUpdate() { return _needUpdate; }

  void messageHandler(meteo::app::Dummy *reply) { delete reply; reply = nullptr; }

public slots:
  void chanDisconnected();
  
protected:
  void setup();
  void send(const meteo::app::OperationStatus& status);

private slots:

  void slotUpdateStatus();

private:
 
  QTimer* _updateTimer;
  rpc::Channel* _managerChannel = nullptr;

  QString _managerName;
  uint _timeout;

  bool _needUpdate;
};

}
}

#endif
