#include "msgcenter.h"

#include <qelapsedtimer.h>
#include <qcoreapplication.h>

#include <cross-commons/debug/tlog.h>

#include <commons/textproto/pbtools.h>

#include <meteo/commons/global/log.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/appstatusthread.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgparser/tlgfuncs.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/proto/msgparser.pb.h>

#include "router.h"

namespace meteo {

MsgCenter::MsgCenter( AppStatusThread* status )
  : status_(status),
  methodbag_(this)
{
  hmsId_ = gSettings()->hmsId();
  wmoId_ = gSettings()->wmoId();
  router_ = new Router( status_, wmoId_, hmsId_ );
  router_->moveToThread(&routerthread_);
  QObject::connect( &routerthread_, &QThread::started, router_, &Router::slotThreadStarted );
  QObject::connect( this, &MsgCenter::newMessage, router_, &Router::slotNewMessage );
  routerthread_.start();
  qRegisterMetaType<ReqResp>("ReqResp");
  qRegisterMetaType<tlg::MessageNew>("tlg::MessageNew");
}

MsgCenter::~MsgCenter()
{
  routerthread_.quit();
  routerthread_.wait();
  delete router_;
  router_ = nullptr;
}

void MsgCenter::ProcessMsg(google::protobuf::RpcController* controller,
                               const msgcenter::ProcessMsgRequest* request,
                                       msgcenter::ProcessMsgReply* response,
                                        google::protobuf::Closure* done)
{
  Q_UNUSED(controller);
  Q_UNUSED(response);
  emit newMessage( request->msg() );
  done->Run();
}

void MsgCenter::DistributeMsg(google::protobuf::RpcController* controller,
                               const msgcenter::DistributeMsgRequest* request,
                                                     tlg::MessageNew* response,
                                           google::protobuf::Closure* done)
{
  Q_UNUSED(done);
  rpc::Controller* ctrl = static_cast<rpc::Controller*>(controller );
  QObject::connect( ctrl->channel(), &rpc::Channel::clientUnsubscribed,
                    this, &MsgCenter::slotClientUnsubscribed, Qt::DirectConnection );
  router_->subscribeClient( ctrl, qMakePair( request, response ) );
}

void MsgCenter::GetUndecodedFaxTelegram( google::protobuf::RpcController* c,
                  const msgcenter::GetUndecodedFaxTelegramRequest* req,
                  msgcenter::GetTelegramResponse* resp,
                  google::protobuf::Closure* d )
{
  Q_UNUSED(c);
  Q_UNUSED(d);
  Q_UNUSED(req);
  Q_UNUSED(resp);
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, &methodbag_, &MethodBag::GetUndecodedFaxTelegram, req, resp );
}

void MsgCenter::GetTelegram(google::protobuf::RpcController* c,
                                   const msgcenter::GetTelegramRequest* req,
                                   msgcenter::GetTelegramResponse* resp,
                                   google::protobuf::Closure* d)
{
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, &methodbag_, &MethodBag::GetTelegram, req, resp );
}
void MsgCenter::GetReport(google::protobuf::RpcController* c,
			  const msgcenter::GetReportRequest* req,
			  msgcenter::GetReportResponse* resp,
			  google::protobuf::Closure* d)
{
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    d->Run();
    return;
  }
  handler->runMethod( c, &methodbag_, &MethodBag::GetReport, req, resp );
}

void MsgCenter::SetDecode( google::protobuf::RpcController* c,
                           const msgcenter::DecodeRequest* req,
                           msgcenter::Dummy* resp,
                           google::protobuf::Closure* d )
{
  QString error;
  rpc::ClientHandler* handler = rpc::Channel::handlerFromController(c);
  if ( nullptr == handler ) {
    error = QObject::tr("Не удалось получить обработчик из контроллера");
    error_log << error;
    d->Run();
    return;
  }
  handler->runMethod( c, &methodbag_, &MethodBag::SetDecode, req, resp );
}
 
bool MsgCenter::prepareTlg( const QString& yygggg, const QString& webtlg, tlg::MessageNew* tlg ) const
{
  QByteArray raw;
  QTextCodec* codec = QTextCodec::codecForName( "UTF-8" );
  raw += codec->fromUnicode(QString(TlgTextObject::kSpecChars["<SOH>"]));
  raw += QByteArray("027 5027/");
  raw += codec->fromUnicode(QString(TlgTextObject::kSpecChars["<STX>"]));
  raw += codec->fromUnicode(QString(TlgTextObject::kSpecChars["<CR>"]));
  raw += codec->fromUnicode(QString(TlgTextObject::kSpecChars["<CR>"]));
  raw += codec->fromUnicode(QString(TlgTextObject::kSpecChars["<LF>"]));
  raw += codec->fromUnicode(QString(TlgTextObject::kSpecChars["<SO>"]));
  raw += QString("obmn91 %1 %2").arg(global::wmoId())
                              .arg(yygggg);
  if ( true == tlg->header().has_bbb() ) {
    raw += codec->fromUnicode( QString(" %1").arg( QString::fromStdString( tlg->header().bbb() ) ) );
  }
  raw += codec->fromUnicode(QString(TlgTextObject::kSpecChars["<CR>"]));
  raw += codec->fromUnicode(QString(TlgTextObject::kSpecChars["<CR>"]));
  raw += codec->fromUnicode(QString(TlgTextObject::kSpecChars["<LF>"]));
  raw += webtlg.trimmed().toUtf8();
  raw += codec->fromUnicode(QString(TlgTextObject::kSpecChars["<LF>"]));
  raw += codec->fromUnicode(QString(TlgTextObject::kSpecChars["<CR>"]));
  raw += codec->fromUnicode(QString(TlgTextObject::kSpecChars["<CR>"]));
  raw += codec->fromUnicode(QString(TlgTextObject::kSpecChars["<LF>"]));
  raw += codec->fromUnicode(QString(TlgTextObject::kSpecChars["<ETX>"]));
  
 
  TlgParser parser(raw);
 
  return parser.parseNextMessage(tlg);
}

