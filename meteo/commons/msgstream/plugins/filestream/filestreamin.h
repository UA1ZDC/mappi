#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_FILESTREAM_FILESTREAMIN_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_FILESTREAM_FILESTREAMIN_H

#include <qdir.h>
#include <qbytearray.h>
#include <qstringlist.h>

#include <commons/container/ts_list.h>
#include <meteo/commons/msgstream/streammodule.h>
#include <meteo/commons/proto/msgstream.pb.h>

class QTextCodec;
class QFileSystemWatcher;

namespace meteo {

class TlgParser;

class FileStreamIn : public StreamModule
{
  Q_OBJECT

  enum StatusParam {
    kRecvCount    = 1024,
    kRecvCountToday,
    kRecvSize,
    kRecvSizeToday,
    kOperation,
  };

public:
  static QString moduleId() { return QString("file-recv"); }

public:
  FileStreamIn(QObject* p = 0);
  virtual ~FileStreamIn();

  virtual void setOptions(const msgstream::Options& opt);

public slots:
  virtual bool slotInit();

  void slotTimeout();
  void slotWatcher(const QString& path);
  void slotProcess(const QString& path);

protected:
  virtual void timerEvent(QTimerEvent* event);

private:
  // параметры
  QString id_;

  QDir  root_;
  bool  deleteProcessed_;

  int _waitBeforeRead;
  QString _metaTemplate;

  int maxQueue_;

  QString magicBegin_;
  QString magicEnd_;

  meteo::msgstream::FileParseOptions parseOption_;

  msgstream::Options opt_;
  msgstream::FileInOptions params_;

  qint64 recvCount_  = 0;
  qint64 recvSize_   = 0;
  qint64 recvCountToday_  = 0;
  qint64 recvSizeToday_   = 0;

  QDateTime lastCheck_;

  // служебные
  QStringList processedFiles_;

  TlgParser*  parser_ = nullptr;
  QFileSystemWatcher* watcher_ = nullptr;
  QTimer* timer_  = nullptr;
  bool run_ = false;
};


} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_FILESTREAM_FILESTREAMIN_H
