#include "pindialog.h"
#include "ui_pindialog.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <qheaderview.h>

#include <meteo/commons/proto/documentviewer.pb.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>

namespace meteo {
namespace documentviewer {

static const QIcon iconRmItem (":/meteo/icons/minus.png");

static const int treeWidgetSavedPinningsColumnCount = 2;
enum TreeWidgetSavedPinningsColumns {
  kDataColumn = 0,
  kWidgetColumn
};


static const QString errInvalidValues = QObject::tr("Допустимые значения:0,1,2 ");


PinDialog::PinDialog( DocumentHeader::DocumentClass docClass, PinningStorage* globalStorage ) :
  QDialog(nullptr),
  ui(new ::Ui::PinDialog),
  docClass_(docClass),
  globalStorage_(globalStorage)
{
  ui->setupUi(this);
  this->ui->lbCurrentDocType->setText( DocumentHeader::documentsDisplayNames[docClass] );
  this->setWindowTitle( QObject::tr("Сохраненные фильтры: %1").arg( DocumentHeader::documentsDisplayNames[docClass ]) );
  this->fillSavedPinningsList();
  this->ui->twSavedPinningsList->setHeaderHidden(false);
  this->ui->twSavedPinningsList->header()->setStretchLastSection(true);
  this->ui->twSavedPinningsList->setColumnCount(treeWidgetSavedPinningsColumnCount);
  this->ui->twSavedPinningsList->setColumnHidden( kDataColumn, true );  
  QObject::connect( this->ui->pbSave, SIGNAL(pressed()), this, SLOT(slotSavePinning()) );
  QObject::connect( this->ui->twSavedPinningsList, SIGNAL(itemSelectionChanged()), this, SLOT(slotTreeWidgetSavedPinningItemSelectionChanged()) );
  QObject::connect( this->ui->pbApply, &QPushButton::pressed, this, &QDialog::accept );
  QObject::connect( this->ui->lePinningName, SIGNAL(textChanged(const QString&)), this, SLOT(slotPinningNameChanged(const QString&)) );
}

PinDialog::PinDialog( DocumentHeader::DocumentClass docClass,
                      PinningStorage* globalStorage,
                      const meteo::GeoVector &target,
                      const QPolygon &source,
                      const QHash<WgtDocViewPanel::TreeWidgetsColumns, Condition> &conditions ) :
  PinDialog( docClass, globalStorage )
{
  if ( target.size() != meteo::map::Faxaction::markersCount||
       source.size() != meteo::map::Faxaction::markersCount ){
    error_log << QObject::tr("Ошибка - недопустимое число точек");
    return;
  }

  currentPin.conditions = conditions;
  for ( int i = 0; i < meteo::map::Faxaction::markersCount; ++i ){
    currentPin.imagePoints[i] = source.at(i);
    currentPin.mapPoints[i] = target.at(i);
  }
  this->currentPin.name = QString();
  this->updatePinningInfo();
}

PinDialog::~PinDialog()
{
  delete ui;
}

void PinDialog::slotPinningNameChanged(const QString& text)
{
  this->ui->pbSave->setEnabled( false == text.isEmpty() );
}

void PinDialog::fillSavedPinningsList()
{
  this->buttonToItemHash_.clear();
  this->ui->twSavedPinningsList->clear();
  auto currentPinningsList = this->globalStorage_->getStorage(this->docClass_);
  for ( auto pinName : currentPinningsList.keys() ) {    
    auto singlePin = currentPinningsList[pinName];
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setData( kDataColumn, Qt::UserRole, singlePin->name );
    this->ui->twSavedPinningsList->addTopLevelItem(item);
    QPushButton* pbRemovePinning = new QPushButton();
    pbRemovePinning->setMaximumHeight(30);
    pbRemovePinning->setMaximumWidth(30);
    pbRemovePinning->setIcon(iconRmItem);
    QObject::connect( pbRemovePinning, SIGNAL(pressed()), this, SLOT(slotRemovePinning()) );

    QWidget *wgt = new QWidget(this->ui->twSavedPinningsList);
    QHBoxLayout *hbl = new QHBoxLayout();
    if ( nullptr != wgt->layout() ) {
      delete wgt->layout();
    }
    wgt->setLayout(hbl);
    hbl->addWidget(new QLabel( singlePin->name,  wgt ));
    hbl->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding));
    hbl->addWidget(pbRemovePinning);

    this->ui->twSavedPinningsList->setItemWidget( item, kWidgetColumn, wgt );
    this->buttonToItemHash_.insert( pbRemovePinning, item );
  }
}

void PinDialog::setConditions( const QHash<WgtDocViewPanel::TreeWidgetsColumns, Condition> &conditions )
{
  this->ui->twPinningConditionsList->clear();
  for ( auto col: conditions.keys() ){
    auto condition = conditions[col];
    if ( false == condition.isNull() ){
      QTreeWidgetItem *item = new QTreeWidgetItem();
      item->setData(0, Qt::DisplayRole, condition.displayCondition() );
      this->ui->twPinningConditionsList->addTopLevelItem(item);
    }
  }
}

