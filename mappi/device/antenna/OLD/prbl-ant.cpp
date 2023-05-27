#include "prbl-ant.h"

#define  _TTY_POSIX_

#include <cross-commons/app/paths.h>
#include <commons/textproto/tprototext.h>
#include <qdebug.h>

#define ANTCONF_FILE MnCommon::etcPath("mappi") + "antenna.conf"

using namespace mappi;
using namespace receive;

//возвращает двоичное представление числа (для отладки)
QString db(uint8_t value)
{
  QString value_binary("\0");
  if (1 != value && 0 != value) {
    value_binary = db(value / 2);
  }

  char a = value % 2 + 48;
  return value_binary += a;
}


PrblAnt::PrblAnt(conf::AntennaType type, QObject* parent /*=*/) :
    QObject(parent),
  type_(type),
  conf_(new conf::AntennaParam),
  isOpen_(false),
  port_(new QSerialPort(this)),
  azCorr_(0),
  elCorr_(0),
  dts_(false),
  rts_(false),
  state_(0),
  isDiagnostics_(false),
  lengthPack_(0)
{
  // port_->setBaudRate(QSerialPort::Baud115200); //Baud2400
  // port_->setParity(QSerialPort::NoParity);
  // port_->setDataBits(QSerialPort::Data8);
  // port_->setStopBits(QSerialPort::OneStop);
  // port_->setFlowControl(QSerialPort::NoFlowControl);

  QObject::connect(port_, &QSerialPort::dataTerminalReadyChanged, this, &PrblAnt::slotDtsChanged);
  QObject::connect(port_, &QSerialPort::requestToSendChanged, this, &PrblAnt::slotRtsChanged);

  readConfig(type);
}

PrblAnt::~PrblAnt()
{
  // float val1, val2;
  // stopPosition(&val1, &val2);

  if (isOpen_) {
    setPower(false);
    closePort();
  }

  delete conf_;
  conf_ = nullptr;
}

//! Открытие порта
/*!
  \param name Полный путь к файлк (напр, /dev/ttyUSB0)
*/
bool PrblAnt::openPort(const QString& name)
{
  trc;
  port_->close();
  port_->setPortName(name);
  port_->setBaudRate(QSerialPort::Baud115200); //Baud2400
  port_->setParity(QSerialPort::NoParity);
  port_->setDataBits(QSerialPort::Data8);
  port_->setStopBits(QSerialPort::OneStop);
  port_->setFlowControl(QSerialPort::NoFlowControl);

  isOpen_ = port_->open(QSerialPort::ReadWrite);
  return isOpen_;
}

void PrblAnt::closePort()
{
  stop();
  port_->close();
  isOpen_ = false;
}

//! Управление питанием
bool PrblAnt::setPower(bool v)
{
  if (v != port_->isDataTerminalReady()) {
    if (false == port_->setDataTerminalReady(v)) {
      error_log << "Ошибка установки DTR";
      return false;
    }
  }

  if (v != port_->isRequestToSend()) {
    if (false == port_->setRequestToSend(v)) {
      error_log << "Ошибка установки RTS";
      return false;
    }
  }

  return true;
}

float PrblAnt::azMin() const
{
  return (isDiagnostics_ ? conf_->az_min_tech() : conf_->az_min());
}

float PrblAnt::azMax() const
{
  return (isDiagnostics_ ? conf_->az_max_tech() : conf_->az_max());
}

float PrblAnt::zeroAzimut() const
{
  return (isDiagnostics_ ? 0 : azCorr_);
}

bool PrblAnt::isAzimutValid(float angle, float zeroCorrect) const
{
  return (((angle + zeroCorrect) >= conf_->az_min()) &&
          ((angle + zeroCorrect) <= conf_->az_max()));
}

float PrblAnt::elMin() const
{
  return (isDiagnostics_ ? conf_->el_min_tech() : conf_->el_min());
}

float PrblAnt::elMax() const
{
  return (isDiagnostics_ ? conf_->el_max_tech() : conf_->el_max());
}

float PrblAnt::zeroElevat() const
{
  return (isDiagnostics_ ? 0 : elCorr_);
}

bool PrblAnt::isElevatValid(float angle, float zeroCorrect) const
{
  return (((angle + zeroCorrect) >= conf_->el_min()) &&
          ((angle + zeroCorrect) <= conf_->el_max()));
}

