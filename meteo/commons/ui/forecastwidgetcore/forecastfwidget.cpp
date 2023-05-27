#include "forecastfwidget.h"
#include "ui_forecastfresultwidget.h"

#include <meteo/commons/ui/custom/stationwidget.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/services/sprinf/sprinfservice.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/zond/placedata.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/novost/forecast/tforecastlist.h>
#include <meteo/novost/forecast/tforecast.h>
#include <commons/obanal/tfield.h>

Q_DECLARE_METATYPE(TForecast*)


namespace meteo{
namespace map{

ForecastFResultWidget::ForecastFResultWidget(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::ForecastFResultWidget)
  {
  ui_->setupUi(this);

  QDateTime dt = QDateTime::currentDateTime();
  dt.setTime(QTime(dt.time().hour(), 0));

  
  connect(ui_->frcstTypeTree, SIGNAL(itemDoubleClicked ( QTableWidgetItem * )),SLOT(slotStartForecast(QTableWidgetItem*)));
  connect(ui_->frcstTypeTree, SIGNAL(itemSelectionChanged ()),SLOT(setActiveMethod()));
  
  connect(ui_->resetBtn, SIGNAL(released()),SLOT(slotAnyChanged()));
  
  ui_->frcstTypeTree->setIconSize(QSize(36,36));
  ui_->frcstTypeTree->setDragEnabled(false);
  ui_->frcstTypeTree->setSelectionMode(QAbstractItemView::SingleSelection);
  ui_->frcstTypeTree->setSortingEnabled(false);
  // ui_->frcstTypeTree->setDragDropMode(QAbstractItemView::InternalMove);
  
  ui_->frcstTypeTree->installEventFilter(this);
  ui_->frcstTypeTree->viewport()->installEventFilter(this);
  ui_->frcstTypeTree->horizontalHeader()->setResizeMode(QHeaderView::Interactive); 
  
    //ui_->frcstTypeTree->resizeColumnsToContents ();
  //fillTree();
}

ForecastFResultWidget::~ForecastFResultWidget(){
    
}

bool ForecastFResultWidget::eventFilter(QObject* object, QEvent* event)
{
  if (object != ui_->frcstTypeTree->viewport()) 
    return  QWidget::eventFilter(object, event);
 // debug_log << "event "<<event->type();
  
  switch (event->type()) {
    
    case QEvent::ContextMenu: {
      /*
      const QPoint& pos = static_cast<QContextMenuEvent*>(event)->pos();
      QMenu menu;
      QAction add(QString::fromUtf8("Просмотр результатов"), &menu);
     // QAction addFore(QString::fromUtf8("Просмотр расчетов"), &menu);
      QAction rm(QString::fromUtf8("Скрыть результаты"), &menu);
      QAction rmFore(QString::fromUtf8("Скрыть расчеты"), &menu);
      QList<QAction*> actions;
      actions.append(&add);
      //QAction* answer = menu.exec(actions, ui_->frcstTypeTree->mapToGlobal(pos));
     */
    }
    break;
    case QEvent::Resize: {
      int width = ui_->frcstTypeTree->viewport()->width();
      ui_->frcstTypeTree->setColumnWidth (kMethodName ,width/2);
      ui_->frcstTypeTree->setColumnWidth (kTotal ,width/4);
      ui_->frcstTypeTree->setColumnWidth (kAccuracy ,width/4);
    }
    break;
//     case QEvent::Show: {
//       debug_log << "Show";
//       WidgetHandler::instance()->showStationBar(true);
//       
//     }
//     break;
//     case QEvent::Hide: {
//       debug_log << "Hide";
//       WidgetHandler::instance()->showStationBar(false);
//     }
//     break;
    default:
      break;
  }
  return QWidget::eventFilter(object, event);
//  return SettingsWidget::eventFilter(object, event);
  
}


void ForecastFResultWidget::fillTree(){
  // ui_->frcstTypeTree->clear();
  ui_->frcstTypeTree->setRowCount(0);
  setCursor(Qt::WaitCursor);
  QString methods_dir = global::kForecastMethodsDir;
  
 rpc::TController * ctrl_forecast = meteo::global::serviceController(meteo::settings::proto::kForecastData);
  if(0 == ctrl_forecast ) {
    error_log.msgBox() << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kForecastData));
    return;
  }
  
  
  ui_->frcstTypeTree->setSortingEnabled(false);

  ui_->frcstTypeTree->setSortingEnabled(true);
  if(0 != ctrl_forecast){
    delete ctrl_forecast; ctrl_forecast = 0;
  }
  unsetCursor(); 
}



}
}
