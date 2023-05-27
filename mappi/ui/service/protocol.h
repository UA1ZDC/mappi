#pragma once

#include "drive.h"
#include <qbytearray.h>
#include <QtDebug>


namespace mappi {

namespace antenna {

class Protocol
{
public :
  enum command_t {
    DIRECT = 1,
    TRACKER,
    STOP,
    TRAVEL_MODE,
    FEEDHORN = 16,
    FEEDHORN_OUTPUT = 17
  };

public :
  Protocol() { }
  virtual ~Protocol() { }

  virtual bool isText() const = 0;
  virtual bool isAsyncExec() const = 0; // флаг определяет способ выполнения команды
  virtual int mtu() const = 0;          // message transport unit
  virtual const char* type() const = 0;

  // синхронное выполнение команды
  virtual QByteArray pack(int id, float azimut, float elevat) = 0;

  // асинхронное выполнение команд
  virtual QByteArray packAzimut(int id, float v) = 0;
  virtual QByteArray packElevat(int id, float v) = 0;

  virtual bool unpack(const QByteArray& buf, Drive* azimut, Drive* elevat, QString* error = nullptr) = 0;

  virtual bool split(const QByteArray& buf) const = 0;
  virtual bool split(const QByteArray& buf, int mtu) const =0 ;

};

}

}
