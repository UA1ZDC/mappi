#include "rpctimer.h"

#include <qtimer.h>
#include <QProcessEnvironment>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/appconf.pb.h>
#include <meteo/commons/global/global.h>


using namespace meteo;
using namespace state;

/*! 
  \param manager название менеджера, которому отправляется состояние
  \param reconnect_timeout время ожидания между попытками подключения к менеджеру (сек)
*/
RpcTimer::RpcTimer(const QString& manager, uint reconnect_timeout /*=5*/):
  _updateTimer(0),
  _managerName(manager),
  _timeout(reconnect_timeout),
  _needUpdate(false)
{
}

RpcTimer::~RpcTimer()
{
  if (0 != _updateTimer) {
    delete _updateTimer;
    _updateTimer = 0;
  }

  if (0 != _managerChannel) {
    delete _managerChannel;
    _managerChannel = 0;
  }
}

void RpcTimer::setup()
{
  if (nullptr == _managerChannel) {
    _managerChannel = meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic);
    if (nullptr != _managerChannel) {
      connect(_managerChannel, SIGNAL(disconnected()), this, SLOT(chanDisconnected()));
    }
  }

  if ( nullptr == _updateTimer ) {
    if ( nullptr != (_updateTimer = new QTimer) ) {
      connect( _updateTimer, SIGNAL(timeout()), SLOT(slotUpdateStatus()) );
    }
  }
  _updateTimer->stop();
  _updateTimer->setInterval( 1000 );
  _updateTimer->start();
}

void RpcTimer::chanDisconnected()
{
  if (nullptr != _managerChannel) {
    disconnect(_managerChannel, SIGNAL(disconnected()), this, SLOT(chanDisconnected()));
  }

  delete _managerChannel;
  _managerChannel = meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic);

  if (nullptr != _managerChannel) {
    connect(_managerChannel, SIGNAL(disconnected()), this, SLOT(chanDisconnected()));
  }
}

void RpcTimer::setAppId(meteo::app::OperationStatus* status) const
{
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  status->set_id(env.value("procid").toLongLong());
}

void RpcTimer::send(const meteo::app::OperationStatus& stat)
{
  if (nullptr == _managerChannel) {
    return;
  }
  
  meteo::app::OperationStatus status(stat);
  setAppId(&status);

  _managerChannel->remoteCall(&meteo::app::ControlService::SetOperaionStatus, status,
			      this, &meteo::state::RpcTimer::messageHandler);

}

void RpcTimer::slotUpdateStatus()
{
  if (nullptr == _managerChannel) {
    chanDisconnected();
  }
  
  _needUpdate = true;
  emit TServiceStatus::updateStatus();
}

