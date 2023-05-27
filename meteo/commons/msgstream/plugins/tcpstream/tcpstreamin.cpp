#include "tcpstreamin.h"

#include <qelapsedtimer.h>
#include <qbytearray.h>
#include <QtNetwork/qtcpsocket.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include "bcmdriver.h"
#include "ipvdriver.h"
#include "lingdriver.h"

namespace meteo {

TcpStreamIn::TcpStreamIn(QObject* p)
  : StreamModule(p)
{
  sock_ = 0;
  driver_ = 0;

  port_ = 0;

  stRecvBytes_ = 0;
  stMsgCount_  = 0;
}

TcpStreamIn::~TcpStreamIn()
{
  if ( 0 != sock_ ) {
    sock_->disconnectFromHost();
    sock_->waitForDisconnected(params_.disconnect_timeout());
    sock_->abort();
    delete sock_;
    sock_ = 0;
  }

  delete driver_;
  driver_ = 0;
}

void TcpStreamIn::setOptions(const msgstream::Options& options)
{
  opt_ = options;
  params_ = opt_.tcp_recv();

  host_ = pbtools::toQString(params_.host());
  port_ = params_.port();

  id_ = pbtools::toQString(opt_.id());

  prefixes_.clear();
  prefixes_ << QString::number(params_.utc_offset());
  prefixes_ << pbtools::toQString(params_.dev_name());
  prefixes_ << QString::number(params_.station());
}

bool TcpStreamIn::slotInit()
{
  if ( 0 == incoming_ ) { return false; }

  status_->setTitle(kRecvBytes, tr("Принято данных"));
  status_->setTitle(kRecvSpeed, tr("Скорость приёма данных"));
  status_->setTitle(kMkMsg, tr("Сформировано сообщений"));
  status_->setTitle(kState, tr("Состояние"));
  status_->setTitle(kCrcError, tr("Количество ошибок CRC"));

  status_->begin();
  status_->setParam(kRecvBytes, 0, app::OperationState_NONE);
  status_->setParam(kRecvSpeed, 0, app::OperationState_NONE);
  status_->setParam(kMkMsg, 0, app::OperationState_NONE);
  status_->setParam(kState, tr("Инициализация"), app::OperationState_NONE);
  status_->end();

  if ( !params_.has_dev() ) {
    error_log << tr("Ошибка инициализации: не указан тип устройства");
    status_->setParam(kState, tr("Ошибка инициализации: не указан тип устройства"), app::OperationState_ERROR);
    status_->send();
    return false;
  }

  if ( 0 == driver_ ) {
    switch ( params_.dev() ) {
      case msgstream::kDeviceBCM:
        driver_ = new BCMDriver(status_);
        break;
      case msgstream::kDeviceIPV:
        driver_ = new IPVDriver(status_);
        break;
      case msgstream::kDeviceLing:
        driver_ = new LingDriver(status_);
        break;
    }
    driver_->setPrefix(prefixes_.join(" ") + "\r\n");
  }

  while ( true == run() )
  {}

  return true;
}

bool TcpStreamIn::reconnect()
{
  if ( 0 != sock_ ) {
    if ( sock_->state() == QAbstractSocket::ConnectedState ) {
      sock_->disconnectFromHost();
      sock_->waitForDisconnected(params_.disconnect_timeout());
    }
    sock_->abort();
    delete sock_;
    sock_ = 0;
  }

  status_->setParam(kState, tr("Подключение к %1:%2").arg(host_).arg(port_));
  status_->send();

  sock_ = new QTcpSocket;
  sock_->connectToHost(host_, port_);
  if ( !sock_->waitForConnected(params_.connect_timeout()) ) {
    delete sock_;
    sock_ = 0;

    error_log << tr("Ошибка при подключении к %1:%2 - (истекло время ожидания)").arg(host_).arg(port_);
    status_->setParam(kState, tr("Нет подключения (истекло время ожидания)"), app::OperationState_ERROR);
    status_->send();

    QMutex m;
    m.lock();
    m.tryLock(params_.reconnect_timeout());

    return false;
  }
  else {
    status_->setParam(kState, tr("Подключено к %1:%2").arg(host_).arg(port_));
    status_->send();
    info_log << tr("Подключено к %1:%2").arg(host_).arg(port_);
  }

  return true;
}

bool TcpStreamIn::run()
{
  while ( !reconnect() )
  {}

  QElapsedTimer sendTimer;
  sendTimer.start();
  int interval = params_.tlg_interval_msec();
  int timeout = interval;

  int chunk = 1024*4;
  QByteArray raw(chunk, '\0');
  while ( true ) {
    qint64 rsz = sock_->read(raw.data(), chunk);

    if ( -1 == rsz ) { break; }

    stRecvBytes_ += rsz;
    status_->setParam(kRecvBytes, stRecvBytes_);
    status_->send();
//    debug_log << tr("Принято %1 байт").arg(rsz) << "total:" << stRecvBytes_;

    driver_->appendData(raw.left(rsz));

    while ( driver_->parse() ) {
      if ( -1 == params_.tlg_interval_msec() || sendTimer.elapsed() >= timeout ) {
        timeout = interval + (interval - sendTimer.restart());

        tlg::MessageNew msg;
        if ( driver_->getMessage(&msg) ) {
          msg.mutable_metainfo()->set_from(pbtools::toString(id_));
          msg.mutable_metainfo()->set_converted_dt(pbtools::toString(QDateTime::currentDateTimeUtc().toString(Qt::ISODate)));
          incoming_->append(msg);
          status_->setParam(kMkMsg, ++stMsgCount_);
          status_->send();
        }
      }
    }

    if ( false == sock_->waitForReadyRead(params_.wait_data_timeout()) && 0 == rsz ) {
      break;
    }
  }

  if ( QAbstractSocket::UnknownSocketError != sock_->error() ) {
    status_->setParam(kState, tr("Подключениие закрыто в результате ошибки"), app::OperationState_ERROR);
    error_log << tr("Подключение с %1:%2 закрыто из-за ошибки").arg(host_).arg(port_);
    debug_log << sock_->error() << ":" << sock_->errorString();

    sock_->abort();
    delete sock_;
    sock_ = 0;

    return true;
  }

  if ( QAbstractSocket::ConnectedState != sock_->state() ) {
    status_->setParam(kState, tr("Нет подключения"), app::OperationState_ERROR);
    error_log << tr("Подключение с %1:%2 закрыто").arg(host_).arg(port_);

    sock_->abort();
    delete sock_;
    sock_ = 0;

    return true;
  }

  return false;
}

} // meteo
