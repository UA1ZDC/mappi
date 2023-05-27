#include "customviewer.h"
#include "ui_customviewer.h"
#include <QDateTime>

#include <meteo/commons/global/dateformat.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/conditionmaker/conditiondialogs/conditiondialoglongint.h>
#include <meteo/commons/ui/conditionmaker/conditiondialogs/conditiondialogdouble.h>
#include <meteo/commons/ui/conditionmaker/conditiondialogs/conditionaldialogstring.h>

namespace meteo {
static const int defaultPageSize = 100;

CustomViewer::CustomViewer(QWidget *parent, CustomViewerDataModel *model) :
  QWidget(parent),
  ui_( new Ui::CustomViewer ),
  columnMenu_(nullptr),
  cellMenu_(nullptr),
  rm_filter_action_(nullptr),
  rm_sort_action_(nullptr),
  actionFilterBySelect_(nullptr),
  settings_(nullptr),
  header_(nullptr),
  timeWidget_(nullptr),
  dataModel_(model)
{
 ui_->setupUi( this );

 connect(ui_->leftPanelBtn, SIGNAL(clicked(bool)), SLOT(slotHideLeftPanel(bool)));
 connect(ui_->mainarea, &QSplitter::splitterMoved,
         this, &CustomViewer::slotSectionMoved);
 installEventFilter(this);
}


CustomViewer::~CustomViewer()
{
 slotSaveGeometryAndState();
 if (nullptr != this->ui_) {
  delete this->ui_;
  ui_ = nullptr;
 }
 if (nullptr != dataModel_){
  delete dataModel_;
  dataModel_ = nullptr;
 }
 if ( nullptr != this->settings_ ){
  delete this->settings_;
  this->settings_ = nullptr;
 }
}

bool CustomViewer::init()
{
 dataModel_->initDataModel();
 QString fileName = this->objectName().replace("::","_");
 settings_ = new QSettings(QDir::homePath() + "/.meteo/" + fileName + ".ini", QSettings::IniFormat);

 createTable();
 createBottomPanel();
 initConnect();
 createMenu();
 qApp->processEvents();
 if ( false == dataModel_->hasConditions() ){
  this->ui_->checkpanel->close();
 }

 QObject::connect(dataModel_, &CustomViewerDataModel::signalDataLoaded, this, &CustomViewer::slotDataLoaded);
 QObject::connect(dataModel_, &CustomViewerDataModel::signalConditionsChanged, this, &CustomViewer::slotFiltersChanged );
 QObject::connect(dataModel_, &CustomViewerDataModel::signalNewData, this, &CustomViewer::slolAllRecordsCountUpdate );
 QObject::connect(dataModel_, &CustomViewerDataModel::signalLock, this, &CustomViewer::slotLock );
 QObject::connect(dataModel_, &CustomViewerDataModel::signalUpdate, this, &CustomViewer::slotUpdate );
 QObject::connect(ui_->sizeBox, &QSpinBox::editingFinished, this, &CustomViewer::slotPageSizeChanged);

 if ( true == dataModel_->hasConditions() ){
  this->slotFiltersChanged();
 }

 this->setWindowTitle(dataModel_->windowTitle());

 auto leftWgt = dataModel_->getLeftPanelWidget();
 if ( nullptr != leftWgt ) {
  ui_->leftPanel->layout()->addWidget(leftWgt);
 }
 else {
  ui_->leftPanel->setHidden(true);
  ui_->leftPanelBtn->setHidden(true);
 }

 slotRestoreGeometryAndState();
 return true;
}

void CustomViewer::slotPageSizeChanged()
{
 int oldPageSize = dataModel_->getPageSize();
 int newPageSize = ui_->sizeBox->value();
 if ( oldPageSize != newPageSize ){
  dataModel_->setPageSize(ui_->sizeBox->value());
  dataModel_->setPage(0);
 }
}

int CustomViewer::logicalIndexByName(const QString& column_name)
{
 return dataModel_->columnIndexByName(column_name);
}

void CustomViewer::addItem(int row, const proto::CustomViewerTableRecord &data)
{
 if( nullptr == this->tablewidget() ){
  return;
 }

 for ( auto field: data.field() ){
  auto columnName = QString::fromStdString(field.name());
  auto value = QString::fromStdString(field.value());
  auto displayValue = QString::fromStdString(field.display_value());
  auto tooltip = QString::fromStdString(field.tooltip());
  auto icon = QString::fromStdString(field.icon());

  int columnIndex = dataModel_->columnIndexByName(columnName);

  CustomViewerItem* item = new CustomViewerItem(value);

  item->setData(Qt::UserRole, value);

  this->tablewidget()->setItem(row, columnIndex, item);

  QString itemText = displayValue;
  QString currentItemToolTip = tooltip;
  QIcon currentItemIcon = QIcon(icon);
  item->setText(itemText);
  item->setToolTip(currentItemToolTip);
  if ( false == currentItemIcon.isNull()) {
   item->setIcon(currentItemIcon);
  }
 }
}

void CustomViewer::fillTableHeaders(){
 for( int i = 0; i < dataModel_->columnSize(); ++i ){
  const QString& column =  dataModel_->columnName(i);
  const QString& title = dataModel_->columnDisplayName(column);
  CustomViewerItem* item = new CustomViewerItem(title);
  item->setData(Qt::UserRole, column);
  auto menu = dataModel_->columnConfig(column);
  if(nullptr != menu){
   auto displayTitle = QString::fromStdString(menu->display_name());
   auto tooltip = QString::fromStdString(menu->tooltip());
   item->setText(displayTitle);
   item->setToolTip(tooltip);
  }
  this->tablewidget()->setHorizontalHeaderItem(i,item);
 }
}

void CustomViewer::slotDataLoaded( const QList<proto::CustomViewerTableRecord> * data, const int &page)
{
 if (nullptr == data) return;
 this->tablewidget()->clear();
 this->tablewidget()->setRowCount(data->size());
 menu_.clear();
 fillTableHeaders();

 for (int i = 0; i < data->size(); ++i){
  this->addItem(i, data->at(i));
 }

 int first = dataModel_->getPageSize() * page ;
 int last = first + data->count() ;

 ui_->currPage->setText(QString::number(page + 1));
 if ( false == data->isEmpty() ) {
  ui_->countRow->setText(QString::fromUtf8("Показаны элементы: %1-%2").arg(first + 1).arg(last));
 }
 else {
  ui_->countRow->setText(QObject::tr("Нет данных"));
 }

 ui_->beginBtn->setDisabled( 0 == page );
 ui_->backBtn->setDisabled( 0 == page );
 ui_->nextBtn->setDisabled( dataModel_->getPageSize() != dataModel_->countDocuments() );
 ui_->endBtn->setDisabled( dataModel_->getPageSize() != dataModel_->countDocuments() );

 for ( auto col: dataModel_->filterColumns() ){
  int colIndex = dataModel_->columnIndexByName(col);
  header_->addFilterColumn(colIndex);
 }

 for ( auto col: dataModel_->filterColumns() ){
  int colIndex = dataModel_->columnIndexByName(col);
  auto menu = dataModel_->menu(col);
  if ( false == menu.isEmpty() ){
   continue;
  }

  auto valuesProto = dataModel_->getFieldsForColumn(col);
  QHash<QString, QString> values;
  for ( auto value: valuesProto ){
   values.insert(QString::fromStdString(value.value()),
                 QString::fromStdString(value.display_value()));
  }


  for ( auto value: values.keys() ) {
   menu_[colIndex] << qMakePair(value, values[value]);
  }
 }

 header_->buildMenu(menu_);
}


void CustomViewer::slotFiltersChanged()
{
 for (auto id : checkmap_.keys()){
  QTimer::singleShot( 0, checkmap_[id], SLOT(deleteLater()));
 }
 checkmap_.clear();

 QHash<int, Condition> conditions = dataModel_->getConditions();
 QHash<int, Condition> enabledConditions = dataModel_->getConditionsEnabled();

 for ( int i = 0; i < dataModel_->columnSize(); ++i ){
  header_->setHaveCondition( i, true == conditions.contains(i) );
 }

 if (conditions.isEmpty()){
  checkpanel()->close();
 }

 for (int column: conditions.keys()){
  auto condition = conditions[column];

  QCheckBox* check = new QCheckBox( condition.displayCondition(), checkpanel());
  check->setProperty("column_number", QString::number(column));
  check->setChecked( true == enabledConditions.contains(column) );
  check->setFocusPolicy(Qt::NoFocus);
  checkpanel()->addCheckBox(check);
  checkmap_.insert(column, check);
  if( false == checkpanel()->isVisible() && checkmap_.count() > 0){
   checkpanel()->show();
  }
  QObject::connect(check, &QCheckBox::clicked, this, &CustomViewer::slotSqlConditionClicked);
 }
}

void CustomViewer::slotSqlConditionClicked(bool ok){
 auto sender = qobject_cast<QCheckBox*> (QObject::sender());
 int col = sender->property("column_number").toInt();
 setConditionEnabled(col, ok);
}

ViewTableWidget* CustomViewer::tablewidget()
{
 return this->ui_->viewtable;
}

void CustomViewer::createTable()
{
 createHeader();

 auto table = tablewidget();
 table->setColumnCount(dataModel_->columnSize());
 fillTableHeaders();
 table->setAlternatingRowColors(true);
 //table->setObjectName(QString("%1_table").arg(dataModel_->tableName()));
 QString qss = QString(".ViewTableWidget{alternate-background-color: rgb(251, 244, 225);}");
 table->setStyleSheet(qss);

 table->setContextMenuPolicy(Qt::CustomContextMenu);
 table->setEditTriggers(QTableWidget::NoEditTriggers);

 table->setHorizontalHeader(header_);
 header_->setSortIndicatorShown(true);
 header_->setVisible(true);


 table->setSelectionBehavior(QAbstractItemView::SelectRows);
 table->setSelectionMode(QAbstractItemView::SingleSelection);
 table->setContextMenuPolicy(Qt::CustomContextMenu);

}

void CustomViewer::createMenu()
{
 if( nullptr == columnMenu_ ){
  columnMenu_ = new QMenu( this );
  columnMenu_->addAction(tr( "Отображаемые столбцы" ));
  columnMenu_->addSeparator();
 }

 foreach( QCheckBox* check, checklist_ ){
  delete check;
 }
 foreach( QAction* w, actlist_ ){
  delete w;
 }
 checklist_.clear();
 actlist_.clear();
 columnMenu_->clear();

 QList<QAction*> acttmp;

 for(int i = 0; i < dataModel_->columnSize(); ++i ){
  const QString& column_name = dataModel_->columnName(i);
  QCheckBox* check = new QCheckBox(columnMenu_);
  checklist_.append(check);
  QString title = dataModel_->columnDisplayName(column_name);
  if( true == title.isEmpty() ){
   title = this->dataModel_->columnHeaderTooltip(column_name);
  }
  if( true == title.isEmpty() ){
   title = column_name;
  }
  check->setText(title);
  check->setObjectName(column_name);
  QWidgetAction *chkBoxAction  = new QWidgetAction(columnMenu_);
  chkBoxAction->setDefaultWidget(check);
  chkBoxAction->setText(column_name);
  chkBoxAction->setObjectName(column_name);
  acttmp.append(chkBoxAction);
 }

 for (int index = 0; index < dataModel_->columnSize(); ++index) {

  if( -1 == index || index >= acttmp.size() ){
   continue;
  }
  actlist_.append(acttmp[index]);
  columnMenu_->addAction(acttmp[index]);
  if( true != this->tablewidget()->isColumnHidden(index) ){
   checklist_.at(index)->setChecked(true);
  }
  connect( checklist_.at(index), SIGNAL(toggled(bool)),
           this, SLOT(slotColumnCheckToggled(bool)) );
 }

 if( nullptr == cellMenu_ ){
  cellMenu_ = new QMenu( this );
  cellMenu_->addAction(tr( "Копировать" ), this, SLOT(slotCopyToClipboard()));
  cellMenu_->addSeparator();
  actionFilterBySelect_ = cellMenu_->addAction(tr( "Фильтр по выделению" ), this, SLOT(slotFilterBySelect()));
  rm_filter_action_ = cellMenu_->addAction(tr( "Удалить фильтр" ), this, SLOT(slotRemoveFilterBySelect()));
  cellMenu_->addSeparator();

  actionSortAsc_ = cellMenu_->addAction(tr( "По возрастанию" ), this, SLOT(slotSortUp()));
  actionSortDes_ = cellMenu_->addAction(tr( "По убыванию" ), this, SLOT(slotSortDown()));
  rm_sort_action_ = cellMenu_->addAction(tr( "Удалить сортировку" ), this, SLOT(slotRemoveSort()));
 }
}


void CustomViewer::createAdditionalMenu()
{
 if( nullptr == header_ ){
  return;
 }

 for ( int i = 0; i < dataModel_->columnSize(); ++i ){
  auto column = dataModel_->columnName(i);
  QList<proto::ViewerColumnMenu> menus = dataModel_->menu(column);
  if ( true == menus.isEmpty() ){
   continue;
  }

  QMenu* menu = new QMenu(header_);
  {
   QAction* act = menu->addAction(this->dataModel_->columnDisplayName(column));
   act->blockSignals(true);
   QFont font = act->font();
   font.setBold(true);
   act->setFont(font);
  }
  menu->addSeparator();
  {
   QAction* act = menu->addAction(QIcon(":/meteo/icons/no-conditions.png"),QObject::tr("(Все)"));
   act->setData(meteo::bank::ViewHeader::conditionNoConditions_);
   act->setIconVisibleInMenu(true);
  }
  for ( auto menuProto: menus ) {
   auto value = QString::fromStdString(menuProto.value());
   QIcon icon( QString::fromStdString(menuProto.icon()) );
   auto text = QString::fromStdString(menuProto.display_value());
   QAction* act = (true == icon.isNull())?menu->addAction(value):
                                          menu->addAction(QIcon(icon),value);
   act->setIconVisibleInMenu(true);
   act->setData(value);
   act->setText(text);
  }

  header_->addMenu(menu, logicalIndexByName(column));
 }
}


void CustomViewer::slotUpdate()
{
 int page = dataModel_->getCurrentPage();// ui_->curr_page->value() - 1;
 int pageSize = ui_->sizeBox->value();

 dataModel_->setPageSize(pageSize);
 dataModel_->setPage(page);

}

void CustomViewer::slotMenuActivated(int col, QAction* act)
{
 QString action_data = act->data().toString();
 QString actionText = act->text();
 auto columnName = dataModel_->columnName(col);
 if( meteo::bank::ViewHeader::conditionNoConditions_ == action_data ){
  bool allowRemove = checkBeforeChaningCondition(col);
  if ( true == allowRemove ) {
   dataModel_->conditionRemove(col);
  }
 }
 else if( meteo::bank::ViewHeader::conditionCustom_ == action_data ){
  createCondition(col);
 }
 else if ( false == action_data.isEmpty() ){
  QHash<int, QString> allData;
  allData.insert(col, action_data);
  dataModel_->addConditionByText(col, action_data, actionText);
 }
}

void CustomViewer::createCondition(int col)
{
 auto datatype = dataModel_->columnType(col);

 Condition cond;
 switch (datatype) {
 case  proto::kLongInt:{
  ConditionDialogLongInt dialog( dataModel_->columnName(col), dataModel_->columnDisplayName(col), 0 );
  if ( QDialog::Accepted != dialog.exec() ){
   return;
  }
  cond = dialog.getCondition();
  break;
 }
 case proto::kBool:{
  break;
 }
 case proto::kDateTime:{
  this->createConditionTime(col);
  break;
 }
 case proto::kUnknown:{
  break;
 }
 case proto::kString:{
  ConditionalDialogString dialog( dataModel_->columnName(col), dataModel_->columnDisplayName(col) );
  if ( QDialog::Accepted != dialog.exec() ){
   return;
  }
  cond = dialog.getCondition();
  break;
 }
 case proto::kDouble:{
  break;
 }
 case proto::kOid:{
  break;
 }
 case proto::kInt:{
  ConditionCreationDialogInt dialog( dataModel_->columnName(col), dataModel_->columnDisplayName(col) );
  if ( QDialog::Accepted != dialog.exec() ){
   return;
  }
  cond = dialog.getCondition();
  break;
 }
 case proto::kArray:{
  break;
 }
 }

 if ( true == cond.isNull() ) {
  this->header_->setFilterEnabled(col, false);
  return;
 }
 this->dataModel_->addCondition(col, cond);
}

void CustomViewer::createConditionTime(int col)
{
 const QString& colname = dataModel_->columnName(col);
 if( nullptr == timeWidget_ ){
  timeWidget_ = new SelectDateWidget(this);
 }
 int res = timeWidget_->exec();

 if ( QDialog::Accepted != res ) {
  this->header_->setFilterEnabled(col, false);
  return;
 }
 QDateTime dtStart = timeWidget_->dt_start();
 QDateTime dtEnd = timeWidget_->dt_end();

 Condition condition = ConditionDateTimeInterval(colname,dataModel_->columnDisplayName(col), dtStart, dtEnd);
 dataModel_->addCondition(col, condition);
}


void CustomViewer::initConnect()
{
 connect(ui_->updateBtn, SIGNAL(clicked()), SLOT(slotUpdate()));
 connect(ui_->beginBtn, SIGNAL(clicked()),SLOT(slotBeginClicked()));
 connect(ui_->backBtn, SIGNAL(clicked()), SLOT(slotBackClicked()));
 connect(ui_->nextBtn, SIGNAL(clicked()), SLOT(slotNextClicked()));
 connect(ui_->endBtn, SIGNAL(clicked()), SLOT(slotEndClicked()));
 connect(ui_->columnsBtn, SIGNAL(clicked()), SLOT(slotColumnsClicked()));
 connect (header_, SIGNAL(activate(int,QAction*)), SLOT(slotMenuActivated(int,QAction*)));
 connect (header_, SIGNAL(movecolumn(int,int)), SLOT(slotMoveColumn(int,int)));
 connect(this->tablewidget(), SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotContextMenu()));
}

