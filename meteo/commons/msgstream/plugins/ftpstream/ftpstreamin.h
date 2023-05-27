#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_FTPSTREAM_FTPSTREAMIN_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_FTPSTREAM_FTPSTREAMIN_H

#include <qbytearray.h>
#include <qdatetime.h>
#include <qelapsedtimer.h>
#include <qmap.h>
#include <qset.h>
#include <qsettings.h>
#include <qstringlist.h>
#include <commons/qftp/qftp.h>
#include <qurl.h>

#include <meteo/commons/msgparser/msgmetainfo.h>

#include <commons/container/ts_list.h>
#include <meteo/commons/proto/msgstream.pb.h>
#include <meteo/commons/msgstream/streammodule.h>
#include <meteo/commons/msgstream/processedfiles.h>

class QTimer;
class QTemporaryFile;

namespace meteo {
class TlgParser;
} // meteo

namespace meteo {

//!
class FtpStreamIn : public StreamModule
{
  Q_OBJECT

  enum StatusParam {
    kRecvCount    = 1024,
    kRecvCountToday,
    kRecvSize,
    kRecvSizeToday,
    kRecvSpeed,
    kOperation,
  };

public:
  static QString moduleId() { return QString("ftp-recv"); }

public:
  FtpStreamIn(QObject* p = 0);
  virtual ~FtpStreamIn();

  virtual void setOptions(const msgstream::Options &options);

public slots:
  virtual bool slotInit();

  void slotCommandFinished(int id, bool error);
  void slotListInfo(const QUrlInfo& i);
  void slotSelfTest();
  void slotProgress(qint64 done, qint64 total);
  void slotStateChanged(int state);

protected:
  virtual void timerEvent(QTimerEvent *event);

private:
  void processDirs();
  void processFiles();
  void parseData();

private:
  // параметры
  QString id_;

  QString host_;
  int     port_;
  QString user_;
  QString password_;

  bool deleteAfterProcess_;
  QFtp::TransferMode mode_;
  QFtp::TransferType transfer_type_ = QFtp::Binary;
  QString path_;
  QRegExp fileFilter_;
  QRegExp pathFilter_;
  QString metaTemplate_;
  int sessionTimeout_;
  int waitBeforeRead_;
  int maxQueue_;
  QString magicBegin_;
  QString magicEnd_;
  int maxFileSize_;
  ProcessedFiles processed_;

  meteo::msgstream::FileParseOptions parseOptions_;

  msgstream::Options opt_;
  msgstream::FtpInOptions params_;

  int timerId_ = -1;

  int lastCommand_;
  int lastCommandId_;
  QStringList processingFiles_;
  QStringList processingDirs_;
  MsgMetaInfo meta_;
  QDateTime lastActivity_;  // сохраняем время последней активности программы, т.к. при физическом разрыве связи QFtp
                            // не генерирует ошибки

  // данные
  qint64 recvCount_  = 0;
  qint64 recvSize_   = 0;
  qint64 recvCountToday_  = 0;
  qint64 recvSizeToday_   = 0;
  QDateTime lastCheck_;

  // служебные
  QFtp* ftp_;
  QTimer* testTimer_;
  TlgParser*  parser_;
  QTemporaryFile* file_;
  QString originalFileName_;
  QElapsedTimer progressTimer_;
  QDateTime dt_;
};


} // meteo


#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_FTPSTREAM_FTPSTREAMIN_H
