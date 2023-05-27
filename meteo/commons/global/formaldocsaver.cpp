#include "formaldocsaver.h"

#include <qapplication.h>
#include <qmessagebox.h>

#include <meteo/commons/rpc/channel.h>
#include <meteo/commons/global/global.h>

static const int defaultRpcTimeout = 180000;

namespace meteo {
FormalDocSaver::FormalDocSaver( QObject* p )
  : QObject(p),
  watcher_( new QFileSystemWatcher(this) ),
  timeout_(defaultRpcTimeout)
{
  QObject::connect( watcher_, &QFileSystemWatcher::fileChanged, this, &FormalDocSaver::slotDocumentChanged );
}

FormalDocSaver::~FormalDocSaver()
{
  delete watcher_; watcher_ = nullptr;
}

void FormalDocSaver::slotCreateFormalDoc( const map::proto::Map& map, const QStringList& users )
{
  qApp->setOverrideCursor(Qt::WaitCursor);

  meteo::rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kMap );
  if ( nullptr == ch ) {
    qApp->restoreOverrideCursor();
    QMessageBox::critical(0, QObject::tr("Внимание"), QObject::tr("Ошибка. Не удалось подключиться к сервису формирования документов") );
    return;
  }
//  QObject::connect( ch, &rpc::Channel::disconnected, this, &FormalDocSaver::slotClientDisconnected );
  clients_.insert( ch, ClientInfo() );
  ClientInfo& info = clients_[ch];
  info.map_.CopyFrom(map);
  info.users_ = users;

  info.resp_createdoc_ = ch->remoteCall(&meteo::map::proto::DocumentService::CreateDocument, info.map_.document(), this->timeout_);
  if ( nullptr == info.resp_createdoc_ ) {
    qApp->restoreOverrideCursor();
    QMessageBox::critical(0, QObject::tr("Внимание"), QObject::tr("При создании документа ответ от сервиса не получен"));
    slotClientDisconnected(ch);
    ch->deleteLater();
    return;
  }
  if ( false == info.resp_createdoc_->result() ) {
    qApp->restoreOverrideCursor();
    QMessageBox::critical(0, QObject::tr("Внимание"),
                          QObject::tr("При попытке создать документ произошла ошибка. Описание ошибки = \n\t%1")
                          .arg( QString::fromStdString( info.resp_createdoc_->comment())) );
    slotClientDisconnected(ch);
    ch->deleteLater();
    return;
  }

  info.resp_createmap_ = ch->remoteCall( &meteo::map::proto::DocumentService::CreateMap, info.map_, this->timeout_);

  if ( nullptr == info.resp_createmap_ ) {
    qApp->restoreOverrideCursor();
    QMessageBox::critical(0, QObject::tr("Внимание"),
                          QObject::tr("Ответ от сервиса при формировании документа не получен. Шаблон = %1. Дата = %2")
                          .arg( QString::fromStdString( info.map_.name() ) )
                          .arg( QString::fromStdString( info.map_.datetime() ) ));
    slotClientDisconnected(ch);
    ch->deleteLater();
    return;
  }

  if ( false ==info.resp_createmap_->result() ) {
    qApp->restoreOverrideCursor();
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(0, QObject::tr("Внимание"),
                                     QObject::tr(info.resp_createmap_->comment().data()) + QObject::tr(" Открыть пустой шаблон?"));
    if ( reply == QMessageBox::StandardButton::Yes ) {
      openEmptyDoc(ch);
      return;
    }
    else {
      slotClientDisconnected(ch);
      ch->deleteLater();
      return;
    }
  }
  if ( true == info.resp_createmap_->has_comment() ) {
    meteo::dbusSendNotification(QObject::tr("Внимание"), QString::fromStdString(info.resp_createmap_->comment()), QString(), 8000, QStringList() );
  }

  info.req_export_map_.set_format( info.map_.formal().format() );
  info.req_export_map_.set_title( info.map_.name() );

  info.resp_savedocum_ = ch->remoteCall( &meteo::map::proto::DocumentService::SaveDocument, info.req_export_map_, this->timeout_);
  if ( nullptr == info.resp_savedocum_ ) {
    qApp->restoreOverrideCursor();
    QMessageBox::critical(0, QObject::tr("Внимание"), QObject::tr("Не получен ответ от сервиса при сохранении документа."));
    slotClientDisconnected(ch);
    ch->deleteLater();
    return;
  }
  if ( false == info.resp_savedocum_->result() ) {
    qApp->restoreOverrideCursor();
    QMessageBox::critical(0, QObject::tr("Внимание"), QObject::tr("Не удалось сохранить документ в СПО Банк."));
    slotClientDisconnected(ch);
    ch->deleteLater();
    return;
  }

