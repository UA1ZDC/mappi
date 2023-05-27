#include "filegetter.h"

namespace mappi {

FileGetter::FileGetter(QObject* parent) : QObject(parent) {}

FileGetter::~FileGetter() {}

bool FileGetter::isWorking() { return working_; }

void FileGetter::setFilename(const QString& fileName) { gettingFilename_ = fileName; }

QString FileGetter::getFilename() { return gettedFilename_; }

void FileGetter::run() {
  working_ = true;
  FileServiceClient* client = new FileServiceClient();
  if(!client->conenctToFileService()) {
    QTimer::singleShot(10000, this, &FileGetter::run);
    return;
  }
  gettedFilename_.clear();
  connect(client, &FileServiceClient::fileResponseFinish, this, &FileGetter::gettedFile);
  connect(client, &FileServiceClient::fileResponseFinish, client, &FileServiceClient::deleteLater);
  client->requestFile(gettingFilename_);
}

void FileGetter::gettedFile(const FileServiceResponse &response)
{
  gettedFilename_  = response.filename;

  Q_EMIT(complete(response.filename));

  working_ = false;
  Q_EMIT(finished());
}

} // mappi