void MsgCenter::SavePogodaInput(google::protobuf::RpcController* controller,
                                    const meteo::msgcenter::SavePogodaRequest* request,
                                    meteo::msgcenter::CreateTelegramReply *response,
                                    google::protobuf::Closure *done)
{
  Q_UNUSED(controller);
  QString msg="";
  QString onestr="";

  QMap< int32_t, QList<msgcenter::MeteoDataStation> > md_with_correction;

  for ( auto weather : request->weather() ) {
    md_with_correction[ weather.cor_number() ].append(weather);
  }
  response->set_result(true);
  done->Run();

  for ( auto it = md_with_correction.begin(), end = md_with_correction.end(); it != end; ++it ) {
    auto correction = it.key();
    auto weatherlist = it.value();
    if ( 0 == weatherlist.size() ) {
      continue;
    }
    for ( auto weather : weatherlist ) {
      QString name = QString::fromStdString(weather.name()).replace(" ", "." );
      onestr = QString("%1 %2.%3 %4 %5 %6 %7 %8 %9 %10 %11 %12")
                .arg( meteo::global::translitNameFuncKoi7(name) )
                .arg( QString::fromStdString(weather.hour()) )
                .arg( QString::fromStdString(weather.minute()) )
                .arg( !weather.has_n()  || weather.n()==""     ? "," : (QString::fromStdString(weather.n()) )  )
                .arg( !weather.has_cl() || weather.cl()==""    ? "," : (QString::fromStdString(weather.cl()).toUpper() )   )
                .arg( !weather.has_h()  || weather.h()==""     ? "," : (QString::fromStdString(weather.h()) )  )
                .arg( !weather.has_ww() || weather.ww()==""    ? "," : ( meteo::global::translitNameFuncKoi7(QString::fromStdString(weather.ww()) ) ) )
                .arg( !weather.has_v()  || weather.v()==""     ? "," : (QString::fromStdString(weather.v()) )  )
                .arg( !weather.has_dd() || weather.dd()==""    ? "," : (QString::fromStdString(weather.dd()) )   )
                .arg( !weather.has_ff() || weather.ff()==""    ? "," : (QString::fromStdString(weather.ff()) )   )
                .arg( !weather.has_t()  || weather.t()==""     ? "," : (QString::fromStdString(weather.t()) )  )
                .arg( !weather.has_u()  || weather.u()==""     ? "," : (QString::fromStdString(weather.u()) )  );
      msg+=onestr+"=\n";
    }
    // запрос
    tlg::MessageNew req;
    tlg::Header* header = req.mutable_header();

    header->set_t1( "o" );
    header->set_t2( "b" );
    header->set_a1( "m" );
    header->set_a2( "n" );
    if ( 0 < correction ) {
      header->set_bbb( global::cornumber2bbb( correction ).toStdString() );
    }

    QDateTime dt = QDateTime::currentDateTime();
    
    // ВМО айди
    header->set_cccc( meteo::global::wmoId().toStdString() );
    header->set_yygggg( dt.toString("ddhhmm").toStdString() );

    req.set_format( tlg::kGMS );
    req.set_isbinary(false);
    req.set_msgtype("alphanum");

    req.set_msg( msg.toStdString() );

    req.set_end( "\003" );

    if ( true == prepareTlg( dt.toString("ddhhmm"), msg , &req) ) {
      debug_log<<"OK";
    }
    else {
      debug_log << tr("Структура сообщения некорректна.");
    }
    req.mutable_metainfo()->set_converted_dt( dt.toString("yyyy-MM-dd hh:mm:ss").toStdString() );
    emit newMessage(req);
  }
}

void MsgCenter::CreateTelegram(google::protobuf::RpcController *controller,
                                                const tlg::MessageNew *request,
                                       msgcenter::CreateTelegramReply *response,
                                            google::protobuf::Closure *done)
{
  Q_UNUSED(controller);
  response->set_result(true);
  emit newMessage( *request );
  done->Run();
}

void MsgCenter::sendMessage(const tlg::MessageNew& tlg )
{
  emit newMessage(tlg);

}

void MsgCenter::slotClientUnsubscribed( rpc::Controller* ctrl )
{
  router_->unsubscribeClient(ctrl);
}


}
