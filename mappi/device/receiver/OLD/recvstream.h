#ifndef MAPPI_DEVICE_RECEIVER_RECV_H
#define MAPPI_DEVICE_RECEIVER_RECV_H

#include <sat-commons/satellite/satellite.h>

#include <mappi/global/streamheader.h>
#include <mappi/proto/reception.pb.h>

#include <QFile>

class QMutex;
class QTimer;

namespace mappi {
namespace receive {
   
//! Тип подписки
enum RecvSource {
  kUnkSrc = -1,
  kFileSrc = 0, //!< Получение имени файла сохранённого потока
  kUdpSrc  = 1, //!<
};
    
//! Получение потока от устройства, сохранение в файл или выдача сразу клиентам
class RecvStream : public QObject {
  Q_OBJECT
public:
  RecvStream();
  ~RecvStream();

  bool init(const QString& site, const meteo::GeoPoint& siteCoord, const QString& resDir);
  bool setFileSource(const QString& file);

  bool start(const QString& name, conf::RecvMode mode, int32_t direct, const MnSat::TLEParams& tle);
  void stop(const mappi::conf::Site& site, QString* fileName, uint64_t* dbId);
  void stop(const mappi::conf::Site& site, const QDateTime& start, const QDateTime& end,
	    QString* fileName, uint64_t* dbId);
  
  void finish();
signals:
  void startSignal();
  void stopSignal();
  void finished();
  //void dataSaved(const QString& fileName, int64_t dbId);
  void receivedData(const QByteArray& data, int status);

private:
  bool startReceive();
  bool startFileRecv();
  bool startUdpRecv();

  uint64_t saveToDb(const mappi::conf::Site& site);

private slots:
  void readFileData();

private:
  QMutex*              editMutex_;
  QMutex*              readMutex_;
  meteo::global::StreamHeader header_;               //!< Формируемый заголовок с мета-данными, добавляемый после сохранения полученных данных
  QFile                result_;               //!< Дескриптор файла, в который сохраняются спутниковые данные
  QString              resultDir_;            //!< Папка для сохранения спутниковых данных
  RecvSource           source_ = kUnkSrc;     //!< Источник получения спутниковых данных

  QFile                sourceFile_;           //!< Дескриптор файла-источника (при получении из файла)
  QString              currentFile_;          //!< Название файла-источника (при получении из файла)
  qint64               pos_;                  //!< Положение в файле-искточнике, до которого данные считаны

  conf::StatusSession  status_  = conf::kEnd; //!< Состояние приёма
  QTimer*              timer_   = 0;
};
    
} // receive
} // mappi

#endif
