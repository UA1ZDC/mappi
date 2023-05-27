#ifndef METEO_COMMONS_UI_PLUGINS_FORECASTWIDGETCORE_FORECASTVIEWER_H
#define METEO_COMMONS_UI_PLUGINS_FORECASTWIDGETCORE_FORECASTVIEWER_H

#include <QtGui>
#include <meteo/commons/ui/customviewer/customviewer.h>
#include <meteo/commons/global/global.h>

namespace meteo {

class ForecastViewer : public CustomViewer {
Q_OBJECT
public:
  ForecastViewer(QWidget* parent=nullptr);
  virtual bool init() override;

protected:
  virtual void setDefaultSectionSize() override;
  virtual void setDefaultSectionOrder() override;
  virtual void setDefaultSectionVisible() override;
  virtual void setMenuColumn();
  void loadReplaceValues();

private:

};

}

#endif
