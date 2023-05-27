#ifndef METEO_COMMONS_UI_OBANALSETTINGS_SELECTDATATYPES_H
#define METEO_COMMONS_UI_OBANALSETTINGS_SELECTDATATYPES_H

#include "selectvalues.h"

#include <QMap>
#include <QString>

namespace meteo {

class SelectDataTypesWidget : public SelectValuesWidget
{
  Q_OBJECT

public:
  explicit SelectDataTypesWidget(QWidget* parent = 0);

private:
  virtual QStringList loadAllValuesList();
  virtual QStringList loadValuesList(const QString& str);
  virtual QString getValues() const;

private:
  QMap<int, QString> allDataTypes_;

};

} // meteo

#endif // METEO_COMMONS_UI_OBANALSETTINGS_SELECTDATATYPES_H
