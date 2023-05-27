#include "fileserviceclient.h"

namespace mappi {

FileServiceClient::FileServiceClient( QObject* parent )
  : QObject(parent),
    channel_(nullptr),
    output_(new QFile(this)),
    totalReceivedBytes_(0)
{

}

FileServiceClient::~FileServiceClient()
{
  delete channel_; channel_ = 0;
  delete output_; output_ = 0;
}

void FileServiceClient::answerReceived(proto::ExportResponse* resp)
{
  QString respResult = QString::fromStdString(resp->comment());
  QString fileName = QString::fromStdString(resp->filename());

  if (respResult == "not found") {
    error_log << QObject::tr("Ошибка. Файл %1 не найден на источнике").arg(fileName);
    emit fileNotFound();
  }
  else if (respResult == "found") {
    receiveStartChunk(resp);
    if (dataChunks_.isEmpty() == false) {
      writeDataChunks();
    }
  }
  else if (respResult == "data") {
    receiveDataChunk(resp);
    if (startChunk_.isNull() == false) {
      writeDataChunks();
    }
  }
  else if (respResult == "ok") {
    receiveFinishChunk(resp);
    if (totalReceivedBytes_ >= resp->file_size()) {
      writeFinishChunk();
    }
    delete resp;
  }
  else if (respResult == "error") {
    output_->close();
    error_log << QObject::tr("Ошибка при передаче файла %1").arg(fileName);
    if (resp->finish() == true) {
      emit fileResponseFinish(FileServiceResponse(clientFileName_, resp->file_size(), false));
      debug_log << QObject::tr("Конец передачи %1 [ %2 ]").arg(fileName).arg(respResult);
      delete resp;
    }
  }
}

bool FileServiceClient::requestFile(const QString &filename, bool rewrite)
{
  if ( channel_ == nullptr ) {
    error_log << QObject::tr("Нет подключения к сервису");
    return false;
  }

  if( filename.isEmpty() == true ) {
    error_log << QObject::tr("Неправильно указано имя файла");
    return false;
  }

  clientFileName_ = filename;
  QFileInfo info(clientFileName_);
  QString fileName(info.fileName());
  QString path(info.path());
  //файл сохраняется по такому же пути, что указан в базе, но со сменой пользователя на текущего
  QDir home = QDir::home();
  QString homePath("home/");
  path = path.right(path.size() - (path.indexOf(homePath) + homePath.size()));
  path = path.right(path.size() - path.indexOf("/"));
  path = home.absolutePath() + path;
  clientFileName_ = path + '/' + fileName;
  QFile file(clientFileName_);
  if (!rewrite && file.exists()) {
    debug_log << "Файл уже существует" << clientFileName_;
    success_ = true;
    emit fileResponseFinish(FileServiceResponse(clientFileName_, file.size(), true));
    return true;
  }
  proto::ExportRequest req;
  req.set_path(filename.toStdString());

  channel_->subscribe(&proto::FileService::ExportFile,
		      req, this, &FileServiceClient::answerReceived);
  return true;
}

void FileServiceClient::receiveStartChunk(const proto::ExportResponse* chunk)
{
  startChunk_ = QSharedPointer<proto::ExportResponse>(new proto::ExportResponse(*chunk));

  QFileInfo info(clientFileName_);
  QString path = info.path();
  QString fileName = info.fileName();
  QDir dir;
  if ( !dir.exists(path) ) {
    dir.mkpath(path);
  }
  output_->setFileName(clientFileName_);
  if (chunk->start() == true) {
    emit fileResponseStart(FileServiceResponse(clientFileName_, chunk->file_size(), true));
    debug_log << QObject::tr("Начало передачи %1").arg(fileName);
  }

  if (output_->open(QIODevice::WriteOnly) == true) {
    output_->resize(chunk->file_size());
    output_->flush();
    emit complete(clientFileName_, chunk->complete());
  }
  else {
    error_log << QString::fromUtf8("Ошибка. Файл %1: %2").arg(output_->fileName()).arg(output_->errorString());
  }
}

void FileServiceClient::receiveDataChunk(const proto::ExportResponse* chunk)
{
  QSharedPointer<proto::ExportResponse> chunkCopy(new proto::ExportResponse(*chunk));
  if (chunkCopy.isNull() == false) {
    dataChunks_.append(chunkCopy);
  }
}

void FileServiceClient::receiveFinishChunk(const proto::ExportResponse* chunk)
{
  finishChunk_ = QSharedPointer<proto::ExportResponse>(new proto::ExportResponse(*chunk));
}

void FileServiceClient::writeDataChunks()
{
  foreach (const QSharedPointer<proto::ExportResponse>& chunk, dataChunks_) {
    QString fileName = QString::fromStdString(chunk->filename());
    int64_t dataSize = chunk->data_size();
    QByteArray chunkData(chunk->data().data(), static_cast<int>(dataSize));

    output_->seek(chunk->chunk_num() * chunk->chunk_size());
    output_->write(chunkData, dataSize);
    output_->flush();

    totalReceivedBytes_ += dataSize;
    emit complete(clientFileName_, (100 * totalReceivedBytes_ / chunk->file_size()));
  }
  dataChunks_.clear();

  if (totalReceivedBytes_ >= startChunk_->file_size()) {
    writeFinishChunk();
  }
}

void FileServiceClient::writeFinishChunk()
{
  if (finishChunk_.isNull() == false) {
    output_->close();
    QString fileName = QString::fromStdString(finishChunk_->filename());
    if (finishChunk_->finish() == true) {
      emit fileResponseFinish(FileServiceResponse(clientFileName_, finishChunk_->file_size(), true));
      debug_log << QObject::tr("Конец передачи %1 [ %2 ]")
                   .arg(fileName).arg(QString::fromStdString(finishChunk_->comment()));
      success_ = true;
    }
  }
}

bool FileServiceClient::conenctToFileService()
{
  channel_ = meteo::global::serviceChannel( meteo::settings::proto::kFile );
  if ( 0 == channel_) {
    error_log << meteo::msglog::kServiceConnectFailedSimple.arg(meteo::settings::proto::kFile);
    return false;
  }
  return true;
}

void FileServiceClient::slotRun()
{
  lastActive_ = QDateTime::currentDateTime();

  if ( !conenctToFileService() ) {
    error_log << "Не удалось подключиться к файловому сервису";
    emit connectionFailure();
    return;
  }

  connect( this, &FileServiceClient::complete, [=](const QString& fn, int p){ debug_log << p << fn; this->lastActive_ = QDateTime::currentDateTime(); } );

  QString name = clientFileName_;
  if ( false == requestFile(name, rewrite_) ) {
    error_log << "Ошибка запроса файла";
    emit requestFailure();
    return;
  }

  startTimer(2000);
}

void FileServiceClient::timerEvent(QTimerEvent*)
{
  QDateTime dt = QDateTime::currentDateTime();

  if ( lastActive_.secsTo(dt) > 30000 ) {
    emit responseTimeout();
  }
}

} // mappi
