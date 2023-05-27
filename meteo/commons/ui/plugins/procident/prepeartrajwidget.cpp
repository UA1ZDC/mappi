
#include "prepeartrajwidget.h"
#include "ui_prepeartrajwidget.h"
#include "object_trajection.h"

#include <cross-commons/debug/tlog.h>
#include <commons/obanal/tfield.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/services/fieldata/fieldanalyse.h>

namespace meteo {
namespace map {

enum {
  Num   = 0,
  Date  = 1,
  FDate  = 2,
  Hour  = 3,
};

PrepearTrajWidget::PrepearTrajWidget(meteo::map::Document* doc ,MapWindow* parent) :
  MapWidget(parent),
  ui_(new Ui::PrepearTrajWidget),
  doc_(doc),
  objtraj_(nullptr),
  cur_layer_(nullptr)
{
  ui_->setupUi(this);
  //ui_->timeBox->setCurrentIndex(8);
  // ui_->intervalBox->setCurrentIndex(1);
  connect( ui_->timeBox, SIGNAL(currentIndexChanged ( int )), this, SLOT(anyChanged ( )) );
  connect( ui_->timeBox_2, SIGNAL(currentIndexChanged ( int )), this, SLOT(anyChanged ( )) );
  connect(ui_->addBtn, SIGNAL(clicked()), SLOT(slotApplyClose()));
  connect(ui_->closeBtn, SIGNAL(clicked()), SLOT(slotClose()));

  //connect( ui_->transfStationWidget, SIGNAL(requestCoord(bool)), SLOT(slotRequestCoord(bool)) );

  QStringList lst;
  lst << "Номер" << "Срок"<<"Срок прогноза"<<"Заблаговременность"<<"Статус";
  ui_->fieldsTree->setHeaderLabels(lst);
  ui_->fieldsTree->setRootIsDecorated(false);
  ui_->fieldsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

  dt_start_ = QDateTime::currentDateTimeUtc();
  dt_end_ = QDateTime::currentDateTimeUtc();
  if(nullptr != doc_){
      connect(doc_->eventHandler(), SIGNAL(layerChanged( map::Layer* , int  )), SLOT(slotLayerChanged( map::Layer* , int )));
  }

  anyChanged();
}

PrepearTrajWidget::~PrepearTrajWidget()
{
  if(nullptr != objtraj_ ){ delete objtraj_; }

}

void PrepearTrajWidget::slotLayerChanged( map::Layer* l, int ct){
  Q_UNUSED(l);
  Q_UNUSED(ct);
  if( nullptr == doc_|| nullptr == ui_|| nullptr == ui_->fieldsTree) return;
  if((l == cur_layer_) && ct == LayerEvent::Deleted ){
      ui_->fieldsTree->clear();
    }

  if((nullptr != objtraj_ ) && (l == objtraj_->layer()) && ct == LayerEvent::Deleted ){
      objtraj_->setLayer(nullptr);
    }

}

bool PrepearTrajWidget::getObjTrajectory()
{
  if( nullptr == doc_|| nullptr == ui_|| nullptr == ui_->fieldsTree) return false;
  cur_layer_ = doc_->activeLayer();

  if( nullptr==cur_layer_|| !cur_layer_->hasField()) return false;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  ui_->addBtn->setEnabled(false);
  meteo::rpc::Channel * ctrl_map_= meteo::global::serviceChannel(meteo::settings::proto::kField);
  if(nullptr == ctrl_map_) {
      error_log.msgBox() << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kField));
      QApplication::restoreOverrideCursor();
      ui_->addBtn->setEnabled(true);
      return false;
    }
  ::meteo::field::SimpleDataRequest request;

  for(int i =0; i < ui_->fieldsTree->topLevelItemCount();++i ){
     QTreeWidgetItem * it = ui_->fieldsTree->topLevelItem(i);
     if(nullptr != it){
        QString id = it->data(Num,Qt::UserRole).toString();
        request.add_id(id.toStdString());
       }
    }
   obanal::TField*  f = cur_layer_->field();
   if(nullptr != f){
       request.set_smootch(f->getSmootch());
     }
  meteo::field::ExtremumTrajReply *reply=ctrl_map_->remoteCall( &field::FieldService::GetExtremumsTraj, request, 30000);
  if ( nullptr == reply ) {
      QApplication::restoreOverrideCursor();
      delete ctrl_map_;ctrl_map_=nullptr;
      error_log.msgBox() << msglog::kServiceInfoAnswerFailed.arg("данные о перемещении барических образований").arg(meteo::global::serviceTitle(meteo::settings::proto::kMap));
      ui_->addBtn->setEnabled(true);
      return false;
    }
  //info_log << tr("Расчет траекторий перемещения барических образований.");//<<reply->DebugString();
  hasData(reply);
  delete ctrl_map_;ctrl_map_=nullptr;
  delete reply;
  QApplication::restoreOverrideCursor();
  return true;

}