void CustomViewer::slotColumnsClicked()
{
 columnMenu_->exec(QCursor::pos());
}

void CustomViewer::createHeader()
{
 header_ = new meteo::bank::ViewHeader(this);
 header_->init();
 for ( auto col: dataModel_->filterColumns() ){
  int colIndex = dataModel_->columnIndexByName(col);
  header_->addFilterColumn(colIndex);
 }
 //header_->setObjectName(QString("%1_header").arg(dataModel_->tableName()));
 header_->setMinimumSectionSize(26);
 header_->init();
 QObject::connect(header_, SIGNAL(geometriesChanged()), this, SLOT(slotHeaderGeometryChanged()));
 QObject::connect(header_, SIGNAL(sectionMoved(int, int, int)), this, SLOT(slotHeaderGeometryChanged()));
 connect(header_, SIGNAL(sectionClicked(int)), this, SLOT(slotSortResult()));
 createAdditionalMenu();
}


void CustomViewer::slotHeaderGeometryChanged(){

}

void CustomViewer::addWidgetToPanel(int index, QWidget* w)
{
 if( nullptr == w ){
  return;
 }
 QHBoxLayout* hbl = qobject_cast<QHBoxLayout*>(ui_->toolPanel->layout());
 if( nullptr == hbl){
  return;
 }
 hbl->insertWidget(index, w);
}

