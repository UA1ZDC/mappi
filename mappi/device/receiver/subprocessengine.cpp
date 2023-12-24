#include "subprocessengine.h"
#include <cross-commons/app/paths.h>
#include <QFileInfo>
#include <QDir>

namespace mappi {
  namespace receiver {

    SubProcessEngine::SubProcessEngine(QObject *parent /*=*/) : Engine(parent), app_(new QProcess(this)) {
      QObject::connect(app_, &QProcess::readyReadStandardOutput, this, &SubProcessEngine::fromOutput);
      QObject::connect(app_, &QProcess::readyReadStandardError, this, &SubProcessEngine::fromError);
      networkManager_ = new QNetworkAccessManager(this);
    }

    SubProcessEngine::~SubProcessEngine() {}

    void SubProcessEngine::deleteOldFiles(const QString &directoryPath) {
      QDir directory(directoryPath);
      if (!directory.exists())  return;

      QStringList filters;
      filters << "*.*";

      directory.setNameFilters(filters);
      directory.setFilter(QDir::Files | QDir::NoDotAndDotDot);

      QFileInfoList fileList = directory.entryInfoList();

      QDateTime currentTime = QDateTime::currentDateTime();
      QDateTime threeHoursAgo = currentTime.addSecs(-3 * 60 * 60); // 3 hours in seconds

      for(const QFileInfo &fileInfo : fileList) {
        QDateTime fileLastModified = fileInfo.lastModified();

        if (fileLastModified < threeHoursAgo) {
          if (!QFile::remove(fileInfo.filePath())) {
            warning_log << "Failed to delete file:" << fileInfo.fileName();
          }
        }
      }
    }

    bool SubProcessEngine::configure(const Configuration &conf) {
      if (conf.appPath.isEmpty()) {
        warning_log << QObject::tr("Не задана папка для скриптов, запускаемых в дочернем процессе");
        return false;
      }
      appFolder_ = conf.appPath;
      rawFile_ = conf.rawFile;
      requestURL_ = QUrl(XMLRPCServer.arg(conf.host.toString()).arg(conf.port));
      separateHeader_ = conf.separateHeader;
      return true;
    }

    bool SubProcessEngine::start(Satellite *satellite, const Parameters &param) {
      QString actualFile = rawFile_;
      QFileInfo rawFileInfo(actualFile);
      while (rawFileInfo.isSymLink()) {
        info_log << QObject::tr("Following symlink: %1 -> %2").arg(rawFileInfo.symLinkTarget());
        actualFile = rawFileInfo.symLinkTarget();
        rawFileInfo = QFileInfo(actualFile);
      }

      if (!QFileInfo::exists(actualFile)) {
        error_log << QObject::tr("File does not exist: %1").arg(actualFile);
        return false;
      }

      deleteOldFiles(rawFileInfo.absolutePath());

      if (separateHeader_) {
        QDir::root().rename(actualFile, actualFile + ".header");
      }

      QString absolutePath = QFileInfo(QString("%1/%2").arg(appFolder_, param.script)).canonicalFilePath();
      QStringList args;
      args << "-n" << satellite->name()
           << "-f" << QString::number(param.freq)
           << "-g" << QString::number(param.gain)
           << "-r" << QString::number(param.rate)
           << "-p" << actualFile;

      info_log << "app:" << absolutePath << QObject::tr("%1").arg(args.join(' '));
      app_->start(absolutePath, args);

      return app_->waitForStarted();
    }

    bool SubProcessEngine::stop() {
      if (app_->state() == QProcess::NotRunning)
        return true;
      app_->terminate();
      return app_->waitForFinished();
    }

    void SubProcessEngine::changeVariable(const QString name, const double value){
      QNetworkRequest request(requestURL_);
      request.setRawHeader("User-Agent", "Python-xmlrpc/3.9");
      request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml");

      QString data = XMLRPC_Request.arg(name).arg(value);

      //Не дожидаемся и не обрабатываем ответ
      //connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));
      networkManager_->post(request, data.toUtf8());
    }

    void SubProcessEngine::updateFreq(const float freq) {
      debug_log << QObject::tr("Обновление частоты приема: %1").arg(freq);
      changeVariable("var_center_freq", freq);
    }

    void SubProcessEngine::fromOutput() {
      debug_log << app_->readAllStandardOutput();
    }

    void SubProcessEngine::fromError() {
      error_log << app_->readAllStandardError();

      // QRegExp rxo("Ctrl Output:([\\w\\s]+)\n");
      // int pos = 0;
      // while ((pos = rxo.indexIn(out, pos)) != -1) {
      //   debug_log << QObject::tr("%1").arg(rxo.cap(1).trimmed());
      //   pos += rxo.matchedLength();
      // }
    }
  }
}
