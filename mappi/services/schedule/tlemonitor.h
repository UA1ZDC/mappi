#pragma once

#include <qobject.h>
#include <qfileinfo.h>


namespace mappi {

namespace schedule {

class TleMonitor :
  public QObject
{
  Q_OBJECT
public :
  explicit TleMonitor(QObject* parent = nullptr);
  virtual ~TleMonitor();

  void start(const QString& path);

signals :
  void update();

protected :
  virtual void timerEvent(QTimerEvent* event);

private :
  int timerId_;
  QFileInfo fileInfo_;
};

}

}
