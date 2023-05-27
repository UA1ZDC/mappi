#ifndef METEO_COMMONS_SERVICES_MSGCENTER_ROUTER_H
#define METEO_COMMONS_SERVICES_MSGCENTER_ROUTER_H

#include <qobject.h>
#include <qmap.h>
#include <qpair.h>
#include <qmutex.h>

#include <sql/dbi/gridfs.h>
#include <meteo/commons/proto/msgcenter.pb.h>
#include <meteo/commons/msgdata/routetable.h>
#include <meteo/commons/msgdata/msgqueue.h>

namespace meteo {

class Dbi;
class AppStatusThread;

using ReqResp = QPair< const msgcenter::DistributeMsgRequest*, tlg::MessageNew* >;

namespace rpc {
class Controller;
}

class Router : public QObject
{
  Q_OBJECT
  public:
    Router( AppStatusThread* status, const QString& wmoid, const QString& hmsid );
    ~Router();

    void unsubscribeClient( rpc::Controller* ctrl );
    void subscribeClient( rpc::Controller* ctrl, const ReqResp& request );

  public slots:
    void slotNewMessage( tlg::MessageNew tlg );
    void slotThreadStarted();

  private:
    void routeMessage( tlg::MessageNew* tlg );
    void routeToSubs( const QStringList& recuids, tlg::MessageNew* tlg, QMap< rpc::Controller*, QList<ReqResp> >* subs );
    bool saveToDb( tlg::MessageNew* tlg );
    int64_t getLastTlgId();
    QString generateFileName( const tlg::MessageNew& msg, const MsgInfo& info ) const ;
    void setUID( Rules* rules, const QString& uid );
    void setMetric( const tlg::MessageNew& msg );

    QStringList messageReceivers( tlg::MessageNew* msg );

    bool checDb();

  private:
    int64_t lastid_;
    QStringList selfids_;
    GridFs* gridfs_ = nullptr;
    Dbi* db_ = nullptr;
    QMap< QString, QMap< rpc::Controller*, QList<ReqResp> > > subs_;
    RouteTable table_;
    QMap< QString, Rules > filters_;
    QMap< rpc::Controller*, QString > subuids_;
    QMutex mutex_;
    AppStatusThread* status_;
    int64_t dup_ = 0;
    QMap<std::string,int> paramId_;
    QMap<std::string,qint64> count_;
    qint64 writeToDb_ = 0;
    int skipCount_ = 0;
    QMap<QString,int> paramIdSend_;
    QMap<QString,qint64> countSend_;
};

}

#endif
