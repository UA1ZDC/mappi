#include "vkaprotocol.h"
#include <qregexp.h>

#define DEBUG_PROTOCOL

namespace mappi {

namespace antenna {

static const QString DLE("5555");
static const QString ETX("\r\n");

#ifndef DEBUG_PROTOCOL
static const QString RESPONSE_PATTERN("^(5555)(\\d{2})(\\d{4})([+-]\\d{5})([+-]\\d{5})(\\d{4})(.*)$");
static const int PAYLOAD = 22;
#else
static const QString RESPONSE_PATTERN("^(5555)(\\d{2})(\\d{4})([+-]\\d{5})([+-]\\d{5})([+-]\\d{5})([+-]\\d{5})(\\d{4})(.*)$");
static const int PAYLOAD = 34;
#endif

int VkaProtocol::mtu() const
{
#ifndef DEBUG_PROTOCOL
    return 30;
#else
    return 42;
#endif

}


VkaProtocol::VkaProtocol() :
  seq_(0)
{
}

VkaProtocol::~VkaProtocol()
{
}

QByteArray VkaProtocol::pack(int id, float azimut, float elevat)
{
  if (seq_ == 100)
    seq_ = 0;

  QString payload = QString("%1%2%3%4%5")
                    .arg(seq_++, 2, 10, QChar('0'))     // seq пакета
                    .arg(id, 2, 10, QChar('0'))         // id команды
                    .arg(0, 6, 10, QChar('0'))          // данные 1
                    .arg(setAngle(azimut))              // данные 2
                    .arg(setAngle(elevat));             // данные 3

  return QString("%1%2%3%4")
      .arg(DLE)                           // "5555"
      .arg(payload)                       // полезная нагрузка
      .arg(crc(payload))                  // LRC
      .arg(ETX)                           // "\r\n"
      .toLocal8Bit();
}

QByteArray VkaProtocol::packAzimut(int id, float v)
{
  // WARNING протокол не поддерживает асинхронное выполнение
  Q_UNUSED(id)
  Q_UNUSED(v)

  return QByteArray();
}

QByteArray VkaProtocol::packElevat(int id, float v)
{
  // WARNING протокол не поддерживает асинхронное выполнение
  Q_UNUSED(id)
  Q_UNUSED(v)

  return QByteArray();
}

bool VkaProtocol::unpack(const QByteArray& buf, Drive* azimut, Drive* elevat, QString* error /*=*/)
{
  QString str(buf);

  int pos = 0;
  QRegExp rx(RESPONSE_PATTERN);
  if ((rx.indexIn(str, pos)) == -1) {
    if (error) *error = QObject::tr("format fail");
    return false;
  }

  QString state = rx.cap(3);
  quint16 state_ctrl = state.leftRef(2).toInt(nullptr, 16);

  // quint16 state_cmd = state.right(2).toInt(nullptr, 16);
  // qDebug() << "seq:   " << rx.cap(2);
  // qDebug() << "state:" << state_ctrl << state_cmd;
  // qDebug() << "azimut:" << rx.cap(4);
  // qDebug() << "elevat:" << rx.cap(5);
#ifndef DEBUG_PROTOCOL
  QString tail = rx.cap(7);
#else
  QString tail = rx.cap(9);
#endif
  if ((DLE != rx.cap(1)) || (!tail.contains(ETX))) {
    if (error) *error = QObject::tr("match DLE, ETX fail");
    return false;
  }

  //if (tail.replace("\r\n", "") != crc(str.mid(DLE.length(), PAYLOAD)).right(2)) {
    if (tail.replace("\r\n", "") != crc(str.mid(DLE.length(), PAYLOAD))) {

    if (azimut != nullptr) azimut->state = Drive::CRC_NOT_VALID;
    if (elevat != nullptr) elevat->state = Drive::CRC_NOT_VALID;

    if (error) *error = QObject::tr("CRC fail");
    return false;
  }

  if (azimut != nullptr) {
    azimut->state = Drive::OK;
    ++azimut->seq;

    quint16 st = state_ctrl & 0xA000;
    //quint16 st = state_ctrl && 0xA000;
    switch (st) {
      case 0x8000 :
        azimut->state = Drive::FAIL;
      break;

      case 0x4000 :
        azimut->state = Drive::SENSOR_ANGLE_FAIL;
      break;

      default :
        // тайну обработки невалидной CRC знает только Юра
        azimut->self = getAngle(rx.cap(4));
      break;
    }
  }

  if (elevat != nullptr) {
    elevat->state = Drive::OK;
    ++elevat->seq;

    quint16 st = state_ctrl & 0x5000;
    //quint16 st = state_ctrl && 0x5000;
    switch (st) {
      case 0x2000 :
        elevat->state = Drive::FAIL;
      break;

      case 0x1000 :
        elevat->state = Drive::SENSOR_ANGLE_FAIL;
      break;

      default :
        // тайну обработки невалидной CRC знает только Юра
        elevat->self = getAngle(rx.cap(5));
      break;
    }
  }

#ifdef DEBUG_PROTOCOL
  if (azimut != nullptr) {
    azimut->dst = getAngle(rx.cap(6));
  }
  if (elevat != nullptr) {
    elevat->dst = getAngle(rx.cap(7));
  }
#endif

  // debug_log << azimut->toString() << elevat->toString();

  return true;
}

bool VkaProtocol::split(const QByteArray& buf) const
{
  return (buf.indexOf(ETX.toLocal8Bit(), 0) != -1);
}

bool VkaProtocol::split(const QByteArray& buf, int mtu) const
{
  return (buf.indexOf(ETX.toLocal8Bit(), 0) == mtu-ETX.length());
}

float VkaProtocol::getAngle(const QString& buf) const
{
  return (buf.toDouble() * 0.01);
}

QString VkaProtocol::setAngle(float v) const
{
  return QString("%1%2")
      .arg(QChar(0 <= v ? '+' : '-'))
      .arg(int(::abs(v) * 100), 5, 10, QChar('0'));
}

QString VkaProtocol::crc(const QString& buf) const
{
  QByteArray str(buf.toLocal8Bit());

  unsigned char sum = 0;
  for (int i = 0; i < str.size(); ++i)
    sum += str[i];

  return QString::number((256 - sum), 16)
      .toUpper();
}

}

}
