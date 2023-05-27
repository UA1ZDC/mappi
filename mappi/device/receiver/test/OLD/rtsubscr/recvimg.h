#ifndef MAPPI_DEVICE_RECEIVER_TEST_RTSUBSR_RECVIMG_H
#define MAPPI_DEVICE_RECEIVER_TEST_RTSUBSR_RECVIMG_H

#include <qobject.h>
#include <meteo/commons/rpc/rpc.h>

namespace mappi {
  namespace conf {
    class DataReply;
  }
}

class RecvImg : public QObject {
  Q_OBJECT
  public:
  RecvImg(QObject* parent = 0);
  ~RecvImg();
  
private:
  
  void callbackStreamRecv(mappi::conf::DataReply* reply);
  void subscribe();

  void saveImg(const QByteArray& data);

public slots:
  void connectToService();
  void disconnected();
  void initChannel();
  
private:
  //rpc::Address _addr;
  ::meteo::rpc::Channel* _channel = 0;
  //rpc::TController* _control = 0;
  
  //  mappi::conf::DataReply* _reply = 0;

  QMap<int, QByteArray> _data;
  QMap<int, int> _rows;
};

#endif
