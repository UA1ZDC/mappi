#include "filestreamout.h"

#include <cross-commons/debug/tlog.h>

#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/global/log.h>
#include <meteo/commons/grib/decoder/tgribdecode.h>
#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/proto/tgribformat.pb.h>

namespace meteo {

FileStreamOut::FileStreamOut(QObject* p)
  : StreamModule(p)
{
  maxMessages_ = 0;
  maxSize_ = 0;
  maxTime_ = 0;
  contentOnly_ = false;
  fileNumb_ = 0;
  status_ = 0;
}

FileStreamOut::~FileStreamOut()
{
  delete status_;
  status_ = 0;
}

void FileStreamOut::setOptions(const msgstream::Options& options)
{
  opt_ = options;
  params_ = options.file_send();

  fileMask_ = pbtools::toQString(params_.mask());
  dir_.setPath(pbtools::toQString(params_.path()) );
  maxMessages_ = params_.msg_limit();
  maxSize_ = params_.size_limit();
  maxTime_ = params_.time_limit();
  contentOnly_ = params_.content_only();

  id_ = pbtools::toQString(opt_.id());
}

bool FileStreamOut::slotInit()
{
  if ( !opt_.has_direction() ) {
    debug_log << tr("Необходимо указать значение параметра common.direction");
    return false;
  }

  if ( msgstream::kSender != opt_.direction() ) {
    error_log << tr("Допустимым значением параметра common.direction для модуля file-send является только kSender");
    return false;
  }

  if ( !dir_.exists() && !dir_.mkpath(dir_.absolutePath()) ) {
    error_log << tr("Не удалось создать директорию %1").arg(dir_.absolutePath());
    return false;
  }

  if ( !lastCheck_.isValid() ) {
    status_->setTitle(kSendCount, tr("Передано сообщений"));
    status_->setTitle(kSendSize, tr("Передано данных"));
    status_->setTitle(kSendCountToday, tr("Передано сообщений (сегодня)"));
    status_->setTitle(kSendSizeToday, tr("Передано данных (сегодня)"));
    status_->setTitle(kOperation, tr("Операция"));

    status_->setParam(kSendCount, 0);
    status_->setParam(kSendCountToday, 0);
    status_->setParam(kSendSize, 0);
    status_->setParam(kSendSizeToday, 0);

    status_->setParam(kOperation, tr("Наполнение файла"));
  }

  lastCheck_ = QDateTime::currentDateTime();

  startTimer(kTimeToLastActivity);

  if ( 0 == outgoing_ ) { return false; }

  slotProcess();

  return true;
}

void FileStreamOut::slotNewIncoming() {
  slotProcess();
}



void FileStreamOut::slotProcess()
{
  static int writeCounter = 0;
  static QDateTime beginDt = QDateTime::currentDateTime();
  if ( 0 == outgoing_ ) { return; }

  if( outgoing_->size() == 0 ){
    if( true == file_.isOpen() && isLimitsReached(writeCounter, file_.size(), beginDt) ) {
      file_.flush();
      file_.close();
      QString fn = file_.fileName();
      if ( ".tmp" == fn.right(4) ) {
        fn = fn.left( fn.length() - 4 );
      }
      if ( false == file_.rename(fn) ) {
        error_log << QObject::tr("Ошибка переименовывания файла = %1. Ошибка = %2")
          .arg(fn)
          .arg( file_.errorString() );
      }
    }
    QTimer::singleShot( 1000, this, SLOT(slotProcess()) );
    return;
  }

  while ( outgoing_->size() > 0 ) {

    tlg::MessageNew msg = outgoing_->first();

    if ( false == file_.isOpen() ) {
      file_.setFileName(generateFileName(msg));
      QFileInfo fi(file_);
      QDir dir = fi.absoluteDir();
      dir.mkpath( fi.absolutePath() );
      beginDt = QDateTime::currentDateTime();
    }

    if ( !file_.isOpen() && !file_.open(QIODevice::WriteOnly|QIODevice::Append) ) {
      error_log << msglog::kFileOpenError.arg(file_.fileName(), file_.errorString());
      //QTimer::singleShot( 5000, this, SLOT(slotProcess()) );
      return;
    }

    if( isLimitsReached(writeCounter, file_.size(), beginDt) ) {
      file_.flush();
      file_.close();
      QString fn = file_.fileName();
      if ( ".tmp" == fn.right(4) ) {
        fn = fn.left( fn.length() - 4 );
      }
      if ( false == file_.rename(fn) ) {
        error_log << QObject::tr("Ошибка переименовывания файла = %1. Ошибка = %2")
          .arg(fn)
          .arg( file_.errorString() );
      }

      writeCounter = 0;

      fn = generateFileName(msg);
      file_.setFileName(fn);
      if ( false == file_.isOpen() ) {
        QFileInfo fi(file_);
        QDir dir = fi.absoluteDir();
        dir.mkpath( fi.absolutePath() );
        beginDt = QDateTime::currentDateTime();
      }

      if( !file_.open(QIODevice::WriteOnly|QIODevice::Truncate) ){
        error_log << msglog::kFileOpenError.arg(file_.fileName(), file_.errorString());
        break;
      }
    }

    QByteArray ba;
    if ( contentOnly_ ) {
      ba = tlg::tlg2image(pbtools::fromBytes(msg.msg()));
    }
    else {
      ba = tlg::raw2tlg(msg);
    }
    qint64 wsz = file_.write(ba);
    if( wsz != ba.size() ){
      error_log << msglog::kFileWriteFailed.arg(file_.fileName(), file_.errorString());
      break;
    }
    outgoing_->removeFirst();

    status_->setParam(kSendCount, ++sendCount_);
    status_->setParam(kSendCountToday, ++sendCountToday_);
    sendSize_ += ba.size();
    sendSizeToday_ += ba.size();
    status_->setParam(kSendSize, AppStatusThread::sizeStr(sendSize_));
    status_->setParam(kSendSizeToday, AppStatusThread::sizeStr(sendSizeToday_));

    ++writeCounter;

    file_.flush();
  }

  QTimer::singleShot( 30000, this, SLOT(slotProcess()) );
}

QString FileStreamOut::generateFileName(const tlg::MessageNew& msg) const
{
  QDateTime dt = QDateTime::currentDateTimeUtc();
  QMap<QString,QString> values;
  values.insert( "{YYYY}", dt.toString("yyyy") );
  values.insert( "{YY}", dt.toString("yy") );
  values.insert( "{MM}", dt.toString("MM") );
  values.insert( "{DD}", dt.toString("dd") );
  values.insert( "{hh}", dt.toString("hh") );
  values.insert( "{mm}", dt.toString("mm") );
  values.insert( "{ss}", dt.toString("ss") );
  values.insert( "{YYGGgg}", pbtools::toQString(msg.header().yygggg()) );
  values.insert( "{ID}", QString("%1").arg(++fileNumb_, 8, 10, QChar('0')) );

  values.insert( "{CCCC}", pbtools::toQString(msg.header().cccc()) );

//  if ( "grib" == msg.msgtype() ) {
//    TDecodeGrib dec;
//    dec.decode(QByteArray::fromRawData(msg.msg().c_str(), msg.msg().size()));
//
//    const grib::TGribData* grib = dec.grib();
//    if ( 0 != grib) {
//      // данные для замены шаблона {GRIB_FORECAST_TIME}
//      QDateTime dt = QDateTime::fromString(QString::fromUtf8(grib->dt().c_str()), Qt::ISODate);
//      dt.setTimeSpec(Qt::UTC);
//
//      if ( !dt.isValid() ) {
//        debug_log << "can't get grib datetime";
//        var(grib->dt());
////        continue;
//      }
//
//      if ( grib->has_product1() ) {
//        if ( grib->product1().timerange() != 0 ) {
////          not_impl;
////          var(grib->product1().Utf8DebugString());
////          continue;
//        }
//        dt = dt.addSecs(grib->product1().p1());
//        values.insert( "{GRIB_FORECAST_TIME}", dt.toString("yyyyMMdd_hh") );
//      }
//      else if ( grib->has_product2() ) {
//        dt = dt.addSecs(grib->product2().forecasttime());
//        values.insert( "{GRIB_FORECAST_TIME}", dt.toString("yyyyMMdd_hh") );
//      }
//      else {
//        debug_log << "too little info to get grib datetime";
////        continue;
//      }
//
//      // данные для замены шаблона {GRIB_VER}
//      values.insert("{GRIB_VER}", QString::number(dec.edition()));
//    }
//  }

  QString fileName = fileMask_;
  foreach( QString t, values.keys() ){
    fileName.replace( t, values[t] );
  }

  int n = 1;
  QString tmp = fileName;
  while ( QFile::exists(dir_.absoluteFilePath(tmp)) ) {
    tmp = QString::number(n++) + "_" + fileName;
  }

  fileName = tmp;

  fileName = dir_.absoluteFilePath(fileName);
  fileName += ".tmp";

  return fileName;
}

bool FileStreamOut::isLimitsReached(int writeCounter,
                                         qint64 fileSize,
                                         const QDateTime& beginDt)
{
  bool messageLimit = (maxMessages_ > 0) && (writeCounter == maxMessages_);
  bool sizeLimit = (maxSize_ > 0) && (fileSize >= maxSize_);

  QDateTime endDt = beginDt.addSecs(maxTime_);
  bool timeLimit = (maxTime_ > 0) && (endDt <= QDateTime::currentDateTime());

  return messageLimit || sizeLimit || timeLimit;
}

void FileStreamOut::timerEvent(QTimerEvent* event)
{
  Q_UNUSED( event );

  QDateTime dt = QDateTime::currentDateTime();
  if ( lastCheck_.date() != dt.date() ) {
    sendSizeToday_ = 0;
    sendCountToday_ = 0;
  }
  lastCheck_ = dt;
}

}
