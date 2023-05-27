#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UnimasBase_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UnimasBase_H

#include "unimas.h"

#include <qobject.h>
#include <QtNetwork/qtcpsocket.h>

#include <commons/container/ts_list.h>
#include <meteo/commons/global/appstatusthread.h>
#include <meteo/commons/proto/msgstream.pb.h>

class QTimer;
class QTcpSocket;

namespace meteo {
  class TlgParser;

  class TlgBox
  {
  public:
    enum State { kPrepare,kSending,kWaitAck };

  public:


    TlgBox(TServicePacket::Mode m = TServicePacket::TCP) :
      head( m ),
       end( m ),
       ack( m )
    {
      state = kPrepare;
      crc = 0;
    }
    ~TlgBox() {}

    TServicePacket head;
    TServicePacket end;
    TServicePacket ack;

    QByteArray data;

    quint16 crc;
    QByteArray md5;

    tlg::MessageNew orig;

    State state;
    QDateTime sentDt;
  };

//! Обеспечивает информационный обмен с MTS-,Unimas-сервером.
class UnimasBase : public QObject
{
  Q_OBJECT

public:

  enum RecvState { kStart, kInfo, kEnd, kAck,kRR, kError };
  enum Mode { MODE_TCP, MODE_UTCP, };
public:
  UnimasBase(const msgstream::Options& opt,
             TSList<tlg::MessageNew>* outgoing,
             TSList<tlg::MessageNew>* incoming,
             AppStatusThread* status, QObject *p);
  //TSList<QByteArray>* storage,

  virtual ~UnimasBase();
  virtual void stop()=0;

protected:
  void deleteSocket();
  void tryResendMsg();
  void initTimers();
  void closeSession();
  void clearData();
  bool sendRR();

private slots:
  virtual void slotReceiveTimeout()=0;
  virtual void slotReconnectTimeout()=0;
  void slotSocketError(QAbstractSocket::SocketError);
  void slotReadyRead();
  void slotNewMessage();
  void slotResendTimeout();
  void slotRRTimeout();

private:
  TlgBox prepareMessage(const tlg::MessageNew& msg)const;
  bool parseRaw(const QByteArray& );

  void sendProcess();
  bool receiveProcess();
  bool sendTlg(TlgBox );
  bool writeTlg(TlgBox );
 // bool readAck();
  bool processAck();

  bool checkSocket() const;
  void readProcess();

  bool getPacket();
  bool getInfoPacket();
  bool writeEndPacket(const TServicePacket &end);

  void appendData(const QByteArray& );
  void removeData(int );

protected:
  QString host_;
  int     port_;
  RecvState       state_;
  QTcpSocket* socket_=nullptr;
  AppStatusThread* status_=nullptr;

  TServicePacket    headPacket_;
  TServicePacket    endPacket_;
  QByteArray  raw_;

  int receiveTimeout_ = 1;
  int reconnectTimeout_ = 1;
  int resendTimeout_ = 1;
  int rr_timeout_ = 1;

  QTimer* work_timer_=nullptr;
  QTimer* reconnectTimer_=nullptr;
  QTimer* resend_timer_=nullptr;
  QTimer* rr_timer_=nullptr;

  TSList<tlg::MessageNew>*  outgoing_=nullptr;
  TSList<tlg::MessageNew>*  incoming_=nullptr;
  QMap<quint16, TlgBox> sentList_;
  TlgParser* parser_ = nullptr;
  bool isWriten = true;

private:

  int    compressionMode_;
  int    nReceived_;
  bool   isTcpMode_;
  int    sendWithoutAck_;

  QString stream_id_;
signals:
  void signalBytesRecv(qint64);
  void signalMessagesRecv(qint64);
  void signalBytesSended(qint64);
  void signalMessagesSended(qint64);
  void signalConfirmSended(qint64);
  void signalConfirmRecv(qint64);
};

} // meteo


#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_SOCKETSPECIAL_UnimasBase_H
