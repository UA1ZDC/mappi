#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_socksp2g_BASE_TRANSMETSTREAM_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_socksp2g_BASE_TRANSMETSTREAM_H

#include <commons/container/ts_list.h>

#include "socksp2g.h"
#include <meteo/commons/msgstream/streammodule.h>

namespace meteo {

class Socksp2gBase: public QObject
{
  Q_OBJECT
public:
  static QString moduleId() { return QString("socksp2g"); }

public:
  explicit Socksp2gBase(const msgstream::Options &opt,
                       TSList<tlg::MessageNew>* outgoing,
                       TSList<tlg::MessageNew>* incoming,
                       AppStatusThread* status,
                       QObject* p = nullptr);
  virtual ~Socksp2gBase();
  virtual void stop()=0;

protected slots:
  void slotReadyRead();
  void slotSocketError(QAbstractSocket::SocketError error);
  void slotNewMessage();
  void slotResendTimeout();
  void slotSendRR();
  virtual void slotStart()=0;
  virtual void slotReceiveTimeout()=0;
  virtual void slotReconnectTimeout()=0;

protected:
  bool checkSocket() const;
  void tryResendMsg() ;
  void deleteSocket();
  void deleteTimers();
  void setMagic();
  void initTimers();

  void clearData();
  void appendData(const QByteArray& ) ;
  void removeData(int );

private:
  bool receiveProcess();
  void sendProcess();
  void readyRead();
  void setAckWaiting(int );

  void setSendWindow(int n) { sendWithoutAck_ = n; }
  void setOptimCompress(bool compress) { compress_ = compress; }
  bool optimCompress() const { return compress_; }
  void setHost(const QString& host) { host_ = host; }
  void setPort(int port) { port_ = port; }
  virtual QString id() const { return id_; }
  void setId(const QString& id) { id_ = id; }

  bool getInfoPacket();
  bool getStartPacket();
  bool getEndPacket();
  bool processAck(const socksp2g::ServicePacket& );
  void writeBox(const socksp2g::SendBox &);
  bool writeEndPacket(const socksp2g::ServicePacket &);

signals:
  void signalBytesRecv(qint64);
  void signalMessagesRecv(qint64);
  void signalBytesSended(qint64);
  void signalMessagesSended(qint64);
  void signalConfirmSended(qint64);
  void signalConfirmRecv(qint64);


//private slots:


private:

  // параметры
  int sendWithoutAck_;
  int8_t magic_[5];
  bool compress_;
  QString id_;

  // данные

  QByteArray raw_;
  u_int32_t num_;

  // служебные
  socksp2g::ServicePacket start_;
  socksp2g::InfoPacket info_;
  //socksp2g::FilePacket file_;
  bool isWriten = true;

protected:
  socksp2g::RecvState state_;
  int port_;
  QString host_;
  QTcpSocket* socket_=nullptr;
  QMap<quint16,socksp2g::SendBox> sentList_;

  AppStatusThread* status_=nullptr;
  TSList<tlg::MessageNew>* incoming_=nullptr;
  TSList<tlg::MessageNew>*  outgoing_=nullptr;

  int receiveTimeout_ = 1;
  int reconnectTimeout_ = 1;
  int resendTimeout_ = 1;
  int rr_timeout_ = 1;
  QTimer* workTimer_=nullptr;
  QTimer* reconnectTimer_=nullptr;
  QTimer* resend_timer_=nullptr;
  QTimer* rr_timer_=nullptr;
  qint64 kChunk = 1024;

};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_SRIV512_TRANSMETSTREAM_H