void CustomViewer::slotColumnCheckToggled(bool on)
{
 QString column_name = sender()->objectName();
 int index = logicalIndexByName(column_name);
 if( on ){
  header_->showSection( index );
  if ( this->tablewidget()->columnWidth(index) < minimumColumnWidth){
   this->tablewidget()->setColumnWidth(index, minimumColumnWidth);
  }
 }
 else {
  header_->hideSection( index );
 }
}

void CustomViewer::slotSaveGeometryAndState()
{
 settings_->setValue(header_->objectName()+".state",header_->saveState());
 settings_->setValue(header_->objectName()+".geometry",header_->saveGeometry());

 auto leftPanelBtn = this->ui_->leftPanelBtn;
 if( nullptr != leftPanelBtn ){
  settings()->setValue(leftPanelBtn->objectName()+".state", QString::number( leftPanelBtn->isChecked() ));
 }
 auto splitter = this->ui_->mainarea;
 if( nullptr != splitter ){
  settings()->setValue(splitter->objectName()+".state", splitter->saveState());
  settings()->setValue(splitter->objectName()+".geometry", splitter->saveGeometry());
 }

 settings_->sync();
}

void CustomViewer::slotRestoreGeometryAndState()
{
 if (false == settings_->contains(header_->objectName()+".state"))
 {
  setDefaultSectionSize();
  setDefaultSectionOrder();
  setDefaultSectionVisible();
 }

 header_->restoreState(settings_->value(header_->objectName()+".state").toByteArray());
 header_->restoreGeometry(settings_->value(header_->objectName()+".geometry").toByteArray());
 dataModel_->setSort(header_->sortIndicatorSection(), header_->sortIndicatorOrder());

 auto leftPanelBtn = this->ui_->leftPanelBtn;
 if( nullptr != leftPanelBtn ){
  leftPanelBtn->setChecked(settings()->value(leftPanelBtn->objectName()+".state").toInt());
  slotHideLeftPanel(settings()->value(leftPanelBtn->objectName()+".state").toInt());
 }

 auto splitter = this->ui_->mainarea;
 if( nullptr != splitter ){
  splitter->restoreState(settings()->value(splitter->objectName()+".state").toByteArray());
  splitter->restoreGeometry(settings()->value(splitter->objectName()+".geometry").toByteArray());
 }

 qApp->processEvents();
 auto actions = columnMenu_->actions();
 auto columnMenuActionByColumnName = [this](const QString& colName){
  for ( QAction* act: columnMenu_->actions() ){
   if ( 0 == colName.compare(act->objectName()) ){
    return qobject_cast<QWidgetAction*>(act);
   }
  }
  return (QWidgetAction*) nullptr;
 };

 for ( int i = 0; i < dataModel_->columnSize(); ++i ){
  auto column = dataModel_->columnName(i);
  int colwidth = tablewidget()->columnWidth(i);
  bool isHidden = tablewidget()->isColumnHidden(i);
  QWidgetAction* act = columnMenuActionByColumnName(column);

  if ( nullptr == act ){
   warning_log << QObject::tr("Колонка не найдена!");
   continue;
  }
  QCheckBox* box = qobject_cast<QCheckBox*>(act->defaultWidget());
  box->setCheckState(false == isHidden && 0 != colwidth ? Qt::Checked: Qt::Unchecked);
 }
}

