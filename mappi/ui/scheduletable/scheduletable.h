#pragma once

#include "checksessiondata.h"

#include <qwidget.h>

#include <meteo/commons/rpc/rpc.h>

class QTableWidget;
class QTableWidgetItem;

namespace Ui {
  class ScheduleTable;
}

namespace mappi
{

class ScheduleTable :
  public QWidget
{
  Q_OBJECT

  enum Column { kAos,kLos, kName, kNapravl, kVitokNum, kElevatMax, kConfl};

public :
  explicit ScheduleTable(QWidget* parent = nullptr);
  virtual ~ScheduleTable();

  void sheckShedule();
private:
  void deleteShedulerChannel();
  bool subscriSheduler();
  //void callbackSheduler(meteo::app::AppState_Proc *reply);
  bool getShedule();
  void insertRow(const schedule::Session &item);
  QTableWidgetItem* createCell(Column , const schedule::Session& );
  void fillCell(QTableWidgetItem *item, Column col, const schedule::Session &);

  Ui::ScheduleTable*         ui_;
  meteo::rpc::Channel*       chSheduler_;         //!< Канал подписки к информации об антенне
private slots:
  void slotConnectToSheduler();
  void slotDisconnectedFromSheduler();

};

}
