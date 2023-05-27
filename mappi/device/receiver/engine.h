#pragma once

#include "config.h"
#include <cross-commons/debug/tlog.h>
#include <sat-commons/satellite/satellite.h>
#include <qobject.h>


namespace mappi {

namespace receiver {

/** Реализация логики работы приёмника. */
class Engine :
  public QObject
{
public :
  explicit Engine(QObject* parent = nullptr) : QObject(parent) { }
  virtual ~Engine() { }

  virtual const char* type() const = 0;

  virtual bool configure(const Configuration& conf) = 0;
  virtual bool start(Satellite* satellite, const Parameters& param) = 0;
  virtual bool stop() = 0;

  // АПЧ с учетом реализации
  virtual void updateFreq(float freq) = 0;
};

}

}