  info.resp_exportmap_ = ch->remoteCall(&meteo::map::proto::DocumentService::GetDocument, info.req_export_map_, this->timeout_);
  if ( nullptr == info.resp_exportmap_ ) {
    qApp->restoreOverrideCursor();
    QMessageBox::critical(0, QObject::tr("Внимание"), QObject::tr("Нет ответа от сервиса при получении сохраненного документа."));
    slotClientDisconnected(ch);
    ch->deleteLater();
    return;
  }
  if ( false == info.resp_exportmap_->result() ) {
    qApp->restoreOverrideCursor();
    error_log << QObject::tr("Ошибка получения документа = %1")
      .arg( QString::fromStdString( info.resp_exportmap_->comment() ) );
    slotClientDisconnected(ch);
    ch->deleteLater();
    return;
  }

  openTemp( ch, users );
  qApp->restoreOverrideCursor();
}

web::UserListResponce FormalDocSaver::getUserList( bool *ok )
{
  if ( nullptr != ok ) {
    *ok = false;
  }
  rpc::Channel* ch = global::serviceChannel( settings::proto::kInterMessagesService );
  if ( nullptr == ch ) {
    error_log << QObject::tr("Нет подключения");
    return protousers_;
  }
  web::UserListRequest req;
  req.set_username( global::currentUserLogin().toStdString() );
  auto resp = ch->remoteCall( &web::InterService::GetUserListForUser, req, this->timeout_);
  if ( nullptr == resp ) {
    error_log << QObject::tr("Список пользователей не получен");
    delete ch;
    return protousers_;
  }
  delete ch;
  protousers_.CopyFrom(*resp);
  delete resp;
  if ( nullptr != ok ) {
    *ok = true;
  }
  return protousers_;
}

void FormalDocSaver::openEmptyDoc( rpc::Channel* ch )
{
  if ( false == clients_.contains(ch) ) {
    warning_log << QObject::tr("Непредвиденная ситуация");
    return;
  }
  ClientInfo& info = clients_[ch];
  QFile shablon( MnCommon::projectPath() + "share/meteo/odtsamples/" + QString::fromStdString( info.map_.formal().template_() ) );
  QString extension = QFileInfo(shablon).suffix();
  shablon.open(QIODevice::ReadOnly);
  QByteArray arr = shablon.readAll();
  shablon.close();
  if ( nullptr != info.file_ ) {
    watcher_->removePath( info.file_->fileName() );
    delete info.file_; info.file_ = nullptr;
  }
  info.file_ = new QTemporaryFile(this);
  info.file_->setFileTemplate(QDir::tempPath()+QString("/mtptmp_Taylored_XXXXXX.") + extension );
  info.file_->open();
  info.file_->write(arr, arr.size());
  info.file_->close();
  watcher_->addPath( info.file_->fileName() );
  createOfficeProc(ch);
}

void FormalDocSaver::createOfficeProc( rpc::Channel* channel )
{
  ClientInfo& i= clients_[channel];
  i.proc_ = new QProcess(this);
  QObject::connect( i.proc_, SIGNAL( finished( int, QProcess::ExitStatus ) ), this, SLOT( slotProcFinished() ) );
  QString off = QString("libreoffice " + i.file_->fileName());
  i.proc_->start(off);
  i.proc_->waitForStarted();
}

void FormalDocSaver::slotProcFinished()
{
  auto proc = static_cast<QProcess*>( sender() );
  rpc::Channel* ch = nullptr;
  for ( auto it = clients_.begin(), end = clients_.end(); it != end; ++it ) {
    if ( it.value().proc_ == proc ) {
      ch = it.key();
      break;
    }
  }
  if ( nullptr == ch ) {
    proc->deleteLater();
    return;
  }
  auto& info = clients_[ch];
  info.proc_->deleteLater();
  info.proc_ = nullptr;
  slotClientDisconnected(ch);
  ch->deleteLater(); //info очистится при удалении клиента
}

