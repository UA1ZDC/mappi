#include "config.h"
#include <commons/textproto/tprototext.h>
#include <mappi/global/global.h>
#include <qdatetime.h>
#include <qfile.h>


namespace mappi {

namespace schedule {

Configuration::Configuration() :
  conflResol(conf::kUnkResol)
{
  isValid_ = false;
}

Configuration::~Configuration()
{
}

bool Configuration::load(const QString& filePath)
{
  conf::Reception conf;
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    isValid_ = false;
    return false;
  }

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  if (!TProtoText::fillProto(text, &conf)) {
    isValid_ = false;
    return false;
  }

  conf::Site site = conf.site();
  meteo::sprinf::GeoPoint p = site.point();
  point.setPoint(p.lat_radian(), p.lon_radian(), p.height_meters());
  coord = Coords::GeoCoord(p.lat_radian(), p.lon_radian(), p.height_meters());

  conf::ScheduleConf schedule = conf.schedule();

  StringList list = schedule.satellite();
  for (int idx = 0; idx < list.item_size(); ++idx)
    satellite.append(list.item(idx).c_str());

  QDateTime dt = QDateTime::currentDateTimeUtc();
  tlePath = meteo::global::findWeatherFile(conf.file_storage().tle().c_str(), dt);

  elevation = schedule.elevation();
  conflResol = schedule.resol();
  period = schedule.period();

  isValid_ = true;

  return true;
}

QString Configuration::toString() const
{
  QDateTime start = QDateTime::currentDateTimeUtc();
  QDateTime stop = start.addSecs(period * 3600);

  QStringList res = {
    QString(""),
    QString("is valid:  %1").arg((isValid_ ? "true" : "false")),
    QString("tle path:  %1").arg(tlePath),
    QString("interval:  %1 - %2").arg(start.toString(Qt::ISODate)).arg(stop.toString(Qt::ISODate)),
    QString("site:      %1;%2;%3").arg(point.latDeg()).arg(point.lonDeg()).arg(point.alt()),
    QString("satellite: %1").arg(satellite.join(", ")),
    QString("elevation: %1;%2").arg(elevation.min()).arg(elevation.max()),
    QString("conflict resolution method: %1").arg(static_cast<int>(conflResol))
  };

  return res.join("\n");
}

}

}
