#include "appstatusthread.h"

#include <qelapsedtimer.h>
#include <qprocess.h>
#include <qtimer.h>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>

#include <commons/textproto/pbtools.h>

namespace meteo {

const quint64 AppStatusThread::kBytesInKiB = 1024;
const quint64 AppStatusThread::kBytesInMiB = AppStatusThread::kBytesInKiB * 1024;
const quint64 AppStatusThread::kBytesInGiB = AppStatusThread::kBytesInMiB * 1024;
const quint64 AppStatusThread::kBytesInTiB = AppStatusThread::kBytesInGiB * 1024;

const QString AppStatusThread::kSpeedByte = QObject::tr(" байт/сек.");
const QString AppStatusThread::kSpeedKiB  = QObject::tr(" Кбайт/сек.");
const QString AppStatusThread::kSpeedMiB  = QObject::tr(" Мбайт/сек.");
const QString AppStatusThread::kSpeedGiB  = QObject::tr(" Гбайт/сек.");

const QString AppStatusThread::kSizeByte = QObject::tr(" байт");
const QString AppStatusThread::kSizeKiB  = QObject::tr(" Кбайт");
const QString AppStatusThread::kSizeMiB  = QObject::tr(" Мбайт");
const QString AppStatusThread::kSizeGiB  = QObject::tr(" Гбайт");
const QString AppStatusThread::kSizeTiB  = QObject::tr(" Тбайт");

const std::string AppStatusThread::kUnkTitle = QObject::tr("< неизвестный параметр >").toUtf8().constData();

QString AppStatusThread::speedStr(quint64 bytes)
{
  QString str;

  if ( bytes >= kBytesInGiB )       { str = QString::number(double(bytes)/kBytesInGiB, 'f', 2) + kSpeedGiB; }
  else if ( bytes >= kBytesInMiB )  { str = QString::number(double(bytes)/kBytesInMiB, 'f', 2) + kSpeedMiB; }
  else if ( bytes >= kBytesInKiB )  { str = QString::number(double(bytes)/kBytesInKiB, 'f', 2) + kSpeedKiB; }
  else                              { str = QString::number(bytes) + kSpeedByte;  }

  return str;
}

QString AppStatusThread::sizeStr(quint64 bytes)
{
  QString str;

  if ( bytes >= kBytesInTiB )       { str = QString::number(double(bytes)/kBytesInTiB, 'f', 2) + kSizeTiB; }
  else if ( bytes >= kBytesInGiB )  { str = QString::number(double(bytes)/kBytesInGiB, 'f', 2) + kSizeGiB; }
  else if ( bytes >= kBytesInMiB )  { str = QString::number(double(bytes)/kBytesInMiB, 'f', 2) + kSizeMiB; }
  else if ( bytes >= kBytesInKiB )  { str = QString::number(double(bytes)/kBytesInKiB, 'f', 2) + kSizeKiB; }
  else                              { str = QString::number(bytes) + kSizeByte; }

  return str;
}


AppStatusThread::AppStatusThread(QObject* parent)
  : QThread(parent),
  m_(QMutex::Recursive),
  channel_(nullptr)
{
  setObjectName("AppStatus");

  status_.set_state(app::OperationState_NORM);

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  status_.set_id(env.value("procid").toLongLong());
}

AppStatusThread::~AppStatusThread()
{
  if ( nullptr != channel_ ) {
    delete channel_;
    channel_ = nullptr;
  }
}

void AppStatusThread::setUpdateLimit(int n)
{
  m_.lock();
  countLimit_ = n;
  m_.unlock();
}

void AppStatusThread::setSendLimit(int msec)
{
  m_.lock();
  sendLimit_ = msec;
  m_.unlock();
}

void AppStatusThread::setTitle(int paramId, const QString& title, qint64 value)
{
  setTitle(paramId, title);
  setParam(paramId, value);
}

QString AppStatusThread::getTitle(int paramId)
{
  m_.lock();
  QString pvalue;
  if ( titles_.contains(paramId) ) {
    pvalue = QString::fromStdString( titles_.value(paramId));
  }

  m_.unlock();
  sem_.release(1);
  return pvalue;
}

bool AppStatusThread::hasId(int paramId)
{
  m_.lock();
  bool res = titles_.contains(paramId);
  m_.unlock();
  return res;
}

void AppStatusThread::setTitle(int paramId, const QString& title)
{
  m_.lock();

  if ( titles_.contains(paramId) ) {
    debug_log << tr("Параметр с идентификатором %1 уже добавлен с список.").arg(paramId);
    m_.unlock();
    return;
  }

  titles_.insert(paramId, title.toUtf8().constData());
  m_.unlock();
}

QString AppStatusThread::getParamValue(int paramId)
{
  m_.lock();
  QString pvalue;
  app::OperationParam* param = 0;
  if ( param2index_.contains(paramId) ) {
    param = status_.mutable_param(param2index_[paramId]);
    pvalue = QString::fromStdString(param->value());
  }
  m_.unlock();
  sem_.release(1);
  return pvalue;
}

void AppStatusThread::setParam(int paramId, const std::string& value, app::OperationState state)
{
  m_.lock();

  app::OperationParam* param = 0;
  if ( param2index_.contains(paramId) ) {
    param = status_.mutable_param(param2index_[paramId]);
  }
  else {
    param = status_.add_param();
    param2index_[paramId] = status_.param_size() - 1;
  }

  param->set_state(state);
  param->set_title(titles_.value(paramId, kUnkTitle));
  param->set_value(value);
  param->set_dt(QDateTime::currentDateTime().toString(Qt::SystemLocaleDate).toUtf8().constData());

  m_.unlock();

  sem_.release(1);
}

void AppStatusThread::setParam(int paramId, const QString& value, app::OperationState state)
{
  setParam(paramId, pbtools::toString(value), state);
}

void AppStatusThread::setParam(int paramId, qint64 value, app::OperationState state)
{
  setParam(paramId, QString::number(value), state);
}

void AppStatusThread::setParam(int paramId, qint64 value1, qint64 value2, app::OperationState state)
{
  setParam(paramId, QString("%1/%2").arg(value1).arg(value2), state);
}

void AppStatusThread::unsetParam(int paramId)
{
  m_.lock();

  if ( !param2index_.contains(paramId) ) {
    m_.unlock();
    return;
  }

  int idx = param2index_[paramId];
  if ( idx >= status_.param_size() ) {
    debug_log << tr("Параметр с индексом %1 не найден.").arg(idx);
    m_.unlock();
    return;
  }
  for ( int i=idx,isz=status_.param_size()-2; i<isz; ++i ) {
    status_.mutable_param()->SwapElements(i,i+1);
  }
  status_.mutable_param()->RemoveLast();
  param2index_.remove(paramId);

  int i = idx + 1;
  while ( true ) {
    int param = param2index_.key(i, -1);
    if ( -1 == param ) { break; }
    param2index_[param] -= 1;
    ++i;
  }
  titles_.remove(paramId);//незнаю где, но где-то надо это сделать
  m_.unlock();

  sem_.release(1);
}

void AppStatusThread::clear()
{
  m_.lock();
  status_.Clear();
  m_.unlock();
}

void AppStatusThread::begin()
{
  m_.lock();
  sending_ = true;
  m_.unlock();
}

void AppStatusThread::end()
{
  m_.lock();
  sending_ = false;
  m_.unlock();

  send();
}

void AppStatusThread::send()
{
  m_.lock();
  int r = countLimit_ - sem_.available();
  m_.unlock();

  if ( 0 < r ) {
    sem_.release(r);
  }
//  else {
//    sem_.acquire(-r);
//  }
}

void AppStatusThread::run()
{
//  QElapsedTimer t;
//  t.start();
  tm_ = new QTimer();
  QObject::connect( tm_, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
  if ( 0 >= sendLimit_ || 1000 < sendLimit_ ) {
    tm_->setInterval(1000);
  }
  else {
    tm_->setInterval(sendLimit_);
  }
  tm_->start();

  QThread::exec();

  delete tm_;
  tm_ = nullptr;

//  while ( true ) {
////    debug_log << QObject::tr("Доступно ресурсов в семафоре: %1").arg( sem_.available() );
////    debug_log << QObject::tr("Установленный предел: %1").arg( countLimit_ );
//    int r = sem_.available();
//    if ( 0 == r ) {
//      r = 1;
//    }
//
//    sem_.acquire(r);
//
//    if ( -1 == sendLimit_ || t.elapsed() >= sendLimit_ ) {
//      sendStatus();
//      t.restart();
//    }
//  }
}

void AppStatusThread::slotTimeout()
{
  sendStatus();
}

bool AppStatusThread::sendStatus()
{
  QMutexLocker lock(&m_);

  if ( sending_ ) { return false; }

  sending_ = true;

  if ( 0 == channel_ ) {
    channel_ = meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic);
    if ( 0 == channel_ ) {
      return false;
    }
  }

  if ( !channel_->isConnected() ) {
    delete channel_;
    channel_ = meteo::global::serviceChannel(meteo::settings::proto::kDiagnostic);
    if ( nullptr == channel_ ) {
      return false;
    }
  }

  int state = app::OperationState_NONE;
  for ( int i=0,isz=status_.param_size(); i<isz; ++i ) {
    state = qMax(state, int(status_.param(i).state()));
  }
  status_.set_state(static_cast<app::OperationState>(state));

  app::OperationStatus st = status_;

  lock.unlock();

  channel_->remoteCall(&app::ControlService::SetOperaionStatus, st, this, &AppStatusThread::messageHandler);
  m_.lock();
  sending_ = false;
  m_.unlock();

  return true;
}

void AppStatusThread::messageHandler(app::Dummy *reply)
{
    delete reply;
    reply = nullptr;
}

} // meteo
