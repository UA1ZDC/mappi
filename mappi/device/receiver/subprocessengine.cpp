#include "subprocessengine.h"
#include <cross-commons/app/paths.h>


namespace mappi {

namespace receiver {

SubProcessEngine::SubProcessEngine(QObject* parent /*=*/) :
    Engine(parent),
  app_(new QProcess(this))
{
  QObject::connect(app_, &QProcess::readyReadStandardOutput, this, &SubProcessEngine::fromOutput);
  QObject::connect(app_, &QProcess::readyReadStandardError, this, &SubProcessEngine::fromError);
}

SubProcessEngine::~SubProcessEngine()
{
}

bool SubProcessEngine::configure(const Configuration& conf)
{
  if (conf.appName.isEmpty()) {
    error_log << QObject::tr("Не задано приложение для запуска в дочернем процессе");
    return false;
  }

  appPath_ = QString("%1/%2")
    .arg(MnCommon::pythonPath())
    .arg(conf.appName);

  return true;
}

bool SubProcessEngine::start(Satellite* satellite, const Parameters& param)
{
  if (app_->state() == QProcess::Running)
    return true;

  QStringList args;
  args << QString("-f %1").arg(param.freq)
       << QString("-n %1").arg(satellite->name());

  info_log << "app:" << appPath_ << QObject::tr("%1").arg(args.join(' '));
  app_->start(appPath_, args);

  return app_->waitForStarted();
}

bool SubProcessEngine::stop()
{
  if (app_->state() == QProcess::NotRunning)
    return true;

  app_->write("quit\n");      // TODO ???
  app_->terminate();

  return app_->waitForFinished();
}

void SubProcessEngine::updateFreq(float freq)
{
  QString cmd = QString("freq = %1\n").arg(freq);
  app_->write(cmd.toUtf8());
}

void SubProcessEngine::fromOutput()
{
  debug_log << app_->readAllStandardOutput();
}

void SubProcessEngine::fromError()
{
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
