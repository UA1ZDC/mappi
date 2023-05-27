#include "wgtdocview/wgtdocview.h"
#include "ui_wgtdocview.h"

#include <meteo/commons/ui/mainwindow/mainwindow.h>

#include <qitemdelegate.h>
#include <cross-commons/debug/tlog.h>
#include <qmenu.h>
#include <qpushbutton.h>
#include <qgraphicsscene.h>
#include <qgraphicsview.h>
#include <qscrollbar.h>
#include <qgraphicsproxywidget.h>
#include <meteo/commons/ui/custom/filedialogrus.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/mainwindow/mdisubwindow.h>
#include <meteo/commons/ui/map/geopixmap.h>
#include <commons/geobasis/geovector.h>
#include <meteo/commons/ui/documentviewer/faxaction.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/conditionmaker/templates/conditiongenericprivate.h>
#include <meteo/commons/ui/conditionmaker/conditions/conditiondateinterval.h>
#include <meteo/commons/ui/documentviewer/custom/animationgraphicsitem.h>

#include "custom/pindialog.h"
#include "faxaction.h"
namespace meteo {
namespace documentviewer {

static const QString configPath = meteo::global::kDocumentViewerConfigPath + QObject::tr("wgtdocview.bin");

static const QString& errorConditionWaitString = QObject::tr("Ожидалось условие для строк, получено неизвестное условие");
static const QString& errorConditionWaitInt32 = QObject::tr("Ожидалось условие для int32, получено неизвестное условие");
static const QString& errorConditionWaitDateTimeInterval = QObject::tr("Ожидалось условие для интервала дат, получено неизвестное условие");

static const QIcon iconDocument = QIcon(QObject::tr(":/meteo/icons/doclist/document.png"));
static const QIcon iconImage = QIcon(QObject::tr(":/meteo/icons/doclist/map.png"));
static const QIcon iconFax = QIcon(QObject::tr(":/meteo/icons/fax-toolbar.png"));

static const QDateTime dtMin = QDateTime::fromMSecsSinceEpoch(0);
static const QDateTime dtMax = QDateTime(QDate(3000, 1, 1), QTime(0,0)); // 0:0 1 января 3000 года

static const auto kConditionConvertError = QObject::tr("Ошибка преобразования условия для поля.");

void createConfigPath()
{
  QDir dir("/");
  dir.mkpath(meteo::global::kDocumentViewerConfigPath);
}


WgtDocView::WgtDocView( DocumentViewerStates state ) :
  QWidget(nullptr),
  ui_(new ::Ui::WgtDocView),
  leftPanel_(nullptr),
  hover_(nullptr),
  pinningAction_(nullptr),
  pinningDialog_(nullptr),
  model_(nullptr),
  animationController_(AnimationController())
{  
  createConfigPath();
  ui_->setupUi(this);
  QObject::connect( ui_->view, &ViewImagePreview::signalDocumentResized, this, &WgtDocView::slotDocumentResized );
  ui_->view->installEventFilter(this);

  DocumentHeader::DocumentClass defaultClass;
  bool pinningEnabled;
  bool animationEnabled = false;
  switch (state){
  case kStateDefault:{
    defaultClass = DocumentHeader::kFax;
    pinningEnabled = false;
    animationEnabled = true;
    break;
  }
  case kStateFaxWithPinning:{
    defaultClass = DocumentHeader::kFax;
    pinningEnabled = true;
    animationEnabled = true;
    break;
  }
  case kStateSateliteWithPinning:{
    defaultClass = DocumentHeader::kSateliteDocument;
    pinningEnabled = true;
    animationEnabled = true;
    break;
  }
  case kStateDefaultFaxPage:{
    defaultClass = DocumentHeader::kFax;
    pinningEnabled = false;
    animationEnabled = true;
    break;
  }
  case kStateDefaultLocalPage:{
    defaultClass = DocumentHeader::kLocalDocument;
    pinningEnabled = false;
    animationEnabled =false;
    break;
  }
  case kStateDefaultSatelitePage:{
    defaultClass = DocumentHeader::kSateliteDocument;
    pinningEnabled = false;
    animationEnabled = true;
    break;
  }
  case kStateDefaultMapImagePage:{
    defaultClass = DocumentHeader::kMapImage;
    pinningEnabled = false;
    animationEnabled = true;
    break;
  }
  case kStateMapImageWithPinning:{
    defaultClass = DocumentHeader::kMapImage;
    pinningEnabled = true;
    animationEnabled = true;
    break;
  }
  case kStateDefaultFormalDocPage:{
    defaultClass = DocumentHeader::kFormalDocument;
    pinningEnabled = false;
    animationEnabled = false;
    break;
  }
  }

  auto scene = ui_->view->scene();

  leftPanel_ = new WgtDocViewPanel(defaultClass, pinningEnabled);
  leftPanel_->setMinimumWidth(363);
  QObject::connect( leftPanel_, &WgtDocViewPanel::signalAction, this, &WgtDocView::slotDocumentLeftPanelAction );
  QObject::connect( leftPanel_, &WgtDocViewPanel::signalConditionAdd, this, &WgtDocView::slotConditionAdd );
  QObject::connect( leftPanel_, &WgtDocViewPanel::signalConditionRemove, this, &WgtDocView::slotConditionRemove );
  QObject::connect( leftPanel_, &WgtDocViewPanel::signalDocumentClassChanged, this, &WgtDocView::slotDocumentClassChanged  );
  QObject::connect( leftPanel_, &WgtDocViewPanel::signalSlidesCountUpdate, this, &WgtDocView::slotSelectedDocumentsCountUpdate );
  QObject::connect( leftPanel_, &WgtDocViewPanel::signalPanelDone, this, &WgtDocView::slotPanelLoaded );


  leftPanel_->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  ui_->wgtLeftPart->layout()->addWidget(leftPanel_);

  hover_ = new WgtHoveringWidget();
  hover_->setPinningEnabled(pinningEnabled);
  hover_->setAnimationEnabled(animationEnabled);
  hoveringWidgetProxy_ = scene->addWidget(hover_);
  hoveringWidgetProxy_->setZValue(2);

  //leftPanelProxy_ = scene->addWidget(leftPanel_);
  //leftPanelProxy_->setZValue(2);

  QObject::connect( ui_->splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(slotSplitterMoved()) );

  QObject::connect( hover_,  SIGNAL(customContextMenuRequested(const QPoint &)),  this, SLOT(slotShowContextMenuForHoverWidget(const QPoint &)) );

  QObject::connect( hover_, &WgtHoveringWidget::signalAction, this, &WgtDocView::slotHoverWidgetAction );

  model_ = new DataModel( defaultClass );
  model_->setLeftPanel(leftPanel_);
  QObject::connect( model_, &DataModel::signalFileLoaded, this, &WgtDocView::slotFileLoaded, Qt::QueuedConnection );
  QObject::connect( model_, &DataModel::signalListLoaded, this, &WgtDocView::slotListLoaded, Qt::QueuedConnection );

  animationController_.setRepeat( hover_->isAnimationCycled() );
  animationController_.setTimeout( hover_->getAnimationDelay() );
  QObject::connect( &animationController_, &AnimationController::signalAnimationStarted, this, &WgtDocView::slotAnimationStarted );
  QObject::connect( &animationController_, &AnimationController::signalAnimationFinished, this, &WgtDocView::slotAnimationFinished );

  QObject::connect( &animationController_, &AnimationController::signalAnimationResumed, this, &WgtDocView::slotAnimationResumed );
  QObject::connect( &animationController_, &AnimationController::signalAnimationPaused, this, &WgtDocView::slotAnimationPaused );
  loadSettings();
  if ( false == spliteerSizes_.isEmpty() && spliteerSizes_.size() == ui_->splitter->sizes().size() ) {
    ui_->splitter->setSizes( spliteerSizes_ );
  }
}

WgtDocView::~WgtDocView()
{
  saveSettings();
  if ( nullptr != hover_ ){
    delete hover_;
    leftPanel_ = nullptr;
    hoveringWidgetProxy_ = nullptr;
  }
  if ( nullptr != leftPanel_ ){
    delete leftPanel_;
    leftPanel_ = nullptr;
  }
  if ( nullptr != pinningDialog_ ){
    delete pinningDialog_;
    pinningDialog_ = nullptr;
  }
  if (nullptr != ui_) {
    delete ui_; ui_ = nullptr;
  }
  if ( nullptr != model_ ){
    delete model_;
  }
}

void WgtDocView::slotAnimationSlideChanged( int slideIndex )
{
  auto documents = leftPanel_->selectedDocuments();
  if ( 0 <= slideIndex && slideIndex < documents.size() ){
    leftPanel_->setCurrentDocument(documents[slideIndex]);
  }
}

void WgtDocView::slotConditionAdd( WgtDocViewPanel::TreeWidgetsColumns column, Condition condition)
{
  switch (column)
  {
  case WgtDocViewPanel::kHiddenDataColumn:{
    error_log << QObject::tr("Не предусмотрено");
    break;
  }
  case WgtDocViewPanel::kFaxTT: {
    model_->filterFaxTT(condition);
    break;
  }
  case WgtDocViewPanel::kFaxAA: {
    model_->filterFaxAA(condition);
    break;
  }
  case WgtDocViewPanel::kFaxCCCC: {
    model_->filterFaxCCCC(condition);
    break;
  }
  case WgtDocViewPanel::kFaxII: {    
    model_->filterFaxII(condition);
    break;
  }
  case WgtDocViewPanel::kFaxDateColumn:{
    ConditionDateTimeInterval dateCondition (condition);
    if ( true == dateCondition.isNull() ) {
      error_log << errorConditionWaitDateTimeInterval;
      return;
    }
    model_->setDateRange( dateCondition.dtBegin(), dateCondition.dtEnd() );
    break;
  }
  case WgtDocViewPanel::kSateliteTT:{
    model_->filterSateliteTT(condition);
    break;
  }
  case WgtDocViewPanel::kSateliteAA:{
    model_->filterSateliteAA(condition);
    break;
  }
  case WgtDocViewPanel::kSateliteCCCC:{
    model_->filterSateliteCCCC(condition);
    break;
  }
  case WgtDocViewPanel::kSateliteDateColumn:{
    ConditionDateTimeInterval dateCondition (condition);
    if ( true == dateCondition.isNull() ){
      error_log << errorConditionWaitDateTimeInterval;
      return;
    }
    auto dtStart = dateCondition.dtBegin();
    auto dtEnd = dateCondition.dtEnd();
    model_->setDateRange(dtStart, dtEnd);
    break;
  }
  case WgtDocViewPanel::kSateliteII:{
    model_->filterSateliteII(condition);
    break;
  }
  case WgtDocViewPanel::kMapImageDateTime:{
    ConditionDateTimeInterval dateCondition (condition);
    if ( true == dateCondition.isNull() ){
      error_log << errorConditionWaitDateTimeInterval;
      return;
    }
    auto dtStart = dateCondition.dtBegin();
    auto dtEnd = dateCondition.dtEnd();
    model_->setDateRange( dtStart, dtEnd );
    break;
  }
  case WgtDocViewPanel::kMapImageCenter:{
    model_->filterMapImageCenter(condition);
    break;
  }
  case WgtDocViewPanel::kMapImageHour:{
    model_->filterMapImageHour(condition);
    break;
  }
  case WgtDocViewPanel::kMapImageJobName:{
    model_->filterMapImageJobName(condition);
    break;
  }
  case WgtDocViewPanel::kMapImageModel:{
    model_->filterMapImageModel(condition);
    break;
  }
  case WgtDocViewPanel::kMapImageName:{
    model_->filterMapImageName(condition);
    break;
  }
  case WgtDocViewPanel::kFormalDocumentCenter:{
    model_->filterFormalDocumentCenter(condition);
    break;
  }
  case WgtDocViewPanel::kFormalDocumentDateTime:{
    ConditionDateTimeInterval dateCondition (condition);
    if ( true == dateCondition.isNull() ) {
      error_log << errorConditionWaitDateTimeInterval;
      return;
    }
    model_->setDateRange( dateCondition.dtBegin(), dateCondition.dtEnd() );
    break;
  }
  case WgtDocViewPanel::kFormalDocumentHour:{
    model_->filterFormalDocumentHour(condition);
    break;
  }
  case WgtDocViewPanel::kFormalDocumentJobName:{
    model_->filterFormalDocumentJobName(condition);
    break;
  }
  case WgtDocViewPanel::kFormalDocumentModel:{
    model_->filterFormalDocumentModel(condition);
    break;
  }
  case WgtDocViewPanel::kFormalDocumentName:{
    model_->filterFormalDocumentName(condition);
    break;
  }
  }
}

void WgtDocView::slotConditionRemove( WgtDocViewPanel::TreeWidgetsColumns column )
{
  switch (column) {
  case WgtDocViewPanel::kHiddenDataColumn:{
    error_log << QObject::tr("Ошибка: непредвиденное значение column");
    break;
  }
  case WgtDocViewPanel::kFaxTT:{
    model_->filterFaxTT(Condition());
    break;
  }
  case WgtDocViewPanel::kFaxAA:{
    model_->filterFaxAA(Condition());
    break;
  }
  case WgtDocViewPanel::kFaxCCCC:{
    model_->filterFaxCCCC(Condition());
    break;
  }
  case WgtDocViewPanel::kFaxII:{
    model_->filterFaxII(Condition());
    break;
  }
  case WgtDocViewPanel::kFaxDateColumn:{
    model_->setDateRange( dtMin, dtMax );
    break;
  }
  case WgtDocViewPanel::kSateliteTT:{
    model_->filterSateliteTT(Condition());
    break;
  }
  case WgtDocViewPanel::kSateliteAA:{
    model_->filterSateliteAA(Condition());
    break;
  }
  case WgtDocViewPanel::kSateliteCCCC:{
    model_->filterSateliteCCCC(Condition());
    break;
  }
  case WgtDocViewPanel::kSateliteDateColumn:{
    model_->setDateRange(dtMin, dtMax);
    break;
  }
  case WgtDocViewPanel::kSateliteII:{
    model_->filterSateliteII(Condition());
    break;
  }
  case WgtDocViewPanel::kMapImageDateTime:{
    model_->setDateRange( dtMin, dtMax );
    break;
  }
  case WgtDocViewPanel::kMapImageCenter:{
    model_->filterMapImageCenter(Condition());
    break;
  }
  case WgtDocViewPanel::kMapImageHour:{
    model_->filterMapImageHour(Condition());
    break;
  }
  case WgtDocViewPanel::kMapImageJobName:{
    model_->filterMapImageJobName(Condition());
    break;
  }
  case WgtDocViewPanel::kMapImageModel:{
    model_->filterMapImageModel(Condition());
    break;
  }
  case WgtDocViewPanel::kMapImageName:{
    model_->filterMapImageName(Condition());
    break;
  }
  case WgtDocViewPanel::kFormalDocumentCenter:{
    model_->filterFormalDocumentCenter(Condition());
    break;
  }
  case WgtDocViewPanel::kFormalDocumentDateTime:{
    model_->setDateRange( dtMin, dtMax );
    break;
  }
  case WgtDocViewPanel::kFormalDocumentHour:{
    model_->filterFormalDocumentHour(Condition());
    break;
  }
  case WgtDocViewPanel::kFormalDocumentJobName:{
    model_->filterFormalDocumentJobName(Condition());
    break;
  }
  case WgtDocViewPanel::kFormalDocumentModel:{
    model_->filterFormalDocumentModel(Condition());
    break;
  }
  case WgtDocViewPanel::kFormalDocumentName:{
    model_->filterFormalDocumentName(Condition());
    break;
  }
  }
}


void WgtDocView::slotListLoaded( const QList<DocumentHeader*> *documentsInfo)
{
  leftPanel_->setDocumentList(documentsInfo);
}

void WgtDocView::slotSplitterMoved()
{
  spliteerSizes_ = ui_->splitter->sizes();
  if ( 2 != spliteerSizes_ .size() ){
    error_log << QObject::tr("Внимание: структура виджета изменена, необходимо изменение данной функции");
    return;
  }
  isLeftWidgetsHidden = ( 0 == spliteerSizes_ [0] );
  hover_->restoreButtonIcons( isLeftWidgetsHidden );
}

void WgtDocView::slotShowContextMenuForHoverWidget( const QPoint& pnt)
{
  QMenu contextMenu;
  QAction act1(QObject::tr("Только изображение"), nullptr);
  act1.setCheckable(true);
  act1.setChecked( hover_->isControlsEnabled() );
  contextMenu.addAction(&act1);
  QObject::connect(&act1, SIGNAL(triggered(bool)), hover_, SLOT(slotActionOnlyImageTriggered(bool)) );

  QAction act2(QObject::tr("Показать панель инструментов"),nullptr);
  QAction act3(QObject::tr("Показать панель анимации"),nullptr);
  if ( true == hover_->isControlsEnabled() ) {
    act2.setCheckable(true);
    act2.setChecked(true == hover_->isToolbarEnabled() );
    contextMenu.addAction(&act2);
    QObject::connect(&act2, SIGNAL(triggered(bool)), hover_, SLOT(slotActionToolbarHide(bool)) );


    act3.setCheckable(true);
    act3.setChecked( true == hover_->isToolbaranimationEnabled() );
    contextMenu.addAction(&act3);
    QObject::connect( &act3, SIGNAL(triggered(bool)), hover_, SLOT(slotActionAnimationToolbarHide(bool)) );
  }
  contextMenu.exec(hover_->mapToGlobal(pnt));
}

bool WgtDocView::prepareAnimationData( bool withTransparent)
{
  auto selectedDocuments = leftPanel_->selectedDocuments();
  if ( true == model_->preloadDocuments(selectedDocuments) ){
    return ( false == withTransparent ) || ( true == model_->premakeTransparentImages(selectedDocuments) );
  }
  else {
    return false;
  }
}


void WgtDocView::slotDocumentLeftPanelAction( WgtDocViewPanel::PanelAction action )
{
  switch (action) {
  case WgtDocViewPanel::kSelectedDocumentsChanged:{
    auto selectedDocuments = leftPanel_->selectedDocuments();
    auto currentDocument = leftPanel_->currentDocument();
    int index = selectedDocuments.indexOf(currentDocument->getId());
    hover_->updateSlidesCount( index, selectedDocuments.size() );
    break;
  }
  case WgtDocViewPanel::kReloadButtonClicked:{
    model_->refresh();
    break;
  }
  case WgtDocViewPanel::kTransparencyChanged:{
    auto mode = leftPanel_->getTranspacencyMode();
    auto value = leftPanel_->getTransparencyValue();
    model_->setTransparencySettings( mode, value );
    auto currentDoc = leftPanel_->currentDocument();

    if ( nullptr != currentDoc ) {
      auto image = model_->getTransparentImage( currentDoc->getClass(), currentDoc->getFileId() );
      ui_->view->setImage( image );

      if ( nullptr != pinningAction_ ){
        pinningAction_->setImage( image );
      }

    }

    switch (mode) {
    case DataModel::kTransparencyBlack:{
      ui_->view->setBackgroundBrush(Qt::black);
      break;
    }
    case DataModel::kTransparencyNone:{
      ui_->view->setBackgroundBrush(QBrush());
      break;
    }
    case DataModel::kTransparencyWhite:{
      ui_->view->setBackgroundBrush(Qt::white);
      break;
    }
    }


    if ( true == animationController_.isActive() ) {
      animationController_.animationPause();
      if ( false == prepareAnimationData( true == leftPanel_->isPinningEnabled() ) ) {
        return;
      }
      auto documents = leftPanel_->selectedDocuments();
      QList<QImage> images;
      for ( auto id: documents ){
        auto document = leftPanel_->getDocumentById(id);
        images << model_->getTransparentImage( document->getClass(), document->getFileId() );
      }
      animationController_.updateImageList(images);
      animationController_.animationResume();
    }
    break;
  }
  case WgtDocViewPanel::kCurrentDocumentChanged:{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    auto doc = leftPanel_->currentDocument();
    auto selectedDocuments = leftPanel_->selectedDocuments();
    if ( nullptr != doc ) {
      DocumentUID uid ( doc->getClass(), doc->getFileId() );
      model_->loadDocument(uid);
      int index = selectedDocuments.indexOf(doc->getId());
      if ( -1 != index) {
        hover_->updateSlidesCount( index, selectedDocuments.size() );
      }
    }
    QApplication::restoreOverrideCursor();
    break;
  }
  case WgtDocViewPanel::kPinningEnable:{
    animationController_.animationStop();
    hover_->setPinningEnabled(leftPanel_->isPinningEnabled());
    break;
  }
  case WgtDocViewPanel::kPinToMapStart:{
    if ( true == leftPanel_->isPinningEnabled() ){
      enablePinningFaxToMap();
    }
    else {
      if ( nullptr != pinningDialog_ ){
        delete pinningDialog_;
        pinningAction_ = nullptr;
      }
    }
    break;
  }
  case WgtDocViewPanel::kPinningAddCurrentDocumentToMap:{
    addCurrentDocumentToMap();
    break;
  }
  case WgtDocViewPanel::kPinningRemoveSelectedDocumentsFromMap:{
    removeSelectedDocumentsFromMap();
    break;
  }
  case WgtDocViewPanel::kPinningRemoveCurrentDocumentFromMap:{
    removeDocumentFromMap(leftPanel_->currentDocument());
    break;
  }
  }
}

void WgtDocView::removeDocumentFromMap(DocumentHeader *document )
{
  if ( nullptr != pinningAction_){
    auto id = document->getFileId();
    if ( nullptr != pinningAction_ ){
      pinningAction_->removeLayer(id);
    }
  }
}

void WgtDocView::removeSelectedDocumentsFromMap()
{
  QStringList documents = leftPanel_->selectedDocuments();

  for ( auto id: documents ){
    auto document = leftPanel_->getDocumentById(id);
    removeDocumentFromMap(document);
  }
}

void WgtDocView::pinSumbenuExtendedOpen()
{
  pinningSettingsOpen();
}

void WgtDocView::pinSumbenuOpen()
{  
  QList<QAction*> actions;
  auto pins = pinningStorage_.getStorage(leftPanel_->getCurrentDocumentClass());
  {
    QAction *actionStore = new QAction(QObject::tr("Сохранить настройки"),nullptr);
    actionStore->setData(kPinSave);
    actions << actionStore;
  }
  if ( false == pins.isEmpty() ) {
    QAction *actionRemove = new QAction(QObject::tr("Удалить настройки"),nullptr);
    actionRemove->setData(kPinRemove);
    actions << actionRemove;
  }

  for ( int i  = 0; i < pins.keys().size(); ++i ) {
    auto pinName = pins.keys()[i];
    QAction *act = new QAction(pinName,nullptr);
    act->setData(i);
    actions << act;;
  }

  QMenu submenu;
  submenu.addActions(actions);

  QObject::connect( &submenu, SIGNAL(triggered(QAction*)), this, SLOT(slotPinningMenuActionActivated(QAction*)) );
  submenu.exec(QCursor::pos());
  while ( false == actions.isEmpty() ){
    auto first = actions.first();
    actions.removeFirst();
    delete first;
  }
}

void WgtDocView::slotPinningMenuActionActivated( QAction* act )
{
  bool ok = false;
  int index = static_cast<PinningActionType>(act->data().toInt(&ok));
  if ( index == kPinRemove ){
    auto pins = pinningStorage_.getStorage(model_->getDocumentClass());
    bool ok = false;
    QString text = QInputDialog::getItem( nullptr,
                                          QObject::tr("Удаление сохраненных настроек отображения"),
                                          QObject::tr("Выберите условие для удаления"),
                                          pins.keys(), 0, false , &ok );
    if ( true == ok ){
      pinningStorage_.removePinning( model_->getDocumentClass(), text );
    }
  }
  else if ( index == kPinSave ){
    auto conditions = getCurrentConditions();
    auto docClass = model_->getDocumentClass();
    QStringList displayConditions;
    for ( auto condition: conditions.values() ){
      if ( false == condition.isNull() ){
        displayConditions << condition.displayCondition();
      }
    }

    QString text;
    while ( true ) {
      bool ok = false;
      text = QInputDialog::getText(nullptr,
                                   QObject::tr("Сохранение настроек отображения"),
                                   QObject::tr("Условия: %1").arg( displayConditions.isEmpty()? QObject::tr("Отсутствуют") : displayConditions.join(" ,")),
                                   QLineEdit::Normal, text, &ok);
      if ( false == ok ){
        break;
      }
      if ( true == pinningStorage_.contains(docClass, text) ){
        meteo::app::MainWindow* mw = WidgetHandler::instance()->mainwindow();
        if ( false == mw->askUser(QObject::tr("Привязка с данным именем уже существует. Продолжить?")) ){
          continue;
        }
        else {
          pinningStorage_.removePinning(docClass, text);
        }
      }
      PinningStorage::Pin *p = new PinningStorage::Pin();
      if ( nullptr != pinningAction_ ) {
        for ( int i = 0; i < meteo::map::Faxaction::markersCount; ++i ) {
          p->mapPoints[i] = pinningAction_->getTargetRect().at(i);
          p->imagePoints[i] = pinningAction_->getSourceRect().at(i);
        }
      }
      else {
        p->mapPoints[0] = meteo::GeoPoint(-30, 0);
        p->mapPoints[1] = meteo::GeoPoint(  0, 30);
        p->mapPoints[2] = meteo::GeoPoint( 30, 0);
        p->imagePoints[0] = QPoint(-30, 0);
        p->imagePoints[1] = QPoint( 0, 30);
        p->imagePoints[2] = QPoint( 30, 0);
      }
      p->name = text;
      p->conditions = conditions;
      for ( auto column: p->conditions.keys() ){
        auto condition = p->conditions[column];
        if ( false == condition.isNull() ) {
          p->conditions[column] = condition;
        }
      }
      pinningStorage_.addPinning( docClass, p );
      break;
    }
  }
  else {
    auto pins = pinningStorage_.getStorage(leftPanel_->getCurrentDocumentClass());
    auto pinName = pins.keys()[index];
    applyPinning( pinningStorage_.getPin( leftPanel_->getCurrentDocumentClass(),  pinName ) );
  }
}


void WgtDocView::applyPinning( const PinningStorage::Pin& pinning )
{
  QHash<WgtDocViewPanel::TreeWidgetsColumns, Condition> conditions ;
  switch ( leftPanel_->getCurrentDocumentClass() ){
  case DocumentHeader::kFax:{
    auto filterFaxTT = pinning.conditions.value(WgtDocViewPanel::kFaxTT, Condition());
    auto filterFaxAA = pinning.conditions.value(WgtDocViewPanel::kFaxAA, Condition());
    auto filterFaxCCCC = pinning.conditions.value(WgtDocViewPanel::kFaxCCCC, Condition());
    auto filterFaxii = pinning.conditions.value(WgtDocViewPanel::kFaxII, Condition());
    conditions.insert( WgtDocViewPanel::kFaxTT, filterFaxTT );
    conditions.insert( WgtDocViewPanel::kFaxAA, filterFaxAA );
    conditions.insert( WgtDocViewPanel::kFaxCCCC, filterFaxCCCC );
    conditions.insert( WgtDocViewPanel::kFaxII, filterFaxii );
    break;
  }
  case DocumentHeader::kMapImage:{
    auto filterMapImageCenter = pinning.conditions.value(WgtDocViewPanel::kMapImageCenter, Condition());
    auto filterMapImageHour = pinning.conditions.value(WgtDocViewPanel::kMapImageHour, Condition());
    auto filterMapImageJobName = pinning.conditions.value(WgtDocViewPanel::kMapImageJobName, Condition());
    auto filterMapImageModel = pinning.conditions.value(WgtDocViewPanel::kMapImageModel, Condition());
    auto filterMapImageImageName = pinning.conditions.value(WgtDocViewPanel::kMapImageName, Condition());

    conditions.insert( WgtDocViewPanel::kMapImageCenter, filterMapImageCenter);
    conditions.insert( WgtDocViewPanel::kMapImageHour, filterMapImageHour );
    conditions.insert( WgtDocViewPanel::kMapImageJobName, filterMapImageJobName );
    conditions.insert( WgtDocViewPanel::kMapImageModel, filterMapImageModel );
    conditions.insert( WgtDocViewPanel::kMapImageName, filterMapImageImageName );
    break;
  }
  case DocumentHeader::kSateliteDocument:{
    auto filterSateliteTT = pinning.conditions.value(WgtDocViewPanel::kSateliteTT, Condition());
    auto filterSateliteAA = pinning.conditions.value(WgtDocViewPanel::kSateliteAA, Condition());
    auto filterSateliteCCCC = pinning.conditions.value(WgtDocViewPanel::kSateliteCCCC, Condition());
    auto filterSateliteII = pinning.conditions.value(WgtDocViewPanel::kSateliteII, Condition());

    conditions.insert( WgtDocViewPanel::kSateliteTT, filterSateliteTT );
    conditions.insert( WgtDocViewPanel::kSateliteAA, filterSateliteAA );
    conditions.insert( WgtDocViewPanel::kSateliteCCCC, filterSateliteCCCC );
    conditions.insert( WgtDocViewPanel::kSateliteII, filterSateliteII );
    break;
  }
  case DocumentHeader::kAmbiguous:
  case DocumentHeader::kFormalDocument:
  case DocumentHeader::kLocalDocument:{
    error_log << QObject::tr("Ошибка: непредвиденное поведение системы");
    break;
  }
  }
  if ( nullptr != pinningAction_ ) {
    pinningAction_->setSourceRect(pinning.imagePoints);
    pinningAction_->setTargetRect(pinning.mapPoints);
  }


  for ( auto key : conditions.keys() ){
    auto value = conditions[key];
    if ( false == value.isNull() ){
      leftPanel_->addCondition( key, value );
    }
    else {
      leftPanel_->removeCondition(key);
    }
  }
}
void WgtDocView::slotDocumentResized()
{
  auto view = ui_->view;
  int x0 = 0, y0 = 0, width = view->width() - 0, height = view->height() - 0;
  auto scenePoint = view->mapToScene(x0, y0);
  QRectF rect( scenePoint.x(), scenePoint.y(), width, height );
  hoveringWidgetProxy_->setGeometry(rect);
  if ( nullptr != pinningAction_)
  {
    pinningAction_->sourceImageRectRestore();
  }
}

bool WgtDocView::eventFilter( QObject* watched, QEvent* event )
{
  if ( event->type() == QEvent::Wheel ) {
    if ( ( nullptr != ui_ ) && ( watched == ui_->view ) ) {
      if ( nullptr != pinningAction_ ){
        pinningAction_->sourceImageRectRestore();
      }
    }
  }
  else {
    if ( ( nullptr != ui_ ) &&  ( watched == ui_->view ) ){
      if ( nullptr != pinningAction_ ){
        pinningAction_->sourceImageRectSave();
      }
    }
  }
  return false;
}

void WgtDocView::slotDestroyed()
{
  QObject* sender = QObject::sender();

  if ( sender == pinningAction_ ){
    pinningAction_ = nullptr;
    return;
  }
  if ( sender == pinningDialog_ ){
    pinningDialog_ = nullptr;
    return;
  }
  error_log << QObject::tr("Уничтожен неизвестный объект");
}

void WgtDocView::setPinningAction( meteo::map::Faxaction* action )
{
  pinningAction_ = action;
  auto currentDocument = leftPanel_->currentDocument();
  if ( nullptr != currentDocument ) {
    pinningAction_->pitToView( ui_->view,
                                     currentDocument->getFileId(),
                                     currentDocument->displayName()
                                     );
  }
  else {
    pinningAction_->pitToView( ui_->view,
                                     QString(),
                                     QString() );
  }
  QObject::connect(pinningAction_, SIGNAL(signalImageLayerAdded( const QString& )), leftPanel_, SLOT(slotImagePinningAdded( const QString& )) );
  QObject::connect(pinningAction_, SIGNAL(signalImageLayerRemoved( const QString& )), leftPanel_, SLOT(slotImagePinningRemoved( const QString& )) );
  QObject::connect( pinningAction_, SIGNAL(destroyed()), this, SLOT(slotDestroyed()) );
}

void WgtDocView::setJobNameFilter(const QString &jobName, const QString &mapName)
{
  leftPanel_->addConditionString(WgtDocViewPanel::TreeWidgetsColumns::kMapImageJobName, jobName);
  leftPanel_->addConditionString(WgtDocViewPanel::TreeWidgetsColumns::kMapImageName, mapName);
}

void WgtDocView::slotFileLoaded(const QList<QImage>& path)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  switch (model_->getDocumentClass()) {
  case DocumentHeader::DocumentClass::kSateliteDocument:{
    auto currentDoc = leftPanel_->currentDocument();
    if ( currentDoc != nullptr ){
      auto image = model_->getTransparentImage(currentDoc->getClass(), currentDoc->getFileId());
      ui_->view->setImage(image);
    }
    break;
  }
  default:{
    ui_->view->setImages(path);
  }
  }
  if ( nullptr != pinningAction_ ){
    auto currentDocument = leftPanel_->currentDocument();
    pinningAction_->setCurrentImageName( currentDocument->getFileId(), currentDocument->displayName() );
  }
  QApplication::restoreOverrideCursor();
}


void WgtDocView::addImageToMap(const QString& name, const QString& displayName, const QImage& image )
{
  pinningAction_->setCurrentImageName( name, displayName );
  pinningAction_->setImage( image );
}

void WgtDocView::addCurrentDocumentToMap()
{
  if ( nullptr != pinningAction_ ) {
    DocumentHeader* document = leftPanel_->currentDocument();
    auto selectedDocumentImage = model_->getTransparentImage(document->getClass(), document->getFileId());
    addImageToMap( document->getFileId(), document->displayName(), selectedDocumentImage );
  }
}



void WgtDocView::enablePinningFaxToMap()
{
  meteo::app::MainWindow* mw = WidgetHandler::instance()->mainwindow();
  if( nullptr != mw ) {
    if ( true == leftPanel_->isPinningEnabled() ){
      meteo::map::proto::Document blank = meteo::global::lastBlankParams(meteo::map::proto::kGeoMap);
      QString loaderName = "geo.old";
      if (blank.has_geoloader()) {
        loaderName = QString::fromStdString(blank.geoloader());
      }
      auto mapWindow_ = new meteo::map::MapWindow( mw, blank, loaderName, MnCommon::etcPath() + "/document.menu.conf" );
      mw->toMdi(mapWindow_);
      QMdiSubWindow *wind = qobject_cast<QMdiSubWindow*>( parent() );
      if ( nullptr != wind ){
        wind->layout()->removeWidget(this);
        wind->setWidget(nullptr);
        mw->mdi()->removeSubWindow(wind);
      }
      pinningDialog_ = meteo::map::Faxaction::toDialog(this, mapWindow_);

      QObject::connect( pinningDialog_, SIGNAL(destroyed()), this, SLOT(slotDestroyed()) );

      mapWindow_->addPluginGroup("meteo.map.common");
      mapWindow_->addPluginGroup("meteo.map.weather");

      if ( nullptr == pinningAction_ ) {
        setPinningAction( qobject_cast<meteo::map::Faxaction*>( mapWindow_->mapscene()->getAction(meteo::map::Faxaction::faxActionName) ) );
      }

      if ( nullptr == pinningAction_) {
        error_log << QObject::tr("Ошибка - не получить action сцены");
        return;
      }
    }
    else {
      pinningAction_->disconnect(this);
    }
  }
}

QHash<WgtDocViewPanel::TreeWidgetsColumns, Condition> WgtDocView::getCurrentConditions()
{
  auto docClass = leftPanel_->getCurrentDocumentClass();
  QHash<WgtDocViewPanel::TreeWidgetsColumns, Condition> conditions;
  switch (docClass) {
  case DocumentHeader::kAmbiguous:
  case DocumentHeader::kFormalDocument:
  case DocumentHeader::kLocalDocument:{
    break;
  }
  case DocumentHeader::kFax:{
    conditions[WgtDocViewPanel::kFaxTT] = model_->getFilterFaxTT();
    conditions[WgtDocViewPanel::kFaxAA] = model_->getFilterFaxAA();
    conditions[WgtDocViewPanel::kFaxCCCC] = model_->getFilterFaxCCCC();
    conditions[WgtDocViewPanel::kFaxII] = model_->getFilterFaxII();
    break;
  }
  case DocumentHeader::kMapImage:{
    conditions[WgtDocViewPanel::kMapImageCenter] = model_->getFilterMapImageCenter();
    conditions[WgtDocViewPanel::kMapImageHour] = model_->getFilterMapImageHour();
    conditions[WgtDocViewPanel::kMapImageJobName] = model_->getFilterMapImageJobName();
    conditions[WgtDocViewPanel::kMapImageModel] = model_->getFilterMapImageModel();
    conditions[WgtDocViewPanel::kMapImageName] = model_->getFilterMapImageName();
    break;
  }
  case DocumentHeader::kSateliteDocument:{
    conditions[WgtDocViewPanel::kSateliteTT] = model_->getFilterSateliteTT();
    conditions[WgtDocViewPanel::kSateliteAA] = model_->getFilterSateliteAA();
    conditions[WgtDocViewPanel::kSateliteCCCC] = model_->getFilterSateliteCCCC();
    conditions[WgtDocViewPanel::kSateliteII] = model_->getFilterSateliteII();
    break;
  }
  }
  return conditions;
}

void WgtDocView::pinningSettingsOpen()
{
  QHash<WgtDocViewPanel::TreeWidgetsColumns, Condition> conditions = getCurrentConditions();
  auto docClass = leftPanel_->getCurrentDocumentClass();


  PinDialog dialog( docClass, &pinningStorage_, pinningAction_->getTargetRect(), pinningAction_->getSourceRect(), conditions );
  dialog.exec();

  if ( dialog.result() == QDialog::Accepted ){
    applyPinning( dialog.getPinning()  );
  }
}
void WgtDocView::printCurrentDocument()
{
  auto currentDocument = leftPanel_->currentDocument();
  if ( nullptr != currentDocument ){
    model_->printCurrentDocument( currentDocument->getClass(), currentDocument->getFileId() );
  }
}

void WgtDocView::slotHoverWidgetAction( WgtHoveringWidget::HoverWidgetActions action )
{
  switch (action) {
  case WgtHoveringWidget::kPinningExtendedSettings:{
    pinSumbenuExtendedOpen();
    break;
  }
  case WgtHoveringWidget::kAnimationPlayPause:{
    animationController_.animationPause();
    break;
  }
  case WgtHoveringWidget::kAnimationPlaySceneOnly:{
    animate( false, true );
    break;
  }
  case WgtHoveringWidget::kAnimationPlayMapOnly:{
    animate( true, false );
    break;
  }
  case WgtHoveringWidget::kAnimationTimeoutChanged:{
    animationController_.setTimeout( hover_->getAnimationDelay() );
    break;
  }
  case WgtHoveringWidget::kHideLeftWidgets:{
    isLeftWidgetsHidden = !isLeftWidgetsHidden;

    ui_->wgtLeftPart->setHidden( isLeftWidgetsHidden );
    hover_->restoreButtonIcons(isLeftWidgetsHidden);
    if ( false == isLeftWidgetsHidden ){
      QList<int> currentSizez = ui_->splitter->sizes();
      if ( currentSizez.size() != 2 ) {
        error_log << QObject::tr("Внимание: структура виджета изменена, необходимо изменение данной функции");
        return;
      }
      if ( 30 >= currentSizez[0] ) {
        int diff = 30 - currentSizez[0];
        currentSizez[0] = 30;
        currentSizez[1] = currentSizez[1] - diff;
        ui_->splitter->setSizes(currentSizez);
      }
    }
    break;
  }
  case WgtHoveringWidget::kAnimationNext:{
    animationController_.animationNext();
    break;
  }
  case WgtHoveringWidget::kAnimationPrew:{
    animationController_.animationPrew();
    break;
  }
  case WgtHoveringWidget::kAnimationPlaySceneAndMap:{
    animate( true, true );
    break;
  }
  case WgtHoveringWidget::kAnimationStop:{
    animationController_.animationStop();
    break;
  }
  case WgtHoveringWidget::kAnimationSave:{
    animationSave();
    break;
  }
  case WgtHoveringWidget::kPinningSettings:{
    pinSumbenuOpen();
    break;
  }
  case WgtHoveringWidget::kCurrentSlideChanged:{
    leftPanel_->setCurrentSlide(hover_->getCurrentSlide());
    break;
  }
  case WgtHoveringWidget::kRotateClockwise:{
    ui_->view->rotate(90);
    break;
  }
  case WgtHoveringWidget::kRotateAntiClockwise:{
    ui_->view->rotate(-90);
    break;
  }
  case WgtHoveringWidget::kAnimationRepeatChanged:{
    animationController_.setRepeat(hover_->isAnimationCycled());
    break;
  }
  case WgtHoveringWidget::kScaleReset:{
    ui_->view->scaleReset();
    break;
  }
  case WgtHoveringWidget::kScaleOptimal:{
    ui_->view->scaleOptimal();
    break;
  }
  case WgtHoveringWidget::kDocumentPrint:{
    printCurrentDocument();
    break;
  }
  case WgtHoveringWidget::kDocumentEdit:{
    DocumentHeader* currentDocument = leftPanel_->currentDocument();
    if ( nullptr != currentDocument ) {
      model_->openDocument( currentDocument );
    }
    break;
  }
  }
}

void WgtDocView::slotDocumentClassChanged( DocumentHeader::DocumentClass docClass )
{
  model_->setDocumentClass(docClass);
  switch (docClass) {
  case DocumentHeader::kFax:
  case DocumentHeader::kMapImage:
  case DocumentHeader::kSateliteDocument:{
    hover_->setAnimationEnabled(true);
    break;
  }
  case DocumentHeader::kAmbiguous:
  case DocumentHeader::kFormalDocument:
  case DocumentHeader::kLocalDocument:{
    hover_->setAnimationEnabled(false);
    break;
  }



  }
}

void WgtDocView::animate( bool onMap, bool onScene )
{
  if ( false == onMap && false == onScene ){
    error_log << QObject::tr("Ошибка: хотя бы одно значение должно быть установлено в true");
  }

  if ( false == prepareAnimationData( onMap ) ) {
    error_log << QObject::tr("Отменено пользователем");
    return;
  }

  QList<QImage> images;
  for ( auto id: leftPanel_->selectedDocuments() ) {
    auto document = leftPanel_->getDocumentById(id);
    if ( true == onMap ) {
      images << model_->getTransparentImage( document->getClass(), document->getFileId() );
    }
    else {
      images << model_->getImage( document->getClass(), document->getFileId() );
    }
  }

  if ( true == onScene ) {
    QObject::connect( &animationController_, &AnimationController::signalAnimationSlideChanged, this, &WgtDocView::slotAnimationSlideChanged );
  }
  else {
    QObject::disconnect( &animationController_, &AnimationController::signalAnimationSlideChanged, this, &WgtDocView::slotAnimationSlideChanged );
  }
  if ( nullptr != pinningAction_ ) {
    if ( true == onMap ) {
      pinningAction_->addAnimationLayer(animationController_);
    }
    else {
      pinningAction_->removeAnimationLayer();
    }
  }

  if ( true == animationController_.isStopped() ) {
    animationController_.updateImageList(images);
    animationController_.animationStart();
  }
  else if ( true == animationController_.isPaused() ){
    animationController_.animationResume();
  }
}

void WgtDocView::animationSave()
{
  QStringList documents = leftPanel_->selectedDocuments() ;

  if ( true == documents.isEmpty() ){
    warning_log.msgBox() << QObject::tr("Выберите хотя бы одно изображение для создания анимации");
  }

  QString caption = QString::fromUtf8("Сохранение данных");

  QString  selectedFilter;
  QString fileName = meteo::FileDialog::getSaveFileName(this, caption,
                                                        QDateTime::currentDateTime().toString("yyyy.MM.dd_hh:mm") + ".gif",
                                                        QString::fromUtf8("Файл в формате GIF (*.gif)"),
                                                        &selectedFilter);

  if ( false == fileName.isEmpty() ) {
    model_->slotSaveAnimation(documents, fileName, hover_->getAnimationDelay() );
  }
}

void WgtDocView::slotSelectedDocumentsCountUpdate( int current, int total )
{
  hover_->updateSlidesCount( current, total );
}

void WgtDocView::slotAnimationStarted()
{
  leftPanel_->setDocumentListEnabled(false);
  hover_->setAnimationState(true);
}

void WgtDocView::slotAnimationFinished()
{
  leftPanel_->setDocumentListEnabled(true);
  hover_->setAnimationState(false);
  if ( nullptr != pinningAction_ ) {
    pinningAction_->removeAnimationLayer();
  }
}

void WgtDocView::slotAnimationResumed()
{
  hover_->setAnimationState(true);
}

void WgtDocView::slotAnimationPaused()
{
  hover_->setAnimationState(false);
}

void WgtDocView::slotPanelLoaded()
{
  if ( false == isInitialized ){
    isInitialized = true;
    emit initialized();
  }
}

void WgtDocView::loadSettings()
{
  QFile ofile(configPath);
  if ( false == ofile.open(QIODevice::ReadOnly) ){
    return;
  }
  auto data = ofile.readAll();
  if ( false == data.isEmpty() ){
    QDataStream stream(data);
    stream >> spliteerSizes_;
  }
}

void WgtDocView::saveSettings()
{
  QFile ofile(configPath);
  if ( false == ofile.open(QIODevice::WriteOnly) ){
    return;
  }
  QDataStream stream(&ofile);
  stream << spliteerSizes_;

}

}
}
