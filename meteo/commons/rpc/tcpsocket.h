#ifndef METEO_COMMONS_RPC_TCPSOCKET_H
#define METEO_COMMONS_RPC_TCPSOCKET_H

#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qtcpsocket.h>
#include <qtimer.h>

namespace meteo {
namespace rpc {

class MethodPack;
class Parser;

static const QByteArray kSrvident("qwertypod");

class TcpSocket : public QThread
{
  Q_OBJECT
  public:
    enum Status {
      kConnecting       = 0,
      kSizeHandshing    = 1,
      kIdentHandshaking = 2,
      kHandshaking      = 3,
      kConnected        = 4,
      kUnknown          = 5
    };
    explicit TcpSocket( const QString& addr );
    explicit TcpSocket( const QString& addr, const QByteArray& uuid );
    explicit TcpSocket( int32_t descr );
    virtual ~TcpSocket();

    TcpSocket* connect();

    Status status() const ;
    void setStatus( Status st );

    const QByteArray& uuid() const { return uuid_; }

    static bool parseAddress( const QString& address, QString* host, int32_t* port );
    static bool parseAddress( const std::string& address, QString* host, int32_t* port );
    static QString stringAddress( const QString& host, int32_t port );
    const QString& address() const { return address_; }

    bool getMessage( MethodPack* pack );
    bool sendMessage( const MethodPack* pack);

    bool waitForAnswer( int32_t msecs_timeout, const QByteArray& callid );
    void setWaitId( const QByteArray& id );
    void resetWaitId();

    void quitThread();

    uint8_t maclevel() const { return maclevel_; }

  protected:
    enum Mode {
      kServer           = 0,
      kClientSender     = 1,
      kClientReceiver   = 2
    };

  protected:
    virtual bool sendHandshakeInfo();
    virtual bool parseSizeHandshake();
    virtual bool parseIdentHandshake();
    virtual bool parseHandshake();
    virtual bool parseRpcMessages();
    virtual bool writeMessage( const MethodPack& pack );
    virtual bool startRecvThread();

  protected:
    int32_t uuidsize_ = -1;
    Parser* parser_ = nullptr;
    QTcpSocket* sock_ = nullptr;
    QByteArray uuid_;
    QString address_;
    qint64 totalwritten_ = 0;
    QByteArray recvbuf_;
    Mode mode_ = kClientSender;
    TcpSocket* recvsock_ = nullptr;

  private:
    Status status_ = kUnknown;
    QMutex mutex_;
    QMutex synchromutex_;
    QWaitCondition synchrocond_;
    int32_t sockdescr_ = -1;
    QList<MethodPack> packlist_;        //!< список полученных/подготовленных для отправки rpc-методов
    QByteArray waitid_;
    QTimer* tmwake_ = nullptr;
    uint8_t maclevel_ = 0;

  private:
    void run();
    bool runServer();
    bool runClient();

    void parseArray();

  protected:
    Q_SLOT void slotSockConnected();
    Q_SLOT void slotSockDisconnected();
    Q_SLOT void slotSockError( QAbstractSocket::SocketError err );
    Q_SLOT void slotSockStateChanged( QAbstractSocket::SocketState st );
    Q_SLOT void slotReadyRead();
    Q_SLOT void slotMessageAdded();
    Q_SLOT void slotBytesWritten( qint64 bytes );

    Q_SLOT void slotStartWakingUntilRecv();
    Q_SLOT void slotStopWakingUntilRecv();
    Q_SLOT void slotWakeClient();
  public:
    Q_SIGNAL void connected( TcpSocket* sock );
    Q_SIGNAL void disconnected( TcpSocket* sock );
    Q_SIGNAL void messageAdded();
    Q_SIGNAL void messageReceived();

    Q_SIGNAL void startWakingUntilRecv();
    Q_SIGNAL void stopWakingUntilRecv();
};

}
}

#endif