void CustomViewer::slotMoveColumn(int oldVisualIndex, int newVisualIndex)
{
 QAction* before;
 if ( actlist_.count() < oldVisualIndex || actlist_.count() < newVisualIndex ){
  return;
 }
 QAction* act = actlist_[oldVisualIndex];
 if( newVisualIndex == actlist_.count()-1){
  columnMenu_->addAction (act);
 }else{
  if( newVisualIndex <  oldVisualIndex){
   before = actlist_[newVisualIndex];
  }else{
   before = actlist_[newVisualIndex+1];
  }
  columnMenu_->insertAction(before, act);
 }
 QAction* rmact = actlist_[oldVisualIndex];
 actlist_.removeAt(oldVisualIndex);
 actlist_.insert(newVisualIndex, rmact);
}


void CustomViewer::slotBeginClicked()
{
 dataModel_->setFirstPage();
}

void CustomViewer::slotBackClicked()
{
 dataModel_->setPrewPage();
}

void CustomViewer::slotNextClicked()
{
 dataModel_->setNextPage();
}

void CustomViewer::slotEndClicked()
{
 auto sortColumn = dataModel_->getSortColumn();
 auto sortOrder = dataModel_->getSortOrder();
 switch( sortOrder ){
 case Qt::SortOrder::AscendingOrder:{
  dataModel_->setSort(sortColumn, Qt::SortOrder::DescendingOrder);
  break;
 }
 case Qt::SortOrder::DescendingOrder:{
  dataModel_->setSort(sortColumn, Qt::SortOrder::AscendingOrder);
  break;
 }
 }
 dataModel_->setFirstPage();
 header_->setSortIndicator(dataModel_->columnIndexByName(dataModel_->getSortColumn()), dataModel_->getSortOrder());
}

