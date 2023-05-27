#include "config.h"
#include <commons/textproto/tprototext.h>


namespace mappi {

namespace receiver {

Parameters Configuration::default_;

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

  conf::ReceiverConf receiver = conf.receiver();
  apch = receiver.apch();
  mode = receiver.mode();
  if (mode == conf::kUnkRate)
    return false;

  engine = receiver.engine();
  switch (engine) {
    case conf::kUdpEngine :
      if (receiver.has_udp()) {
        host = receiver.udp().host().c_str();
        port = receiver.udp().port();
        break ;
      }
      return false;

    case conf::kSubEngine :
      if (receiver.has_sub_process()) {
        appName = receiver.sub_process().app().c_str();
        break ;
      }
      return false;

    default :
      return false;
  }

  conf::FileStorage storage = conf.file_storage();
  sessionDir = QString("%1/%2")
    .arg(storage.root().c_str())
    .arg(storage.session().c_str());

  dumpDir = QString("%1/%2")
    .arg(storage.root().c_str())
    .arg(storage.dump().c_str());

  rawFile = QString("%1/%2")
    .arg(storage.root().c_str())
    .arg(receiver.raw_file().c_str());

  for (auto satellite : conf.satellite()) {
    for (auto receiver : satellite.receiver()) {
      if (mode == receiver.mode()) {
        Parameters::fromProto(receiver, &param_[satellite.name().c_str()]);
        // debug_log << QObject::tr("Загружены параметры приема, спутник:") << satellite.name();
      }
    }
  }

  isValid_ = true;

  return true;
}

QString Configuration::toString() const
{
  QStringList res = {
    "",
    QString("is valid:    %1").arg(isValid_ ? "true" : "false"),
    QString("demo mode:   %1").arg(demoMode ? "true" : "false"),
    QString("site name:   %1").arg(siteName),
    QString("site:        %1;%2;%3").arg(point.latDeg()).arg(point.lonDeg()).arg(point.alt()),
    QString("engine:      %1").arg(engine == conf::kSubEngine ? "sub process" : "udp"),
    QString("apch:        %1").arg((apch ? "true" : "false")),
    QString("rate mode:   %1").arg(mode == conf::kHiRate ? "hi" : "low"),
    QString("raw file:    %1").arg(rawFile),
    QString("session dir: %1").arg(sessionDir),
    QString("dump dir:    %1").arg(dumpDir),
    QString("satellite:   %1").arg(param_.keys().join(", "))
  };

  return res.join("\n");
}

bool Configuration::hasParameters(const QString& satellite) const
{
  return param_.contains(satellite);
}

Parameters Configuration::parameters(const QString& satellite) const
{
  Parameters item = param_.value(satellite, default_);
  if (item.dump.isUsed)
    item.dump.file = QString("%1/%2")
      .arg(dumpDir)
      .arg(item.dump.file);

  return item;
}

}

}
