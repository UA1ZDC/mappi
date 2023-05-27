#ifndef METEO_COMMONS_UI_OBANALSETTINGS_SELECTCENTERS_H
#define METEO_COMMONS_UI_OBANALSETTINGS_SELECTCENTERS_H

#include "selectvalues.h"

#include <QMap>
#include <QString>

namespace meteo {

class SelectCentersWidget : public SelectValuesWidget
{
  Q_OBJECT

public:
  explicit SelectCentersWidget(QWidget* parent = 0);

private:
  virtual QStringList loadAllValuesList();
  virtual QStringList loadValuesList(const QString& str);
  virtual QString getValues() const;

protected:
  QMap<int, QString> allCenters_;

};

class SelectStationsWidget : public SelectCentersWidget
{
  Q_OBJECT

public:
  explicit SelectStationsWidget(QWidget* parent = 0);

  void setType(int type);

private:
  virtual QStringList loadAllValuesList();

  QList<int> getStationTypes() const;

private:
  int type_;

};

} // meteo

#endif // METEO_COMMONS_UI_OBANALSETTINGS_SELECTCENTERS_H
