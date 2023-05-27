#include "controllerappclient.h"

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/proto/appconf.pb.h>

namespace meteo {

ControllerAppClient::ControllerAppClient(QObject* parent)
  : QObject(parent)
{
}

void ControllerAppClient::slotConnectToManager( const QString& address, int protocol )
{
  ManagerData m;
  if ( managers_.contains(address) ) {
    m = managers_[address];
  }

  if ( nullptr != m.channel && m.channel->isConnected() ) {
    return;
  }

  info_log << QObject::tr("Подключение к %1 ...").arg(address);

  delete m.channel; m.channel = nullptr;
  if ( nullptr == m.channel ) {
    if ( rpc::Transport::Novost == protocol ) {
      m.channel = rpc::Channel::connect_v1(address);
    }
    else {
      m.channel = rpc::Channel::connect(address);
    }
    managers_[address] = m;
  }

  if ( nullptr != m.channel ) {
    m.protocol = m.channel->protocol();
    emit managerConnected(address);
  }
  else {
    emit managerConnectionFail(address);
    return;
  }

  connect( m.channel, SIGNAL(disconnected()), SLOT(slotDisconnected()) );

  app::AppState* resp = m.channel->remoteCall(&app::ControlService::GetProcList, app::Dummy(), 60000);
  if ( nullptr == resp ) {
    error_msg << QObject::tr("Не удалось получить список запущенных процессов.");
    return;
  }
  for ( auto proc : resp->procs() ) {
    m.procidlist.append( QString("%1@%2") 
      .arg(address)
      .arg( proc.id() ) );
  }

  emit procListChanged(m.channel->address(), *resp);

//  m.proc = new app::AppState::Proc;
  bool res = false;
//  if ( rpc::Transport::Novost== m.channel->protocol() ) {
//    res = m.channel->subscribe(
//          &app::old::ControlService::ProcChanged,app::Dummy(),this,&ControllerAppClient::procStateChanged
//          );
//  }
//  else {
    res = m.channel->subscribe(
          &app::ControlService::ProcChanged,app::Dummy(),this,&ControllerAppClient::procStateChanged
          );
//  }

  if ( false == res ) {
    error_log << QObject::tr("Не удалось подписаться на изменения запущенных программ");
  }

  for ( int i=0,isz=resp->procs_size(); i<isz; ++i ) {
    int64_t id = resp->procs(i).id();
    app::AppOutRequest r;
    r.set_id(id);
 //   rpc::Controller* c = new rpc::Controller(m.channel);
//    app::AppOutReply* rep = new app::AppOutReply;
//    m.logControl.insert(id, c);
//    m.logReply.insert(id, rep);
    bool res = m.channel->subscribe(
          &app::ControlService::GetAppOut,r,this,&ControllerAppClient::logReceived
          );

    if ( false == res ) {
      error_log << QObject::tr("Не удалось подписаться на получение логов");
    }
  }

  managers_[address] = m;
}

void ControllerAppClient::slotReconnectToManager(const QString& managerId)
{
  QString addr;
  foreach ( const QString& a, managers_.keys() ) {
    if ( a == managerId ) {
      addr = a;
      slotConnectToManager( addr, managers_[managerId].protocol );
      return;
    }
  }
}

void ControllerAppClient::slotDisconnectFromManager(const QString& managerId)
{
  debug_log << tr("Отключение от %1 ...").arg(managerId);

  ManagerData m;
  QString addr;
  foreach ( const QString& a, managers_.keys() ) {
    if ( a == managerId ) {
      addr = a;
      m = managers_[a];
      break;
    }
  }

  if ( 0 == m.channel ) {
    return;
  }

  if ( m.channel->isConnected() ) {
    m.channel->disconnect();
  }

//  foreach ( int64_t id, m.logControl.keys() ) {
//    m.logControl[id]->deleteLater();
//    delete m.logReply[id];
//  }
//  m.control->deleteLater();
  m.channel->deleteLater();
//  delete m.proc;

  managers_.remove(addr);

  emit managerDisconnected(managerId);
}

void ControllerAppClient::slotStartProc(const app::AppState_Proc& id, const QString& managerId)
{
  debug_log << tr("Запуск процесса '%1' на %2").arg(id.app().id().c_str()).arg(managerId);

  ManagerData m;
  foreach ( const QString& a, managers_.keys() ) {
    if ( a == managerId ) {
      m = managers_[a];
      break;
    }
  }

  if ( nullptr == m.channel ) {
    return;
  }
  rpc::Channel* channel = nullptr;
  if ( rpc::Transport::Novost == m.channel->protocol() ) {
    channel = rpc::Channel::connect_v1(m.channel->address());
  }
  else {
    channel = rpc::Channel::connect(m.channel->address());
  }
  if ( 0 == channel ) {
    error_msg << tr("Не удалось отправить запрос на запуск процесса.");
    return;
  }
//  rpc::Controller control(channel);
  app::Dummy* resp = channel->remoteCall(&app::ControlService::StartProc, id, 60000);
  channel->disconnect();
  delete channel;
  if ( 0 == resp ) {
    error_msg << QObject::tr("Ответ на запрос StartProc не получен.");
    return;
  }
  delete resp;
}

void ControllerAppClient::slotStopProc(const app::AppState_Proc& id, const QString& managerId)
{
  debug_log << tr("Остановка процесса '%1' на %2").arg(id.app().id().c_str()).arg(managerId);

  ManagerData m;
  foreach ( const QString& a, managers_.keys() ) {
    if ( a == managerId ) {
      m = managers_[a];
      break;
    }
  }

  if ( 0 == m.channel ) {
    return;
  }
  rpc::Channel* channel = nullptr;
  if ( rpc::Transport::Novost == m.channel->protocol() ) {
    channel = rpc::Channel::connect_v1(m.channel->address());
  }
  else {
    channel = rpc::Channel::connect(m.channel->address());
  }
  if ( nullptr == channel ) {
    error_msg << QObject::tr("Не могу выполнить удаленный вызов ControlService::StopProc");
    return;
  }
  app::Dummy* resp = channel->remoteCall(&app::ControlService::StopProc, id, 60000);
  channel->disconnect();
  delete channel;
  if ( 0 == resp ) {
    error_msg << QObject::tr("Ответ на запрос StopProc не получен.");
    return;
  }
  delete resp;
}

void ControllerAppClient::slotDisconnected()
{
  rpc::Channel* ch = qobject_cast<rpc::Channel*>(sender());
  if ( 0 == ch ) {
    return;
  }

  debug_log << tr("Разрыв связи с %1").arg(ch->address());

  QString id = ch->address();
  emit managerConnectionLost(id);
}

void ControllerAppClient::procStateChanged(app::AppState::Proc* proc)
{
//  static QHash<QString,int> recv;
//  recv[proc->app().id().c_str()] += proc->ByteSize();
//  var(recv);
  for( auto it = managers_.begin(), end = managers_.end(); it != end; ++it ) {
    QString id = QString("%1@%2").arg( it.key() ).arg( proc->id() );
    if ( true == it.value().procidlist.contains(id) ) {
      emit procChanged( it.key(), *proc);
      return;
    }
  }
}

void ControllerAppClient::logReceived(app::AppOutReply* reply)
{
  foreach ( const QString& a, managers_.keys() ) {
    int64_t id = reply->id();    
    if ( -1 != id ) {
      emit logReceived(a, id, *reply);
    }
  }
}

} // meteo
