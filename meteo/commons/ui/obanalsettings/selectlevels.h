#ifndef METEO_COMMONS_UI_OBANALSETTINGS_SELECTLEVELS_H
#define METEO_COMMONS_UI_OBANALSETTINGS_SELECTLEVELS_H

#include "selectvalues.h"

namespace meteo {

class SelectLevelsWidget : public SelectValuesWidget
{
  Q_OBJECT

public:
  explicit SelectLevelsWidget(int levelType, QWidget* parent = 0);

  static QList<int> allLevelsList(int levelType);

private:
  virtual QStringList loadAllValuesList();
  virtual QStringList loadValuesList(const QString& str);
  virtual QString getValues() const;

private:
  int levelType_;

};

} // meteo

#endif // METEO_COMMONS_UI_OBANALSETTINGS_SELECTLEVELS_H