void CustomViewer::slotLock(bool state){
 if ( true == state ){
  QApplication::setOverrideCursor(Qt::WaitCursor);
 }
 else {
  QApplication::restoreOverrideCursor();
 }
}

void CustomViewer::slolAllRecordsCountUpdate(long int value){
 if ( 0 > value ) {
  error_log << QObject::tr("Некорректное значение value: %1").arg(value);
 }
 auto totalTemplate = dataModel_->stringTemplateTotalRecords();
 ui_->countTlg->setText(totalTemplate.arg(value));
}

void CustomViewer::createBottomPanel()
{
 if( nullptr != checkpanel() ){
  connect(checkpanel(), SIGNAL(panelClosed()), SLOT(slotPanelClosed()));
 }
}

void CustomViewer::setConditionEnabled(int col, bool on)
{
 bool allowEnable = checkBeforeChaningCondition(col);
 if ( true == allowEnable ) {
  dataModel_->conditionEnable(col, on);
 }
 else {
  if ( (true == checkmap_.contains(col)) && (false == on) ) {
   checkmap_[col]->setCheckState(Qt::Checked);
  }
 }
}


void CustomViewer::slotDateChecked(bool on)
{
 QCheckBox* check = qobject_cast<QCheckBox*>(sender());
 if( nullptr == check ){
  return;
 }
 int colnumber = checkmap_.key(check);
 setConditionEnabled(colnumber, on);
}