void FormalDocSaver::openTemp( rpc::Channel* ch, const QStringList& users )
{
  if ( false == clients_.contains(ch) ) {
    error_log.msgBox() << QObject::tr("Сценарий работы прерван с непредвиденной ошибкой. Не найден канал. Обратитесь в службу поддержки maslo.team.");
    return;
  }
  ClientInfo& info = clients_[ch];
  QString extension;

  switch ( info.map_.formal().format() ) {
  case meteo::map::proto::kOdt: {
    extension = "odt";
    break;
  }
  case meteo::map::proto::kOds: {
    extension = "ods";
    break;
  }
  default: {
    if ( false == extension.isEmpty() ){
      error_log.msgBox() << QObject::tr("Формат документа %1 не поддерживается.'").arg(extension);
    }
    else {
      error_log.msgBox() << QObject::tr("Формат документа не указан.");
    }
    return;
    }
  }

  info.file_ = new QTemporaryFile(this);
  info.file_->setFileTemplate(QDir::tempPath()+QString("/mtptmp_XXXXXX.") + extension);

  QByteArray bar = QByteArray::fromStdString( info.resp_exportmap_->data() );

  info.file_->open();
  info.file_->write(bar);
  info.file_->close();

  std::string stations;
  for ( ::meteo::sprinf::Station s : info.map_.station() ) {
    stations += "_" + s.station() + "_" ;
  }

  info.filename_ = QString::fromStdString( info.map_.title() + stations + info.map_.datetime() ) + "." + extension;
  info.users_ = users;

  if ( 0 != info.users_.size() ) {
    sendFilesToUsers(ch);
  }

  watcher_->addPath( info.file_->fileName() );

  createOfficeProc(ch);

  qApp->restoreOverrideCursor();
}

void FormalDocSaver::sendFilesToUsers( rpc::Channel* ch )
{
  if ( false == clients_.contains(ch) ) {
    return;
  }
  ClientInfo& info = clients_[ch];
  if ( 0 == info.users_.size() ) {
    return;
  }
  std::string stations;
  for ( auto s : info.map_.station() ) {
    stations += "_" + s.station() + "_" ;
  }
  QFile shablon( MnCommon::projectPath() + "share/meteo/odtsamples/" + QString::fromStdString( info.map_.formal().template_() ) );
  QString extension = QFileInfo(shablon).suffix();
  info.filename_ = QString::fromStdString( info.map_.title() + stations + info.map_.datetime() ) + "." + extension;
  bool res = uploadFile(ch);
  if ( false == res ) {
    error_log << QObject::tr("Файл не загружен");
    return;
  }
  sendMessages(ch);
}

void FormalDocSaver::sendMessages( rpc::Channel* ch )
{
  if ( false == clients_.contains(ch) ) {
    return;
  }
  ClientInfo& info = clients_[ch];
  web::InterDialogMessageSend req;
  req.set_username( global::currentUserLogin().toStdString() );
  req.mutable_send_request()->set_text( QObject::tr("Вам отправлен файл.").toStdString());
  req.mutable_send_request()->add_file( info.fileid_.toStdString() );
  auto channel = global::serviceChannel( settings::proto::kInterMessagesService );
  if ( nullptr == channel ) {
    error_log << "nikak";
    return;
  }
  for ( auto u : info.users_ ) {
    req.mutable_send_request()->set_to( u.toStdString() );
    auto resp = channel->remoteCall( &web::InterService::SendDialogMessage, req, this->timeout_ );
    delete resp;
  }
  delete channel;
}

bool FormalDocSaver::uploadFile( rpc::Channel* ch )
{
  if ( false == clients_.contains(ch) ) {
    return false;
  }
  ClientInfo& info = clients_[ch];
  auto channel = global::serviceChannel(settings::proto::kInterTaskService);
  if ( nullptr == channel ) {
    error_log << "not connected TaskService";
    return false;
  }
  if ( nullptr == info.resp_exportmap_ || 0 == info.resp_exportmap_->data().size() ) {
    error_log << "nulevoi razmer";
    delete channel;
    return false;
  }
  QList<QByteArray> chunks = chunksFromFile( QByteArray::fromStdString(info.resp_exportmap_->data()) );
  if ( 0 == chunks.size() ) {
    error_log << "net chukov";
    delete channel;
    return false;
  }
  inter::proto::FileUploadStartRequest startreq;
  startreq.set_file_name( info.filename_.toStdString() );
  auto respstart = channel->remoteCall( &inter::proto::TaskService::FileUploadStart, startreq, this->timeout_ );
  if ( nullptr == respstart ) {
    error_log << "not started";
    delete channel;
    return false;
  }
  int number = 0;
  inter::proto::FileUploadChunkResponce* respchunk = nullptr;
  for ( auto chunk : chunks ) {
    inter::proto::FileUploadChunkRequest chunkrequest;
    chunkrequest.set_upload_id( respstart->upload_id() );
    chunkrequest.set_chunk_id(number);
    chunkrequest.set_data( chunk.data(), chunk.size() );
    respchunk = channel->remoteCall( &inter::proto::TaskService::FileUploadSend, chunkrequest, this->timeout_ );
    if ( nullptr == respchunk || false == respchunk->result() ) {
      delete respchunk;
      delete channel;
      delete respstart;
      return false;
    }
    delete respchunk;
    ++number;
  }
  inter::proto::FileUploadEndRequest endrequest;
  endrequest.set_upload_id( respstart->upload_id() );
  endrequest.set_chunks_total( chunks.size() );
  delete respstart;
  auto respend = channel->remoteCall( &inter::proto::TaskService::FileUploadEnd, endrequest, this->timeout_ );
  if ( nullptr == respend || false == respend->result() ) {
    delete respend;
    delete channel;
    return false;
  }
  info.fileid_ = QString::fromStdString( respend->file_id() );
  delete respend;
  delete channel;
  return true;
}

