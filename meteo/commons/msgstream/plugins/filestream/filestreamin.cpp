#include "filestreamin.h"

#include <cmath>
#include <climits>

#include <qelapsedtimer.h>
#include <qfilesystemwatcher.h>

#include <cross-commons/debug/tlog.h>

#include <cross-commons/debug/tmap.h>
#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/msgparser/msgmetainfo.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/faxes/packer.h>

namespace meteo {

FileStreamIn::FileStreamIn(QObject* p)
  : StreamModule(p)
{
  parser_ = nullptr;
  watcher_ = nullptr;
  status_ = nullptr;
}

FileStreamIn::~FileStreamIn()
{
  delete parser_;
  parser_ = nullptr;
  delete status_;
  status_ = nullptr;
  delete timer_;
  timer_ = nullptr;
}

void FileStreamIn::setOptions(const msgstream::Options& opt)
{
  opt_ = opt;
  params_ = opt.file_recv();
}

bool FileStreamIn::slotInit()
{
  if ( !opt_.has_direction() ) {
    return false;
  }
  if ( msgstream::kReceiver != opt_.direction() ) {
    error_log << tr("Допустимым значением параметра common.direction для модуля file-in являются только kReceiver");
    return false;
  }

  if ( !lastCheck_.isValid() ) {
    status_->setTitle(kRecvCount, tr("Принято сообщений"));
    status_->setTitle(kRecvSize, tr("Принято данных"));
    status_->setTitle(kRecvCountToday, tr("Принято сообщений (сегодня)"));
    status_->setTitle(kRecvSizeToday, tr("Принято данных (сегодня)"));
    status_->setTitle(kOperation, tr("Операция"));

    status_->setParam(kRecvCount, 0);
    status_->setParam(kRecvCountToday, 0);
    status_->setParam(kRecvSize, 0);
    status_->setParam(kRecvSizeToday, 0);
  }

  lastCheck_ = QDateTime::currentDateTime();
  QObject::startTimer(kTimeToLastActivity);
  if ( 0 == incoming_ ) { return false; }
  parser_ = new TlgParser(QByteArray(), pbtools::toQString(opt_.rule() ), opt_.split_stucked() );
  parseOption_ = opt_.file_parse_mode();
  maxQueue_ = opt_.center_window();
  deleteProcessed_ = params_.delete_files();
  if ( opt_.has_magic_begin() ) { magicBegin_ = pbtools::toQString(opt_.magic_begin()); }
  if ( opt_.has_magic_end() ) { magicEnd_ = pbtools::toQString(opt_.magic_end()); }
  root_.setPath(pbtools::toQString(params_.path()));
  if( !root_.exists() && !root_.mkpath(root_.absolutePath()) ){
    error_log << tr("Не удалось создать директорию %1").arg(root_.absolutePath());
    return false;
  }
  if ( params_.has_file_filter() ) {
    root_.setNameFilters( QStringList(pbtools::toQString(params_.file_filter())) );
  }
  if ( params_.has_meta_template() ) {
    _metaTemplate = pbtools::toQString(params_.meta_template());
  }
  _waitBeforeRead = params_.wait_before_read();
  id_ = pbtools::toQString(opt_.id());

  if ( nullptr != watcher_ ) {
    delete watcher_;
  }
  if ( nullptr != timer_ ) {
    timer_->deleteLater();
  }
  watcher_ = new QFileSystemWatcher( this );
  watcher_->addPath( root_.absolutePath() );
  timer_ = new QTimer();
  run_ = false;
  QObject::connect( watcher_, SIGNAL(directoryChanged(QString)), SLOT(slotWatcher(QString)) );
  QObject::connect( timer_, SIGNAL(timeout()), SLOT(slotTimeout()) );
  slotProcess( root_.absolutePath() );
  timer_->start(300000);
  return true;
}

void FileStreamIn::slotTimeout()
{
  if (run_) {
    return;
  }
  run_ = true;
  slotProcess( root_.absolutePath() );
  run_ = false;
}

void FileStreamIn::slotWatcher( const QString& path )
{
  if (run_) {
    return;
  }
  run_ = true;
  slotProcess( path );
  run_ = false;
}

void FileStreamIn::slotProcess(const QString& path)
{
  if ( 0 == watcher_ ) { return; }
  QStringList watchingPaths = watcher_->directories();
  QDir dir( path );
  QString p;
  QStringList dirs = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot );
  foreach ( const QString& name, dirs ) {
    p = dir.absoluteFilePath(name);
    slotProcess( p );
    if( !watchingPaths.contains(p) ){
      watcher_->addPath( p );
    }
  }

  QStringList entryList = dir.entryList( QDir::Files | QDir::NoDotAndDotDot );

  QStringList files;
  foreach( const QString& name, entryList ) {
    files += dir.absoluteFilePath(name);
  }
  foreach( const QString& filepath, processedFiles_ ){
    files.removeOne(filepath);
  }

