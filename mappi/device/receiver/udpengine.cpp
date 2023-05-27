#include "udpengine.h"
#include <qnetworkdatagram.h>


namespace mappi {

namespace receiver {

UdpEngine::UdpEngine(QObject* parent /*=*/) :
    Engine(parent),
  socket_(new QUdpSocket(this)),
  file_(new QFile(this)),
  bytesLost_(0)
{
  QObject::connect(socket_, &QUdpSocket::readyRead, this, &UdpEngine::onRecv);
}

UdpEngine::~UdpEngine()
{
}

bool UdpEngine::configure(const Configuration& conf)
{
  if (conf.host.isNull()) {
    error_log << QObject::tr("Не задан host ip");
    return false;
  }

  file_->setFileName(conf.rawFile);

  return socket_->bind(conf.host, conf.port);
}

bool UdpEngine::start(Satellite* satellite, const Parameters& param)
{
  Q_UNUSED(satellite)
  Q_UNUSED(param)

  if (file_->isOpen())
    return true;

  return file_->open(QIODevice::WriteOnly);
}

bool UdpEngine::stop()
{
  if (!file_->isOpen())
    return true;

  debug_log << QObject::tr("tx, bytes lost: %1").arg(bytesLost_);
  bytesLost_ = 0;

  file_->close();

  return true;
}

void UdpEngine::updateFreq(float freq)
{
  Q_UNUSED(freq)
}

void UdpEngine::onRecv()
{
  while (socket_->hasPendingDatagrams()) {
    QByteArray buf = socket_->receiveDatagram().data();
    int size = buf.size();
    // debug_log << QObject::tr("tx, buffer size: %1").arg(size);
    if (file_->isOpen())
      bytesLost_ += size - file_->write(buf.data(), size);
  }
}

}

}