void CustomViewer::slotPanelClosed()
{
 if ( false == dataModel_->hasConditions() ) return;
 meteo::app::MainWindow* mw = WidgetHandler::instance()->mainwindow();

 bool allow = mw->askUser("Удаление условий отбора может замедлить скорость выполнения запросов. Продолжить?");
 if ( true == allow ) {
  dataModel_->clearConditions();
 }
 else {
  QTimer::singleShot(0, checkpanel(), &CheckPanel::show);
 }
}

void CustomViewer::slotContextMenu()
{
 if( nullptr == cellMenu_ ){
  return;
 }
 QTableWidgetItem* tableItem = this->tablewidget()->currentItem();
 if( nullptr != tableItem ){
  int col = tableItem->column();
  auto colName = dataModel_->columnName(col);
  if (nullptr != rm_filter_action_){
   rm_filter_action_->setEnabled(checkmap().contains(col));
  }
  if (nullptr != actionFilterBySelect_) {
   actionFilterBySelect_->setEnabled(dataModel_->filterColumns().contains(colName));
  }
  if ( nullptr != actionSortAsc_ ){
   actionSortAsc_->setEnabled(dataModel_->columnCanSort(col));
  }
  if ( nullptr != actionSortDes_ ){
   actionSortDes_->setEnabled(dataModel_->columnCanSort(col));
  }
  if( nullptr != header_ && nullptr != rm_sort_action_ ){
   rm_sort_action_->setEnabled( (header_->isSortIndicatorShown()) && (col == header_->sortIndicatorSection()) );
  }
 }
 cellMenu_->exec(QCursor::pos());
}

