#include "wgtdocviewpanel.h"
#include <cross-commons/debug/tlog.h>
#include "ui_wgtdocviewpanel.h"
#include <qlistwidget.h>
#include <qscrollbar.h>
#include "datamodel/wgtdocviewModel.h"
#include "custom/wgtdocviewpanel.h"

#include <meteo/commons/global/dateformat.h>
#include <meteo/commons/ui/custom/filedialogrus.h>
#include <meteo/commons/ui/documentviewer/wgtdocview/wgtdocview.h>

#include <meteo/commons/ui/conditionmaker/conditions/conditiondateinterval.h>
#include <meteo/commons/ui/conditionmaker/conditiondialogs/conditioncreationdialogint.h>
#include <meteo/commons/ui/conditionmaker/conditiondialogs/conditionaldialogstring.h>
#include <meteo/commons/ui/conditionmaker/conditions/conditionstring.h>
#include <meteo/commons/ui/conditionmaker/conditions/conditionint.h>
#include "custom/pindialog.h"

namespace meteo {
namespace documentviewer {

static const QString treeConfigPath = meteo::global::kDocumentViewerConfigPath + QObject::tr("panel.bin");

static const QIcon iconAdd(QObject::tr(":/meteo/icons/plus.png"));
static const QIcon iconRemove (QObject::tr(":/meteo/icons/minus.png"));

static QHash<WgtDocViewPanel::TreeWidgetsColumns, QString> fillDisplayColumnNames()
{
  QHash<WgtDocViewPanel::TreeWidgetsColumns, QString> displayColumnNames;
  displayColumnNames[WgtDocViewPanel::kHiddenDataColumn] = QString();
  displayColumnNames[WgtDocViewPanel::kFaxTT] = QObject::tr("TT");
  displayColumnNames[WgtDocViewPanel::kFaxAA] = QObject::tr("AA");
  displayColumnNames[WgtDocViewPanel::kFaxCCCC] = QObject::tr("CCCC");
  displayColumnNames[WgtDocViewPanel::kFaxII] = QObject::tr("ii");
  displayColumnNames[WgtDocViewPanel::kFaxDateColumn] = QObject::tr("Срок");
  displayColumnNames[WgtDocViewPanel::kSateliteTT] = QObject::tr("TT");
  displayColumnNames[WgtDocViewPanel::kSateliteAA] = QObject::tr("AA");
  displayColumnNames[WgtDocViewPanel::kSateliteCCCC] = QObject::tr("CCCC");
  displayColumnNames[WgtDocViewPanel::kSateliteII] = QObject::tr("ii");
  displayColumnNames[WgtDocViewPanel::kSateliteDateColumn] = QObject::tr("Срок");


  displayColumnNames[WgtDocViewPanel::kMapImageDateTime] = QObject::tr("Срок");
  displayColumnNames[WgtDocViewPanel::kMapImageName] = QObject::tr("Тип карты");
  displayColumnNames[WgtDocViewPanel::kMapImageHour] = QObject::tr("Срок прогноза");
  displayColumnNames[WgtDocViewPanel::kMapImageCenter] = QObject::tr("Центр");
  displayColumnNames[WgtDocViewPanel::kMapImageModel] = QObject::tr("Модель");
  displayColumnNames[WgtDocViewPanel::kMapImageJobName] = QObject::tr("Заголовок");


  displayColumnNames[WgtDocViewPanel::kFormalDocumentDateTime] = QObject::tr("Срок");
  displayColumnNames[WgtDocViewPanel::kFormalDocumentName] = QObject::tr("Тип документа");
  displayColumnNames[WgtDocViewPanel::kFormalDocumentHour] = QObject::tr("Срок прогноза");
  displayColumnNames[WgtDocViewPanel::kFormalDocumentCenter] = QObject::tr("Центр");
  displayColumnNames[WgtDocViewPanel::kFormalDocumentModel] = QObject::tr("Модель");
  displayColumnNames[WgtDocViewPanel::kFormalDocumentJobName] = QObject::tr("Заголовок");
  return displayColumnNames;
}
static const QHash<WgtDocViewPanel::TreeWidgetsColumns, QString> displayColumnNames = fillDisplayColumnNames();


static const char* propertyKey = "key";

QString generateDtDisplayText( const QString& name, const QDateTime& dtBegin, const QDateTime& dtEnd )
{
  return QObject::tr("%1: %2 - %3").arg(name)
      .arg(dateToHumanFull(dtBegin))
      .arg(dateToHumanFull(dtEnd));
}

WgtDocViewPanel::WgtDocViewPanel( DocumentHeader::DocumentClass defaultClass, bool pinningEnabled, QWidget *parent ) :
  QWidget(parent),
  ui(new ::Ui::WgtDocViewPanel),
  isPinningEnabled_(pinningEnabled)
{
  this->setObjectName("WgtDocViewPanel");
  ui->setupUi(this);

  dateSelectWidget_ = new SelectDateWidget();


  auto faxHeader = new meteo::bank::ViewHeader();
  faxHeader->setStretchLastSection(true);  
  faxHeader->setFilterColumns(QList<int>()
                              << kFaxTT
                              << kFaxAA
                              << kFaxCCCC
                              << kFaxII
                              << kFaxDateColumn
                              << kSateliteTT
                              << kSateliteAA
                              << kSateliteCCCC
                              << kSateliteII
                              << kSateliteDateColumn
                              << kMapImageDateTime
                              << kMapImageName
                              << kMapImageHour
                              << kMapImageCenter
                              << kMapImageModel
                              << kMapImageJobName
                              << kFormalDocumentCenter
                              << kFormalDocumentDateTime
                              << kFormalDocumentHour
                              << kFormalDocumentJobName
                              << kFormalDocumentModel
                              << kFormalDocumentName );
  faxHeader->setOriginalValueHidden(true);

  faxHeader->init();  
  faxHeader->buildMenu(QMap<int, QSet<QPair<QString, QString>>>());  
  QObject::connect( faxHeader, &QHeaderView::geometriesChanged, this, &WgtDocViewPanel::slotHeaderGeometryChanged );
  QObject::connect( faxHeader, &QHeaderView::sectionMoved, this, &WgtDocViewPanel::slotHeaderGeometryChanged );
  QObject::connect( faxHeader, &QHeaderView::sectionResized, this, &WgtDocViewPanel::slotHeaderGeometryChanged );
  QObject::connect( faxHeader, SIGNAL(activate(int, QAction*)), this, SLOT(slotFilterActivated(int, QAction*)) );
  this->ui->twFaxDocuments->setHeader(faxHeader);

  QObject::connect( ui->wgtConditionsPanel, SIGNAL(panelClosed()), this, SLOT(slotConditionsPanelClosed()) );

  this->ui->cbDocumentClass->addItem( DocumentHeader::documentsDisplayNames[DocumentHeader::kSateliteDocument], DocumentHeader::kSateliteDocument );
  this->ui->cbDocumentClass->addItem( DocumentHeader::documentsDisplayNames[DocumentHeader::kFax], DocumentHeader::kFax );
  this->ui->cbDocumentClass->addItem( DocumentHeader::documentsDisplayNames[DocumentHeader::kMapImage], DocumentHeader::kMapImage);
  this->ui->cbDocumentClass->addItem( DocumentHeader::documentsDisplayNames[DocumentHeader::kLocalDocument], DocumentHeader::kLocalDocument );
  this->ui->cbDocumentClass->addItem( DocumentHeader::documentsDisplayNames[DocumentHeader::kFormalDocument], DocumentHeader::kFormalDocument );
  this->ui->pbReload->setIcon(QIcon(":/meteo/icons/map/rotate.png"));
  this->ui->pbReload->setToolTip(QObject::tr("Обновить"));
  connect(this->ui->pbReload, &QPushButton::clicked, this, &WgtDocViewPanel::slotOnPbReloadClicked);

  for (int i = 0; i < this->ui->cbDocumentClass->count(); ++i ){
    if ( this->ui->cbDocumentClass->itemData(i).toInt() == defaultClass ){
      this->ui->cbDocumentClass->setCurrentIndex(i);
      break;
    }
  }



  QObject::connect( ui->cbDocumentClass, SIGNAL(currentIndexChanged(int)), this, SLOT(slotComboBoxSelectionChanged()) );

  this->ui->twFaxDocuments->setColumnCount(kColumnCount);
  this->ui->twFaxDocuments->setRootIsDecorated(false);
  QStringList headerLabels;
  for ( int i = 0; i < kColumnCount; ++i ){
    TreeWidgetsColumns column = static_cast<TreeWidgetsColumns>(i);
    if ( true == displayColumnNames.contains(column) ) {
      headerLabels << displayColumnNames[column];
    }
    else {
      error_log << QObject::tr("Ошибка - наименование колонки не обнаружено: %1").arg(i);
    }
  }
  this->ui->twFaxDocuments->setHeaderLabels( headerLabels );

  QObject::connect( this->ui->twFaxDocuments, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
                    this, SLOT(slotCurrentDocumentChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

  QObject::connect( this->ui->wgtFileDystem, SIGNAL(signalFileSelected(QString)), this, SLOT(slotFileSelected(const QString&)));
  this->setAttribute(Qt::WA_NoSystemBackground);

  {
    auto data = DocumentHeader::getLocalDocumentTypes();
    QStringList allTypes;
    for ( auto types : data.keys() ) {
      allTypes << types;
    }
    this->ui->wgtFileDystem->setNameFilters(allTypes);
  }

  QObject::connect( this->ui->pbPinToMapEnable, SIGNAL(pressed()), this, SLOT(slotPinToMapEnable()) );
  QObject::connect( this->ui->pbPinToMapAddImageToMap, SIGNAL(pressed()), this, SLOT(slotPinToMapAddImageToMap()) );
  QObject::connect( this->ui->pbPinToMapRemoveImageFromMap, SIGNAL(pressed()), this, SLOT(slotPinRemoveImageFromMap()) );


  this->ui->pbPinToMapEnable->setText(QObject::tr("Открыть карту"));
  this->ui->pbPinToMapAddImageToMap->setText(QObject::tr("Добавить"));
  this->ui->pbPinToMapAddImageToMap->setToolTip(QObject::tr("Добавить текущее изображение на карту"));
  this->ui->pbPinToMapRemoveImageFromMap->setText(QObject::tr("Удалить"));
  this->ui->pbPinToMapRemoveImageFromMap->setText(QObject::tr("Удалить выбранные изображения с карты"));

  this->ui->transparencyComboBox->addItem( QObject::tr("Нет"), DataModel::kTransparencyNone );
  this->ui->transparencyComboBox->addItem( QObject::tr("Белый"), DataModel::kTransparencyWhite );
  this->ui->transparencyComboBox->addItem( QObject::tr("Черный"), DataModel::kTransparencyBlack );
  QObject::connect( this->ui->transparencyComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotGbTransparencySelectionChanged(int)) );
  QObject::connect( this->ui->transparencySlider, SIGNAL(valueChanged(int)), this, SLOT(slotSliderTransparencyValueChanged()) );

  this->restoreControlsVisibility();
  QTimer::singleShot(0, this, SLOT(slotInitialLoad()) );

  QObject::connect( this->ui->twFaxDocuments, SIGNAL(itemSelectionChanged()), this, SLOT(slotTreeWidgetSelectionChanged()), Qt::QueuedConnection );


  this->ui->twFaxDocuments->setContextMenuPolicy(Qt::CustomContextMenu);  
  QObject::connect( this->ui->twFaxDocuments, SIGNAL(customContextMenuRequested(const QPoint &)),  this, SLOT(slotShowContextMenuForDocumentWidget(const QPoint &)) );
}

WgtDocViewPanel::~WgtDocViewPanel()
{
  this->treeStatesSaveToFS();
  delete ui;
  if ( nullptr != dateSelectWidget_ ){
    delete dateSelectWidget_;
  }
  for ( auto doc: this->localDocuments_.values() ){
    delete  doc;
  }
  while ( false == this->conditions_.isEmpty() ){
    auto key = this->conditions_.keys().first();    
    this->conditions_.remove(key);    
  }
}

void WgtDocViewPanel::slotShowContextMenuForDocumentWidget( const QPoint& pnt )
{
  QMenu contextMenu;

  QAction actitonSelectAll( QObject::tr("Выбрать все"),nullptr );
  QAction actionAddDocumentToMap( QObject::tr("Добавить на карту"),nullptr );
  QAction actionRemoveDocumentFromMap( QObject::tr("Удалить с карты"),nullptr );

  if ( true == this->isToolbarAnimationActive() ) {
    int totalSelected = this->selectedItems().size();
    int total = this->ui->twFaxDocuments->topLevelItemCount();

    actitonSelectAll.setEnabled( totalSelected != total );    
    QObject::connect(&actitonSelectAll, SIGNAL(triggered(bool)), this, SLOT(slotActionAnimateAll()) );
    contextMenu.addAction( &actitonSelectAll );
  }
  if ( true == this->isPinningEnabled() ){
    if ( false == this->isItemMarked( this->currentItem() ) ) {
      QObject::connect( &actionAddDocumentToMap, SIGNAL(triggered(bool)), this, SLOT(slotPinToMapAddImageToMap()) );
      actionAddDocumentToMap.setIcon(iconAdd);
      contextMenu.addAction(&actionAddDocumentToMap);
    }
    else {
      QObject::connect( &actionRemoveDocumentFromMap, SIGNAL(triggered(bool)), this, SLOT(slotRemoveCurrentDocumentFromMap()) );
      actionRemoveDocumentFromMap.setIcon(iconRemove);
      contextMenu.addAction(&actionRemoveDocumentFromMap);
    }
  }
  contextMenu.exec( this->ui->twFaxDocuments->mapToGlobal(pnt) );
}

void WgtDocViewPanel::slotRemoveCurrentDocumentFromMap()
{
  emit signalAction(kPinningRemoveCurrentDocumentFromMap);
}

void WgtDocViewPanel::slotActionAnimateAll()
{
  QList<QTreeWidgetItem*> items;
  for ( int i = 0; i < this->ui->twFaxDocuments->topLevelItemCount(); ++i ){
    items << this->ui->twFaxDocuments->topLevelItem(i);
  }
  this->setItemsSelected( items, true );
}

void WgtDocViewPanel::slotActionAnimateNone()
{
  QList<QTreeWidgetItem*> items;
  for ( int i = 0; i < this->ui->twFaxDocuments->topLevelItemCount(); ++i ){
    items << this->ui->twFaxDocuments->topLevelItem(i);
  }
  this->setItemsSelected( items, false );
}

void WgtDocViewPanel::setItemsSelected( const QList<QTreeWidgetItem*> items, bool isSelected )
{
  for ( auto item: items ){
    item->setSelected( isSelected );
  }
}

void WgtDocViewPanel::slotSliderTransparencyValueChanged( )
{
  emit signalAction(kTransparencyChanged);
}

void WgtDocViewPanel::slotSliderAnimationSlideValueChanged(int value)
{
  auto treeWidgetItems = this->selectedItems();
  if ( ( value >= 0 ) && ( value < treeWidgetItems.count() ) ){
    this->setCurrentItem( treeWidgetItems[value] );
  }
}

void WgtDocViewPanel::slotGbTransparencySelectionChanged( int index ){
  DataModel::TransparencyMode trSettings = static_cast<DataModel::TransparencyMode>(this->ui->transparencyComboBox->itemData(index, Qt::UserRole).toInt());
  switch (trSettings) {
  case DataModel::kTransparencyBlack:
  case DataModel::kTransparencyWhite:{
    this->ui->transparencySlider->setEnabled(true);
    break;
  }
  case DataModel::kTransparencyNone:{
    this->ui->transparencySlider->setEnabled(false);
    break;
  }
  }
  emit signalAction(kTransparencyChanged);

}

DataModel::TransparencyMode WgtDocViewPanel::getTranspacencyMode()
{
  return static_cast<DataModel::TransparencyMode>(this->ui->transparencyComboBox->currentData(Qt::UserRole).toInt());
}

int WgtDocViewPanel::getTransparencyValue()
{
  return this->ui->transparencySlider->value();
}

void WgtDocViewPanel::addCondition( TreeWidgetsColumns column, Condition condition )
{
  this->ui->wgtConditionsPanel->setVisible(true);
  emit signalConditionAdd( column, condition );

  QCheckBox* check;  
  if ( true == this->conditionsCheckBoxes_.contains(column) )
  {
    check = this->conditionsCheckBoxes_[column];
  }
  else
  {
    check = new QCheckBox();    
    QObject::connect( check, SIGNAL(stateChanged(int)), this, SLOT(slotCheckActivated()) );
    this->conditionsCheckBoxes_[column] = check;
    this->ui->wgtConditionsPanel->addCheckBox(check);
  }

  check->setProperty(propertyKey, column);
  this->conditions_[column] = condition;
  check->blockSignals(true);
  check->setCheckState(Qt::Checked);
  check->blockSignals(false);
  check->setText(condition.displayCondition());  
}

void WgtDocViewPanel::disableCondition( TreeWidgetsColumns column )
{
  emit signalConditionRemove(column);
}

void WgtDocViewPanel::removeCondition( TreeWidgetsColumns column )
{
  if ( true == this->conditionsCheckBoxes_.contains(column) ){
    auto check = this->conditionsCheckBoxes_[column];
    this->conditionsCheckBoxes_.remove(column);
    auto oldCondition = this->conditions_[column];
    this->conditions_.remove(column);    
    delete check;
  }
  this->disableCondition(column);
  auto faxHeader = qobject_cast<meteo::bank::ViewHeader*>(this->ui->twFaxDocuments->header());
  faxHeader->setFilterEnabled(column, false);
}

void WgtDocViewPanel::generateCustomCondition(TreeWidgetsColumns col)
{
  auto faxHeader = qobject_cast<meteo::bank::ViewHeader*>(this->ui->twFaxDocuments->header());
  switch (col) {  
  case kHiddenDataColumn: {
    error_log << QObject::tr("NOT IMPLEMENTED");
    break;
  }
  case kFaxDateColumn:
  case kSateliteDateColumn:
  case kMapImageDateTime:
  case kFormalDocumentDateTime:{
    this->dateSelectWidget_->exec();    
    if ( QDialog::Accepted == this->dateSelectWidget_->result() ) {
      QDateTime dtBeg = this->dateSelectWidget_->dt_start(),
          dtEnd = this->dateSelectWidget_->dt_end();
      this->addCondition( col, ConditionDateTimeInterval( displayColumnNames[col], displayColumnNames[col], dtBeg, dtEnd ) );
    }
    else {
      faxHeader->setFilterEnabled(col, false);
    }
    break;
  }
  case kFaxII:
  case kSateliteII:
  case kMapImageCenter:  case kMapImageHour:
  case kMapImageModel:
  case kFormalDocumentCenter:
  case kFormalDocumentHour:
  case kFormalDocumentModel: {
    ConditionCreationDialogInt dialog(displayColumnNames[col], displayColumnNames[col], 0, 1000 );
    if ( dialog.exec() == QDialog::Accepted ){
      this->addCondition( col, dialog.getCondition() );
    }
    else {
      faxHeader->setFilterEnabled(col, false);
    }
    break;
  }
  case kSateliteTT:
  case kSateliteAA:
  case kSateliteCCCC:
  case kFaxTT:
  case kFaxAA:
  case kFaxCCCC:
  case kMapImageJobName:
  case kMapImageName:
  case kFormalDocumentJobName:
  case kFormalDocumentName: {
    ConditionalDialogString dialog(displayColumnNames[col], displayColumnNames[col]);
    if ( QDialog::Accepted == dialog.exec() ){
      this->addCondition(col, dialog.getCondition());
    }
    else {
      faxHeader->setFilterEnabled(col, false);
    }
    break;
  }
  }
}

void WgtDocViewPanel::slotFilterActivated( int colIndex, QAction* colMenu )
{
  TreeWidgetsColumns col =  static_cast<TreeWidgetsColumns>(colIndex);
  auto data = colMenu->data();
  if ( data == meteo::bank::ViewHeader::conditionNoConditions_ ) {
    this->removeCondition(col );
  }
  else if ( data == meteo::bank::ViewHeader::conditionCustom_ ) {
    this->generateCustomCondition(col);
  }
  else {
    auto text = colMenu->text();
    auto value = colMenu->data().toString();
    switch (col) {
    case kHiddenDataColumn:
    case kMapImageDateTime:
    case kFaxDateColumn:
    case kSateliteDateColumn:
    case kFormalDocumentDateTime:{
      error_log << QObject::tr("Непредвиденное поведение");
      break;
    }
    case kFaxTT:
    case kFaxAA:
    case kFaxCCCC:
    case kSateliteTT:
    case kSateliteAA:
    case kSateliteCCCC:
    case kMapImageJobName:
    case kMapImageName:
    case kFormalDocumentJobName:
    case kFormalDocumentName:{      
      auto condition = ConditionString( displayColumnNames[col], ConditionString::kMatch, false, Qt::CaseSensitive, value );
      this->addCondition( col, condition );
      break;
    }
    case kFaxII:
    case kSateliteII:
    case kMapImageCenter:
    case kMapImageHour:
    case kMapImageModel:
    case kFormalDocumentCenter:
    case kFormalDocumentHour:
    case kFormalDocumentModel:{      
      auto condition = ConditionInt32( displayColumnNames[col], ConditionInt32::kEqual, value.toInt());
      this->addCondition( col, condition );      
      break;
    }
    }
  }
}

bool WgtDocViewPanel::isToolbarAnimationActive()
{
  switch (this->getCurrentDocumentClass()) {
  case DocumentHeader::kFax:
  case DocumentHeader::kMapImage:
  case DocumentHeader::kSateliteDocument:{
    return true;
  }
  case DocumentHeader::kAmbiguous:
  case DocumentHeader::kFormalDocument:
  case DocumentHeader::kLocalDocument:{
    return false;
  }
  }
  return false;
}

void WgtDocViewPanel::setDocumentList(const QList<DocumentHeader*> *documentsInfo)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  this->clearDocumentList();
  this->addDocuments(*documentsInfo);
  qApp->processEvents();
  if ( 0 != ui->twFaxDocuments->topLevelItemCount() ) {
    ui->twFaxDocuments->setCurrentItem(ui->twFaxDocuments->topLevelItem(0));
  }

  QApplication::restoreOverrideCursor();
}

QStringList WgtDocViewPanel::selectedDocuments()
{
  auto selectedItems = this->selectedItems();
  QStringList documents;
  for ( auto item: selectedItems ){
    auto data = item->data( kHiddenDataColumn, kTreeItemDataRole ).toString();
    documents << data;
  }
  return documents;
}



DocumentHeader::DocumentClass WgtDocViewPanel::getCurrentDocumentClass()
{
  DocumentHeader::DocumentClass currentClass = static_cast<DocumentHeader::DocumentClass>(ui->cbDocumentClass->currentData().toInt());
  return currentClass;
}

DocumentHeader *WgtDocViewPanel::getDocumentById(const QString &id)
{
  switch (getCurrentDocumentClass()) {
    case DocumentHeader::DocumentClass::kFax:
    if ( true == faxDocuments_.contains(id) ) {
      return &faxDocuments_[id];
    }
    error_log << QObject::tr("Неверный класс документа");
    break;
    case DocumentHeader::DocumentClass::kMapImage:
      if ( true == mapImageDocuments_.contains(id) ) {
        return &mapImageDocuments_[id];
      }
      error_log << QObject::tr("Неверный класс документа");
      break;
    case DocumentHeader::DocumentClass::kFormalDocument:
      if ( true == formalDocuments_.contains(id) ) {
        return &formalDocuments_[id];
      }
      error_log << QObject::tr("Неверный класс документа");
      break;
    case DocumentHeader::DocumentClass::kSateliteDocument:
      if ( true == satelliteDocuments_.contains(id) ) {
        return &satelliteDocuments_[id];
      }
      error_log << QObject::tr("Неверный класс документа");
      break;
  case DocumentHeader::kAmbiguous:
  case DocumentHeader::DocumentClass::kLocalDocument:
    error_log << QObject::tr("Неверный класс документа");
    break;
  }
  return &dummy_;
}

void WgtDocViewPanel::slotImagePinningRemoved( const QString& name )
{
  this->markedItems_.removeAll(name);
  this->updateItemsMarks();
  //this->markItemByName(this->getCurrentDocumentClass(), name, false );
}

void WgtDocViewPanel::slotImagePinningAdded( const QString& name)
{
  this->markedItems_ << name;
  this->updateItemsMarks();
  //this->markItemByName(this->getCurrentDocumentClass(), name, true);
}

void WgtDocViewPanel::updateItemsMarks()
{
  QTreeWidget *wgt = nullptr;
  switch (this->getCurrentDocumentClass()) {
  case DocumentHeader::kFormalDocument:
  case DocumentHeader::kMapImage:
  case DocumentHeader::kFax:
  case DocumentHeader::kSateliteDocument:{
    wgt = this->ui->twFaxDocuments;
    break;
  }
  case DocumentHeader::kAmbiguous:
  case DocumentHeader::kLocalDocument:{
    break;
  }
  }

  if ( nullptr != wgt ){
    for ( int i = 0; i < wgt->topLevelItemCount(); ++i ){
      auto item = wgt->topLevelItem(i);
      auto data = item->data(kHiddenDataColumn, kTreeItemDataRole).toString();
      auto doc = getDocumentById(data);
      auto fileId = doc->getFileId();
      this->markTreeWidgetItem(item, this->markedItems_.contains(fileId));
    }
  }
}

void WgtDocViewPanel::markItemByName( DocumentHeader::DocumentClass docClass, const QString& name, bool mark )
{
  QTreeWidget *wgt = nullptr;
  switch (docClass) {
  case DocumentHeader::kFormalDocument:
  case DocumentHeader::kMapImage:
  case DocumentHeader::kFax:
  case DocumentHeader::kSateliteDocument:{
    wgt = this->ui->twFaxDocuments;
    break;
  }
  case DocumentHeader::kAmbiguous:
  case DocumentHeader::kLocalDocument:{
    break;
  }
  }

  if ( wgt != nullptr ) {
    for ( int i = 0; i < wgt->topLevelItemCount(); ++i ){
      auto item = wgt->topLevelItem(i);
      auto data = item->data(kHiddenDataColumn, kTreeItemDataRole).toString();
      auto doc = getDocumentById(data);
      if ( doc->getFileId() == name ){
        this->markTreeWidgetItem(item, mark);
      }
    }
  }
}

bool WgtDocViewPanel::isItemsAnyMarked( const QList<QTreeWidgetItem*> items )
{
  for ( auto item: items )  {
    if ( true == isItemMarked(item) ){
      return true;
    }
  }
  return false;
}


bool WgtDocViewPanel::isItemMarked( QTreeWidgetItem* item )
{
  if ( nullptr != item ){
    return item->data( kHiddenDataColumn, kTreeItemMarkRole).toBool();
  }
  return false;
}

void WgtDocViewPanel::markTreeWidgetItem( QTreeWidgetItem* item, bool mark )
{
  if ( nullptr != item ){
    bool oldMark = this->isItemMarked(item);
    if ( oldMark != mark ) {
      for ( int i = 0; i < item->columnCount(); ++i )  {
        item->setData(kHiddenDataColumn, kTreeItemMarkRole, mark);
        item->setBackground(i, true == mark? Qt::yellow : Qt::white );
      }
    }
    this->restoreTreeControlsVisibility();
  }
}

void WgtDocViewPanel::restoreTreeControlsVisibility()
{  
  this->ui->pbPinToMapAddImageToMap->setHidden(true);  
  this->ui->pbPinToMapRemoveImageFromMap->setHidden(true);  
  if ( true == this->isPinningEnabled() ){
     auto selectedItems = this->selectedItems();
     int pinnedCnt = 0;
     for ( auto item : selectedItems ){
       bool isMarked = this->isItemMarked(item);
       pinnedCnt += (true == isMarked)? 1: 0;
     }

     this->ui->pbPinToMapRemoveImageFromMap->setHidden( pinnedCnt == 0 );     
     this->ui->pbPinToMapAddImageToMap->setHidden( true == this->isItemMarked(this->currentItem()) );;
  }
  this->ui->pbPinToMapEnable->setHidden( true == this->isPinningEnabled() );
}

void WgtDocViewPanel::setDocumentListEnabled( bool enabled )
{
  this->ui->twFaxDocuments->setEnabled(enabled);
  this->ui->wgtFileDystem->setEnabled(enabled);
}

QList<WgtDocViewPanel::TreeWidgetsColumns> WgtDocViewPanel::columnsForDocumentClass( DocumentHeader::DocumentClass docClass )
{
  switch (docClass) {
  case DocumentHeader::kFormalDocument:{
    return { kFormalDocumentCenter, kFormalDocumentDateTime, kFormalDocumentHour, kFormalDocumentJobName, kFormalDocumentModel, kFormalDocumentName };
  }
  case DocumentHeader::kMapImage:{
    return { kMapImageDateTime, kMapImageName, kMapImageHour, kMapImageCenter, kMapImageModel, kMapImageJobName };
  }
  case DocumentHeader::kFax:{
    return { kFaxTT, kFaxAA, kFaxCCCC, kFaxII, kFaxDateColumn };
  }
  case DocumentHeader::kSateliteDocument:{
    return { kSateliteTT, kSateliteAA, kSateliteCCCC, kSateliteII, kSateliteDateColumn };
  }
  case DocumentHeader::kAmbiguous:
  case DocumentHeader::kLocalDocument:{
    break;
  }
  }
  return QList<TreeWidgetsColumns>();
}
void WgtDocViewPanel::restoreControlsVisibility()
{  
  bool wgtToolbarUnderListHidden = true;
  bool cbDocumentClassHidden = false;
  bool twFaxDocumentsHidden = false;
  bool wgtFileDystemHidden = true;
  bool wgtConditionsPanelHidden = false;
  bool transparencyGroupBoxHidden = true;

  this->restoreTreeControlsVisibility();
  DocumentHeader::DocumentClass docClass = getCurrentDocumentClass();


  switch (docClass) {
  case DocumentHeader::kAmbiguous:
  case DocumentHeader::kFormalDocument:{
    break;
  }
  case DocumentHeader::kMapImage:{
    wgtToolbarUnderListHidden = false;
    cbDocumentClassHidden = true == isPinningEnabled();
    transparencyGroupBoxHidden = (false == isPinningEnabled());
    break;
  }
  case DocumentHeader::kFax:{
    wgtToolbarUnderListHidden = false;
    cbDocumentClassHidden = ( true == isPinningEnabled() );    
    break;
  }
  case DocumentHeader::kSateliteDocument:{
    wgtToolbarUnderListHidden = false;
    transparencyGroupBoxHidden = (false == isPinningEnabled());
    cbDocumentClassHidden = (true == isPinningEnabled());
    break;
  }
  case DocumentHeader::kLocalDocument:{    
    twFaxDocumentsHidden = true;
    wgtFileDystemHidden = false;
    wgtConditionsPanelHidden = true;
    break;
  }
  }

  this->ui->wgtToolbarUnderList->setHidden(wgtToolbarUnderListHidden);
  this->ui->cbDocumentClass->setHidden( cbDocumentClassHidden );
  this->ui->twFaxDocuments->setHidden(twFaxDocumentsHidden);
  this->ui->wgtFileDystem->setHidden(wgtFileDystemHidden);
  this->ui->wgtConditionsPanel->setHidden(wgtConditionsPanelHidden);
  this->ui->transparencyGroupBox->setHidden(transparencyGroupBoxHidden);
  this->layout()->update();
}

void WgtDocViewPanel::slotFileSelected( const QString& fileName )
{
  Q_UNUSED(fileName);
  emit signalAction(kCurrentDocumentChanged);
}

void WgtDocViewPanel::slotInitialLoad()
{
  slotGbTransparencySelectionChanged( this->ui->transparencyComboBox->currentIndex() );  
  this->treeStatesRestoreFromFs();
  emit signalDocumentClassChanged( this->getCurrentDocumentClass() );
  this->slotComboBoxSelectionChanged();
  emit signalPanelDone();
}

void WgtDocViewPanel::slotComboBoxSelectionChanged( )
{
  this->ui->wgtConditionsPanel->clearPanel();
  conditionsCheckBoxes_.clear();
  this->clearDocumentList();
  this->treeStateRestore();
  DocumentHeader::DocumentClass docClass = getCurrentDocumentClass();  
  emit signalDocumentClassChanged( docClass );
  this->restoreControlsVisibility();
  switch (getCurrentDocumentClass()) {
  case DocumentHeader::kFormalDocument:{
    QDateTime dtBeg = this->dateSelectWidget_->dt_start(),
        dtEnd = this->dateSelectWidget_->dt_end();
    this->addCondition( kFormalDocumentDateTime,
                        ConditionDateTimeInterval(displayColumnNames[kFormalDocumentDateTime],
                                                  displayColumnNames[kFormalDocumentDateTime],
                                                  dtBeg,
                                                  dtEnd ));
    break;
  }
  case DocumentHeader::kMapImage:{
    QDateTime dtBeg = this->dateSelectWidget_->dt_start(),
        dtEnd = this->dateSelectWidget_->dt_end();
    this->addCondition( kMapImageDateTime,
                        ConditionDateTimeInterval(displayColumnNames[kMapImageDateTime],
                                                  displayColumnNames[kMapImageDateTime],
                                                  dtBeg,
                                                  dtEnd ));
    break;
  }
  case DocumentHeader::kFax:{
    QDateTime dtBeg = this->dateSelectWidget_->dt_start(),
        dtEnd = this->dateSelectWidget_->dt_end();
    this->addCondition( kFaxDateColumn,
                        ConditionDateTimeInterval(displayColumnNames[kFaxDateColumn],
                                                  displayColumnNames[kFaxDateColumn],
                                                  dtBeg,
                                                  dtEnd ));
    break;
  }
  case DocumentHeader::kSateliteDocument:{
    QDateTime dtBeg = this->dateSelectWidget_->dt_start(),
        dtEnd = this->dateSelectWidget_->dt_end();
    this->addCondition( kSateliteDateColumn,
                        ConditionDateTimeInterval(displayColumnNames[kSateliteDateColumn],
                                                  displayColumnNames[kSateliteDateColumn],
                                                  dtBeg,
                                                  dtEnd ));
    break;
  }
  case DocumentHeader::kAmbiguous:
  case DocumentHeader::kLocalDocument:{
    break;
  }
  }  
}

DocumentHeader* WgtDocViewPanel::currentDocument()
{
  if ( DocumentHeader::kLocalDocument == this->getCurrentDocumentClass() ) {
    auto fileName = this->ui->wgtFileDystem->getSelectedFile();
    if ( false == QFile::exists(fileName) ) {
      return nullptr;
    }

    if ( false == localDocuments_.contains(fileName) ) {
      QFileInfo info(fileName);
      localDocuments_.insert( fileName, new DocumentLocal(fileName, info.created()) );
    }
    return localDocuments_[fileName];
  }
  auto currentItem = this->ui->twFaxDocuments->currentItem();
  QString id;
  if ( nullptr != currentItem ) {
    id = currentItem->data(kHiddenDataColumn, Qt::UserRole ).toString();
  }
  else {
    return nullptr;
  }
  DocumentHeader* doc = nullptr;;
  switch (this->getCurrentDocumentClass() ) {
  case DocumentHeader::kFax:
    if ( true == faxDocuments_.contains(id) ) {
      doc = &faxDocuments_[id];
      break;
    }
    else {
      return nullptr;
    }
  case DocumentHeader::kMapImage:
    if ( true == mapImageDocuments_.contains(id) ) {
      doc = &mapImageDocuments_[id];
      break;
    }
    else {
      return nullptr;
    }
  case DocumentHeader::kSateliteDocument:
    if ( true == satelliteDocuments_.contains(id) ) {
      doc = &satelliteDocuments_[id];
      break;
    }
    else {
      return nullptr;
    }
  case DocumentHeader::kFormalDocument:
    if ( true == formalDocuments_.contains(id) ) {
      doc = &formalDocuments_[id];
      break;
    }
    else {
      return nullptr;
    }
  case DocumentHeader::kAmbiguous:
  case DocumentHeader::kLocalDocument:
    return nullptr;
  }
  return doc;
}

const QDateTime WgtDocViewPanel::getDateBegin()
{
  return this->dateSelectWidget_->dt_start();
}

const QDateTime WgtDocViewPanel::getDateEnd()
{
  return this->dateSelectWidget_->dt_end();
}

void WgtDocViewPanel::clearDocumentList()
{
  this->ui->twFaxDocuments->clear();
}

void WgtDocViewPanel::slotCurrentDocumentChanged(QTreeWidgetItem *newItem, QTreeWidgetItem *oldItem)
{
  if ( nullptr != newItem ){
    for ( int i = 0; i < newItem->columnCount(); ++i ){
      auto font = newItem->font(i);
      font.setUnderline(true);
      newItem->setFont(i,font);
    }
  }

  if ( nullptr != oldItem ){
    for ( int i = 0; i < oldItem->columnCount(); ++i ){
      auto font = oldItem->font(i);
      font.setUnderline(false);
      oldItem->setFont(i,font);
    }
  }

  emit signalAction(kCurrentDocumentChanged);
}


void WgtDocViewPanel::addDocuments(const QList<DocumentHeader*> &documents)
{
  static const QString& errorDocumentClassCast = QObject::tr("Ошибка при определении изначального типа документа");
  auto currentDocumentClass = this->getCurrentDocumentClass();
  QMap<int,QSet< QPair<QString, QString> > > menu;

  QList<QTreeWidgetItem*> allTreeWidgetItems;
  for ( auto document: documents ){
    switch (currentDocumentClass) {
    case DocumentHeader::kFormalDocument:{
      if ( document->getClass() != DocumentHeader::kFormalDocument ) {
        error_log << errorDocumentClassCast;
        continue;
      }
      auto formalDocument = *(static_cast<DocumentFormal*>(document));
      formalDocuments_[formalDocument.getFileId()] = formalDocument;
      QTreeWidgetItem* item = new QTreeWidgetItem();
      item->setFlags( item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
      item->setData( kHiddenDataColumn, Qt::UserRole, formalDocument.getFileId() );
      item->setData( kFormalDocumentDateTime, Qt::DisplayRole, dateToHumanFull(formalDocument.getDt()));
      item->setData( kFormalDocumentName, Qt::DisplayRole, formalDocument.getName() );
      item->setData( kFormalDocumentHour, Qt::DisplayRole, formalDocument.getHour() );
      item->setData( kFormalDocumentCenter, Qt::DisplayRole, formalDocument.getCenter() );
      item->setData( kFormalDocumentModel, Qt::DisplayRole, formalDocument.getModel() );
      item->setData( kFormalDocumentJobName, Qt::DisplayRole, formalDocument.getJobName() );
      markTreeWidgetItem( item, false );

      menu[kFormalDocumentName] << QPair<QString, QString>( formalDocument.getName(), formalDocument.getName() );
      menu[kFormalDocumentHour] << QPair<QString, QString>( QString::number(formalDocument.getHour()), QString::number(formalDocument.getHour()) );
      menu[kFormalDocumentCenter] << QPair<QString, QString>( QString::number(formalDocument.getCenter()), QString::number(formalDocument.getCenter()) );
      menu[kFormalDocumentModel] << QPair<QString, QString>( QString::number(formalDocument.getModel()),  QString::number(formalDocument.getModel()) );
      menu[kFormalDocumentJobName] << QPair<QString, QString>( formalDocument.getJobName(), formalDocument.getJobName() );
      allTreeWidgetItems << item;
      break;
    }
    case DocumentHeader::kMapImage:{
      if ( document->getClass() != DocumentHeader::kMapImage ) {
        error_log << errorDocumentClassCast;
        continue;
      }
      auto mapImage = *(static_cast<DocumentMapImage*>(document));
      mapImageDocuments_[mapImage.getFileId()] = mapImage;
      QTreeWidgetItem* item = new QTreeWidgetItem();
      item->setFlags( item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
      item->setData( kHiddenDataColumn, Qt::UserRole, mapImage.getFileId() );
      item->setData( kMapImageDateTime, Qt::DisplayRole, dateToHumanFull(mapImage.getDt()));
      item->setData( kMapImageName, Qt::DisplayRole, mapImage.getName() );
      item->setData( kMapImageHour, Qt::DisplayRole, mapImage.getHour() );
      item->setData( kMapImageCenter, Qt::DisplayRole, mapImage.getCenter() );
      item->setData( kMapImageModel, Qt::DisplayRole, mapImage.getModel() );
      item->setData( kMapImageJobName, Qt::DisplayRole, mapImage.getJobName() );
      markTreeWidgetItem( item, false );

      menu[kMapImageName] << QPair<QString, QString>( mapImage.getName(), mapImage.getName() );
      menu[kMapImageHour] << QPair<QString, QString>( QString::number(mapImage.getHour()), QString::number(mapImage.getHour()) );
      menu[kMapImageCenter] << QPair<QString, QString>( QString::number(mapImage.getCenter()), QString::number(mapImage.getCenter()) );
      menu[kMapImageModel] << QPair<QString, QString>( QString::number(mapImage.getModel()),  QString::number(mapImage.getModel()) );
      menu[kMapImageJobName] << QPair<QString, QString>( mapImage.getJobName(), mapImage.getJobName() );

      allTreeWidgetItems << item;
      break;
    }
    case DocumentHeader::kFax:{
      if ( document->getClass() != DocumentHeader::kFax ) {
        error_log << errorDocumentClassCast;
        continue;
      }
      auto faxDocument = *(static_cast<DocumentFax*>(document));
      faxDocuments_[faxDocument.getFileId()] = faxDocument;
      QTreeWidgetItem* item = new QTreeWidgetItem();
      item->setFlags( item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
      item->setData( kHiddenDataColumn, Qt::UserRole, faxDocument.getFileId() );
      item->setData( kFaxTT, Qt::DisplayRole, faxDocument.getT1T2() );
      item->setData( kFaxAA, Qt::DisplayRole, faxDocument.getA1A2() );
      item->setData( kFaxDateColumn, Qt::DisplayRole, dateToHumanFull(faxDocument.getDt()));
      item->setData( kFaxII, Qt::DisplayRole, faxDocument.getII() );
      item->setData( kFaxCCCC, Qt::DisplayRole, faxDocument.getCCCC() );
      markTreeWidgetItem(item, false);
      menu[kFaxTT] << QPair<QString, QString>( faxDocument.getT1T2(), faxDocument.getT1T2() );
      menu[kFaxAA] << QPair<QString, QString>( faxDocument.getA1A2(), faxDocument.getA1A2() );
      menu[kFaxCCCC] << QPair<QString, QString>( faxDocument.getCCCC(), faxDocument.getCCCC() );
      auto ii = QString::number(faxDocument.getII());
      menu[kFaxII] << QPair<QString, QString>( ii, ii );

      allTreeWidgetItems << item;
      break;
    }
    case DocumentHeader::kSateliteDocument:{
      if ( document->getClass() != DocumentHeader::kSateliteDocument ) {
        error_log << errorDocumentClassCast;
        continue;
      }
      auto sateliteDocument = *(static_cast<DocumentSatelite*>(document));
      satelliteDocuments_[sateliteDocument.getFileId()] = sateliteDocument;
      QTreeWidgetItem *item = new QTreeWidgetItem();
      item->setFlags( item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
      item->setData( kHiddenDataColumn, Qt::UserRole, sateliteDocument.getFileId() );
      item->setData( kSateliteTT, Qt::DisplayRole, sateliteDocument.getT1T2() );
      item->setData( kSateliteAA, Qt::DisplayRole, sateliteDocument.getA1A2() );

      item->setData( kSateliteDateColumn, Qt::DisplayRole, meteo::dateToHumanFull(sateliteDocument.getDt()) );
      item->setData( kSateliteII, Qt::DisplayRole, sateliteDocument.getII() );
      item->setData( kSateliteCCCC, Qt::DisplayRole, sateliteDocument.getCCCC() );

      markTreeWidgetItem(item, false);

      menu[kSateliteTT] << QPair<QString, QString>( sateliteDocument.getT1T2(), sateliteDocument.getT1T2() );
      menu[kSateliteAA] << QPair<QString, QString>( sateliteDocument.getA1A2(), sateliteDocument.getA1A2() );
      menu[kSateliteCCCC] << QPair<QString, QString>( sateliteDocument.getCCCC(), sateliteDocument.getCCCC() );
      auto ii = QString::number(sateliteDocument.getII());
      menu[kSateliteII] << QPair<QString, QString>(ii, ii );

      allTreeWidgetItems << item;
      break;
    }
    case DocumentHeader::kAmbiguous:
    case DocumentHeader::kLocalDocument:{
      break;
    }
    }
  }

  menu[kFaxDateColumn] = QSet<QPair<QString, QString>>();

  switch (currentDocumentClass) {
  case DocumentHeader::kFormalDocument:
  case DocumentHeader::kSateliteDocument:
  case DocumentHeader::kMapImage:
  case DocumentHeader::kFax:{
    auto faxHeader = qobject_cast<meteo::bank::ViewHeader*>(this->ui->twFaxDocuments->header());
    this->ui->twFaxDocuments->addTopLevelItems(allTreeWidgetItems);    
    faxHeader->buildMenu(menu);
    break;
  }
  case DocumentHeader::kAmbiguous:
  case DocumentHeader::kLocalDocument:{
    //NOTHING
    return;
  }
  }  
  this->updateItemsMarks();
}


void WgtDocViewPanel::slotPinToMapEnable()
{  
  this->setPinningEnabled( );
  emit signalAction(kPinToMapStart);
}

void WgtDocViewPanel::slotPinToMapAddImageToMap()
{  
  emit signalAction(kPinningAddCurrentDocumentToMap);
}

void WgtDocViewPanel::setPinningEnabled( )
{    
  this->isPinningEnabled_ = true;  
  for (int i = 0; i < ui->twFaxDocuments->topLevelItemCount(); ++i ) {
    this->markTreeWidgetItem(ui->twFaxDocuments->topLevelItem(i), false);
  }
  this->restoreControlsVisibility();
  emit signalAction(kPinningEnable);
}

bool WgtDocViewPanel::isPinningEnabled( )
{
  return this->isPinningEnabled_;
}

void WgtDocViewPanel::removeAllConditions()
{
  for ( int i = 0; i < kColumnCount; ++i ){
    this->removeCondition(static_cast<TreeWidgetsColumns>(i));
  }
}

void WgtDocViewPanel::slotCheckActivated()
{
  QCheckBox* sender = qobject_cast<QCheckBox*> (QObject::sender());
  if ( nullptr == sender ){
    error_log << QObject::tr("Ошибка: не удается получить информацию об отправителе");
    return;
  }
  auto col = static_cast<TreeWidgetsColumns>( sender->property(propertyKey).toInt() );
  if ( Qt::Checked == sender->checkState() ){    
    auto condition = this->conditions_[col];
    this->addCondition(col, condition);
  }
  else {
    this->disableCondition(col);
  }  
}

void WgtDocViewPanel::slotConditionsPanelClosed()
{
  switch (getCurrentDocumentClass()) {
  case DocumentHeader::kFormalDocument:{
    this->removeCondition(kFormalDocumentCenter);
    this->removeCondition(kFormalDocumentDateTime);
    this->removeCondition(kFormalDocumentHour);
    this->removeCondition(kFormalDocumentJobName);
    this->removeCondition(kFormalDocumentModel);
    this->removeCondition(kFormalDocumentName);
    break;
  }
  case DocumentHeader::kMapImage:{
    this->removeCondition( kMapImageDateTime );
    this->removeCondition( kMapImageCenter );
    this->removeCondition( kMapImageHour );
    this->removeCondition( kMapImageJobName );
    this->removeCondition( kMapImageModel );
    this->removeCondition( kMapImageName );
    break;
  }
  case DocumentHeader::kFax:{
    QString nullstr;    
    this->removeCondition( kFaxTT );
    this->removeCondition( kFaxAA );
    this->removeCondition( kFaxCCCC );
    this->removeCondition( kFaxII );
    this->removeCondition( kFaxDateColumn );
    break;
  }
  case DocumentHeader::kAmbiguous:
  case DocumentHeader::kLocalDocument:{
    break;
  }
  case DocumentHeader::kSateliteDocument:{
    this->removeCondition( kSateliteTT );
    this->removeCondition( kSateliteAA );
    this->removeCondition( kSateliteCCCC );
    this->removeCondition( kSateliteCCCC );
    this->removeCondition( kSateliteII );
    this->removeCondition( kSateliteDateColumn );
    break;
  }
  }
}

QString WgtDocViewPanel::getDisplayFilterForColumn( TreeWidgetsColumns searchColumn)
{
  auto checks = this->ui->wgtConditionsPanel->getCheckboxList();
  for ( auto check: checks ){
    int col = check->property(propertyKey).toInt();
    if ( col == searchColumn ){
      return check->text();
    }
  }
  return QString();
}

void WgtDocViewPanel::slotTreeWidgetSelectionChanged()
{  
  auto selectedItems = this->selectedItems();
  bool nothingChanged = true;
  if ( this->oldSelectedItems_.size() == selectedItems.size() ){
    for ( auto item : selectedItems ){
      if ( false == oldSelectedItems_.contains(item) ){
        nothingChanged = false;
        break;
      }
    }
  }
  else {
    nothingChanged = false;
  }

  if ( false == nothingChanged ){
    this->oldSelectedItems_ = selectedItems;
    emit signalAction(kSelectedDocumentsChanged);
    this->restoreTreeControlsVisibility();
  }
}
void WgtDocViewPanel::slotPinRemoveImageFromMap()
{
  emit signalAction(kPinningRemoveSelectedDocumentsFromMap);
}

QList<QTreeWidgetItem*> WgtDocViewPanel::selectedItems()
{
  QList<QTreeWidgetItem*> items;
  for ( int i = 0; i < this->ui->twFaxDocuments->topLevelItemCount(); ++i ){
    auto item = this->ui->twFaxDocuments->topLevelItem(i);
    if ( true == item->isSelected() ){
      items << item;
    }
  }

  return items;
}

QTreeWidgetItem* WgtDocViewPanel::currentItem()
{
  return this->ui->twFaxDocuments->currentItem();
}

void WgtDocViewPanel::setCurrentItem(QTreeWidgetItem* item)
{  
  this->ui->twFaxDocuments->blockSignals(true);
  if ( nullptr != item ){
    auto oldItem = this->ui->twFaxDocuments->currentItem();
    auto selectedItems = this->selectedItems();
    this->ui->twFaxDocuments->setCurrentItem(item);
    item->setSelected(false);
    for ( auto item: selectedItems ){
      item->setSelected(true);
    }
    slotCurrentDocumentChanged( item, oldItem );
  }
  this->ui->twFaxDocuments->blockSignals(false);
}

void WgtDocViewPanel::setCurrentDocument(const QString &id )
{
  for ( int i = 0; i < this->ui->twFaxDocuments->topLevelItemCount(); ++i )  {
    auto item = this->ui->twFaxDocuments->topLevelItem(i);
    if ( id == item->data(kHiddenDataColumn, Qt::UserRole).toString() ){
      this->setCurrentItem(item);
      return;
    }
  }
}


void WgtDocViewPanel::setCurrentSlide( int slide )
{
  auto selectedItems = this->selectedItems();
  if ( ( 0 <= slide ) && ( slide < selectedItems.count() ) ){
    this->setCurrentItem( selectedItems[slide] );
  }
}

void WgtDocViewPanel::treeStateSave()
{
  auto state = this->ui->twFaxDocuments->header()->saveState();
  auto geom = this->ui->twFaxDocuments->header()->saveGeometry();
  auto docClass = this->getCurrentDocumentClass();
  this->savedStates_[docClass] =  qMakePair(state, geom);
}

void WgtDocViewPanel::treeStatesSaveToFS()
{  
  QFile ofile(treeConfigPath);
  QFileInfo info(treeConfigPath);
  if ( false == info.dir().exists() ) {
    QDir("/").mkpath(info.dir().absolutePath());
  }
  if ( false == ofile.open(QIODevice::WriteOnly) ) {
    warning_log << QObject::tr("Ошибка при сохранении настроек отображения.");
    return;
  }
  QDataStream stream(&ofile);
  stream << this->savedStates_;
}

void WgtDocViewPanel::treeStatesRestoreFromFs()
{  
  this->savedStates_.clear();
  QFile ofile(treeConfigPath);
  QFileInfo info(treeConfigPath);
  if ( false == info.dir().exists() ) {
    QDir("/").mkpath(info.dir().absolutePath());
  }
  if ( false == ofile.open(QIODevice::ReadOnly) ) {
    warning_log << QObject::tr("Ошибка при загрузке настроек отображения.");
    return;
  }
  auto data = ofile.readAll();  
  if ( false == data.isEmpty() ) {
    QDataStream stream(data);
    stream >> this->savedStates_;
  }
  else {
    info_log << QObject::tr("Настройки не загружены. Сброс настроек к настройкам по умолчанию");
  }
}

void WgtDocViewPanel::treeStateRestore()
{
  auto docClass = this->getCurrentDocumentClass();  
  auto state = this->savedStates_[docClass].first;
  auto geom = this->savedStates_[docClass].second;
  if ( false == state.isEmpty() && false == geom.isEmpty() ) {
    this->ui->twFaxDocuments->blockSignals(true);
    this->ui->twFaxDocuments->header()->blockSignals(true);
    this->ui->twFaxDocuments->header()->restoreState(state);
    this->ui->twFaxDocuments->header()->restoreGeometry(geom);
    this->ui->twFaxDocuments->blockSignals(false);
    this->ui->twFaxDocuments->header()->blockSignals(false);
    qApp->processEvents();
  }
  else {
    auto columns = WgtDocViewPanel::columnsForDocumentClass(docClass);
    this->ui->twFaxDocuments->blockSignals(true);
    this->ui->twFaxDocuments->header()->blockSignals(true);
    for ( int i = 0; i < kColumnCount; ++i ) {
      TreeWidgetsColumns col = static_cast<TreeWidgetsColumns>(i);
      this->ui->twFaxDocuments->setColumnHidden( i, false == columns.contains(col) );
    }
    this->ui->twFaxDocuments->blockSignals(false);
    this->ui->twFaxDocuments->header()->blockSignals(false);
    qApp->processEvents();
    this->treeStateSave();
  }
}

void WgtDocViewPanel::slotHeaderGeometryChanged()
{
  this->treeStateSave();
}

void WgtDocViewPanel::addConditionString(TreeWidgetsColumns column, const QString& value)
{
  auto cond = ConditionString(displayColumnNames[column], ConditionString::kMatch, false, Qt::CaseSensitive, value);
  this->addCondition(column, cond);
}

void WgtDocViewPanel::slotOnPbReloadClicked()
{
  emit signalAction(PanelAction::kReloadButtonClicked);

}
}
}
