#include "geodez.h"
#include "ui_geodez.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>


namespace meteo {
namespace map {

Geodez::Geodez(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::Geodez),
  view_(0)
{
  ui_->setupUi(this);

  QWidget::setWindowTitle("Геодезические свойства");

//  QWidget::setFixedSize(400,200);
  
  ui_->modLatAuto->isChecked();
  ui_->modLonAuto->isChecked();

  ui_->latStepBox->setCurrentIndex( ui_->latStepBox->findText(QString::number(5)));
  ui_->lonStepBox->setCurrentIndex( ui_->lonStepBox->findText(QString::number(5)));


  ui_->latStepBox->setDisabled(true);
  ui_->lonStepBox->setDisabled(true);
  ui_->visibLat->setDisabled(true);
  ui_->visibLon->setDisabled(true);

//  ui_->latStepEdit->setValidator( new QIntValidator(-90, 90, this) );
//  ui_->lonStepEdit->setValidator( new QIntValidator(-180, 180, this) );

  view_ = qobject_cast<MapView*>(parent);

//  connect(ui_->modLatAuto, SIGNAL(clicked()), SLOT(slotModLatAuto()) );
//  connect(ui_->modLatMan, SIGNAL(clicked()), SLOT(slotModLatAuto()) );
//  connect(ui_->modLonAuto, SIGNAL(clicked()), SLOT(slotModLatAuto()) );
//  connect(ui_->modLonMan, SIGNAL(clicked()), SLOT(slotModLatAuto()) );
  connect(ui_->acceptBtn, SIGNAL(clicked()), SLOT(slotSetNewParallels()));
  connect(ui_->cancelBtn, SIGNAL(clicked()), SLOT(slotReject()));

  connect(ui_->modLatAuto, SIGNAL(clicked()), SLOT(slotSetDefaultValue()));
  connect(ui_->modLonAuto, SIGNAL(clicked()), SLOT(slotSetDefaultValue()));
  connect(ui_->modLatMan, SIGNAL(clicked()), SLOT(slotSetDefaultValue()));
  connect(ui_->modLonMan, SIGNAL(clicked()), SLOT(slotSetDefaultValue()));



//  connect(ui_->latStepEdit, SIGNAL(textChanged(QString)), SLOT(slotModLatAuto()));
//  connect(ui_->lonStepEdit, SIGNAL(textChanged(QString)), SLOT(slotModLatAuto()));
}

Geodez::~Geodez()
{
  //  delete ui_;
}

void Geodez::slotSetNewParallels()
{
  if( 0 == view_ ){
    return;
  }
  if ( false == view_->hasMapscene() ) {
    return;
  }
  if( 0 == view_->mapscene()->document() ){
    return;
  }

//  if( 180%(ui_->latStepEdit->text().toInt()) != 0 ){
//    error_log << "not appropriate value";
//    return;
//  }
//  if( 360%(ui_->lonStepEdit->text().toInt()) != 0 ){
//    error_log << "not appropriate value";
//    return;
//  }


  if( ui_->modLatAuto->isChecked() && ui_->modLonAuto->isChecked()){


//    view_->mapscene()->document()->loadCoordLayer();

  }
  if( ui_->modLatMan->isChecked() && ui_->modLonMan->isChecked()){
//    Document::loadCoordLayerCustom(kManualAll,ui_->latStepEdit->text().toInt(),
//                                   ui_->lonStepEdit->text().toInt());

//    view_->mapscene()->document()->loadCoordLayerCustom(kManualAll,ui_->latStepBox->currentText().toInt(),
//                                                        ui_->lonStepBox->currentText().toInt());

  }
  if( ui_->modLatMan->isChecked() && ui_->modLonAuto->isChecked()){
//    Document::loadCoordLayerCustom(kManualLat,ui_->latStepEdit->text().toInt(),
//                                   ui_->lonStepEdit->text().toInt());

//    view_->mapscene()->document()->loadCoordLayerCustom(kManualLat,ui_->latStepBox->currentText().toInt(),
//                                                        ui_->lonStepBox->currentText().toInt());


  }
  if( ui_->modLatAuto->isChecked() && ui_->modLonMan->isChecked()){

//    Document::loadCoordLayerCustom(kManualLon,ui_->latStepEdit->text().toInt(),
//                                   ui_->lonStepEdit->text().toInt());

//    view_->mapscene()->document()->loadCoordLayerCustom(kManualLon,ui_->latStepBox->currentText().toInt(),
//                                                        ui_->lonStepBox->currentText().toInt());
  }



}

void Geodez::slotSetDefaultValue()
{
  if( sender() == ui_->modLatAuto){
    ui_->latStepBox->setCurrentIndex( ui_->latStepBox->findText(QString::number(5)));
    ui_->latStepBox->setDisabled(true);
//    view_->mapscene()->document()->loadCoordLayer();
  }
  if( sender() == ui_->modLonAuto){
    ui_->lonStepBox->setCurrentIndex( ui_->lonStepBox->findText(QString::number(5)));
    ui_->lonStepBox->setDisabled(true);
//    view_->mapscene()->document()->loadCoordLayer();
  }
  if( sender() == ui_->modLatMan){
    ui_->latStepBox->setDisabled(false);
  }
  if( sender() == ui_->modLonMan){
     ui_->lonStepBox->setDisabled(false);
  }
//  if( sender() == ui_->cancelBtn){
//    ui_->latStepEdit->setText(QString::number(5));
//    ui_->latStepEdit->setDisabled(true);
//    ui_->lonStepEdit->setText(QString::number(5));
//    ui_->lonStepEdit->setDisabled(true);
//    view_->mapscene()->document()->loadCoordLayer();

//    ui_->modLatAuto->setChecked(true);
//    ui_->modLonAuto->setChecked(true);


  //}

}

void Geodez::slotReject()
{
  QDialog::reject();

}




}
}
