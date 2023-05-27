#pragma once

#include "engine.h"
#include <qprocess.h>


namespace mappi {

namespace receiver {

/**
 * Движок запускает внешнее приложение в дочернем процессе.
 * В данном случае запускается python script (gnu radio).
 * raw-файл пишет непосредственно внешнее приложение.
 */
class SubProcessEngine :
  public Engine
{
  Q_OBJECT
public :
  explicit SubProcessEngine(QObject* parent = nullptr);
  virtual ~SubProcessEngine();

  virtual const char* type() const { return "sub process engine"; }

  virtual bool configure(const Configuration& conf);
  virtual bool start(Satellite* satellite, const Parameters& param);
  virtual bool stop();

  virtual void updateFreq(float freq);

private slots :
  void fromOutput();
  void fromError();

private :
  QProcess* app_;
  QString appPath_;
};

}

}