  foreach ( const QString& filepath, files )
  {
    QFileInfo info(filepath);
    QDateTime dt = info.lastModified().addSecs(_waitBeforeRead);
    int delta = QDateTime::currentDateTime().secsTo( dt );
    if( delta > 0 ){
      continue;
    }

    MsgMetaInfo meta;
    if( !_metaTemplate.isEmpty() ){
      meta = MsgMetaInfo(_metaTemplate, info.fileName());
    }

    QFile file( dir.absoluteFilePath(filepath) );
    if ( !file.open(QIODevice::ReadOnly) ) {
//      emit error( ERROR_READ_FILE, tr("Ошибка при открытии файла") );
      debug_log << file.errorString();
      debug_log << "file:" << file.fileName();
      continue;
    }

    if ( !magicBegin_.isEmpty() ) {
      parser_->setData(magicBegin_.toUtf8());
    }

    status_->setParam(kOperation, tr("Обработка файла %1").arg(file.fileName()));

    switch(this->parseOption_){
    case meteo::msgstream::FileParseOptions::PARSE: {
      const int chunk = 1024*1024;
      while ( false == file.atEnd() ) {
        QByteArray ba(chunk, '\0');
        int64_t rsz = file.read(ba.data(), chunk);
        parser_->setData(ba.left(rsz));

        if ( file.atEnd() && !magicEnd_.isEmpty() ) {
          parser_->appendData(magicEnd_.toUtf8());
        }

        tlg::MessageNew msg;
        while ( parser_->parseNextMessage(&msg) ) {
          if ( !msg.header().has_t1() && !meta.t1.isEmpty() ) { msg.mutable_header()->set_t1(meta.t1.toUtf8().constData()); }
          if ( !msg.header().has_t2() && !meta.t2.isEmpty() ) { msg.mutable_header()->set_t2(meta.t2.toUtf8().constData()); }
          if ( !msg.header().has_a1() && !meta.a1.isEmpty() ) { msg.mutable_header()->set_a1(meta.a1.toUtf8().constData()); }
          if ( !msg.header().has_a2() && !meta.a2.isEmpty() ) { msg.mutable_header()->set_a2(meta.a2.toUtf8().constData()); }
          if ( !msg.header().has_ii() && !meta.ii.isEmpty() ) { msg.mutable_header()->set_ii(meta.ii.toInt()); }
          if ( !msg.header().has_cccc() && !meta.cccc.isEmpty() ) { msg.mutable_header()->set_cccc(meta.cccc.toUtf8().constData()); }
          if ( !msg.header().has_yygggg() && !meta.yygggg.isEmpty() ) { msg.mutable_header()->set_yygggg(meta.yygggg.toUtf8().constData()); }

          meta.update(msg);

          msg.mutable_metainfo()->set_from(id_.toUtf8().constData());

          QDateTime dt = meta.calcConvertedDt();
          if ( dt.isValid() ) {
            msg.mutable_metainfo()->set_converted_dt(pbtools::toString(dt.toString(Qt::ISODate)));
          }
          appendNewIncoming(msg);

          status_->setParam(kRecvCount, ++recvCount_);
          status_->setParam(kRecvCountToday, ++recvCountToday_);
          recvSize_ += msg.msg().size();
          recvSizeToday_ += msg.msg().size();
          status_->setParam(kRecvSize, AppStatusThread::sizeStr(recvSize_));
          status_->setParam(kRecvSizeToday, AppStatusThread::sizeStr(recvSizeToday_));

          msg.Clear();
        }

        while ( incoming_->size() >= maxQueue_ ) {
          QThread::msleep(500);
        }
      }
      break;
    }
    case meteo::msgstream::FileParseOptions::NO_PARSE: {
      QByteArray ba = magicBegin_.toUtf8() + file.readAll() + magicEnd_.toUtf8();

      tlg::MessageNew msg;
      msg.set_format(tlg::kWMO);
      msg.set_msg(pbtools::toBytes(ba));

      if ( !meta.t1.isEmpty() ) { msg.mutable_header()->set_t1(meta.t1.toUtf8().constData()); }
      if ( !meta.t2.isEmpty() ) { msg.mutable_header()->set_t2(meta.t2.toUtf8().constData()); }
      if ( !meta.a1.isEmpty() ) { msg.mutable_header()->set_a1(meta.a1.toUtf8().constData()); }
      if ( !meta.a2.isEmpty() ) { msg.mutable_header()->set_a2(meta.a2.toUtf8().constData()); }
      if ( !meta.ii.isEmpty() ) { msg.mutable_header()->set_ii(meta.ii.toInt()); }
      if ( !meta.cccc.isEmpty() ) { msg.mutable_header()->set_cccc(meta.cccc.toUtf8().constData()); }
      if ( !meta.yygggg.isEmpty() ) { msg.mutable_header()->set_yygggg(meta.yygggg.toUtf8().constData()); }

      if ( !msg.header().has_t1() && opt_.has_predef_t1() ) { msg.mutable_header()->set_t1(opt_.predef_t1()); }
      if ( !msg.header().has_t2() && opt_.has_predef_t2() ) { msg.mutable_header()->set_t2(opt_.predef_t2()); }
      if ( !msg.header().has_a1() && opt_.has_predef_a1() ) { msg.mutable_header()->set_a1(opt_.predef_a1()); }
      if ( !msg.header().has_a2() && opt_.has_predef_a2() ) { msg.mutable_header()->set_a2(opt_.predef_a2()); }
      if ( !msg.header().has_ii() && opt_.has_predef_ii() ) { msg.mutable_header()->set_ii(opt_.predef_ii()); }
      if ( !msg.header().has_cccc() && opt_.has_predef_cccc() ) { msg.mutable_header()->set_cccc(opt_.predef_cccc()); }

      msg.mutable_metainfo()->set_from(pbtools::toString(id_));

      ba = tlg::proto2tlg(msg);

      parser_->setData(ba);
      while ( parser_->parseNextMessage(&msg) ) {
        debug_log << "poshelprocess";
        if ( opt_.has_predef_msgtype() ) { msg.set_msgtype(opt_.predef_msgtype()); }

        QDateTime dt = meta.calcConvertedDt();
        if ( dt.isValid() ) {
          msg.mutable_metainfo()->set_converted_dt(pbtools::toString(dt.toString(Qt::ISODate)));
        }
        appendNewIncoming(msg);
        //incoming_->append(msg);
        status_->setParam(kRecvCount, ++recvCount_);
        status_->setParam(kRecvCountToday, ++recvCountToday_);
        recvSize_ += msg.msg().size();
        recvSizeToday_ += msg.msg().size();
        status_->setParam(kRecvSize, AppStatusThread::sizeStr(recvSize_));
        status_->setParam(kRecvSizeToday, AppStatusThread::sizeStr(recvSizeToday_));
      }
      //else {
      //  debug_log << tr("Не удалось распарсить сформированное сообщение.");
      //}
      break;
    }
    case meteo::msgstream::FileParseOptions::SPLIT: {

      int chunkCount = 0;
      meteo::faxes::Packer packer;
      // генерируем уникальный айди файла     
      QList<QByteArray> messages;
      QByteArray content = file.readAll();
      QString originalFileName = file.fileName();
      int fileid = meteo::faxes::Packer::generateFileId(originalFileName);

      bool isPacked = packer.createFileMessages(originalFileName, content,&messages,fileid,meteo::faxes::FaxSegment::DOC);
      if ( false == isPacked ) {
        error_log << QObject::tr("При упаковке файла телеграммы в формете DOCCHAIN произошла ошибка");
        break;
      }

      for ( const auto &data: qAsConst(messages) ) {
        ++chunkCount;
        QByteArray ba = magicBegin_.toUtf8() + data + magicEnd_.toUtf8();
        tlg::MessageNew msg;
        msg.set_format(tlg::kWMO);
        msg.set_msg(pbtools::toBytes(ba));

        if ( !meta.t1.isEmpty() ) { msg.mutable_header()->set_t1(meta.t1.toUtf8().constData()); }
        if ( !meta.t2.isEmpty() ) { msg.mutable_header()->set_t2(meta.t2.toUtf8().constData()); }
        if ( !meta.a1.isEmpty() ) { msg.mutable_header()->set_a1(meta.a1.toUtf8().constData()); }
        if ( !meta.a2.isEmpty() ) { msg.mutable_header()->set_a2(meta.a2.toUtf8().constData()); }

        if ( !meta.ii.isEmpty() ) { msg.mutable_header()->set_ii(meta.ii.toInt()); }
        if ( !meta.cccc.isEmpty() ) { msg.mutable_header()->set_cccc(meta.cccc.toUtf8().constData()); }
        if ( !meta.yygggg.isEmpty() ) { msg.mutable_header()->set_yygggg(meta.yygggg.toUtf8().constData()); }

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

          QDateTime dt = meta.calcConvertedDt();
          if ( dt.isValid() ) {
            msg.mutable_metainfo()->set_converted_dt(pbtools::toString(dt.toString(Qt::ISODate)));
          }
          msg.mutable_metainfo()->set_from(id_.toUtf8().constData());
          incoming_->append(msg);
          status_->setParam(kRecvCount, ++recvCount_);
          status_->setParam(kRecvCountToday, ++recvCountToday_);
        }
        else {
          debug_log << tr("Не удалось распарсить сформированное сообщение.");
        }

        while ( incoming_->size() >= maxQueue_ ) {
          QThread::msleep(500);
        }
      }
      break;
    }

    }

    if( deleteProcessed_ ){
      if( !file.remove() ){
        debug_log << file.errorString();
        debug_log << "file:" << file.fileName();
      }
    }
    else {
      processedFiles_ += info.absoluteFilePath();
    }
  }

  status_->setParam(kOperation, tr("Обработка завершена"));
}

void FileStreamIn::timerEvent(QTimerEvent* event)
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