QList<QByteArray> FormalDocSaver::chunksFromFile( const QByteArray& file )
{
  int chunksz = 8*1024*1024;
  int shift = 0;
  int sz = file.size();
  QList<QByteArray> list;
  while ( shift < sz ) {
    int bricksz = chunksz;
    if ( shift + bricksz > sz ) {
      bricksz = sz - shift;
    }
    QByteArray chunk = QByteArray( file.data() + shift, bricksz );
    list.append(chunk);
    shift += bricksz;
  }
  return list;
}

void FormalDocSaver::slotClientDisconnected( rpc::Channel* ch )
{
//  auto ch = qobject_cast<rpc::Channel*>( sender() );
  if ( nullptr == ch ) {
    error_log << QObject::tr("Сценарий работы прерван с непредвиденной ошибкой. 0 == rpc::Channel. Обратитесь в службу поддержки maslo.team.");
    return;
  }
  if ( false == clients_.contains(ch) ) {
    return;
  }
  auto info = clients_[ch];
  info.clear();
  clients_.remove(ch);
}

void FormalDocSaver::ClientInfo::clear()
{
  delete file_; file_ = nullptr;
  delete resp_createdoc_; resp_createdoc_ = nullptr;
  delete resp_createmap_; resp_createmap_ = nullptr;
  delete resp_exportmap_; resp_exportmap_ = nullptr;
}

void FormalDocSaver::slotDocumentChanged( const QString& filename )
{
  meteo::rpc::Channel* ch = nullptr;
  for ( auto it = clients_.begin(), end = clients_.end(); it != end; ++it ) {
    if ( nullptr != it.value().file_ && filename == it.value().file_->fileName() ) {
      ch = it.key();
      break;
    }
  }
  if ( nullptr == ch ) {
      watcher_->removePath(filename);
      error_log << QObject::tr("Ошибка определения изменяемого на сервере документа");
      return;
  }
  ClientInfo& info = clients_[ch];
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  if ( true == filename.contains("Taylored")) {
    auto map = info.map_;
    info.req_export_map_.set_format( map.formal().format() );
    info.req_export_map_.set_title( map.name() );
    if ( nullptr != info.resp_savedocum_ ) {
      delete info.resp_savedocum_;
      info.resp_savedocum_ = nullptr;
    }
    info.resp_savedocum_ = ch->remoteCall( &meteo::map::proto::DocumentService::SaveDocument, info.req_export_map_, this->timeout_);
    if ( nullptr == info.resp_savedocum_ ) {
      qApp->restoreOverrideCursor();
      QMessageBox::critical(0, QObject::tr("Внимание"), QObject::tr("Не получен ответ от сервиса при сохранении документа."));
    slotClientDisconnected(ch);
      ch->deleteLater();
      return;
    }
    if ( false == info.resp_savedocum_->result() ) {
      qApp->restoreOverrideCursor();
      QMessageBox::critical(0, QObject::tr("Внимание"), QObject::tr("Не удалось сохранить документ в базу."));
    slotClientDisconnected(ch);
      ch->deleteLater();
      return;
    }
  }

  meteo::map::proto::ImportRequest request;

  QFile file;
  file.setFileName( filename );
  file.open( QIODevice::ReadOnly );
  QByteArray filedata = file.readAll();
  file.close();
  if ( nullptr == info.resp_exportmap_ ) {
    info.resp_exportmap_ = new map::proto::ExportResponse;
  }
  info.resp_exportmap_->set_data( filedata.data(), filedata.size() );
  request.set_data( filedata.data(), filedata.size() );
  auto response = ch->remoteCall(&meteo::map::proto::DocumentService::ImportDocument, request, this->timeout_ );
  QApplication::restoreOverrideCursor();

  if ( false == response->result() ) {
    QMessageBox::critical(0, QObject::tr("Внимание"),
                          QObject::tr("При попытке изменить документ произошла ошибка. Описание ошибки = \n\t%1")
                          .arg( QString::fromStdString( response->comment())) );
    delete response;
    slotClientDisconnected(ch);
    ch->deleteLater();
    return;
  }

  delete response;
  if ( 0 != info.users_.size() ) {
    sendFilesToUsers(ch);
  }
}

int FormalDocSaver::setTimeout(int timeout)
{
  int old = this->timeout_;
  this->timeout_ = timeout;
  return old;
}

}
