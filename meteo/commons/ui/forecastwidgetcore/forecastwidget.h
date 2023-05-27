#ifndef METEO_COMMONS_UI_PLUGINS_FORECASTWIDGETCORE_FORECASTWIDGET_H
#define METEO_COMMONS_UI_PLUGINS_FORECASTWIDGETCORE_FORECASTWIDGET_H

//#include <QtGui>
#include <qwidget.h>
#include <qstandarditemmodel.h>
#include <qtreewidget.h>
#include <QTimer>
#include <qmainwindow.h>
#include <meteo/commons/services/forecast/tforecast.h>
#include <meteo/commons/services/forecast/tforecastlist.h>
#include <meteo/commons/zond/placedata.h>

#include "forecastresultwidget.h"

class TForecast;
class StationWidget;

const QColor ALARM  = QColor::fromRgb( 248, 215, 218 );
const QColor GOOD   = QColor::fromRgb( 212, 237, 218 );
const QColor MANUAL = QColor::fromRgb( 255, 243, 205 );

namespace Ui {
class ForecastWidget;
}
namespace meteo {
namespace map {
  class TraceAction;
} // map
} // meteo
namespace meteo{
namespace map{

class ForecastWidget : public QMainWindow
{
  Q_OBJECT

public:
  enum Columns { kMethodName, kTotal, kAccuracy, kAuto};

  explicit ForecastWidget(QMainWindow *parent = nullptr);
  ~ForecastWidget();


private:
    
  // запонение дерева
  void fillTree();
  QTableWidgetItem *newItem(int row, int column, const QString& acc, const QBrush& br);
  // установка автоматического расчета прогнозов
  void setAutoItem( int row, bool isauto);
  void setAccuracyItem(int, int, int );
  void updateRow(QTableWidgetItem *item);
    
  void loadSynSit();
  bool eventFilter(QObject* object, QEvent* event);
  void rmForecastForm();
  void enableControls();
  void disableControls();
  bool addStWidget();

  // ищем номер строки по названию метода
  int findRowByMethodName( QList<TForecast*> list, QString method );

  Ui::ForecastWidget *ui_;
  StationWidget *station_widget_;
  TForecastList *methods_list_;
  StationData   cur_station_; 
  QDateTime cur_dt_;
  ForecastResultWidget *result_wdgt_;
  QWidget * fw_;
  TForecast* fm_;  
  QSplitter *splitter_;

  // список уникального идентификатора метода
  QStringList* method_namelist_order;

  int curSynSit_;
  int curSeason_;
  int curFrontSit_;
  QMainWindow *p;

  /*
   * таймер на изменение с задержкой чтобы избежать дребезга
   */
  QTimer  *mTimer;  

private slots:
  void slotStartForecast(QTableWidgetItem* item);
  void slotAnyChanged();
  void slotDtChanged();
  void slotShowResult(const QString&);
  void slotHideResult();
  QTableWidgetItem* setActiveMethod();
  void slotChangeCurSit(int);
  void slotChangeCurSeason(int);
  void slotChangeCurFrontSit(int i);

  // обрабатываем нажатие на ячейку
  void slotCellDoubleClicked(int,int);

  // отменяем все
  void slotCancelAll();

public slots:
  // слот по сбросу таймера
  void slotEmitFizzBuzzChaged();

signals:
  void cancelBtnClicked();

  // двойное нажатие на ячейку
  void cellDoubleClicked(int, int);


};


}
}

#endif // METEO_COMMONS_UI_PLUGINS_FORECASTWIDGET_FORECASTWIDGET_H