void CustomViewer::slotCopyToClipboard()
{
 if( nullptr == this->tablewidget() ){
  return;
 }
 QClipboard *clipboard = QApplication::clipboard();
 QTableWidgetItem* tableItem = this->tablewidget()->currentItem();
 if( nullptr != clipboard && nullptr != tableItem ){
  clipboard->setText(tableItem->text());
 }
}

void CustomViewer::slotFilterBySelect()
{
 if( nullptr == this->tablewidget() ){
  return;
 }
 QTableWidgetItem* tableItem = this->tablewidget()->currentItem();
 if( nullptr != tableItem ){
  int col = tableItem->column();
  QString action_data = tableItem->data(Qt::UserRole).toString();
  QString displayValue = tableItem->text().isEmpty()? tableItem->toolTip() : tableItem->text();

  dataModel_->addConditionByText(col, action_data, displayValue);
 }
}

bool CustomViewer::checkBeforeChaningCondition(int col)
{
 bool allowRemove = true;
 meteo::app::MainWindow* mw = WidgetHandler::instance()->mainwindow();
 if ( true == dataModel_->ifWillRemoveLastRecomendedField(col) ) {
  allowRemove = mw->askUser(
     QObject::tr("Удаление условия отбора по полю '%1' может привести к увеличению длительности запросов. Продолжить?")
     .arg(dataModel_->columnDisplayName(col)));
 }
 return allowRemove;
}


