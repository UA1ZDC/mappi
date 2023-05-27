#ifndef MAPPI_DEVICE_ANTENNA_PRBL_ANT_H
#define MAPPI_DEVICE_ANTENNA_PRBL_ANT_H

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <mappi/proto/reception.pb.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <qobject.h>
#include <qserialport.h>


namespace mappi {

namespace receive {

// Режим работы антенны
enum AntennaMode {
  kInitMode  = 0, //!< инициализация
  kFixMode   = 1, //!< наведение
  kMoveMode  = 2, //!< сопровождение
  kStopMode  = 3, //!< остановлена
  kTravelMode = 4 //!< походное положение
};


//! Управление антенной. Все углы в градусах
class PrblAnt :
  public QObject
{
  Q_OBJECT
public :
  explicit PrblAnt(conf::AntennaType type, QObject* parent = nullptr);
  virtual ~PrblAnt();

  inline conf::AntennaType type() const { return type_; }
  inline conf::AntennaParam conf() { return *conf_; }

  inline QSerialPort* port() const { return port_; }
  inline bool isOpenPort() { return isOpen_; }
  bool openPort(const QString& name);
  void closePort();

  virtual bool isInit() { return true; }
  virtual bool setPower(bool v);

  float azMin() const;
  float azMax() const;
  float zeroAzimut() const;
  inline void setZeroAzimut(float v) { azCorr_ = v; }

  virtual bool isAzimutValid(float angle, float zeroCorrect) const;
  virtual bool currentAzimut(float* az) = 0;
  virtual bool setAzimut(float angle, float* curAzimut = nullptr) = 0;
  virtual bool moveAzimut(float angle, float* curAzimut = nullptr) = 0;

  float elMin() const;
  float elMax() const;
  float zeroElevat() const;
  inline void setZeroElevat(float v) { elCorr_ = v; }

  virtual bool isElevatValid(float angle, float zeroCorrect) const;
  virtual bool currentElevat(float* el) = 0;
  virtual bool setElevat(float angle, float* curEl = nullptr) = 0;
  virtual bool moveElevat(float angle, float* curEl = nullptr) = 0;

  virtual bool currentAngles(float* az, float* el) = 0;
  virtual bool setPosition(float azimut, float elev, bool wait) = 0;
  virtual bool movePosition(float azimut, float elev, float* curAzimut = nullptr, float* curEl = nullptr) = 0;
  virtual bool stop(float* azimut, float* elevation) = 0;
  virtual void stop() = 0;

  inline float dsaMax() const { return conf_->dsa_max(); }
  inline float dseMax() const { return conf_->dse_max(); }

  int state() const { return state_; }

  bool isDiagnostics() const { return isDiagnostics_; }
  void setDiagnostics(bool isSet) { isDiagnostics_ = isSet; }

signals :
  void packetReady(const QByteArray& pack);

protected :
  bool readConfig(conf::AntennaType type);

  // асинхронное чтение
  void startAsyncRead(const QByteArray& start, const QByteArray& stop, int length);
  void stopAsyncRead();

  // синхронное чтение
  virtual bool writeBytes(const QByteArray& data);
  virtual bool readBytes(QByteArray* cmd);
  // uint8_t createCrc(uint8_t b1, uint8_t b2);

private slots :
  // питание
  void slotDtsChanged(bool fl);
  void slotRtsChanged(bool fl);

  // асинхронное чтение
  void handleReadyRead();
  void handleError(QSerialPort::SerialPortError error);

private :
  conf::AntennaType type_;
  conf::AntennaParam* conf_;

  bool isOpen_;
  QSerialPort* port_;

  float azCorr_; //!< поправка по азимуту
  float elCorr_; //!< поправка по углу места

  bool dts_;
  bool rts_;

  int state_; //костыль, см в коде (исп. в antfollow())
  bool isDiagnostics_;

  // для синхронного чтения
  int lengthPack_; //!< размер пакета
  QByteArray startPack_; //!< указание начала пакета
  QByteArray stopPack_; //!< указание окончания пакета
  QByteArray pack_; //!< данные
};

}

}

#endif // MAPPI_DEVICE_ANTENNA_PRBL_ANT_H
