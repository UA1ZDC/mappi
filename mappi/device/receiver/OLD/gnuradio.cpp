#include "gnuradio.h"
#include <cross-commons/debug/tlog.h>
#include <QTimerEvent>
#include <QtDebug>


namespace mappi {

namespace receiver {

// static const int READER_TIMEOUT = 100;
static const int ADJUST_FREQ_TIMEOUT = 5000;


// class FileReader
// FileReader::FileReader(const QString& file, QObject* parent /*=*/) :
//     QObject(parent),
//   file_(new QFile(file, this)),
//   timerId_(-1)
// {
// }
/*
FileReader::~FileReader()
{
}

bool FileReader::open()
{
  if (file_->open(QIODevice::ReadOnly)) {
    timerId_ = startTimer(READER_TIMEOUT);
    return true;
  }

  return false;
}

void FileReader::close()
{
  file_->close();
  killTimer(timerId_);
  timerId_ = -1;
}

void FileReader::timerEvent(QTimerEvent* event)
{
  if (event->timerId() != timerId_)
    return ;

  QByteArray buf = file_->readAll();
  if (!buf.isEmpty()) {
    qDebug() << "tx:" << buf.length();
    emit read(buf);
  }
}
*/


// class GnuRadio
const char* GnuRadio::SCRIPT_STUB = "mappi_rxstub.py";

// GnuRadio::GnuRadio(const QString& rawFile, QObject* parent /*=*/) :
GnuRadio::GnuRadio(QObject* parent /*=*/) :
    QObject(parent),
  // reader_(new FileReader(rawFile, this)),
  pyapp_(new QProcess(this)),
  satellite_(nullptr),
  timerId_(-1)
{
  QObject::connect(pyapp_, &QProcess::readyReadStandardOutput, this, &GnuRadio::fromOutput);
  QObject::connect(pyapp_, &QProcess::readyReadStandardError,  this, &GnuRadio::fromError);
  // QObject::connect(reader_, &FileReader::read,  this, &GnuRadio::readyRead);
}

GnuRadio::~GnuRadio()
{
  stop();
  satellite_ = nullptr;
}

bool GnuRadio::start(const QString& script, Satellite* satellite, const GnuRadio::param_t& param)
{
  if (pyapp_->state() == QProcess::Running)
    return true;

  satellite_ = satellite;
  param_ = param;

  // adjust_freq
  if (timerId_ != -1)
    killTimer(timerId_);

  QStringList args;
  args << QString("-f %1").arg(param_.freq)
       << QString("-r %1").arg(param_.rate)
       << QString("-g %1").arg(param_.gain)
       << QString("-n %1").arg(satellite_->name());

  pyapp_->start(script, args);
  if (pyapp_->waitForStarted() == false)
    return false;

  // adjust_freq
  if (param.adjust_freq)
    timerId_ = startTimer(ADJUST_FREQ_TIMEOUT);

  // return reader_->open();
  return true;
}

void GnuRadio::timerEvent(QTimerEvent* event)
{
  if (event->timerId() != timerId_)
    return ;

  qDebug() << pyapp_->state();
  if (pyapp_->state() == QProcess::Running) {
    Coords::TopoCoord topo;
    bool ok = satellite_->getPosition(QDateTime::currentDateTimeUtc(), param_.site, &topo);
    if (ok) {
      static const uint64_t kc = 299792458;     // скорость света
      float freq = param_.freq * (1 - topo.rangeRate * 1000 / kc);
      debug_log << "adjust_freq =" << (freq / 1e6);
/*
      QString cmd = QString("freq = %1\n").arg(freq);
      pyapp_->write(cmd.toUtf8());
*/
    } else
      warning_log << QObject::tr("Ошибка получения радиальной скорости спутника");

    return ;
  }

  if (timerId_ != -1)
    killTimer(timerId_);
}

bool GnuRadio::stop()
{
  if (pyapp_->state() == QProcess::NotRunning)
    return true;

  // reader_->close();

  pyapp_->write("quit\n"); // TODO в заглушке это не обрабатывается, в реальном тоже закомментировано
  pyapp_->terminate();
  if (pyapp_->waitForFinished()) {
    if (timerId_ != -1)
      killTimer(timerId_);

    return true;
  }

  return false;
}

void GnuRadio::fromOutput()
{
  debug_log << pyapp_->readAllStandardOutput();
}

void GnuRadio::fromError()
{
  error_log << pyapp_->readAllStandardError();

  // QRegExp rxo("Ctrl Output:([\\w\\s]+)\n");
  // int pos = 0;
  // while ((pos = rxo.indexIn(out, pos)) != -1) {
  //   debug_log << QObject::tr("%1").arg(rxo.cap(1).trimmed());
  //   pos += rxo.matchedLength();
  // }
}

}

}