void CustomViewer::slotRemoveFilterBySelect()
{
 if( nullptr == this->tablewidget() ){
  return;
 }
 QTableWidgetItem* tableItem = this->tablewidget()->currentItem();
 if( nullptr != tableItem ){
  int col = tableItem->column();
  bool allowRemove = checkBeforeChaningCondition(col);
  if (allowRemove) {
   dataModel_->conditionRemove(col);
  }
 }
}

void CustomViewer::slotSortUp()
{
 if( nullptr == this->tablewidget() ){
  return;
 }
 QTableWidgetItem* tableItem = this->tablewidget()->currentItem();
 if( nullptr != tableItem ){
  int col = tableItem->column();
  if( nullptr != header_ ){
   header_->setSortIndicator(col, Qt::AscendingOrder);
   header_->setSortIndicatorShown(true);
   slotSortResult();
  }
 }
}

CheckPanel* CustomViewer::checkpanel()
{
 return this->ui_->checkpanel;
}

void CustomViewer::slotSortDown()
{
 if( nullptr == this->tablewidget() ){
  return;
 }
 QTableWidgetItem* tableItem = this->tablewidget()->currentItem();
 if( nullptr != tableItem ){
  int col = tableItem->column();
  if( nullptr != header_ ){
   header_->setSortIndicator(col, Qt::DescendingOrder);
   header_->setSortIndicatorShown(true);
   slotSortResult();
  }
 }
}

void CustomViewer::slotRemoveSort()
{
 if( nullptr != header_ ){
  int col_id = logicalIndexByName(id_column_);
  header_->setSortIndicator(col_id, Qt::AscendingOrder);
  slotSortResult();
  header_->setSortIndicatorShown(false);
 }
}


void CustomViewer::slotSortResult()
{
 int sortColumn = header_->sortIndicatorSection();
 Qt::SortOrder sortOrder =  header_->sortIndicatorOrder();
 header_->setSortIndicatorShown(true);
 if ( true == dataModel_->columnCanSort(sortColumn)){
  dataModel_->setSort(sortColumn, sortOrder);
  dataModel_->setFirstPage();
 }
 else {
  auto restoredSortOrder = dataModel_->getSortOrder();
  QString restoredSortColumn = dataModel_->getSortColumn();
  int sortColIndex = dataModel_->columnIndexByName(restoredSortColumn);
  header_->setSortIndicator(sortColIndex, restoredSortOrder  );
 }
}

void CustomViewer::setDefaultSectionSize()
{
 auto splitter = this->ui_->mainarea;
 splitter->setSizes({ 1, 1});
}

void CustomViewer::setDefaultSectionOrder()
{

}

void CustomViewer::setDefaultSectionVisible()
{

}

void CustomViewer::slotHideLeftPanel(bool on)
{
 auto leftPanelBtn = this->ui_->leftPanelBtn;
 auto splitter = this->ui_->mainarea;
 auto sizes = splitter->sizes();
 if ( sizes.size() != 2 ){
  error_log << QObject::tr("Ошибка: структура виджета изменена. Ожидается ровно 2 части");
  return;
 }
 auto total = sizes[0] + sizes[1];

 if( on == true ){
  sizes[0] = 0;
  sizes[1] = total;

  if( nullptr != leftPanelBtn ){
   leftPanelBtn->setIcon(QIcon(":/meteo/icons/showbtn.png"));
  }
 }else {
  if( nullptr != leftPanelBtn ){
   leftPanelBtn->setIcon(QIcon(":/meteo/icons/hidebtn.png"));
  }
  if ( sizes[0] < total / 4 ) {
   sizes[0] = total / 4;
   sizes[1] = total - sizes[0];
  }
 }
 splitter->setSizes(sizes);
}

void CustomViewer::slotSectionMoved(int pos, int index)
{
 if ( 1 == index ){
  this->ui_->leftPanelBtn->setChecked(0 == pos);
  this->slotHideLeftPanel(0 == pos);
 }
}

void CustomViewer::reload()
{
 this->dataModel_->threadDataLoad();
}

}
