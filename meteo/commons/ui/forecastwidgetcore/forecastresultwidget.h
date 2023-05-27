#ifndef METEO_COMMONS_UI_PLUGINS_FORECASTWIDGETCORE_FORECASTRESULTWIDGET_H
#define METEO_COMMONS_UI_PLUGINS_FORECASTWIDGETCORE_FORECASTRESULTWIDGET_H

#include <qstandarditemmodel.h>
#include <QtWidgets>
#include <meteo/commons/services/forecast/tforecast.h>
#include <meteo/commons/services/forecast/tforecastlist.h>

class TForecast;
class StationWidget;

namespace Ui {
class ForecastResultWidget;
}

namespace meteo{
namespace map{

class ForecastResultWidget : public QWidget
{
  Q_OBJECT

public:
  enum Columns {
    kDescr = 0,
    kDtProd =1,
    kMethodName =2,
    kDtForecastStart =3,
    kDtForecastEnd =4,
    kRezult =5,
    kTextRezult =6,
    kRealValue =7,
    kAccuracy =8
  };

  explicit ForecastResultWidget(QWidget *parent = 0);
  ~ForecastResultWidget();

  void setMethodName(const QString &);
  void setStation(const StationData &);
  
  
private:
  bool eventFilter(QObject* object, QEvent* event);
  QTableWidgetItem *setMethodName(int row,int col, const QString &mn);
    
  Ui::ForecastResultWidget *ui_;
  StationData   cur_station_; 
  QDateTime cur_dt_;
  QString   cur_method_; 
  
  
private slots:
  void slotAnyChanged();
  void fillTree();
  
};


}
}

#endif // FORECASTWIDGET_H
