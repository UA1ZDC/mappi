#include "appstatus.h"

#include <qprocess.h>
#include <qtimer.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/appconf.pb.h>

#include "methodbag.h"

namespace meteo {
namespace map {

StatusSender::StatusSender( MethodBag* h )
  : QObject(h),
  handler_(h),
  appctrl_(nullptr),
  timer_( new QTimer(this) ),
  mutex_(QMutex::Recursive)
{
  QObject::connect( timer_, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
  timer_->setInterval(1000);
  timer_->start();
}

StatusSender::~StatusSender()
{
  delete appctrl_; appctrl_ = nullptr;
  delete timer_; timer_ = nullptr;
}

bool StatusSender::init()
{
  if ( nullptr != appctrl_ ) {
    delete appctrl_; appctrl_ = nullptr;
  }
  appctrl_ = global::serviceChannel(settings::proto::kDiagnostic);
  if ( nullptr == appctrl_ ) {
    error_log << QObject::tr("Не удалось подключиться к сервису диагностики");
    return false;
  }
  return true;
}

void StatusSender::incMap( const QString& title, int mapsize, int mapsucsize  )
{
  QMutexLocker lock(&mutex_);
  
  if ( false == mapstat_.contains(title) ) {
    mapstat_[title].second.first = 0;
    mapstat_[title].second.second = 0;
  }
  auto counter = mapstat_[title];
  counter.first = QDateTime::currentDateTime();
  counter.second.first += mapsize;
  counter.second.second += mapsucsize;
  mapstat_[title] = counter;
}

void StatusSender::incJob( const QString& title, int mapsize, int mapsucsize  )
{
  QMutexLocker lock(&mutex_);
  
  if ( false == jobstat_.contains(title) ) {
    jobstat_[title].second.first = 0;
    jobstat_[title].second.second = 0;
  }
  auto counter = jobstat_[title];
  counter.first = QDateTime::currentDateTime();
  counter.second.first += mapsize;
  counter.second.second += mapsucsize;
  jobstat_[title] = counter;
}

void StatusSender::incDoc( int createdsize, int delsize )
{
  QMutexLocker lock(&mutex_);
  if ( false == docstat_.first.isValid() ) {
    docstat_.second.first = 0;
    docstat_.second.second = 0;
  }
  docstat_.first = QDateTime::currentDateTime();
  docstat_.second.first += createdsize;
  docstat_.second.second += delsize;

}

void StatusSender::incJobSize( int running, int queue)
{
  QMutexLocker lock(&mutex_);
  if ( false == jobsize_.first.isValid() ) {
    jobsize_.second.first = 0;
    jobsize_.second.second = 0;
  }
  jobsize_.first = QDateTime::currentDateTime();
  jobsize_.second.first += running;
  jobsize_.second.second += queue;

}

void StatusSender::slotTimeout()
{
  if ( nullptr == appctrl_ || false == appctrl_->isConnected() ) {
    warning_log << QObject::tr("Нет соединения с сервисом дигностики");
    if ( false == init() ) {
      return;
    }
  }
  app::OperationStatus status = getStatus();
  if ( false == status.IsInitialized() ) {
    return;
  }
  app::Dummy* resp = appctrl_->remoteCall( &app::ControlService::SetOperaionStatus, status, 1000 );
  delete resp;
}

app::OperationStatus StatusSender::getStatus() const
{
  app::OperationStatus status;
  int64_t procid = -1;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  if ( false == env.contains("procid") ) {
//    error_log << QObject::tr("Не удалось получить идентифкатор процесса 'procid'. Отправка статистики невозможна");
    return status;
  }
  procid = env.value("procid").toLongLong();
  status.set_id(procid);

  app::OperationParam* param = status.add_param();
  QMutexLocker lock(&mutex_);

  param->set_title( QObject::tr("Заданий выполняется/в очереди").toStdString() );
  param->set_value( QString("%1/%2").arg( jobsize_.second.first ).arg( jobsize_.second.second ).toStdString() );
  param->set_dt( jobsize_.first.toString(Qt::ISODate).toStdString() );

  param = status.add_param();
  param->set_title( QObject::tr("Документов создано/удалено").toStdString() );
  param->set_value( QString("%1/%2").arg( docstat_.second.first ).arg( docstat_.second.second ).toStdString() );
  param->set_dt( docstat_.first.toString(Qt::ISODate).toStdString() );

  auto mit = mapstat_.begin();
  auto mend = mapstat_.end();
  while ( mit != mend ) {
    param = status.add_param();
    param->set_title( QObject::tr("Построено карт - '%1' всего/успешно").arg( mit.key() ).toStdString() );
    param->set_value( QString("%1/%2").arg( mit.value().second.first ).arg( mit.value().second.second ).toStdString() );
    param->set_dt( mit.value().first.toString(Qt::ISODate).toStdString() );
    ++mit;
  }

  auto jit = jobstat_.begin();
  auto jend = jobstat_.end();
  while ( jit != jend ) {
    param = status.add_param();
    param->set_title( QObject::tr("Выполнено заданий '%1' всего/успешно").arg( jit.key() ).toStdString() );
    param->set_value( QString("%1/%2").arg( jit.value().second.first ).arg( jit.value().second.second ).toStdString() );
    param->set_dt( jit.value().first.toString(Qt::ISODate).toStdString() );
    ++jit;
  }
  
//  param = status.add_param();
//  param->set_title( QObject::tr("Всего клиентов").toStdString() );
//  param->set_value( QString::number( handler_->connections_.size()).toStdString() );
//  param->set_dt( QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() );
//  param->set_state( app::OperationState_NORM );
//
//  for(const auto& c : handler_->connections_) {
//    app::OperationParam* param = status.add_param();
//    param->set_title(QObject::tr("Клиент").toStdString());
//    if( nullptr == c->channel() ) {
//      param->set_value( QObject::tr("Не установлен клиент").toStdString() );
//      param->set_dt(QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() );
//      param->set_state( app::OperationState_ERROR );
//    }
//    else {
//      param->set_value(QObject::tr("%1").arg(c->channel()->address() ).toStdString());
//      param->set_dt(QDateTime::currentDateTime().toString(Qt::ISODate).toStdString());
//      param->set_state( app::OperationState_NORM);
//    }
//    param = status.add_param();
//    param->set_title(QObject::tr("Выполняется метод").toStdString());
//    param->set_value(c->runnedstat().second.toStdString());
//    param->set_dt(c->runnedstat().first.toString(Qt::ISODate).toStdString());
//    for( const auto m : c->deferredstat() ) {
//      param = status.add_param();
//      param->set_title( QObject::tr("Отложен метод").toStdString() );
//      param->set_value( m.second.toStdString() );
//      param->set_dt( m.first.toString(Qt::ISODate).toStdString() );
//    }
//  }
  return status;
}

void StatusSender::slotClientConnected()
{
}

void StatusSender::slotClientDisconnected()
{
}

}
}
