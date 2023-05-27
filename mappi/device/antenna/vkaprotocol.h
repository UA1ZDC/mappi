#pragma once

#include "protocol.h"


namespace mappi {

namespace antenna {

class VkaProtocol :
  public Protocol
{
public :
  typedef quint8 seq_t;

public :
  VkaProtocol();
  virtual ~VkaProtocol();

  virtual bool isText() const { return true; }
  virtual bool isAsyncExec() const { return false; }
  virtual bool hasVelocity() const { return false; }
  virtual bool hasTime() const { return false; }
  virtual int mtu() const { return 30; }
  virtual const char* type() const { return "vka protocol"; }

  virtual QByteArray pack(int id, const MnSat::TrackTopoPoint& point);
  virtual QByteArray packAzimut(int id, const MnSat::TrackTopoPoint& point);
  virtual QByteArray packElevat(int id, const MnSat::TrackTopoPoint& point);

  virtual bool unpack(const QByteArray& buf, Drive* azimut, Drive* elevat, QString* error = nullptr);

  virtual bool split(const QByteArray& buf) const;
  virtual bool split(const QByteArray& buf, int mtu) const;

protected :
  float getAngle(const QString &buf) const;
  QString angleToString(const MnSat::AngleWithVelocity& angle);
  QString crc(const QString& buf) const;
  quint8 seq_;
};

}

}
