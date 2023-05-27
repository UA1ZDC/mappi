#ifndef METEO_NOVOST_UI_COLORALERT_ALERTCHECK_H
#define METEO_NOVOST_UI_COLORALERT_ALERTCHECK_H

#include <qhash.h>
#include <qvariant.h>

#include <meteo/commons/proto/meteotablo.pb.h>

namespace meteo {

class AlertCheck
{
public:
  AlertCheck();

  void addAlert(const tablo::ColorAlert& alert);

  tablo::Color color(const QHash<QString,double>& values) const;
  tablo::Color check(const QString& descr, QVariant value) const;

private:

  tablo::Color max(tablo::Color c1, tablo::Color c2) const;

private:
  QHash< QString, QList<tablo::Condition> > data_;
};

} // meteo

#endif // METEO_NOVOST_UI_COLORALERT_ALERTCHECK_H
