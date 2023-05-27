#pragma once

#include "protocol.h"


namespace mappi {

namespace antenna {

/**  */
class NiitvProtocol :
  public Protocol
{
public :
  typedef quint8 seq_t;

public :
  NiitvProtocol();
  virtual ~NiitvProtocol();

  virtual bool isText() const { return false; }
  virtual bool isAsyncExec() const { return true; }
  virtual bool hasVelocity() const { return false; }
  virtual bool hasTime() const { return false; }
  virtual int mtu() const { return 3; }
  virtual const char* type() const { return "niitv protocol"; }

  virtual QByteArray pack(int id, const MnSat::TrackTopoPoint& point);
  virtual QByteArray packAzimut(int id, const MnSat::TrackTopoPoint& point);
  virtual QByteArray packElevat(int id, const MnSat::TrackTopoPoint& point);

  virtual bool unpack(const QByteArray& buf, Drive* azimut, Drive* elevat, QString* error = nullptr);
  virtual bool split(const QByteArray& buf) const;
  virtual bool split(const QByteArray&, int mtu) const;

private :
  void toPack(int id, float v, float factor, QByteArray& buf) const;

  float getAngle(const QByteArray& buf, float factor) const;
  void setAngle(float angle, float factor, QByteArray& buf) const;

  qint8 crc(const QByteArray& buf) const;
};

}

}
