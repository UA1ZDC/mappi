#ifndef METEO_COMMONS_UI_PLUGINS_FORECASTWIDGETCORE_FORECASTACCURACY_H
#define METEO_COMMONS_UI_PLUGINS_FORECASTWIDGETCORE_FORECASTACCURACY_H

#include <QtGui>
#include <meteo/commons/ui/customviewer/customviewer.h>
#include <meteo/commons/global/global.h>

namespace meteo {
class ForecastAccuracyViewer : public CustomViewer {
Q_OBJECT
public:
  ForecastAccuracyViewer(QWidget* parent = nullptr);
  virtual bool load();
  bool init();

protected:
  virtual void setDefaultSectionSize();
  virtual void setDefaultSectionOrder();
  virtual void setDefaultSectionVisible();
  virtual void setMenuColumn();

  QList<meteo::Document>* loadFromDataSet(int);

private:  
  void loadPunkts();
  void loadLevelTypes();

  void loadDescr();
};

}

#endif // FORECASTVIEWER_H
