#pragma once

#include "engine.h"
#include <qfile.h>


namespace mappi {

namespace receiver {

/**
 * Движок позволяет проигрывать dump из пула заранее записанных dump'ов.
 * Dump выбирается согласно названия спутника.
 * Для каждого dump задается:
 * - скорость воспроизведения, соответствует настоящей скорости приёма (выбирается из документации);
 * - кол-во пакетов в секунду, выбирается произвольно для балансировки нагрузки.
 */
class DumpEngine :
  public Engine
{
  Q_OBJECT
public :
  explicit DumpEngine(QObject* parent = nullptr);
  virtual ~DumpEngine();

  virtual const char* type() const { return "dump engine"; }

  virtual bool configure(const Configuration& conf);
  virtual bool start(Satellite* satellite, const Parameters& param);
  virtual bool stop();

  virtual void updateFreq(float freq);

protected :
  virtual void timerEvent(QTimerEvent* event);

private :
  QFile* src_;
  QFile* dst_;

  QByteArray* buf_;
  int timerId_;
};

}

}
