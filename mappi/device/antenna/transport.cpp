#include "transport.h"


namespace mappi {

namespace antenna {

static const int MAX_BUFFER_SIZE = 8192;  // bytes

Transport::Transport(Protocol* protocol, QObject* parent /*=*/) :
    QObject(parent),
  protocol_(protocol),
  port_(new QSerialPort(this)),
  demoMode_(false)
{
  QObject::connect(port_, &QSerialPort::readyRead, this, &Transport::onRead);
  QObject::connect(port_, &QSerialPort::errorOccurred, this, &Transport::onError);
}

Transport::~Transport()
{
  protocol_ = nullptr;
}

bool Transport::init(const Configuration& conf)
{
  demoMode_ = conf.demoMode;
  param_ = conf.serialPort;

  port_->setPortName(param_.tty);

  return (port_->setBaudRate(param_.baudRate) &&
    port_->setParity(QSerialPort::NoParity) &&
    port_->setDataBits(QSerialPort::Data8) &&
    port_->setStopBits(QSerialPort::OneStop) &&
    port_->setFlowControl(QSerialPort::NoFlowControl));
}

bool Transport::open()
{
  if (port_->isOpen())
    return true;

  if (port_->open(QSerialPort::ReadWrite) == true) {
    port_->clear();

    if (param_.DTR && port_->setDataTerminalReady(param_.DTR))
      info_log << "dtr signal on";

    if (param_.RTS && port_->setRequestToSend(param_.RTS))
      info_log << "rts signal on";

    return true;
  }

  if (demoMode_)
    return true;

  return false;
}

bool Transport::isOpen() const
{
  return port_->isOpen();
}

void Transport::close()
{
  if (param_.DTR && port_->setDataTerminalReady(false))
    info_log << "dtr signal off";

  if (param_.RTS && port_->setRequestToSend(false))
    info_log << "rts signal off";

  port_->close();
}

QString Transport::pinoutSignals() const
{
  QSerialPort::PinoutSignals pin = port_->pinoutSignals();
  QStringList res = {
    QString("no signal: %1").arg(pin & QSerialPort::NoSignal ? "on" : "off"),          // линия не активна
    QString("TxD: %1").arg(pin & QSerialPort::TransmittedDataSignal ? "on" : "off"),   // устарело, transmitted data
    QString("RxD: %1").arg(pin & QSerialPort::ReceivedDataSignal ? "on" : "off"),      // устарело, received data
    QString("DTR: %1").arg(pin & QSerialPort::DataTerminalReadySignal ? "on" : "off"), // готовность терминала, data terminal ready
    QString("DCD: %1").arg(pin & QSerialPort::DataCarrierDetectSignal ? "on" : "off"), // обнаружена несущая, data carrier detect
    QString("DSR: %1").arg(pin & QSerialPort::DataSetReadySignal ? "on" : "off"),      // готовность данных, data set ready
    QString("RNG: %1").arg(pin & QSerialPort::RingIndicatorSignal ? "on" : "off"),     // ring indicator
    QString("RTS: %1").arg(pin & QSerialPort::RequestToSendSignal ? "on" : "off"),     // запрос на отправку, request to send
    QString("CTS: %1").arg(pin & QSerialPort::ClearToSendSignal ? "on" : "off"),       // очистить для отправки, clear to send
    QString("STD: %1").arg(pin & QSerialPort::SecondaryTransmittedDataSignal ? "on" : "off"),  // secondary transmitted data
    QString("SRD: %1").arg(pin & QSerialPort::SecondaryReceivedDataSignal ? "on" : "off")      // secondary received data
  };

  return res.join(", ");
}

bool Transport::send(const QByteArray& buf)
{
  // qDebug() << pinoutSignals();
  int size = buf.size();
  if (port_->isOpen() && port_->write(buf.data(), size) == size) {
    debug_log << "TX:" << (protocol_->isText() ? buf : buf.toHex()) << ", size:" << size;
    return true;
  }

  if (demoMode_)
    return true;

  error_log << "TX: failed";

  return false;
}

void Transport::onRead()
{
  if (port_->bytesAvailable() == 0)
    return ;

  buf_rx_.append(port_->readAll());
  // debug_log << "RX:" << buf_rx_.size() << (protocol_->isText() ? buf_rx_ : buf_rx_.toHex());

  int mtu = protocol_->mtu();
  while (mtu <= buf_rx_.size()) {
    if (protocol_->split(buf_rx_) &&
       protocol_->split(buf_rx_, mtu)) {
      QByteArray pack = buf_rx_.left(mtu);
      // debug_log << "RX:" << (protocol_->isText() ? pack : pack.toHex());

      emit recv(pack);  // emit signal
      buf_rx_.remove(0, mtu);

      continue ;
    }

    buf_rx_.remove(0, 1);
  }

  if (MAX_BUFFER_SIZE < buf_rx_.size()) {
    debug_log << buf_rx_.toHex();
    warning_log << QObject::tr("RX: переполнение буфера, буфер будет очищен");
    buf_rx_.clear();
  }
}

void Transport::onError(QSerialPort::SerialPortError error)
{
  static const QMap<QSerialPort::SerialPortError, QString> dict = {
    { QSerialPort::DeviceNotFoundError, "An error occurred while attempting to open an non-existing device." },
    { QSerialPort::PermissionError, "An error occurred while attempting to open an already opened device by another process or a user not having enough permission and credentials to open." },
    { QSerialPort::OpenError, "An error occurred while attempting to open an already opened device in this object." },
    { QSerialPort::NotOpenError, "This error occurs when an operation is executed that can only be successfully performed if the device is open. This value was introduced in QtSerialPort 5.2." },
    { QSerialPort::ParityError, "Parity error detected by the hardware while reading data. This value is obsolete. We strongly advise against using it in new code." },
    { QSerialPort::FramingError, "Framing error detected by the hardware while reading data. This value is obsolete. We strongly advise against using it in new code." },
    { QSerialPort::BreakConditionError, "Break condition detected by the hardware on the input line. This value is obsolete. We strongly advise against using it in new code." },
    { QSerialPort::WriteError, "An I/O error occurred while writing the data." },
    { QSerialPort::ReadError, "An I/O error occurred while reading the data." },
    { QSerialPort::ResourceError, "An I/O error occurred when a resource becomes unavailable, e.g. when the device is unexpectedly removed from the system." },
    { QSerialPort::UnsupportedOperationError, "The requested device operation is not supported or prohibited by the running operating system." },
    { QSerialPort::TimeoutError, "A timeout error occurred. This value was introduced in QtSerialPort 5.2." },
    { QSerialPort::UnknownError, "An unidentified error occurred." }
  };

  error_log << error << ": " << dict.value(error, "Unknown error");
}

}

}
