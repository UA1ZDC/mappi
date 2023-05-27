#include <dumpengine.h>
#include <QTimerEvent>


namespace mappi {

namespace receiver {

DumpEngine::DumpEngine(QObject* parent /*=*/) :
    Engine(parent),
  src_(new QFile(this)),
  dst_(new QFile(this)),
  buf_(new QByteArray),
  timerId_(-1)
{
}

DumpEngine::~DumpEngine()
{
  delete buf_;
  buf_ = nullptr;
}

bool DumpEngine::configure(const Configuration& conf)
{
  dst_->setFileName(conf.rawFile);
  return true;
}

bool DumpEngine::start(Satellite* satellite, const Parameters& param)
{
  Parameters::dump_t dump = param.dump;
  if (!dump.isUsed) {
    error_log << QObject::tr("Не заданы dump параметры, спутник: %1").arg(satellite->name());
    return false;
  }

  buf_->resize(((dump.bitrate * 1000) / 8) / dump.balancer);
  debug_log << QString("dump file:%1").arg(dump.file);
  debug_log << QString("dump(bitrate:%2, balancer:%3)")
    .arg(dump.bitrate)
    .arg(dump.balancer);

  src_->setFileName(dump.file);
  if (src_->open(QIODevice::ReadOnly) == false) {
    error_log << QObject::tr("Не удалось открыть dump file: %1").arg(dump.file);
    return false;
  }

  // StreamHeader пишется в начало файла сервисом, который этот файл и подготавливает
  if (dst_->open(QIODevice::WriteOnly | QIODevice::Append) == false) {
    error_log << QObject::tr("Не удалось открыть raw file: %1").arg(dst_->fileName());
    return false;
  }

  timerId_ = startTimer((1000 / dump.balancer), Qt::PreciseTimer);

  return true;
}

bool DumpEngine::stop()
{
  if (timerId_ != -1)
    killTimer(timerId_);

  src_->close();
  dst_->close();

  return true;
}

void DumpEngine::updateFreq(float freq)
{
  Q_UNUSED(freq)
}

void DumpEngine::timerEvent(QTimerEvent* event)
{
  if (event->timerId() == timerId_) {
    qint64 bytes = src_->read(buf_->data(), buf_->size());
    // debug_log << "RX/TX (bytes)" << bytes;
    if (0 < bytes)
      dst_->write(buf_->data(), bytes);
  }
}

}

}
