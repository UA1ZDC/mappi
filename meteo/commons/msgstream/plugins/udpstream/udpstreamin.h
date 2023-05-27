#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_UDPSTREAM_UdpStreamIn_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_UDPSTREAM_UdpStreamIn_H

#include <unordered_set>

#include <qdir.h>
#include <qbytearray.h>
#include <qstringlist.h>
#include <qudpsocket.h>

#include <commons/container/ts_list.h>
#include <meteo/commons/msgstream/streammodule.h>
#include <meteo/commons/proto/msgstream.pb.h>
#include "udp_packet.h"
#include "udpreader.h"

namespace meteo {

class TlgParser;
class UdpPacketMap;

class UdpStreamIn : public StreamModule
{
    Q_OBJECT

    enum StatusParam {
      kRecvCount    = 1024,
      kRecvCountToday,
      kRecvSize,
      kRecvSizeToday,
      kRecvChunkStreamCount,
      kRecvChunkCount,
      kOperation,
      kRecvFileCount,
      kRecvChunkUnkCount,
      kCorrectedCount,
      kNotCorrectedCount
    };

  public:
    static QString moduleId() { return QString("udp-recv"); }

  public:
    UdpStreamIn(QObject* p = 0);
    virtual ~UdpStreamIn();

    virtual void setOptions(const msgstream::Options& opt);
    void process(const QByteArray& datagram);
    bool processPackets(UdpPacketMap* packets, const QString& dataname, bool has_errors);

  public slots:
    virtual bool slotInit();
    void slotNewData();
    void slotError(QString er);
    void slotStateChange(QString st);
    void slotReceiveTimeout();
    void slotCheckTimeout();

  protected:
    virtual void timerEvent(QTimerEvent* event);

  private:
    void clearExpired();
    int countPackets();

    void processPendingDatagrams();

    // параметры
    //QString meta_template_="{hh}{mm}{ss}{ID}{CCCC}";

    QString meta_template_="{YYYY}{MM}{DD}_{ID}";
    int center_window_;
    QString id_;

    msgstream::Options opt_;

    quint64 recvCount_  = 0;
    quint64 fileRecvCount_  = 0;
    quint64 recvSize_   = 0;
    quint64 recvProcessedChunkCount_   = 0;
    quint64 recvCountToday_  = 0;
    quint64 recvSizeToday_   = 0;
    quint64 recvCorrected_   = 0;
    quint64 recvNotCorrected_   = 0;

    QDateTime lastCheck_;

    std::unordered_set<uint32_t> received_files_;
    TlgParser*  parser_ = nullptr;

    QString host_ = "10.10.11.150";
   // QString osh_host_ = "192.168.204.2";
    int port_=32511;
    int expire_time_=600;
    uint32_t chunk_size_=512;
    int state_interval_ = 60;
    int read_interval_ = 1000;
    QMap< QString, QMap <uint32_t, UdpPacketMap > > packets_;
    UdpReader* reader_ =nullptr;
    TSList<QByteArray> packets_list_;
    bool process_=false;
    msgstream::CompressMode compressionMode_ = msgstream::COMPRESS_NONE;
    bool need_fec_ = true;

};


} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_FILESTREAM_UdpStreamIn_H
