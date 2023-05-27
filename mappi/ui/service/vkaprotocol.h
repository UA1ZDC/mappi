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
  virtual int mtu() const ;
  virtual const char* type() const { return "vka protocol"; }

  virtual QByteArray pack(int id, float azimut, float elevat);
  virtual QByteArray packAzimut(int id, float v);
  virtual QByteArray packElevat(int id, float v);

  virtual bool unpack(const QByteArray& buf, Drive* azimut, Drive* elevat, QString* error = nullptr);

  virtual bool split(const QByteArray& buf) const;
  virtual bool split(const QByteArray& buf, int mtu) const;

private :
  float getAngle(const QString& buf) const;
  QString setAngle(float v) const;

  QString crc(const QString& buf) const;

private :
  seq_t seq_;
};

}

}
