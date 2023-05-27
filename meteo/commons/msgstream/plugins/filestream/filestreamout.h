#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_FILESTREAM_FILESTREAMOUT_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_FILESTREAM_FILESTREAMOUT_H

#include <qdir.h>
#include <qfile.h>

#include <commons/container/ts_list.h>
#include <meteo/commons/proto/msgstream.pb.h>
#include <meteo/commons/msgstream/streammodule.h>

namespace tlg {
class MessageNew;
} // tlg

namespace meteo {

class FileStreamOut : public StreamModule
{
  Q_OBJECT

  enum StatusParam {
    kSendCount    = 1024,
    kSendCountToday,
    kSendSize,
    kSendSizeToday,
    kOperation,
  };

public:
  static QString moduleId() { return QString("file-send"); }

public:
  FileStreamOut(QObject* p = 0);
  virtual ~FileStreamOut();

  virtual void setOptions(const msgstream::Options &options);

  //! Устанавливает максимальное количество телеграмм, записываемых в один файл.
  //! При достижении максимального значения будет создан новый файл.
  //! По умолчаню равно 0 - без ограничений.
  void setMaxMessagesPerFile(int n)       { maxMessages_ = n; }
  //! Устанавливает максимальный объём данных, записываемых в один файл.
  //! При достижении максимального значения будет создан новый файл.
  //! По умолчаню равно 0 - без ограничений.
  void setMaxSizePerFile(qint64 n)        { maxSize_ = n; }
  //! Устанавливает максимальное время записи в файл.
  //! При достижении максимального значения будет создан новый файл.
  //! По умолчаню равно 0 - без ограничений.
  void setMaxTimePerFile(int n)           { maxTime_ = n; }

public slots:
  virtual bool slotInit();
  virtual void slotNewIncoming();

private slots:
  void slotProcess();

private:
  QString generateFileName(const tlg::MessageNew& msg) const;
  bool isLimitsReached(int writeCounter, qint64 fileSize, const QDateTime& beginDt);

protected:
  void timerEvent(QTimerEvent* event);

private:
  // параметры
  msgstream::Options opt_;
  msgstream::FileOutOptions params_;

  QString id_;
  bool contentOnly_;

  QDir  dir_;
  QFile file_;
  QString fileMask_;

  int    maxMessages_;
  qint64 maxSize_;
  qint64 maxTime_;

  // данные
  mutable int fileNumb_;

  qint64 sendCount_  = 0;
  qint64 sendSize_   = 0;
  qint64 sendCountToday_  = 0;
  qint64 sendSizeToday_   = 0;

  QDateTime lastCheck_;

  // служебные
};

} // meteo


#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_FILESTREAM_FILESTREAMOUT_H
