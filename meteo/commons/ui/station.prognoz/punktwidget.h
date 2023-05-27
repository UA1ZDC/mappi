#ifndef PUNKTWIDGET_H
#define PUNKTWIDGET_H

#include <QtWidgets>
#include <meteo/commons/ui/mainwindow/mainwindow.h>

namespace Ui{
class PunktWidget;
}

class StationWidget;

namespace meteo {
namespace map {

class PunktWidget : public QDialog
{
  Q_OBJECT
public:
  PunktWidget(meteo::app::MainWindow *parent = 0);
  ~PunktWidget();
  bool init();

private:
  void addPunktForecast();
  void rmPunktForecast();
  bool loadPunkts();
  bool changePunktActive(const QString& oid, bool isActive);

  Ui::PunktWidget* ui_;
  StationWidget* station_widget_;
  meteo::app::MainWindow *mainWindow_;

private slots:
  void slotHideStationBar();
  void slotAddPunkt();
  void slotRemovePunkt();
  void slotIndexChange();
  void slotAnyChange();
  void slotItemChanged(QTreeWidgetItem*,int);

};

}
}

#endif
