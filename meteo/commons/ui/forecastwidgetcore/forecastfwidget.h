#ifndef METEO_COMMONS_UI_FORECASTWIDGETCORE_FORECASTFIELDWIDGET_H
#define METEO_COMMONS_UI_FORECASTWIDGETCORE_FORECASTFIELDWIDGET_H

#include <QWidget>
#include <meteo/commons/zond/placedata.h>
#include "forecastresultwidget.h"
#include <meteo/novost/forecast/tforecastlist.h>
#include <qstandarditemmodel.h>
#include <QtGui>

//NOTE: в сборке не участвует

class TForecast;
class StationWidget;

namespace Ui {
  class ForecastFResultWidget;
}

namespace meteo{
namespace map{

class ForecastFResultWidget : public QWidget
{
  Q_OBJECT

public:
  enum Columns { kMethodName, kTotal, kAccuracy};

  explicit ForecastFResultWidget(QWidget *parent = 0);
  ~ForecastFResultWidget();

private:
  void fillTree();
  bool eventFilter(QObject* object, QEvent* event);
  
  Ui::ForecastFResultWidget *ui_;
  StationData   cur_station_; 
private slots:

      
};


}
}

#endif
