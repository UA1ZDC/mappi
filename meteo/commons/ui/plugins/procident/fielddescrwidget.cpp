
#include "fielddescrwidget.h"
#include "ui_fielddescrwidget.h"

#include <cross-commons/debug/tlog.h>
#include <commons/obanal/tfield.h>
#include <meteo/commons/global/global.h>

namespace meteo {
namespace map {
  

  
  FieldDescrWidget::FieldDescrWidget(MapView* view) :
  QDialog(view),
  is_ok_(false),ui_(new Ui::FieldDescrWidget) 

{
  ui_->setupUi(this);
  
  connect(ui_->addBtn, SIGNAL(clicked()), SLOT(slotOk()));
  connect(ui_->cancelBtn, SIGNAL(clicked()), SLOT(slotCancel()));
  
}

FieldDescrWidget::~FieldDescrWidget()
{
  
}

void FieldDescrWidget::setProcess(const QMap<int,QString>&  ){
  
}

int FieldDescrWidget::numField(){
  if(nullptr == ui_) return -1;
  return ui_->fnumbersp->value();
  
}


void FieldDescrWidget::setProcName(const QString & a){
  if(nullptr == ui_) return;
  int indx = ui_->processcb->findText(a);
  
  if (indx < 0) {
    ui_->processcb->setEditText(a);  
  } else {
    ui_->processcb->setCurrentIndex(indx);
  }
}

void FieldDescrWidget::fillProcessList(const QStringList & a){ 
  if(nullptr == ui_) return ;
  ui_->processcb->clear();
  ui_->processcb->addItems(a);
}

void FieldDescrWidget::setFnum(const QString & a){ 
  if(nullptr == ui_) return ;
  ui_->fnumbersp->setValue(a.toInt());
}

void FieldDescrWidget::setSeason(const QString & a){
  if(nullptr == ui_) return ;
  ui_->seasoncb->setCurrentIndex (0 > ui_->seasoncb->findText(a) ? 0:ui_->seasoncb->findText(a) );
}

void FieldDescrWidget::setHour(const QString & a){ 
  if(nullptr == ui_) return ;
  ui_->hoursb->setValue(a.toInt());
}

int FieldDescrWidget::hour(){
  if(nullptr == ui_) return -1;
  return ui_->hoursb->value();
}

int FieldDescrWidget::getProc( QString *process){
  if(nullptr == ui_) return -1;
  *process =  ui_->processcb->currentText();
  return ui_->processcb->itemData(ui_->processcb->currentIndex()).toInt();
}

 QString  FieldDescrWidget::season(){
  if(nullptr == ui_) return "";
  return ui_->seasoncb->currentText();
}

void FieldDescrWidget::setBtnTitle(const QString & a){ 
  if(nullptr == ui_) return ;
  ui_->addBtn->setWindowTitle(a);
}



void FieldDescrWidget::slotOk(){
  is_ok_ = true;
  close();  
}

void FieldDescrWidget::slotCancel(){
  is_ok_ = false; 
close();  
}


}
}
