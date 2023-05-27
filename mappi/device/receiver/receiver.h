#pragma once

#include "apch.h"
#include "config.h"
#include "engine.h"


namespace mappi {

namespace receiver {

/** Интерфейс приёмника. */
class Receiver :
  public QObject
{
  Q_OBJECT
public :
  explicit Receiver(QObject* parent = nullptr);
  virtual ~Receiver();

  virtual bool tuned(const Configuration& conf);
  virtual bool start(Satellite* satellite, const Parameters& param);
  virtual bool stop();

private slots :
  void adjustFreq(float freq);

private :
  ApchUnit* apch_;      // QObject
  Engine* engine_;      // QObject
};

}

}
