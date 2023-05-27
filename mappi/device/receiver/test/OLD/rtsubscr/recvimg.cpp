#include "recvimg.h"

#include <commons/textproto/tprototext.h>
#include <commons/textproto/pbtools.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <meteo/commons/global/global.h>

#include <mappi/proto/reception.pb.h>

#include <qimage.h>

RecvImg::RecvImg(QObject* parent /*= 0*/):
  QObject(parent)
{
  initChannel();
}

RecvImg::~RecvImg()
{
  if ( 0 != _channel ) {
    QObject::disconnect( _channel, 0, this, 0 );
    delete _channel;
    _channel = 0;
  }

}

void RecvImg::initChannel()
{
  if (0 != _channel) {
    QObject::disconnect(_channel, 0, this, 0);
    _channel->deleteLater();
    _channel = 0;
  }

  connectToService();
}


void RecvImg::disconnected()
{

  QTimer::singleShot( 5000, this, SLOT(connectToService()) );
}

void RecvImg::connectToService()
{
  if (0 == _channel) {
    _channel = meteo::global::serviceChannel(meteo::settings::proto::kRecvStream);
    if (0 == _channel) {
      QTimer::singleShot(10000, this, SLOT(connectToService()));
      return;
    }
    QObject::connect( _channel, SIGNAL(disconnected()), this, SLOT(disconnected()));
  }

  if ( !_channel->isConnected() ) {
    //    if ( !_channel->connect() ) {
      QObject::disconnect( _channel, 0, this, 0 );
      _channel->deleteLater();
      _channel = 0;
      QTimer::singleShot( 10000, this, SLOT(connectToService()) );
      return;
      //}
  }

  debug_log << tr("Установлено соединение с сервисом по адресу %1").arg(_channel->address());

  subscribe();
}

void RecvImg::subscribe()
{  
  if (0 == _channel) { return; }

  if (!_channel->isConnected()) { return; }

  mappi::conf::DataRequest req;
  req.set_subscr(mappi::conf::SubscrType::kDataSubscr);
  
  bool ok = _channel->subscribe(&mappi::conf::ReceiveService::GetDataMulti, req, 
				this, &RecvImg::callbackStreamRecv);

  if (!ok) {
    error_log << QObject::tr("Не удалось подписаться на получение информации о принятых данных");
    QTimer::singleShot(5000, this, SLOT(initChannel()));
  }
}


void RecvImg::callbackStreamRecv(mappi::conf::DataReply* reply)
{
  if (0 == reply || 0 == reply->has_data()) return;

  saveImg(QByteArray(reply->data().data(), reply->data().size()));
  
}

void RecvImg::saveImg(const QByteArray& data)
{
  //может не успевать создавать изображение, если большая скорость передачи
  QVector<QRgb> palette(256);
  for (uint i=0; i< 256; i++) {
    palette[i] = QRgb((i<<16)+ (i<<8) + i);
  }

  int thinning;
  int chcnt;
  int number;
  int rows, cols;
  QByteArray chData;
  QDataStream ds(data);
  ds >> thinning;
  ds >> chcnt;
  //debug_log << thinning << chcnt;
  for (int idx = 0; idx < chcnt; idx++) {
    ds >> number;
    ds >> chData;
    ds >> rows;
    ds >> cols;

    //var(number);
    //var(rows);
    //var(cols);

    if (!_rows.contains(number)) {
      _rows.insert(number, rows);
    } else {
      _rows[number] += rows;
    }
    _data[number].append(chData);
    
    if (number == 0) {
      debug_log << "ch=" << number << "rows=" << _rows[number];
    }
    //var(chData.size());
    //var(_data[number].size());
    QImage imqt((const uchar*)(_data[number].data()), cols, _rows[number], cols, QImage::Format_Indexed8);
    imqt.setColorCount(256);
    
    imqt.setColorTable(palette);
    imqt.save("img_" + QString::number(number) + ".bmp", "BMP");
  }
  

}