void PinDialog::slotRemovePinning()
{
  QPushButton* pb = qobject_cast<QPushButton*>(QObject::sender());
  if ( true == this->buttonToItemHash_.contains(pb) ){
    auto item = this->buttonToItemHash_[pb];
    auto name = item->data(kDataColumn, Qt::UserRole).toString();
    this->ui->twSavedPinningsList->clear();
    this->buttonToItemHash_.clear();
    this->globalStorage_->removePinning( this->docClass_, name );
    this->buttonToItemHash_.remove(pb);
    this->fillSavedPinningsList();
  }
}


QPoint PinDialog::imagePoint( int index )
{
  switch (index) {
  case 0:{
    return QPoint( this->ui->sbHor1->value(), this->ui->sbVert1->value() );
  }
  case 1:{
    return QPoint( this->ui->sbHor2->value(), this->ui->sbVert2->value() );
  }
  case 2:{
    return QPoint( this->ui->sbHor3->value(), this->ui->sbVert3->value() );
  }
  default:{
    return QPoint();
  }
  }
}

meteo::GeoPoint PinDialog::mapPoint( int index )
{
  switch (index) {
  case 0:{
    return this->ui->mapPoint1->coord();
  }
  case 1: {
    return this->ui->mapPoint2->coord();
  }
  case 2:{
    return this->ui->mapPoint3->coord();
  }
  default:{
    error_log << errInvalidValues;
    return meteo::GeoPoint();
  }
  }

}


void PinDialog::setImagePoint( int index, const QPoint& point )
{
  QSpinBox *hor = nullptr, *vert = nullptr;

  switch (index) {
  case 0:{
    hor = this->ui->sbHor1;
    vert = this->ui->sbVert1;
    break;
  }
  case 1: {
    hor = this->ui->sbHor2;
    vert = this->ui->sbVert2;
    break;
  }
  case 2:{
    hor = this->ui->sbHor3;
    vert = this->ui->sbVert3;
    break;
  }
  default: {
    error_log << errInvalidValues;
    return;
  }
  }
  hor->setValue(point.x());
  vert->setValue(point.y());
}

void PinDialog::setMapPoint( int index, const meteo::GeoPoint& mapPoint )
{
  switch (index) {
  case 0: {
    this->ui->mapPoint1->setCoord(mapPoint);
    break;
  }
  case 1:{
    this->ui->mapPoint2->setCoord(mapPoint);
    break;
  }
  case 2:{
    this->ui->mapPoint3->setCoord(mapPoint);
    break;
  }
  default:{
    error_log << errInvalidValues;
    break;
  }
  }
}

void PinDialog::slotSavePinning()
{
  PinningStorage::Pin *p = new PinningStorage::Pin();
  for ( int i = 0; i < meteo::map::Faxaction::markersCount; ++i ){
    p->mapPoints[i] = this->mapPoint(i);
    p->imagePoints[i] = this->imagePoint(i);
  }
  p->name = this->ui->lePinningName->text();
  p->conditions = this->currentPin.conditions;
  for ( auto column: p->conditions.keys() ){
    auto condition = p->conditions[column];
    if ( false == condition.isNull() ) {
      p->conditions[column] = condition;
    }
  }
  if ( true == this->globalStorage_->contains( docClass_, p->name ) ){
    meteo::app::MainWindow* mw = WidgetHandler::instance()->mainwindow();
    if ( false == mw->askUser(QObject::tr("Привязка с данным именем уже существует. Продолжить?")) ){
      return;
    }
  }
  this->globalStorage_->addPinning( docClass_, p);
  this->fillSavedPinningsList();
}

void PinDialog::slotTreeWidgetSavedPinningItemSelectionChanged()
{
  auto selectedItems = this->ui->twSavedPinningsList->selectedItems();
  if ( 1 == selectedItems.size() ){    
    auto name = selectedItems.first()->data(kDataColumn, Qt::UserRole).toString();
    if ( true == globalStorage_->contains( docClass_, name ) ) {
      this->currentPin = globalStorage_->getPin( docClass_, name );
    }
  }
  this->updatePinningInfo();
}

void PinDialog::updatePinningInfo()
{
  for ( int i = 0; i < meteo::map::Faxaction::markersCount; ++i ){
    this->setImagePoint( i, this->currentPin.imagePoints[i] );
    this->setMapPoint( i, this->currentPin.mapPoints[i] );
  }

  this->ui->lePinningName->setText( this->currentPin.name );
  this->setConditions( this->currentPin.conditions );
}

const PinningStorage::Pin& PinDialog::getPinning()
{
  return this->currentPin;
}

}
}
