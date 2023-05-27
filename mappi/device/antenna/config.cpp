#include "config.h"
#include <commons/textproto/tprototext.h>


namespace mappi {

namespace antenna {

Configuration::Configuration() :
  demoMode(false)
{
  isValid_ = false;
}

Configuration::~Configuration()
{
}

bool Configuration::load(const QString& filePath)
{
  isValid_ = false;

  QFile file(filePath);
  if (file.open(QIODevice::ReadOnly) == false)
    return false;

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  conf::Reception conf;
  if (TProtoText::fillProto(text, &conf) == false)
    return false;

  demoMode = conf.demo_mode();

  conf::Site site = conf.site();
  siteName = site.name().c_str();
  meteo::sprinf::GeoPoint p = site.point();
  point.setPoint(p.lat_radian(), p.lon_radian(), p.height_meters());
  coord = Coords::GeoCoord(p.lat_radian(), p.lon_radian(), p.height_meters());

  conf::AntennaConf antenna = conf.antenna();

  conf::AntennaType type = antenna.type();
  for (auto item : conf.antenna_profile()) {
    if (type == item.type())
      Profile::fromProto(item, &profile);
  }

  azimutCorrect = antenna.azimut_correct();

  // serial port
  conf::SerialPort sPort = antenna.serial_port();
  serialPort.tty = sPort.tty().c_str();
  serialPort.baudRate = sPort.baud_rate();
  serialPort.txTimeout = sPort.tx_timeout();
  serialPort.DTR = sPort.dtr_signal();
  serialPort.RTS = sPort.rts_signal();

  isValid_ = true;

  return true;
}

QString Configuration::toString() const
{
  QStringList res = {
    "",
    QString("is valid:  %1").arg(isValid_ ? "true" : "false"),
    QString("demo mode: %1").arg(demoMode ? "true" : "false"),
    QString("site name: %1").arg(siteName),
    QString("site:      %1;%2;%3").arg(point.latDeg()).arg(point.lonDeg()).arg(point.alt()),
    QString("azimut correct: %1").arg(azimutCorrect),
    "serial port",
    QString("  tty:        %1").arg(serialPort.tty),
    QString("  baud rate:  %1").arg(serialPort.baudRate),
    QString("  tx timeout: %1").arg(serialPort.txTimeout),
    QString("  DTR:        %1").arg(serialPort.DTR ? "on" : "off"),
    QString("  RTS:        %1").arg(serialPort.RTS ? "on" : "off"),
    profile.toString()
  };

  return res.join('\n');
}

}

}
