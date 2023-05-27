#include "ptkppstreamrecv.h"

#include <cross-commons/debug/tlog.h>

#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/rpc/transport.h>
#include <meteo/commons/proto/msgcenter.pb.h>
#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgparser/msgmetainfo.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/msgstream/plugins/ptkpp/ptkppstreamservice.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

namespace meteo {

PtkppStreamRecv::PtkppStreamRecv(QObject* p) :
  StreamModule(p)
{
}

PtkppStreamRecv::~PtkppStreamRecv()
{
  delete server_;
  server_ = nullptr;

  delete service_;
  service_ = nullptr;

  for ( ConnectionItem c : connections_ ) {
    delete c.channel;
    delete c.message;
  }
}

void PtkppStreamRecv::setOptions(const msgstream::Options& options)
{
  opt_ = options;

  var(opt_.Utf8DebugString());

  for ( int i = 0, isz = opt_.ptkpp_recv().subscribe_size(); i < isz; ++i ) {
    const msgstream::SubscribeData& sub = opt_.ptkpp_recv().subscribe(i);

    ConnectionItem c;
    c.message = new tlg::MessageNew;
    c.subscribe.CopyFrom(sub);
    connections_.insert(c.message, c);
  }
}

QList<msgstream::SubscribeData> PtkppStreamRecv::subscribeList() const
{
  QList<msgstream::SubscribeData> list;
  for ( int i = 0, isz = opt_.ptkpp_recv().subscribe_size(); i < isz; ++i ) {
    list << opt_.ptkpp_recv().subscribe(i);
  }
  return list;
}

msgstream::SubscribeData PtkppStreamRecv::startSubscribe(const msgstream::SubscribeData& subscribe)
{  
  int idx = -1;
  for ( int i = 0, isz = opt_.ptkpp_recv().subscribe_size(); i < isz; ++i ) {
    if ( opt_.ptkpp_recv().subscribe(i).params().uid() == subscribe.params().uid() ) {
      idx = i;
      break;
    }
  }

  var(subscribe.Utf8DebugString());

  msgstream::SubscribeData s = subscribe;

  if ( -1 == idx ) {
    opt_.mutable_ptkpp_recv()->add_subscribe()->CopyFrom(subscribe);

    ConnectionItem c;
    c.subscribe.CopyFrom(s);
    c.message = new tlg::MessageNew;
    connections_.insert(c.message, c);
  }
  else {
    opt_.mutable_ptkpp_recv()->mutable_subscribe(idx)->CopyFrom(subscribe);

    QMutableMapIterator<tlg::MessageNew*,ConnectionItem> it(connections_);
    while ( it.hasNext() ) {
      it.next();

      if ( it.value().subscribe.params().uid() == s.params().uid() ) {
        it.value().subscribe.CopyFrom(s);
        break;
      }
    }
  }

  gSettings()->setMsgstreamConf(opt_);
  if ( !gSettings()->save() ) {
    s.add_error_text(tr("Не удалось сохранить параметры.").toStdString());
  }

  return s;
}

msgstream::SubscribeData PtkppStreamRecv::stopSubscribe(const msgstream::SubscribeData& subscribe)
{
  int idx = -1;
  for ( int i = 0, isz = opt_.ptkpp_recv().subscribe_size(); i < isz; ++i ) {
    if ( opt_.ptkpp_recv().subscribe(i).params().uid() == subscribe.params().uid() ) {
      idx = i;
      break;
    }
  }

  msgstream::SubscribeData* s = nullptr;

  if ( -1 == idx ) {
    s = opt_.mutable_ptkpp_recv()->add_subscribe();
  }
  else {
    s = opt_.mutable_ptkpp_recv()->mutable_subscribe(idx);
  }

  s->CopyFrom(subscribe);

  msgstream::SubscribeData res = *s;
  gSettings()->setMsgstreamConf(opt_);
  if ( !gSettings()->save() ) {
    res.add_error_text(tr("Не удалось сохранить параметры.").toStdString());
  }

  QMutableMapIterator<tlg::MessageNew*,ConnectionItem> it(connections_);
  while ( it.hasNext() ) {
    it.next();

    if ( it.value().subscribe.params().uid() != s->params().uid() ) {
      continue;
    }

    it.value().subscribe.CopyFrom(*s);

    it.value().channel->disconnect(this, 0);
    it.value().channel->deleteLater();
    it.value().channel = nullptr;

    info_log << tr("Приём телеграмм от %1 остановле").arg(QString::fromStdString(s->address()));
    break;
  }

  return res;
}

msgstream::SubscribeData PtkppStreamRecv::removeSubscribe(const msgstream::SubscribeData& subscribe)
{
  msgstream::SubscribeData res = subscribe;

  for ( int i = 0, isz = opt_.ptkpp_recv().subscribe_size(); i < isz; ++i ) {
    if ( opt_.ptkpp_recv().subscribe(i).params().uid() == subscribe.params().uid() ) {
      opt_.mutable_ptkpp_recv()->mutable_subscribe()->SwapElements(i, isz - 1);
      opt_.mutable_ptkpp_recv()->mutable_subscribe()->RemoveLast();
      res.set_action(msgstream::SubscribeData::kNoneAction);

      gSettings()->setMsgstreamConf(opt_);
      if ( !gSettings()->save() ) {
        res.add_error_text(tr("Не удалось сохранить параметры.").toStdString());
      }

      QMutableMapIterator<tlg::MessageNew*,ConnectionItem> it(connections_);
      while ( it.hasNext() ) {
        it.next();

        if ( it.value().subscribe.params().uid() != subscribe.params().uid() ) {
          continue;
        }

        it.value().channel->disconnect(this, 0);
        it.value().channel->deleteLater();
        it.value().channel = nullptr;

        delete it.value().message;

        it.remove();

        break;
      }

      break;
    }
  }

  return res;
}

bool PtkppStreamRecv::slotInit()
{
  if ( nullptr == incoming_ ) { return false; }

  status_->setTitle(kRecvCount, tr("Принято сообщений"));
  status_->setParam(kRecvCount, 0);

  if ( nullptr != service_ ) {
    delete service_;
    service_ = nullptr;
  }

  service_ = new PtkppStreamService;
  service_->setReceiver(this);

  bool ok = false;
  int port;
  QString host;
  QString addr = meteo::Global::instance()->serviceAddress( meteo::settings::proto::kNodeCenter, &ok );
  if ( !ok || !rpc::TcpSocket::parseAddress(addr,&host,&port) ) {
    error_log << QObject::tr("Не задан порт для создания сервера");
    delete service_;
    service_ = nullptr;
    return false;
  }

  if ( nullptr != server_ ) {
    server_->deleteLater();
    server_ = nullptr;
  }

  server_ = new rpc::Server(service_, QString("0.0.0.0:%1").arg(port));
  if ( nullptr == server_ ) {
    error_log << QObject::tr("Не удалось создать сервер (порт %1)").arg(port);
    return false;
  }
  if ( false == server_->init() ) {
    error_log << QObject::tr("Не удалось инициализировать сервер (порт %1)").arg(port);
    return false;
  }

  info_log << tr("сервис запущен на порту %1").arg(port);

  QTimer* timer = new QTimer(this);
  connect( timer, &QTimer::timeout, this, &PtkppStreamRecv::slotConnect );
  timer->start(3000);

  slotConnect();

  return true;
}

void PtkppStreamRecv::slotConnect()
{
  if ( runConnect_ ) { return; }
  runConnect_ = true;

  QMutableMapIterator<tlg::MessageNew*,ConnectionItem> it(connections_);
  while ( it.hasNext() ) {
    it.next();

    if ( nullptr != it.value().channel || (it.value().dt.isValid() && it.value().dt.secsTo(QDateTime::currentDateTime()) < 60) ) {
      continue;
    }

    if ( it.value().subscribe.action() != msgstream::SubscribeData::kRunAction ) {
      continue;
    }

    it.value().dt = QDateTime::currentDateTime();

    QString name;
    QString addr;
    if ( false == parsePeerInfo(QString::fromStdString(it.value().subscribe.address()), &name, &addr) ) {
      error_log << tr("Не удалось получить адрес подключения к ЦКС");
      var(it.value().subscribe.Utf8DebugString());
      continue;
    }

    rpc::Channel* channel = rpc::Channel::connect(addr);
    if ( nullptr == channel ) {
      error_log << tr("Не удалось подключиться к %1").arg(addr);
      var(it.value().subscribe.Utf8DebugString());
      continue;
    }

    it.value().channel = channel;

    it.value().channel->subscribe(&msgcenter::MsgCenterService::DistributeMsg,
                                   it.value().subscribe.params(),
                                   this,
                                   &PtkppStreamRecv::callbackMessage);
    connect( it.value().channel, &rpc::Channel::disconnected, this, &PtkppStreamRecv::slotDisconnected );

    info_log << tr("Оформлена подписка к %1").arg(addr);
  }

  runConnect_ = false;
}

void PtkppStreamRecv::slotDisconnected()
{
  rpc::Channel* channel = qobject_cast<rpc::Channel*>(sender());
  if ( nullptr == channel ) {
    return;
  }

  info_log << tr("Разрыв связи с %1").arg(channel->address());

  QMutableMapIterator<tlg::MessageNew*,ConnectionItem> it(connections_);
  while ( it.hasNext() ) {
    it.next();

    if ( it.value().channel == channel ) {
      delete it.value().channel;
      it.value().channel = nullptr;
      break;
    }
  }
}

void PtkppStreamRecv::callbackMessage(tlg::MessageNew* message)
{
//  if ( connections_.contains(message) ) {
//    message->mutable_metainfo()->set_from(connections_[message].subscribe.address());
    message->mutable_metainfo()->set_from("ptkpp");
    incoming_->append(*message);
//  }
}

bool PtkppStreamRecv::parsePeerInfo(const QString& text, QString* name, QString* address) const
{
  *name = text.section(":",0,0);
  *address = text.section(":",1,2);

  return ( !address->isEmpty() );
}

} // meteo
