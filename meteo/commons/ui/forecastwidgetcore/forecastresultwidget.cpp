#include "forecastresultwidget.h"
#include "ui_forecastresultwidget.h"

#include <commons/textproto/pbtools.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/services/sprinf/sprinfservice.h>
#include <meteo/commons/ui/custom/stationwidget.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/zond/placedata.h>



Q_DECLARE_METATYPE(TForecast*)


namespace meteo{
namespace map{

ForecastResultWidget::ForecastResultWidget(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::ForecastResultWidget)
{
  ui_->setupUi(this);
  ui_->dateIntervalEdit->setHorisontal(true);
  connect(ui_->resetBtn, SIGNAL(released ()),SLOT(fillTree()));
  ui_->frcstTypeTree->setIconSize(QSize(36,36));
  ui_->frcstTypeTree->setDragEnabled(true);
  // ui_->frcstTypeTree->setDragDropMode(QAbstractItemView::InternalMove);
  ui_->frcstTypeTree->installEventFilter(this);
  ui_->frcstTypeTree->viewport()->installEventFilter(this);
  ui_->frcstTypeTree->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
//
}

ForecastResultWidget::~ForecastResultWidget(){

}

void ForecastResultWidget::setStation(const StationData & st){
    cur_station_ = st;
}

bool ForecastResultWidget::eventFilter(QObject* object, QEvent* event)
{
  if (object != ui_->frcstTypeTree->viewport()) return  QWidget::eventFilter(object, event);
  switch (event->type()) {
    case QEvent::ContextMenu: {
      QMenu menu;
      QAction rm(QString::fromUtf8("Скрыть"), &menu);
      QList<QAction*> actions;
      actions.append(&rm);
      const QPoint& pos = static_cast<QContextMenuEvent*>(event)->pos();
      QAction* answer = menu.exec(actions, ui_->frcstTypeTree->mapToGlobal(pos));
      if (answer == &rm) {
           hide();
      }
    }
    break;
    case QEvent::Resize: {
  /*    int width = ui_->frcstTypeTree->viewport()->width();
      ui_->frcstTypeTree->setColumnWidth (kMethodName ,width/2);
      ui_->frcstTypeTree->setColumnWidth (kTotal ,width/4);
      ui_->frcstTypeTree->setColumnWidth (kAccuracy ,width/4);
   */
    }
    default:
      break;
  }
  return QWidget::eventFilter(object, event);
//  return SettingsWidget::eventFilter(object, event);
}


void ForecastResultWidget::slotAnyChanged(){
  fillTree();
}


void ForecastResultWidget::fillTree(){
 // station_widget_->setEnabled(false);
  //ui_->frcstTypeTree->clear();
  ui_->frcstTypeTree->setRowCount(0);
  setCursor(Qt::WaitCursor);
  QString methods_dir = global::kForecastMethodsDir;

  rpc::Channel* ctrl_forecast = meteo::global::serviceChannel(meteo::settings::proto::kForecastData);
  if(nullptr == ctrl_forecast ) {
    error_log.msgBox() << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kForecastData));
 //   station_widget_->setEnabled(true);
    return;
  }
  meteo::forecast::ForecastResultRequest req;
  req.set_method_name(pbtools::toString(cur_method_));
  req.set_time_start(ui_->dateIntervalEdit->getFirstDate().toString(Qt::ISODate).toStdString());
  req.set_time_end(ui_->dateIntervalEdit->getSecondDate().toString(Qt::ISODate).toStdString());
  meteo::surf::Point* p = req.mutable_coord();
  p->set_index(pbtools::toString(cur_station_.index));
  p->set_name(pbtools::toString(cur_station_.name));
 //FIXME p->set_eng_name(cur_station_.);
  p->set_fi(cur_station_.coord.fi());
  p->set_la(cur_station_.coord.la());
  p->set_height(cur_station_.coord.alt());
  meteo::forecast::ForecastResultReply* reply = ctrl_forecast->remoteCall(&meteo::forecast::ForecastData::GetForecastResult, req, 30000);
  if ( nullptr == reply ) {
    error_log.msgBox() << msglog::kServerAnswerFailed;
  //  station_widget_->setEnabled(true);

    return;
  }
  int kol_data = reply->data_size();
  ui_->frcstTypeTree->setSortingEnabled(false);
  for (int row=0; row< kol_data; ++row){
    const meteo::forecast::ForecastResult& cfr = reply->data(row);

    if(ui_->frcstTypeTree->rowCount() < row+1){
        ui_->frcstTypeTree->insertRow(row);
    }
    QString name = TMeteoDescriptor::instance()->description(TMeteoDescriptor::instance()->name(cfr.val_descr()));

    setMethodName(row, kDescr, name )->setTextAlignment ( Qt::AlignLeft );

    setMethodName(row, kDtProd,pbtools::toQString(cfr.time_prod()) );
    setMethodName(row, kMethodName,pbtools::toQString(cfr.method_name()) );

    setMethodName(row, kDtForecastStart,pbtools::toQString(cfr.time_start()) );
    setMethodName(row, kDtForecastEnd,pbtools::toQString(cfr.time_end()) );
    setMethodName(row, kRezult,QString::number(cfr.value(),'f',1) );
    bool ok = false;
    pbtools::toQString(cfr.forecast_text()).toDouble(&ok);
    if(!ok){
      setMethodName(row, kTextRezult,pbtools::toQString(cfr.forecast_text()) );
    } else {
      setMethodName(row, kTextRezult,QObject::tr("Числовое значение"));
    }
    QString rv;
    QBrush br;
    br.setStyle(Qt::SolidPattern);
    br.setColor(Qt::white);
    QString tooltip;
    if(!cfr.has_fflag_obr() || 0 == cfr.fflag_obr()){
      rv = QObject::tr("Нет данных");
      br.setColor(Qt::yellow);
    } else {
        if(cfr.has_freal_value()){
          rv = QString::number(cfr.freal_value(),'f',1);
          tooltip = QObject::tr("Значение получено по фактическим наблюдениям");
          br.setColor(Qt::lightGray);
        }else{
          if(cfr.has_freal_field_value()){
            rv = QString::number(cfr.freal_field_value(),'f',1);
            br.setColor(Qt::lightGray);
            tooltip = QObject::tr("Значение получено по анализу");
          }
        }
    }
    QTableWidgetItem* it_vs = setMethodName(row, kRealValue,rv );
    if(nullptr != it_vs){
      it_vs->setBackground(br);
      it_vs->setToolTip(tooltip);
    }
    br.setColor(Qt::yellow);
    QString acc = QObject::tr("Нет данных");
    if(0 < cfr.fflag_obr()){
      if(true == cfr.opr()){
        br.setColor(Qt::green);
        acc = QObject::tr("Да");
      } else {
        br.setColor(Qt::red);
        acc = QObject::tr("Нет");
      }
    }
    setMethodName(row, kAccuracy,acc)->setBackground(br);
  }
  for (int i=1;i<ui_->frcstTypeTree->columnCount();++i){
   ui_->frcstTypeTree->resizeColumnToContents(i);
  }

  ui_->frcstTypeTree->setSortingEnabled(true);
  ui_->frcstTypeTree->setColumnHidden(kMethodName,true);
  unsetCursor();
 // station_widget_->setEnabled(true);
  delete reply;reply = nullptr;
  delete ctrl_forecast; ctrl_forecast = nullptr;
}


QTableWidgetItem * ForecastResultWidget::setMethodName(int row,int col, const QString &mn){

  QTableWidgetItem *newItem = new QTableWidgetItem(mn);
  newItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  newItem->setTextAlignment ( Qt::AlignCenter );
  ui_->frcstTypeTree->setItem(row, col, newItem);
  return newItem;
}


void ForecastResultWidget::setMethodName(const QString &mn){
  cur_method_ = mn;
  fillTree();
}


}

}
