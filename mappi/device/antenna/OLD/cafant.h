#ifndef MAPPI_DEVICE_ANTENNA_CAFANT_H
#define MAPPI_DEVICE_ANTENNA_CAFANT_H

#include "prbl-ant.h"


namespace mappi {

namespace receive {

class CafAnt :
  public PrblAnt
{
public :
  explicit CafAnt(QObject* parent = nullptr);
  virtual ~CafAnt();

  virtual bool currentAzimut(float* az) override;
  virtual bool setAzimut(float angle, float* curAzimut = nullptr) override;
  virtual bool moveAzimut(float, float*) override { return false; }

  virtual bool currentElevat(float* el) override;
  virtual bool setElevat(float angle, float* curEl = nullptr) override;
  virtual bool moveElevat(float, float*) override { return false; }

  virtual bool currentAngles(float* az, float* el) override;
  virtual bool setPosition(float azimut, float elev, bool wait) override;
  virtual bool movePosition(float azimut, float elev, float* curAzimut = nullptr, float* curEl = nullptr) override;
  virtual bool stop(float* azimut, float* elevation) override;
  virtual void stop() override {}

  AntennaMode mode();
  bool setTravelPosition();
  bool status(uint32_t* status);

private slots :
  void parseReadPack(const QByteArray& pack);

private :
  QByteArray angle2ascii(float angle);
  float ascii2angle(const QByteArray& ba);

  bool sendCommand(const QByteArray& cmd, const QByteArray& data1, const QByteArray& data2, const QByteArray& data3);
  QByteArray createCrc(const QByteArray& data);

private :
  uint32_t _status = 0;
  float _azimut = -9999;
  float _elev = -9999;

  uint8_t _currentPack = 0;
  bool _isCoordOk = false;
};

}

}

#endif // MAPPI_DEVICE_ANTENNA_CAFANT_H
