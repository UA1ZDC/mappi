#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_UDPSTREAM_UdpStreamOut_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_UDPSTREAM_UdpStreamOut_H

#include <qdir.h>
#include <qudpsocket.h>

#include <commons/container/ts_list.h>
#include <meteo/commons/proto/msgstream.pb.h>
#include <meteo/commons/msgstream/streammodule.h>
#include <meteo/commons/proto/msgstream.pb.h>
#include "udp_packet.h"

namespace tlg {
class MessageNew;
} // tlg

namespace meteo {
class UdpPacketMap;
class TlgParser;

class UdpStreamOut : public StreamModule
{
  Q_OBJECT

  enum StatusParam {
    kSendCount    = 1024,
    kSendCountToday,
    kSendSize,
    kSendSizeToday,
    kOperation,
    kSendChunkCount,
    kSendFileCount
  };

public:
  static QString moduleId() { return QString("udp-send"); }

public:
  UdpStreamOut(QObject* p = 0);
  virtual ~UdpStreamOut();

  virtual void setOptions(const msgstream::Options &options);

  //! Устанавливает максимальное количество телеграмм, записываемых в один файл.
  //! При достижении максимального значения будет создан новый файл.
  //! По умолчаню равно 0 - без ограничений.
  void setMaxMessagesPerFile(int n)       { msg_limit_ = n; }
  //! Устанавливает максимальный объём данных, записываемых в один файл.
  //! При достижении максимального значения будет создан новый файл.
  //! По умолчаню равно 0 - без ограничений.
  void setMaxSizePerFile(qint64 n)        { size_limit_ = n; }
  //! Устанавливает максимальное время записи в файл.
  //! При достижении максимального значения будет создан новый файл.
  //! По умолчаню равно 0 - без ограничений.
  void setMaxTimePerFile(int n)           { maxTime_ = n; }

public slots:
  virtual bool slotInit();
  virtual void slotNewIncoming();

private slots:
  void slotProcess();

private:
  void test(const QByteArray &ba);


  QString generateDataName(const tlg::MessageNew& msg) const;
  bool isLimitsReached(int writeCounter, qint64 fileSize, const QDateTime& beginDt);
  qint64 writePackets();
  void sendPackets();
  qint64 write(const QByteArray &datagram);

protected:
  void timerEvent(QTimerEvent* event);

private:
  // параметры
  msgstream::UdpOptions params_;
 // QString meta_template_="{hh}{mm}{ss}{ID}{CCCC}";
  QString meta_template_="{YYYY}{MM}{DD}_{CCCC}_{ID}.tlg";
  TlgParser*  parser_ = nullptr;

  QString id_;
  uint32_t chunk_size_=256;

  bool contentOnly_;

  int    msg_limit_=10000;
  qint64 size_limit_ = 9048;
  qint64 maxTime_;
  qint32 read_interval_=1000;
  bool   need_fec_ = true;
  // данные
  mutable int fileNumb_;

  quint64 sendCount_  = 0;
  quint64 sendTryCount_  = 1;
  quint64 sendFileCount_  = 0;
  quint64 sendChunkCount_  = 0;
  quint64 sendSize_   = 0;
  quint64 sendCountToday_  = 0;
  quint64 sendSizeToday_   = 0;

  QDateTime lastCheck_= QDateTime::currentDateTime() ;
  UdpPacketMap packets_;
  QUdpSocket *udpSocket_=nullptr;
  QString host_ = "10.10.11.118";
 // QString osh_host_ = "192.168.204.2";
  QByteArray outba_;
  int port_=32511;
  msgstream::CompressMode compressionMode_ = msgstream::COMPRESS_NONE;
  // служебные
};

} // meteo


#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_FILESTREAM_UdpStreamOut_H