void PrepearTrajWidget::hasData(meteo::field::ExtremumTrajReply* trj){
  if ( nullptr != objtraj_ ) {
    delete objtraj_->layer();
    delete objtraj_;
    objtraj_ = nullptr;
  }
  objtraj_ = new ObjectTrajection();

  cur_layer_ = doc_->activeLayer();
  if(nullptr == cur_layer_  || nullptr == objtraj_ || nullptr == doc_) {
      QApplication::restoreOverrideCursor();
      ui_->addBtn->setEnabled(true);
      return;
    }
  QString lname = tr("Траектории перемещения барических образований с %1 по %2")
      .arg(dt_start_.toString("yyyy-MM-dd hh:mm"))
      .arg(dt_end_.toString("yyyy-MM-dd hh:mm"));
  if(cur_layer_->field()->getLevel() > 0 ){
      lname +=  tr(" уровень %1 гПа ").arg(cur_layer_->field()->getLevel());
    } else {
      lname += " "+ cur_layer_->field()->getLevelTypeName()+" ";
    }
  lname +=  cur_layer_->field()->getCenterName();

  objtraj_->setLayer(new Layer(doc_,lname));
  objtraj_->setCurDt(cur_layer_->field()->getDate());
  objtraj_->hasExtremums(trj);
  cur_layer_->setActive();
  ui_->addBtn->setEnabled(true);
  QApplication::restoreOverrideCursor();
}


void PrepearTrajWidget::anyChanged ( ){
  ui_->fieldsTree->clear();

  if( nullptr == doc_) return;
  cur_layer_ = doc_->activeLayer();

  if(  nullptr == cur_layer_ || !cur_layer_->hasField()) {
      return;
    }
  meteo::rpc::Channel * ctrl_field = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if(nullptr == ctrl_field ) {
      error_log.msgBox() << msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kField));
      QApplication::restoreOverrideCursor();
      return ;
    }

  obanal::TField * afield = cur_layer_->field();
  if(nullptr == afield) {
      QApplication::restoreOverrideCursor();
      return;
    }
  dt_start_ =  afield->getDate().addSecs(-timeStart() * 3600);
  dt_end_ =  afield->getDate().addSecs(timeEnd() * 3600);

  request_.Clear();
  request_.add_center(afield->getCenter());
  request_.set_net_type(afield->typeNet());
  // request_.add_hour(afield->getHour());
  request_.set_forecast_start(dt_start_.toString(Qt::ISODate).toStdString());
  request_.set_forecast_end(dt_end_.toString(Qt::ISODate).toStdString());
  //request_.set_model(afield->getModel());
  request_.add_meteo_descr(afield->getDescr());
  request_.add_level(afield->getLevel());
  request_.add_type_level(afield->getLevelType());
  //request_.set_only_last(true);
  //request_.set_only_best(true);


  meteo::field::DataDescResponse * reply =
      ctrl_field->remoteCall( &meteo::field::FieldService::GetAvailableData, request_,  30000);
  delete ctrl_field;ctrl_field=nullptr;
  if ( nullptr == reply ) {
      error_log.msgBox() << msglog::kServiceInfoAnswerFailed.arg("прогностические данные ").arg(meteo::global::serviceTitle(meteo::settings::proto::kField));
      return ;
    }

  struct trf{
    int32_t hour;
    QString id;
    QDateTime dt;
    QDateTime fdt;
  };

  QMap<QDateTime,trf> trfl;

  for(int i =0; i< reply->descr_size(); ++i){
      trf t;
      const meteo::field::DataDesc &d = reply->descr(i);
      QString date_str = QString::fromStdString (d.date());
      t.dt = QDateTime::fromString(date_str, Qt::ISODate);
      t.id =  QString::fromStdString(d.id());
      t.fdt = t.dt.addSecs(d.hour());
      t.hour = d.hour();

      QMap<QDateTime,trf>::iterator ii = trfl.find(t.fdt);
      while (ii != trfl.end() && ii.key() == t.fdt) {
          if(ii.value().hour > t.hour){
              ii = trfl.erase(ii);
            } else { ++ii; }
        }
      trfl.insert(t.fdt,t);
    }
  delete reply;

  QMap<QDateTime,trf>::iterator ii;
  int i =1;
  for (ii = trfl.begin(); ii != trfl.end(); ++ii){
      const trf &t = ii.value();
      QTreeWidgetItem* item = new QTreeWidgetItem(ui_->fieldsTree);
      item->setText(Num, QString::number(i++));
      item->setData(Num,Qt::UserRole,t.id);
      item->setText(Date, t.dt.toString("dd.MM.yy hh:mm"));
      item->setData(Date,Qt::UserRole,t.dt);
      item->setText(FDate, t.fdt.toString("dd.MM.yy hh:mm"));
      item->setData(FDate,Qt::UserRole, t.fdt);
      item->setData(Hour,Qt::UserRole,t.hour);
      if(t.hour > 0){
          item->setText(Hour, QString("%1").arg(t.hour/3600));
        } else {
          item->setText(Hour, "Анализ");
        }
    }

  QString lname = tr("Траектории перемещения барических образований");
  if(cur_layer_->field()->getLevel() > 0 ){
      lname +=  tr(" уровень %1 гПа ").arg(cur_layer_->field()->getLevel());
    } else {
      lname += " "+ cur_layer_->field()->getLevelTypeName()+" ";
    }
  lname +=  cur_layer_->field()->getCenterName();
  setWindowTitle(lname);
}

int PrepearTrajWidget::timeEnd()
{
  return ui_->timeBox_2->currentText().toInt();
}

int PrepearTrajWidget::timeStart()
{
  return ui_->timeBox->currentText().toInt();
}


void PrepearTrajWidget::slotApplyClose(){
  getObjTrajectory();
  // close();
}

void PrepearTrajWidget::slotClose(){
  hide();
}


}
}
