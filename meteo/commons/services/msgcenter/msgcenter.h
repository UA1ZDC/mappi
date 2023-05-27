#ifndef METEO_COMMONS_SERVICES_MSGCENTER_MSGCENTERSERVICE_H
#define METEO_COMMONS_SERVICES_MSGCENTER_MSGCENTERSERVICE_H

#include <qmap.h>
#include <qset.h>
#include <qhash.h>
#include <qstring.h>
#include <qthread.h>

#include <meteo/commons/proto/msgcenter.pb.h>
#include <meteo/commons/proto/msgparser.pb.h>

#include <meteo/commons/rpc/controller.h>

#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/msgparser/tlgfuncs.h>
#include <commons/funcs/tlgtextobject.h>

#include <meteo/commons/services/msgcenter/router.h>
#include <meteo/commons/services/msgcenter/methodbag.h>

namespace meteo {

class AppStatusThread;

class MsgCenter : public QObject, public msgcenter::MsgCenterService
{
  Q_OBJECT

  public:
    MsgCenter( AppStatusThread* status );
    ~MsgCenter();

    void ProcessMsg( google::protobuf::RpcController* controller,
                     const msgcenter::ProcessMsgRequest* request,
                     msgcenter::ProcessMsgReply* response,
                     google::protobuf::Closure* done );

    void DistributeMsg( google::protobuf::RpcController* controller,
                        const msgcenter::DistributeMsgRequest* request,
                        tlg::MessageNew* response,
                        google::protobuf::Closure* done );

    void GetTelegram( google::protobuf::RpcController* controller,
                      const msgcenter::GetTelegramRequest* request,
                      msgcenter::GetTelegramResponse* response,
                      google::protobuf::Closure* done );

    void GetUndecodedFaxTelegram( google::protobuf::RpcController* controller,
                      const msgcenter::GetUndecodedFaxTelegramRequest* request,
                      msgcenter::GetTelegramResponse* response,
                      google::protobuf::Closure* done );

    void GetReport( google::protobuf::RpcController* controller,
		    const msgcenter::GetReportRequest* request,
		    msgcenter::GetReportResponse* response,
		    google::protobuf::Closure* done );

    void SetDecode( google::protobuf::RpcController* controller,
                    const meteo::msgcenter::DecodeRequest* request,
                    meteo::msgcenter::Dummy* response,
                    google::protobuf::Closure* done );

    void CreateTelegram( google::protobuf::RpcController *controller,
                         const tlg::MessageNew *request,
                         msgcenter::CreateTelegramReply *response,
                         google::protobuf::Closure *done );

    // сохраняем погоду
    void SavePogodaInput( google::protobuf::RpcController* controller,
                          const meteo::msgcenter::SavePogodaRequest* request,
                          meteo::msgcenter::CreateTelegramReply *response,
                          google::protobuf::Closure *done );
    void sendMessage(const tlg::MessageNew& tlg);
  private:
    bool prepareTlg( const QString& yygggg, const QString& webtlg, tlg::MessageNew* tlg ) const ;

  private slots:
    void slotClientUnsubscribed( rpc::Controller* ctrl );


  private:
    // параметры
    QString wmoId_;
    QString hmsId_;
    Router* router_ = nullptr;
    QThread routerthread_;
    AppStatusThread* status_;
    MethodBag methodbag_;

signals:
  void newMessage( tlg::MessageNew tlg );
};

}

#endif
