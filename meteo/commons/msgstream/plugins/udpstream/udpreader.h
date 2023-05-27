#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_UDPSTREAM_UdpReader_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_UDPSTREAM_UdpReader_H

#include <qdir.h>
#include <qbytearray.h>
#include <qstringlist.h>
#include <qudpsocket.h>
#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <commons/container/ts_list.h>

namespace meteo {

class UdpReader : public QObject
{
    Q_OBJECT
  public:
    UdpReader(QObject* p = 0);
    virtual ~UdpReader();
    void setParams(const QString &, int,int);
    bool takeFirst(QByteArray*);
    bool isBind();
    int packetsCount() {return packets_list_.size();}
    QAbstractSocket::SocketState socketState() ;
    bool reconnect();
    void setMulticast(bool ism);

  public slots:
    void slotProcessPendingDatagrams();
    bool slotInit();
    void slotSocketStateChanged(QAbstractSocket::SocketState) ;
    void slotSocketError(QAbstractSocket::SocketError err);
  signals:
    void newData();
    void errorSignal(QString );
    void stateSignal(QString );

  private:
    // параметры
    QUdpSocket *udpSocket_=nullptr;
    QString host_ = "10.10.11.118";
    int port_=32511;
    TSList<QByteArray> packets_list_;
    bool isb_ = false;
    int datagram_size_ = 1024;
    QByteArray datagram_;
    bool is_multicast_=false;

};


} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_FILESTREAM_UdpReader_H
