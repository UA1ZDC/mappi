#pragma once

#include "engine.h"
#include <qudpsocket.h>
#include <qfile.h>


namespace mappi {

namespace receiver {

/**
 * TODO заглушка udp движка (предполагается, что raw-поток будет отдаваться по udp).
 * Сейчас заглушка умеет слушать сокет, писать raw-поток (начинать и останавливать запись).
 * Сокет слушается постоянно, сразу после настройки и пока существует объект движка.
 */
class UdpEngine :
  public Engine
{
  Q_OBJECT
public :
  explicit UdpEngine(QObject* parent = nullptr);
  virtual ~UdpEngine();

  virtual const char* type() const { return "udp engine"; }

  virtual bool configure(const Configuration& conf);
  virtual bool start(Satellite* satellite, const Parameters& param);
  virtual bool stop();

  virtual void updateFreq(float freq);

private slots :
  void onRecv();

private :
  QUdpSocket* socket_;
  QFile* file_;
  quint64 bytesLost_;
};

}

}
