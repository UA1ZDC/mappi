#include "tlemonitor.h"
#include <qdatetime.h>
#include <QTimerEvent>


namespace mappi {

namespace schedule {

static const int INTERVAL = 10;

TleMonitor::TleMonitor(QObject* parent /*=*/) :
    QObject(parent),
  timerId_(-1)
{
}

TleMonitor::~TleMonitor()
{
}

void TleMonitor::start(const QString& path)
{
  fileInfo_ = QFileInfo(path);
  timerId_ = startTimer(INTERVAL * 1000);
}

void TleMonitor::timerEvent(QTimerEvent* event)
{
  if (event->timerId() != timerId_)
    return ;

  QFileInfo cFileInfo(fileInfo_.absoluteFilePath());
  if (!cFileInfo.exists())
    return ;

  if (fileInfo_.birthTime() < cFileInfo.birthTime() ||
      fileInfo_.lastModified() < cFileInfo.lastModified()) {

    fileInfo_ = cFileInfo;
    emit update();      // signal
  }
}

}

}
