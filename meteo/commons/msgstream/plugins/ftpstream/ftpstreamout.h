#ifndef METEO_COMMONS_MSGSTREAM_PLUGINS_FTPSTREAM_FTPSTREAMOUT_H
#define METEO_COMMONS_MSGSTREAM_PLUGINS_FTPSTREAM_FTPSTREAMOUT_H

#include <qdir.h>
#include <qmap.h>
#include <qfile.h>
#include <qelapsedtimer.h>

#include <commons/qftp/qftp.h>
#include <commons/container/ts_list.h>

#include <meteo/commons/proto/msgstream.pb.h>
#include <meteo/commons/msgstream/streammodule.h>

class QTemporaryFile;

namespace meteo {

//!
class FtpStreamOut : public StreamModule
{
  Q_OBJECT

  enum StatusParam {
    kSendCount    = 1024,
    kSendCountToday,
    kSendSize,
    kSendSizeToday,
    kSendSpeed,
    kOperation,
  };

public:
  static QString moduleId() { return QString("ftp-send"); }

public:
  FtpStreamOut(QObject* p = 0);
  virtual ~FtpStreamOut();

  virtual void setOptions(const msgstream::Options &options);

public slots:
  bool slotInit();

private slots:
  void slotCommandFinished(int id, bool error);
  void slotProgress(qint64 done, qint64 total);

private:
  bool process();
  void sendFile();
  QString generateFileName(const tlg::MessageNew& msg) const;

private:
  // параметры
  msgstream::Options opt_;
  msgstream::FtpOutOptions params_;

  QString id_;

  QString path_;
  QString fileMask_;

  int    maxMessages_;
  qint64 maxSize_;
  qint64 maxTime_;

  QFtp*   ftp_;
  QString host_;
  int     port_;
  QString user_;
  QString password_;

  QFtp::TransferMode  mode_;
  QFtp::TransferType transfer_type_ = QFtp::Binary;

  QMap<int,int> commands_;

  // данные
  qint64 sendCount_  = 0;
  qint64 sendSize_   = 0;
  qint64 sendCountToday_  = 0;
  qint64 sendSizeToday_   = 0;

  QDateTime lastActivity_;

  // служебные
  mutable int fileNumb_;
  QElapsedTimer progressTimer_;
  QTemporaryFile* buffer_;
  tlg::MessageNew lastMsg_;
};

} // meteo

#endif // METEO_COMMONS_MSGSTREAM_PLUGINS_FTPSTREAM_FTPSTREAMOUT_H
