#include "ftpstreamout.h"

#include <qtemporaryfile.h>
#include <commons/qftp/qftp.h>

#include <cross-commons/debug/tlog.h>

#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/global/log.h>
#include <meteo/commons/msgparser/common.h>

namespace meteo {

FtpStreamOut::FtpStreamOut(QObject* p)
  : StreamModule(p)
{
  maxMessages_ = 0;
  maxSize_ = 0;
  maxTime_ = 0;
  ftp_ = 0;
  buffer_ = 0;
  fileNumb_ = 0;
}

FtpStreamOut::~FtpStreamOut()
{
  delete buffer_;
  delete ftp_;
}

void FtpStreamOut::setOptions(const msgstream::Options& options)
{
  opt_ = options;
  params_ = options.ftp_send();

  fileMask_ = QString::fromUtf8( params_.mask().c_str() );
  path_ = QString::fromUtf8( params_.path().c_str() );
  maxMessages_ = params_.msg_limit();
  maxSize_ = params_.size_limit();
  maxTime_ = params_.time_limit();

  host_ = QString::fromUtf8( params_.server().host().c_str() );
  port_ = params_.server().port();
  user_ = QString::fromUtf8( params_.server().user().c_str() );
  password_ = QString::fromUtf8( params_.server().password().c_str() );

  mode_ = QFtp::Passive;
  if ( params_.server().has_mode() ) {
    mode_ = params_.server().mode() == msgstream::kActiveFtpMode ? QFtp::Active : QFtp::Passive;
  }
  transfer_type_ = QFtp::Binary;
  if ( params_.server().has_transfer_type() ) {
    transfer_type_ = params_.server().transfer_type() == msgstream::kBinaryTransferType ? QFtp::Binary : QFtp::Ascii;
  }


  id_ = pbtools::toQString(opt_.id());
}

bool FtpStreamOut::slotInit()
{
  if ( 0 == outgoing_ ) { return false; }

  if ( !lastActivity_.isValid() ) {
    status_->setTitle(kSendCount, tr("Передано сообщений"));
    status_->setTitle(kSendSize, tr("Передано данных"));
    status_->setTitle(kSendCountToday, tr("Передано сообщений (сегодня)"));
    status_->setTitle(kSendSizeToday, tr("Передано данных (сегодня)"));
    status_->setTitle(kSendSpeed, tr("Скорость передачи данных"));
    status_->setTitle(kOperation, tr("Операция"));

    status_->setParam(kSendCount, 0);
    status_->setParam(kSendCountToday, 0);
    status_->setParam(kSendSize, 0);
    status_->setParam(kSendSizeToday, 0);
    status_->setParam(kSendSpeed, 0);
  }

  lastActivity_ = QDateTime::currentDateTime();

  if ( 0 != ftp_) {
    ftp_->disconnect(this);
    ftp_->deleteLater();
  }
  ftp_ = new QFtp;
  ftp_->setTransferMode(mode_);
  connect( ftp_, SIGNAL(commandFinished(int,bool)), SLOT(slotCommandFinished(int,bool)) );
  connect( ftp_, SIGNAL(dataTransferProgress(qint64,qint64)), SLOT(slotProgress(qint64,qint64)) );

  if( !process() ){
    QTimer::singleShot( 1000, this, SLOT(slotInit()) );
  }

  return true;
}

bool FtpStreamOut::process()
{
  static QDateTime beginDt = QDateTime::currentDateTime();

  status_->setParam(kOperation, tr("Наполнение файла"));
  status_->setParam(kSendSpeed, 0);

  if ( outgoing_->size() > 0 ) {
    if( 0 == buffer_ ){
      buffer_ = new QTemporaryFile;
    }

    if( !buffer_->open() ){
      error_log << tr("Ошибка при открытии файла '%1'").arg(buffer_->fileName());
      return false;
    }

    while( outgoing_->size() > 0 ){
      static int counter = 0;

      lastMsg_ = outgoing_->first();

      QByteArray ba;
      if ( params_.content_only() ) {
        ba = tlg::tlg2image(pbtools::fromBytes(lastMsg_.msg()));
      }
      else {
        ba = tlg::raw2tlg(lastMsg_);
      }
      qint64 wsz = buffer_->write( ba );
      if ( ba.size() != wsz ) {
        error_log << msglog::kFileWriteIncomplete.arg(buffer_->fileName());
        return false;
      }

      outgoing_->removeFirst();

      ++counter;

      status_->setParam(kSendCount, ++sendCount_);
      status_->setParam(kSendCountToday, ++sendCountToday_);

      sendSize_ += ba.size();
      sendSizeToday_ += ba.size();
      status_->setParam(kSendSize, AppStatusThread::sizeStr(sendSize_));
      status_->setParam(kSendSizeToday, AppStatusThread::sizeStr(sendSizeToday_));

      const int delta = beginDt.secsTo( QDateTime::currentDateTime() );

      if( (maxMessages_ > 0 && counter == maxMessages_) ||
          (maxSize_ > 0 && buffer_->size() >= maxSize_) ||
          (maxTime_ > 0 && delta >= maxTime_)
         )
      {
        beginDt = QDateTime::currentDateTime();
        counter = 0;

        buffer_->close();
        sendFile();

        return true;
      }
    }
  }

  QTimer::singleShot( 1000, this, SLOT(slotInit()) );

  return true;
}

void FtpStreamOut::sendFile()
{
  info_log << tr("Подключение к") << host_ << ":" << port_;
  status_->setParam(kOperation, tr("Подключение к %1:%2").arg(host_).arg(port_));

  int id = ftp_->connectToHost(host_,port_);
  commands_[id] = QFtp::ConnectToHost;

  progressTimer_.start();
}

QString FtpStreamOut::generateFileName(const tlg::MessageNew& msg) const
{
  const QDateTime dt = QDateTime::currentDateTime();
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

  QString fileName = fileMask_;
  foreach( QString t, values.keys() ){
    fileName.replace( t, values[t] );
  }

  return path_+"/"+fileName;
}

void FtpStreamOut::slotCommandFinished(int id, bool error)
{
  QDateTime dt = QDateTime::currentDateTime();
  if ( lastActivity_.date() != dt.date() ) {
    sendSizeToday_ = 0;
    sendCountToday_ = 0;
  }
  lastActivity_ = dt;

  if ( error ) {
    error_log << tr("Ошибка при взаимодействии с ftp-сервером.")
              << ftp_->errorString();
    QTimer::singleShot( 1000, this, SLOT(slotInit()) );
    return ;
  }

  if( QFtp::ConnectToHost == commands_[id] ){
    info_log << tr("Авторизация на %1").arg(host_);
    status_->setParam(kOperation, tr("Авторизация на %1").arg(host_));
    int id = ftp_->login( user_, password_ );
    commands_[id] = QFtp::Login;
  }
  else if( QFtp::Login == commands_[id] ){
    info_log << tr("Передача данных...");

    if( 0 == buffer_ ){
      error_log << tr("Нет данных для передачи");
      int id = ftp_->close();
      commands_[id] = QFtp::Close;
      return;
    }

    if( !buffer_->open() ){
      error_log << tr("Ошибка при открытии файла '%1'").arg(buffer_->fileName());
      QTimer::singleShot( 1000, this, SLOT(slotInit()) );
      return;
    }

    status_->setParam(kOperation, tr("Отправка данных"));

    int id = ftp_->put(buffer_, generateFileName(lastMsg_),transfer_type_);
    commands_[id] = QFtp::Put;
  }
  else if( QFtp::Put == commands_[id] ){
    trc << "4";
    info_log << tr("Данные успешно переданы");
    int id = ftp_->close();
    commands_[id] = QFtp::Close;

    delete buffer_;
    buffer_ = 0;

  }
  else if( QFtp::Close == commands_[id] ){
    QTimer::singleShot( 1000, this, SLOT(slotInit()) );
  }
}

void FtpStreamOut::slotProgress(qint64 done, qint64 total)
{
  Q_UNUSED( total );
  Q_UNUSED( done );

  qint64 speed = static_cast<double>(done) / progressTimer_.elapsed() * 1000;
  status_->setParam(kSendSpeed, AppStatusThread::speedStr(speed));
}

} // meteo