bool PrblAnt::readConfig(conf::AntennaType type)
{
  conf::AntennaCollect iconf;
  if (!TProtoText::fromFile(ANTCONF_FILE, &iconf)) {
    iconf.Clear();
    error_log << QObject::tr("ошибка загрузки конфигурации: %1").arg(ANTCONF_FILE);
    return false;
  }

  var(type);
  for (int i = 0; i < iconf.antenna_size(); ++i) {
    const conf::AntennaParam& item = iconf.antenna(i);
    if (item.type() == type) {
      conf_->CopyFrom(item);
      // qDebug() << "az_min_tech :" << item.az_min_tech();
      // qDebug() << "az_max_tech :" << item.az_max_tech();
      // qDebug() << "az_min :" << item.az_min();
      // qDebug() << "az_max :" << item.az_max();
      // qDebug() << "el_min_tech :" << item.el_min_tech();
      // qDebug() << "el_max_tech :" << item.el_max_tech();
      // qDebug() << "el_min :" << item.el_min();
      // qDebug() << "el_max :" << item.el_max();

      return true;
    }
  }

  error_log << QObject::tr("не найдены аттрибуты антенны типа %1").arg(type);
  return false;
}

//--------- асинхронное чтение
void PrblAnt::startAsyncRead(const QByteArray& start, const QByteArray& stop, int length)
{
  startPack_ = start;
  stopPack_ = stop;
  lengthPack_ = length;

  connect(port_, &QSerialPort::readyRead, this, &PrblAnt::handleReadyRead);
  connect(port_, &QSerialPort::errorOccurred, this, &PrblAnt::handleError);
}

void PrblAnt::stopAsyncRead()
{
  disconnect(port_, &QSerialPort::readyRead, this, &PrblAnt::handleReadyRead);
  disconnect(port_, &QSerialPort::errorOccurred, this, &PrblAnt::handleError);
}

//------- синхронное чтение
bool PrblAnt::writeBytes(const QByteArray& cmd)
{
  int nb = cmd.size();
  const char* data = cmd.data();

  while ( 0 != nb ) {
    int res = port_->write(data, nb );
    if ( 0 > res ) {
      return false;
    }

    if ( false == port_->waitForBytesWritten(1000) ) {
      error_log << "Ошибка ожидания данных для записи";
    }

    nb -= res;
    data += res;
    //error_log << QObject::tr("Ошибка записи данных, число записанных байт = %1").arg(nb);
  }

  return true;
}

bool PrblAnt::readBytes(QByteArray* cmd)
{
  if (nullptr == cmd) {
    return false;
  }

  //int msecs = 5000;
  int msecs = 160;
  bool ready = false;
  while (0 < msecs && false == ready) {
    ready  = port_->waitForReadyRead(40);
    msecs -= 40;
  }

  while (true == port_->waitForReadyRead(100)) {
  }

  int nb = port_->bytesAvailable();

  if (nb < 3) {
    error_log << QObject::tr("Количество считанных байт = %1, ожидаемое = 3").arg(nb);
    state_ = 1; // TODO пока так
    return false;
  }

  cmd->resize(nb);
  char* data = cmd->data();
  while (0 != nb) {
    int res = port_->read(data, nb);
    if (0 > res) {
      state_ = 1; // TODO пока так
      return false;
    }
    nb -= res;
    data += res;
  }

  state_ = 0;

  return true;
}

void PrblAnt::slotDtsChanged(bool fl)
{
  dts_ = fl;
}

void PrblAnt::slotRtsChanged(bool fl)
{
  rts_ = fl;
}

void PrblAnt::handleReadyRead()
{
  // очищаем старый
  if (pack_.size() >= (lengthPack_ * 3)) {
    pack_.clear();
  }

  pack_.append(port_->readAll());

  // var(_pack);
  // проверяем начало пакета
  if (pack_.size() > startPack_.size()) {
    int pos = pack_.indexOf(startPack_);
    if (pos == -1) {
      debug_log << pos << pack_.size();
      return;
    }
    pack_ = pack_.mid(pos);
  }

  // проверяем конец пакета
  if (pack_.size() >= lengthPack_) {
    int pos = pack_.indexOf(stopPack_);
    if (pos != lengthPack_ - stopPack_.size()) {
      warning_log << QObject::tr("Ошибка пакета") << pack_.size() << pack_ << pos;
      pack_.clear();
      return;
    }

    // var(_pack);
    emit packetReady(pack_.left(lengthPack_));

    if (pack_.size() > lengthPack_) {
      pack_ = pack_.mid(lengthPack_);
    } else {
      pack_.clear();
    }
  }
}

void PrblAnt::handleError(QSerialPort::SerialPortError error)
{
  if (error == QSerialPort::ReadError) {
    error_log << QObject::tr("Ошибка чтения данных порта: %2")
                   .arg(port_->errorString());
  }
}
