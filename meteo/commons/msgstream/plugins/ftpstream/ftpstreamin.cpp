#include "ftpstreamin.h"

#include <cmath>
#include <climits>

#include <qdir.h>
#include <qtimer.h>
#include <qtemporaryfile.h>

#include <cross-commons/debug/tmap.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgparser/msgmetainfo.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/faxes/packer.h>

namespace meteo {

FtpStreamIn::FtpStreamIn(QObject* p) :
  StreamModule(p)
{
  qRegisterMetaType<QUrlInfo>("QUrlInfo");

  ftp_ = 0;
  file_ = 0;
  parser_ = 0;
  testTimer_ = 0;
  recvCount_ = 0;
}

FtpStreamIn::~FtpStreamIn()
{
  delete parser_;
  parser_ = 0;
  delete ftp_;
  ftp_ = 0;
  delete file_;
  file_ = 0;
}

void FtpStreamIn::setOptions(const msgstream::Options& options)
{
  opt_ = options;
  params_ = options.ftp_recv();

  parseOptions_ = opt_.file_parse_mode();

  parser_ = new TlgParser( QByteArray(), pbtools::toQString(opt_.rule()), opt_.split_stucked() );
  maxQueue_ = opt_.max_queue();

  maxFileSize_ = params_.file_size_limit()*1024*1024;

  if ( opt_.has_magic_begin() ) { magicBegin_ = pbtools::toQString(opt_.magic_begin()); }
  if ( opt_.has_magic_end() ) { magicEnd_ = pbtools::toQString(opt_.magic_end()); }

  path_ = pbtools::toQString(params_.path());
  if( params_.has_meta_template() ){
    metaTemplate_ = pbtools::toQString(params_.meta_template());
  }
  QString filePattern = params_.has_file_filter() ? pbtools::toQString(params_.file_filter()) : "*";
  fileFilter_.setPattern(filePattern);
  fileFilter_.setPatternSyntax( QRegExp::Wildcard );

  QString pathPattern = params_.has_path_filter() ? pbtools::toQString(params_.path_filter()) : "*";
  pathFilter_.setPattern(pathPattern);
  pathFilter_.setPatternSyntax( QRegExp::Wildcard );

  host_ = pbtools::toQString(params_.server().host());
  port_ = params_.server().port();
  user_ = pbtools::toQString(params_.server().user());
  password_ = pbtools::toQString(params_.server().password());

  deleteAfterProcess_ = params_.delete_files();
  waitBeforeRead_ = params_.wait_before_read();
  sessionTimeout_ = params_.session_timeout();

  if ( params_.has_skip_older() ) { dt_ = QDateTime::fromString(pbtools::toQString(params_.skip_older()), Qt::ISODate); }

  mode_ = QFtp::Passive;
  if ( params_.server().has_mode() ) {
    mode_ = params_.server().mode() == msgstream::kActiveFtpMode ? QFtp::Active : QFtp::Passive;
  }

  transfer_type_ = QFtp::Binary;
  if ( params_.server().has_transfer_type() ) {
    transfer_type_ = params_.server().transfer_type() == msgstream::kBinaryTransferType ? QFtp::Binary : QFtp::Ascii;
  }

  id_ = pbtools::toQString(opt_.id());

  QDir localDir(QDir::homePath() + "/." + MnCommon::applicationName());
  if ( !localDir.exists() ) {
    localDir.mkpath(".");
  }
  QString confFilePath = localDir.absoluteFilePath(id_ + ".ini");
  processed_.load(confFilePath);
}

bool FtpStreamIn::slotInit()
{
  if ( 0 == incoming_ ) { return false; }

  if ( !lastCheck_.isValid() ) {
    status_->setTitle(kRecvCount, tr("Принято сообщений"));
    status_->setTitle(kRecvSize, tr("Принято данных"));
    status_->setTitle(kRecvCountToday, tr("Принято сообщений (сегодня)"));
    status_->setTitle(kRecvSizeToday, tr("Принято данных (сегодня)"));
    status_->setTitle(kRecvSpeed, tr("Скорость приёма данных"));
    status_->setTitle(kOperation, tr("Операция"));

    status_->setParam(kRecvCount, 0);
    status_->setParam(kRecvCountToday, 0);
    status_->setParam(kRecvSize, 0);
    status_->setParam(kRecvSizeToday, 0);
    status_->setParam(kRecvSpeed, 0);
  }
  lastCheck_ = QDateTime::currentDateTime();

  lastActivity_ = QDateTime::currentDateTime();

//  status_->setInQueueOverflow(false);

  if ( 0 == testTimer_ ) {
    testTimer_ = new QTimer(this);
    connect( testTimer_, SIGNAL(timeout()), SLOT(slotSelfTest()) );
    testTimer_->start(10000);
  }

  if ( incoming_->size() >= maxQueue_ ) {
//    status_->setInQueueOverflow(true);
    QTimer::singleShot( sessionTimeout_*1000, this, SLOT(slotInit()) );
    return true;
  }

  if ( 0 != ftp_) {
    ftp_->disconnect(this);
    ftp_->deleteLater();
  }
  ftp_ = new QFtp;
  ftp_->setTransferMode(mode_);
  connect( ftp_, SIGNAL(commandFinished(int,bool)), SLOT(slotCommandFinished(int,bool)) );
  connect( ftp_, SIGNAL(listInfo(const QUrlInfo&)), SLOT(slotListInfo(const QUrlInfo&)) );
  connect( ftp_, SIGNAL(dataTransferProgress(qint64,qint64)), SLOT(slotProgress(qint64,qint64)) );
//  connect( ftp_, SIGNAL(stateChanged(int)), SLOT(slotStateChanged(int)) );

  processingDirs_.clear();
  processingFiles_.clear();

  processed_.removeOld();

  if( processingDirs_.isEmpty() ){
    processingDirs_ += path_;
  }

 // debug_log << tr("Подключение к %1:%2").arg(host_).arg(port_);

  status_->setParam(kOperation, tr("Подключение к %1:%2").arg(host_).arg(port_));

  lastCommandId_ = ftp_->connectToHost(host_,port_);
  lastCommand_ = QFtp::ConnectToHost;

  if ( -1 == timerId_ ) {
    timerId_ = startTimer(60000);
  }

  return true;
}

void FtpStreamIn::processDirs()
{
  if( processingDirs_.size() > 0 ){
    lastCommandId_ = ftp_->list( processingDirs_.first() );
    lastCommand_ = QFtp::List;
   // debug_log << tr("Запрос содержимого директории '%1'").arg(processingDirs_.first());
    status_->setParam(kOperation, tr("Запрос содержимого директории '%1'").arg(processingDirs_.first()));
  }
  else {
   // debug_log << tr("Завершение сессии.");
    status_->setParam(kOperation, tr("Завершение сессии."));
    lastCommandId_ = ftp_->close();
    lastCommand_ = QFtp::Close;
  }
}

void FtpStreamIn::processFiles()
{
  if ( incoming_->size() >= maxQueue_ ) {
    lastCommandId_ = ftp_->close();
    lastCommand_ = QFtp::Close;
    return;
  }

  while ( processingFiles_.size() > 0 ) {
    if ( !processed_.isProcessed(processingFiles_.first()) ) {
      break;
    }
    processingFiles_.removeFirst();
  }

  if( processingFiles_.size() > 0 ){
    const QString name = processingFiles_.first();
  //  debug_log << tr("Загрузка файла: %1").arg(name);

    status_->setParam(kOperation, tr("Загрузка файла %1").arg(name));

    progressTimer_.start();

    if( !metaTemplate_.isEmpty() ){
      QString filePath = name;
      meta_ = MsgMetaInfo(metaTemplate_, filePath);
    }

    delete file_;
    originalFileName_ = QString();

    file_ = new QTemporaryFile;    
    if ( false == file_->open() ) {
      error_log << tr("Не удалось открыть временный файл");
      return;
    }
    QFileInfo fileInfo(name);
    originalFileName_ = fileInfo.fileName();

    lastCommandId_ = ftp_->get(name,file_,transfer_type_);
    lastCommand_ = QFtp::Get;
  }
  else {
    processingDirs_.removeFirst();
    processDirs();
  }
}

void FtpStreamIn::parseData()
{
  if( !file_->open() ){
    error_log << tr("Ошибка при открытии файла '%1'").arg(file_->fileName());
    return;
  }

  recvSize_ += file_->size();
  status_->setParam(kRecvSize, AppStatusThread::sizeStr(recvSize_));
  recvSizeToday_ += file_->size();
  status_->setParam(kRecvSizeToday, AppStatusThread::sizeStr(recvSizeToday_));

 // debug_log << "THIS IS !!!" << this->parseOptions_;
  switch (this->parseOptions_) {
  case meteo::msgstream::FileParseOptions::NO_PARSE: {
 //   debug_log << "NO PARSE";
 //   debug_log << magicBegin_;
    QByteArray ba = magicBegin_.toUtf8() + file_->readAll() + magicEnd_.toUtf8();

    tlg::MessageNew msg;
    msg.set_format(tlg::kWMO);
    msg.set_msg(pbtools::toBytes(ba));

    if ( !meta_.t1.isEmpty() ) { msg.mutable_header()->set_t1(meta_.t1.toUtf8().constData()); }
    if ( !meta_.t2.isEmpty() ) { msg.mutable_header()->set_t2(meta_.t2.toUtf8().constData()); }
    if ( !meta_.a1.isEmpty() ) { msg.mutable_header()->set_a1(meta_.a1.toUtf8().constData()); }
    if ( !meta_.a2.isEmpty() ) { msg.mutable_header()->set_a2(meta_.a2.toUtf8().constData()); }
    if ( !meta_.ii.isEmpty() ) { msg.mutable_header()->set_ii(meta_.ii.toInt()); }
    if ( !meta_.cccc.isEmpty() ) { msg.mutable_header()->set_cccc(meta_.cccc.toUtf8().constData()); }
    if ( !meta_.yygggg.isEmpty() ) { msg.mutable_header()->set_yygggg(meta_.yygggg.toUtf8().constData()); }

    if ( !msg.header().has_t1() && opt_.has_predef_t1() ) { msg.mutable_header()->set_t1(opt_.predef_t1()); }
    if ( !msg.header().has_t2() && opt_.has_predef_t2() ) { msg.mutable_header()->set_t2(opt_.predef_t2()); }
    if ( !msg.header().has_a1() && opt_.has_predef_a1() ) { msg.mutable_header()->set_a1(opt_.predef_a1()); }
    if ( !msg.header().has_a2() && opt_.has_predef_a2() ) { msg.mutable_header()->set_a2(opt_.predef_a2()); }
    if ( !msg.header().has_ii() && opt_.has_predef_ii() ) { msg.mutable_header()->set_ii(opt_.predef_ii()); }
    if ( !msg.header().has_cccc() && opt_.has_predef_cccc() ) { msg.mutable_header()->set_cccc(opt_.predef_cccc()); }

    msg.mutable_metainfo()->set_from(id_.toUtf8().constData());

    ba = tlg::proto2tlg(msg);

    parser_->setData(ba);
    if ( parser_->parseNextMessage(&msg) ) {
      if ( opt_.has_predef_msgtype() ) { msg.set_msgtype(opt_.predef_msgtype()); }

      QDateTime dt = meta_.calcConvertedDt();
      if ( dt.isValid() ) {
        msg.mutable_metainfo()->set_converted_dt(pbtools::toString(dt.toString(Qt::ISODate)));
      }

      incoming_->append(msg);
      status_->setParam(kRecvCount, ++recvCount_);
      status_->setParam(kRecvCountToday, ++recvCountToday_);
    }
    else {
   //   debug_log << tr("Не удалось распарсить сформированное сообщение.");
    }

    break;
  }
  case msgstream::FileParseOptions::PARSE: {
    if ( !magicBegin_.isEmpty() ) {
      parser_->setData(magicBegin_.toUtf8());
    }

    const int chunk = 1024*16;
    for( int i=0,isz=file_->size(); i<isz; i+=chunk ){
      QByteArray ba(chunk, '\0');
      int rsz = file_->read( ba.data(), chunk );
      parser_->appendData( ba.left(rsz) );

      if ( i+chunk>=isz && !magicEnd_.isEmpty() ) {
        parser_->appendData(magicEnd_.toUtf8());
      }

      tlg::MessageNew msg;
      while ( parser_->parseNextMessage(&msg) ) {
        if ( !msg.header().has_t1() && !meta_.t1.isEmpty() ) { msg.mutable_header()->set_t1(meta_.t1.toUtf8().constData()); }
        if ( !msg.header().has_t2() && !meta_.t2.isEmpty() ) { msg.mutable_header()->set_t2(meta_.t2.toUtf8().constData()); }
        if ( !msg.header().has_a1() && !meta_.a1.isEmpty() ) { msg.mutable_header()->set_a1(meta_.a1.toUtf8().constData()); }
        if ( !msg.header().has_a2() && !meta_.a2.isEmpty() ) { msg.mutable_header()->set_a2(meta_.a2.toUtf8().constData()); }
        if ( !msg.header().has_ii() && !meta_.ii.isEmpty() ) { msg.mutable_header()->set_ii(meta_.ii.toInt()); }
        if ( !msg.header().has_cccc() && !meta_.cccc.isEmpty() ) { msg.mutable_header()->set_cccc(meta_.cccc.toUtf8().constData()); }
        if ( !msg.header().has_yygggg() && !meta_.yygggg.isEmpty() ) { msg.mutable_header()->set_yygggg(meta_.yygggg.toUtf8().constData()); }

        msg.mutable_metainfo()->set_from(id_.toUtf8().constData());

        meta_.update(msg);
        QDateTime dt = meta_.calcConvertedDt();
        if ( dt.isValid() ) {
          msg.mutable_metainfo()->set_converted_dt(pbtools::toString(dt.toString(Qt::ISODate)));
        }

        incoming_->append(msg);
        status_->setParam(kRecvCount, ++recvCount_);
        status_->setParam(kRecvCountToday, ++recvCountToday_);
      }
    }
    break;
  }
  case msgstream::FileParseOptions::SPLIT: {    

    int chunkCount = 0;
    meteo::faxes::Packer packer;
    // генерируем уникальный айди файла
    int fileid = meteo::faxes::Packer::generateFileId(originalFileName_);
    QList<QByteArray> messages;
    QByteArray content = file_->readAll();

    bool isPacked = packer.createFileMessages(originalFileName_, content,&messages,fileid,meteo::faxes::FaxSegment::DOC);
    if ( false == isPacked ) {
      error_log << QObject::tr("При упаковке файла телеграммы в формете DOCCHAIN произошла ошибка");
      break;
    }

    for ( auto data: messages ) {
      ++chunkCount;
      QByteArray ba = magicBegin_.toUtf8() + data + magicEnd_.toUtf8();
      tlg::MessageNew msg;
      msg.set_format(tlg::kWMO);
      msg.set_msg(pbtools::toBytes(ba));      

      if ( !meta_.t1.isEmpty() ) { msg.mutable_header()->set_t1(meta_.t1.toUtf8().constData()); }
      if ( !meta_.t2.isEmpty() ) { msg.mutable_header()->set_t2(meta_.t2.toUtf8().constData()); }
      if ( !meta_.a1.isEmpty() ) { msg.mutable_header()->set_a1(meta_.a1.toUtf8().constData()); }
      if ( !meta_.a2.isEmpty() ) { msg.mutable_header()->set_a2(meta_.a2.toUtf8().constData()); }

      if ( !meta_.ii.isEmpty() ) { msg.mutable_header()->set_ii(meta_.ii.toInt()); }
      if ( !meta_.cccc.isEmpty() ) { msg.mutable_header()->set_cccc(meta_.cccc.toUtf8().constData()); }
      if ( !meta_.yygggg.isEmpty() ) { msg.mutable_header()->set_yygggg(meta_.yygggg.toUtf8().constData()); }

      if ( !msg.header().has_t1() && opt_.has_predef_t1() ) { msg.mutable_header()->set_t1(opt_.predef_t1()); }
      if ( !msg.header().has_t2() && opt_.has_predef_t2() ) { msg.mutable_header()->set_t2(opt_.predef_t2()); }
      if ( !msg.header().has_a1() && opt_.has_predef_a1() ) { msg.mutable_header()->set_a1(opt_.predef_a1()); }
      if ( !msg.header().has_a2() && opt_.has_predef_a2() ) { msg.mutable_header()->set_a2(opt_.predef_a2()); }
      if ( !msg.header().has_ii() && opt_.has_predef_ii() ) { msg.mutable_header()->set_ii(opt_.predef_ii()); }
      if ( !msg.header().has_cccc() && opt_.has_predef_cccc() ) { msg.mutable_header()->set_cccc(opt_.predef_cccc()); }


      ba = tlg::proto2tlg(msg);
      parser_->setData(ba);
      if ( parser_->parseNextMessage(&msg) ) {
        if ( true == opt_.has_predef_msgtype() ) {
          msg.set_msgtype(opt_.predef_msgtype());
        }

        QDateTime dt = meta_.calcConvertedDt();
        if ( dt.isValid() ) {
          msg.mutable_metainfo()->set_converted_dt(pbtools::toString(dt.toString(Qt::ISODate)));
        }
        msg.mutable_metainfo()->set_from(id_.toUtf8().constData());

        incoming_->append(msg);
        status_->setParam(kRecvCount, ++recvCount_);
        status_->setParam(kRecvCountToday, ++recvCountToday_);
      }
      else {
     //   debug_log << tr("Не удалось распарсить сформированное сообщение.");
      }

      while ( incoming_->size() >= maxQueue_ ) {
        QThread::msleep(500);
      }
    }
    break;
  }
  }
}

void FtpStreamIn::slotCommandFinished(int id, bool error)
{
  lastActivity_ = QDateTime::currentDateTime();

  if( error ){
    error_log << tr("Ошибка при взаимодействии с ftp-сервером.") << ftp_->errorString();
    if ( QFtp::Close == lastCommand_ ) {
      QTimer::singleShot( sessionTimeout_*1000, this, SLOT(slotInit()) );
    }
    else {
      int id = ftp_->close();
      lastCommand_ = QFtp::Close;
      lastCommandId_ = id;
    }

    status_->setParam(kOperation, tr("Ошибка при взаимодействии с ftp-сервером"), app::OperationState_ERROR);
    return ;
  }

  if(  lastCommandId_ != id ){
  //  debug_log << tr("Неверный идентификатор команды") << varname(id) << varname(lastCommandId_);
    return;
  }

  if( QFtp::ConnectToHost == lastCommand_ ){
   // info_log << tr("Авторизация...");
    lastCommandId_ = ftp_->login( user_, password_ );
    lastCommand_ = QFtp::Login;

    status_->setParam(kOperation, tr("Авторизация"));
  }
  else if( QFtp::Login == lastCommand_ ){
    processDirs();
  }
  else if( QFtp::List == lastCommand_ ){
   // debug_log << tr("Количество файлов: %1").arg( processingFiles_.size() );
   // debug_log << tr("Количество директорий: %1").arg( processingDirs_.size() );
    processFiles();
  }
  else if( QFtp::Get == lastCommand_ ){
    file_->close();

    QString fileName = processingFiles_.first();
    processed_.addProcessed(fileName);

    if( deleteAfterProcess_ ){
    //  debug_log << tr("Удаление файла '%1'").arg(processingFiles_.first());
      status_->setParam(kOperation, tr("Удаление файла '%1'").arg(processingFiles_.first()));
      lastCommandId_ = ftp_->remove(processingFiles_.first());
      lastCommand_ = QFtp::Remove;
    }
    else {
      parseData();
      processFiles();
    }
  }
  else if( QFtp::Remove == lastCommand_ ){
    parseData();
    processFiles();
  }
  else if( QFtp::Close == lastCommand_ ){
   // debug_log << tr("Сессия завершена.");

    status_->setParam(kOperation, tr("Обработка завершена"));

    QTimer::singleShot( sessionTimeout_*1000, this, SLOT(slotInit()) );
  }
  else {
   // debug_log << "unknown command";
    var(lastCommand_);
  }
}

void FtpStreamIn::slotListInfo(const QUrlInfo& i)
{
  if( i.name() == "." || i.name() == ".." ){
    return;
  }

  QString filePath = processingDirs_.first() + "/" +i.name();

  if( i.isDir() ){
    if ( pathFilter_.exactMatch(filePath) ) {
      processingDirs_ += filePath;
    }
    else {
   //   debug_log << tr("[пропуск] %1 - не соответствует фильтру (%2)").arg(filePath, pathFilter_.pattern());
    }
  }
  else if( i.isFile() ){
    if ( maxFileSize_ >= 0 && i.size() > maxFileSize_ ) {
   //   debug_log << tr("[пропуск]") << i.name() << "too large file" << i.size()/1024/1024 << "Mb";
      return;
    }
    if ( dt_.isValid() && dt_.secsTo(i.lastModified()) < 0 ) {
   //   debug_log << tr("[пропуск]") << "old file" << i.name();
      return;
    }

    const int delta = i.lastModified().secsTo( QDateTime::currentDateTime() );

    if ( fileFilter_.exactMatch(i.name()) ) {
      if ( delta >= waitBeforeRead_ ) {
          processingFiles_ += filePath;
      }
      else {
//        debug_log << tr("[пропуск] %1 - ожидаем %2 сек. с момента последнего изменения (осталось ещё %3 сек.)")
   //                  .arg(filePath).arg(waitBeforeRead_).arg(waitBeforeRead_ - delta);
      }
    }
    else {
    //  debug_log << tr("[пропуск] %1 - не соответствует фильтру (%2)").arg(i.name(), fileFilter_.pattern());
    }
  }
}

void FtpStreamIn::slotSelfTest()
{
  if ( lastActivity_.secsTo(QDateTime::currentDateTime()) >= sessionTimeout_*2 ) {
    slotInit();
  }
}

void FtpStreamIn::slotProgress(qint64 done, qint64 total)
{
  Q_UNUSED( total );

  qint64 speed = static_cast<double>(done) / (progressTimer_.elapsed() + 1) * 1000; // +1 защита от деления на 0
  status_->setParam(kRecvSpeed, AppStatusThread::speedStr(speed));
}

void FtpStreamIn::slotStateChanged(int state)
{
  switch ( state ) {
    case QFtp::Unconnected:
      debug_log << "There is no connection to the host.";
      break;
    case QFtp::HostLookup:
      debug_log << "A host name lookup is in progress.";
      break;
    case QFtp::Connecting:
      debug_log << "An attempt to connect to the host is in progress.";
      break;
    case QFtp::Connected:
      debug_log << "Connection to the host has been achieved.";
      break;
    case QFtp::LoggedIn:
      debug_log << "Connection and user login have been achieved.";
      break;
    case QFtp::Closing:
      debug_log << "The connection is closing down, but it is not yet closed. (The state will be Unconnected when the connection is closed.)";
      break;
  }
}

void FtpStreamIn::timerEvent(QTimerEvent* event)
{
  Q_UNUSED( event );

  QDateTime dt = QDateTime::currentDateTime();
  if ( lastCheck_.date() != dt.date() ) {
    recvSizeToday_ = 0;
    recvCountToday_ = 0;
  }
  lastCheck_ = dt;
}

} // meteo
