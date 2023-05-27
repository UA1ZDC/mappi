#include "monitorpagewidget.h"
#include "ui_monitorpagewidget.h"
#include "notfoundwidget.h"

#include <qcheckbox.h>
#include <qdir.h>
#include <qlabel.h>
#include <qmenu.h>
#include <qwidgetaction.h>
#include <qmenu.h>
#include <qaction.h>

#include <cross-commons/debug/tlog.h>

namespace meteo
{

MonitorPageWidget::MonitorPageWidget(const QString& managerId, QWidget* parent)
  : QWidget(parent),
    ui_(new Ui::MonitorPageWidget),
    state_(),
    currentTab_(0)
{
  this->setObjectName("MonitorPageWidget");
  spoFilter_ = "no_conditions";
  managerId_ = managerId;

  icoProcStatus_[app::OperationState_NONE] = QPixmap(":/meteo/appclient/icons/process_none.png").scaled(17,17, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  icoProcStatus_[app::OperationState_NORM] = QPixmap(":/meteo/appclient/icons/ledgreen.png").scaled(17,17, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  icoProcStatus_[app::OperationState_WARN] = QPixmap(":/meteo/appclient/icons/ledyellow.png").scaled(17,17, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  icoProcStatus_[app::OperationState_ERROR] = QPixmap(":/meteo/appclient/icons/ledred.png").scaled(17,17, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

  icoProcState_[app::AppState::PROC_RUNNING] = QPixmap(":/meteo/appclient/icons/ok.png").scaled(17,17, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  icoProcState_[app::AppState::PROC_STOPPED] = QPixmap(":/meteo/appclient/icons/process_error.png").scaled(17,17, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

  state_ = new app::AppState;
  menu_ = new QMenu(this);
  settings_ = new QSettings(QDir::homePath() + "/.meteo/" + this->objectName() + ".ini", QSettings::IniFormat);
  notFound_ = new NotFoundWidget;

  ui_->setupUi(this);
  ui_->restartBtn->setDisabled(true);

  QAction* actStart = new QAction(tr("Запустить процесс"), 0);
  actStart->setIcon(QIcon(":/meteo/appclient/icons/run_proc.png"));
  actStart->setIconVisibleInMenu(true);
  QObject::connect( actStart, SIGNAL(triggered()), SLOT(slotStart()));
  menu_->addAction(actStart);

  QAction* actStop = new QAction(tr("Остановить процесс"), 0);
  actStop->setIcon(QIcon(":/meteo/appclient/icons/stop_proc.png"));
  actStop->setIconVisibleInMenu(true);
  QObject::connect( actStop, SIGNAL(triggered()), SLOT(slotStop()));
  menu_->addAction(actStop);
  ui_->tableProcs->setContextMenuPolicy(Qt::CustomContextMenu);
  ui_->tableProcs->horizontalHeader()->setSectionsMovable(true);

  QHeaderView *verticalHeader = ui_->tableProcs->verticalHeader();
  verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
  verticalHeader->setDefaultSectionSize(40);
  ui_->tableProcs->setIconSize(QSize(36, 36));

  ui_->columnBtn->setObjectName("ColumnButton");
  ui_->splitter->setObjectName("splitter");
  ui_->tableProcs->horizontalHeader()->setObjectName("header");
  ui_->restartBtn->setProperty("state", "stop");
  ui_->restartBtn->setIcon(QIcon(":/meteo/appclient/icons/restart_red.png"));
  ui_->restartBtn->setToolTip(tr("Остановить процессы"));

  ui_->tableProcs->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
  ui_->tableProcs->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
  _settingsMenu = new QMenu( this );
  _settingsMenu->addAction(tr( "Отображаемые столбцы" ));
  _settingsMenu->addSeparator();

  ui_->stackedWidget->setCurrentIndex(0);

  QString label;
  QStringList headerLabels;

  for(int i = 0; i < ui_->tableProcs->columnCount(); i++){
    if( ui_->tableProcs->horizontalHeaderItem(i) != 0 ){
      headerLabels.append(ui_->tableProcs->horizontalHeaderItem(i)->text());
    }
  }
  int i = 0;
  foreach( label, headerLabels ){
    QCheckBox* check = new QCheckBox(_settingsMenu);
    checklist_.append(check);
    check->setText(label);
    QWidgetAction *chkBoxAction  = new QWidgetAction(_settingsMenu);
    chkBoxAction->setDefaultWidget(check);
    chkBoxAction->setText(label);
    acttmp.append(chkBoxAction);
    QObject::connect( check, SIGNAL(toggled(bool)), this, SLOT(slotColumnCheckToggled(bool)) );
    check->blockSignals(true);
    checkmap_.insert(i, check);
    i++;
  }


  createHeader();
  createAdditionalMenu();
  header_->setSectionsMovable(true);
  header_->setObjectName("header");
  header_->setSectionResizeMode(1, QHeaderView::Fixed);
  header_->setSectionResizeMode(2, QHeaderView::Fixed);
  if(settings_->contains("spoFilter_"+managerId_)) {
    spoFilter_ = settings_->value("spoFilter_"+managerId_).toString();
  }
  auto it = checkmap_.begin();
  auto end = checkmap_.end();
  for (; it != end; ++it ) {
     if ( ( it.value()->text() == "Id")
           || ( it.value()->text() == "Файл")) {
       ui_->tableProcs->hideColumn(it.key());
     }
  }  // Столбцы Файл и ID по умолчанию скрыты
  slotRestoreGeometryAndState();
  QObject::connect(ui_->tableProcs, SIGNAL(currentCellChanged(int,int,int,int)), SLOT(slotItemClicked()));
  QObject::connect(ui_->tableProcs, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), SLOT(slotItemDoubleClicked()));
  QObject::connect(header_, SIGNAL(sectionMoved(int,int,int)), SLOT(slotMoveColumn(int,int,int)));
  QObject::connect(ui_->tableProcs, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotContextMenu(QPoint)));
  QObject::connect(  ui_->tableBtn, SIGNAL(toggled(bool)), this, SLOT(slotHeaderCheckToggled(bool)) );
  QObject::connect(ui_->updateBtn, SIGNAL(clicked()), SIGNAL(updateServices()));
  QObject::connect(ui_->columnBtn, SIGNAL(clicked()), SLOT(slotColumnsClicked()));
  QObject::connect(ui_->restartBtn, SIGNAL(clicked()), SLOT(slotRestartServices()));
  buildMenu();
  if(true ==  ui_->tableBtn->isChecked() ){
    slotHeaderCheckToggled(true);
  }else{
    slotHeaderCheckToggled(false);
  }

  slotSaveSplitterState();
  bool headerBlocked = header_->signalsBlocked();
  header_->blockSignals(true);
  ui_->tableProcs->selectRow(0);
  ui_->tableProcs->sortByColumn(0);
  header_->blockSignals(headerBlocked);
  header_->fixComboPositions();
  createAdditionalMenu();
  foreach (auto check, checklist_) {
    check->blockSignals(false);
  }
}

MonitorPageWidget::~MonitorPageWidget()
{
  slotSaveTableState();
  slotSaveSplitterState();
}

void MonitorPageWidget::setAppIconVisible(bool visible)
{
  appIconVisible_ = visible;
  for ( int i = 0, isz = ui_->tableProcs->rowCount(); i < isz; ++i ) {
    ui_->tableProcs->item(i,0)->setIcon(QIcon());
  }
}

void MonitorPageWidget::setLogDefaultFontSize(int size)
{ logFontSize_ = size; }

void MonitorPageWidget::setProcList(const app::AppState& list)
{
  *state_ = list;

  ui_->restartBtn->setEnabled(state_->procs_size() > 0);

  bool sort = ui_->tableProcs->isSortingEnabled();
  ui_->tableProcs->setSortingEnabled(false);

  ui_->tableProcs->setRowCount(state_->procs_size());
  for ( int i = 0, sz = state_->procs_size(); i < sz; ++i ) {
    const app::AppState::Proc& proc = state_->procs(i);

    QTableWidgetItem* itemApp = new QTableWidgetItem(QString::fromUtf8(proc.app().title().c_str()));
    if ( appIconVisible_ ) {
      itemApp->setIcon(QIcon(QString::fromUtf8(proc.app().icon().c_str())));
    }
    ui_->tableProcs->setItem(i, 0, itemApp);

    QTableWidgetItem* itemState = new QTableWidgetItem();
    ui_->tableProcs->setItem(i, 1, itemState);
    setCellPixmap(icoProcState_.value(proc.state()), i, 1);
    itemState->setToolTip(toolTipFromState(proc.state()));

    QTableWidgetItem* itemStatus = new QTableWidgetItem();
    ui_->tableProcs->setItem(i, 2, itemStatus);
    setCellPixmap(icoProcStatus_.value(proc.status().state()), i, 2);
    itemStatus->setToolTip(toolTipFromStatus(proc.status().state()));

    QTableWidgetItem* itemPath = new QTableWidgetItem(QString::fromUtf8(proc.app().path().c_str()));
    ui_->tableProcs->setItem(i, 3, itemPath);

    QTableWidgetItem* itemId = new QTableWidgetItem(QString::number(proc.id()));
    ui_->tableProcs->setItem(i, 4, itemId);
    QTableWidgetItem* spoName = new QTableWidgetItem(QString::fromUtf8(proc.app().sponame().c_str()));
    ui_->tableProcs->setItem(i, 5, spoName);
    if (!spoList_.contains(QString::fromUtf8(proc.app().sponame().c_str()))) {
      spoList_.append(QString::fromUtf8(proc.app().sponame().c_str()));
    }

    itemmap_.insert(proc.id(), itemId);
    statemap_.insert(proc.id(), proc.state());

    ProcMonitorWidget* procw = new ProcMonitorWidget(this);
    procw->setLogFontSize(logFontSize_);
    procwidgetmap_.insert(proc.id(), procw);

    windexmap_.insert(proc.id(), ui_->stackedWidget->addWidget(procw));

    QString args(QObject::tr("Параметры запуска: "));
    if( proc.app().arg_size() == 0 ){
      args += QObject::tr("не заданы");
    }
    for ( int i = 0, sz = proc.app().arg_size(); i < sz; ++i ){
      args += "\n arg: " + QString::fromStdString(proc.app().arg(i));
    }

    ui_->tableProcs->item(itemId->row(), 0)->setToolTip(args);

    QObject::connect( procw, SIGNAL(start()), SLOT(slotStart()) );
    QObject::connect( procw, SIGNAL(stop()), SLOT(slotStop()) );
    QObject::connect( procw, SIGNAL(stateUpdate(app::AppState::Proc)), SLOT(slotUpdate(app::AppState::Proc)) );
    QObject::connect( procw, SIGNAL(currentTabChange(int)), SLOT(slotChangeTabIndex(int)) );
    QObject::connect( procw, SIGNAL(scrollbackChange(int)), SLOT(slotScrollBackChange(int)) );

    procw->update(proc);
  }

  bool headerBlocked = header_->signalsBlocked();
  header_->blockSignals(true);
  ui_->tableProcs->setSortingEnabled(sort);
  header_->blockSignals(headerBlocked);
//  slotRestoreGeometryAndState();
  ui_->tableProcs->selectRow(0);
  slotItemClicked();
  createAdditionalMenu();
  filterProcsList();
}

app::AppState_Proc MonitorPageWidget::procById(int64_t id) const
{
  for ( int i=0,isz=state_->procs_size(); i<isz; ++i ) {
    if ( state_->procs(i).id() == id ) {
      return state_->procs(i);
    }
  }
  return app::AppState_Proc();
}

void MonitorPageWidget::buildMenu()
{
  for (int i = 0; i < ui_->tableProcs->columnCount(); i++) {
    actlist_.append(acttmp[header_->logicalIndex(i)]);
    _settingsMenu->addAction(acttmp[header_->logicalIndex(i)]);
    foreach (auto box, checklist_) {
      box->blockSignals(true);
    }
    if(false == ui_->tableProcs->isColumnHidden(header_->logicalIndex(i))){
      checklist_.at(header_->logicalIndex(i))->setChecked(true);
    }else{
      checklist_.at(header_->logicalIndex(i))->setChecked(false);
    }
    foreach (auto box, checklist_) {
      box->blockSignals(false);
    }
  }
}

void MonitorPageWidget::slotMoveColumn(int /*logicalIndex*/, int oldVisualIndex, int newVisualIndex)
{
  QAction* before;
  QAction* act = actlist_[oldVisualIndex];
  if( newVisualIndex == actlist_.count()-1){
    _settingsMenu->addAction (act);
  }else{
    if( newVisualIndex <  oldVisualIndex){
      before = actlist_[newVisualIndex];
    }else{
      before = actlist_[newVisualIndex+1];
    }
    _settingsMenu->insertAction(before, act);
  }
  QAction* rmact = actlist_[oldVisualIndex];
  actlist_.removeAt(oldVisualIndex);
  actlist_.insert(newVisualIndex, rmact);
}

void MonitorPageWidget::slotStop()
{
  int64_t id = ui_->tableProcs->item(ui_->tableProcs->currentRow(), 4)->text().toLongLong();
  emit stop(id, managerId_);
}

void MonitorPageWidget::slotStart()
{
  int64_t id = ui_->tableProcs->item(ui_->tableProcs->currentRow(), 4)->text().toLongLong();
  emit start(id, managerId_);
}

void MonitorPageWidget::slotRestart()
{
  int64_t id = ui_->tableProcs->item(ui_->tableProcs->currentRow(), 4)->text().toLongLong();
  emit stop(id, managerId_);
  emit start(id, managerId_);
}

void MonitorPageWidget::slotChangeTabIndex(int index)
{
  currentTab_ = index;
}

void MonitorPageWidget::slotRestartServices()
{
  QToolButton* btn = qobject_cast<QToolButton*>(sender());
  if ( 0 == btn ) {
    return;
  }
  if ( "start" == btn->property("state").toString() ) {
    btn->setProperty("state", "stop");
    btn->setIcon(QIcon(":/meteo/appclient/icons/restart_red.png"));
    btn->setToolTip(tr("Остановить процессы"));
    foreach ( int64_t procid, restartlist_ ) {
      emit start(procid, managerId_);
    }
  }
  else if ( "stop" == btn->property("state").toString() ) {
    btn->setProperty("state", "start");
    btn->setIcon(QIcon(":/meteo/appclient/icons/restart_green.png"));
    btn->setToolTip(tr("Запустить процессы"));
    restartlist_.clear();
    if ("no_conditions" == spoFilter_ ) {
      foreach ( int64_t procid, statemap_.keys() ) {
        if ( statemap_[procid] == app::AppState::PROC_RUNNING ) {
          restartlist_.append(procid);
          emit stop(procid, managerId_);
        }
      }
    }
    else {
      foreach ( int64_t procid, statemap_.keys() ) {
        if ( statemap_[procid] == app::AppState::PROC_RUNNING ) {
          for(int i = 0; i < ui_->tableProcs->rowCount(); ++i) {
            if (ui_->tableProcs->item(i,5)->text() == spoFilter_
             && ui_->tableProcs->item(i,4)->text().toLongLong() == procid) {
              restartlist_.append(procid);
              emit stop(procid, managerId_);
            }
          }
        }
      }
    }
  }
}

void MonitorPageWidget::slotSaveSplitterState()
{
  settings_->setValue(ui_->splitter->objectName() + tabName_ +".state",ui_->splitter->saveState());
  settings_->setValue(ui_->splitter->objectName() + tabName_ +".geometry",ui_->splitter->saveGeometry());
  settings_->sync();
}

void MonitorPageWidget::slotSaveButtonState()
{
  settings_->setValue(ui_->tableBtn->objectName() + tabName_ +".state", QString::number( ui_->tableBtn->isChecked() ));
}

void MonitorPageWidget::slotSaveTableState()
{
  header_->fixComboPositions();

  settings_->setValue(header_->objectName() + tabName_ +".state",header_->saveState());
  settings_->setValue(header_->objectName() + tabName_ +".geometry",header_->saveGeometry());
  settings_->sync();
}

void MonitorPageWidget::slotScrollBackChange(int value)
{
  QMap<int64_t, ProcMonitorWidget*>::iterator it = procwidgetmap_.begin();
  QMap<int64_t, ProcMonitorWidget*>::iterator end = procwidgetmap_.end();
  while( it != end ){
    it.value()->setScrollBackSize(value);
    it++;
  }
}

void MonitorPageWidget::slotRestoreGeometryAndState()
{
  bool headerBlocked = header_->signalsBlocked();
  header_->blockSignals(true);
  header_->restoreGeometry(settings_->value(header_->objectName()  + tabName_ +".geometry").toByteArray());
  header_->restoreState(settings_->value(header_->objectName()  + tabName_ +".state").toByteArray());
  header_->blockSignals(headerBlocked);

  buildMenu();

  ui_->splitter->restoreState(settings_->value(ui_->splitter->objectName() + tabName_ +".state").toByteArray());
  ui_->splitter->restoreGeometry(settings_->value(ui_->splitter->objectName() + tabName_ +".geometry").toByteArray());

  ui_->tableBtn->setChecked(settings_->value(ui_->tableBtn->objectName() + tabName_ +".state").toInt());

  ui_->tableProcs->setColumnWidth(1, 40);
  ui_->tableProcs->setColumnWidth(2, 40);

  int r = ui_->tableProcs->currentRow();
  if ( -1 == r && ui_->tableProcs->rowCount() > 0 ) {
    ui_->tableProcs->selectRow(0);
  }
}

void MonitorPageWidget::slotItemClicked()
{
  if ( ui_->tableProcs->rowCount() == 0 ) { return; }

  int row = ui_->tableProcs->currentRow();
  if ( -1 == row ) {
    ui_->tableProcs->selectRow(0);
    return;
  }

  int64_t procid = ui_->tableProcs->item(row, 4)->text().toLongLong();
  if ( false == windexmap_.contains(procid) ) {
    return;
  }
  ui_->stackedWidget->setCurrentIndex(windexmap_[procid]);
  if( procwidgetmap_.contains(procid) ){
    procwidgetmap_[procid]->setCurrentTab(currentTab_);
    procwidgetmap_[procid]->slotAdjustParamColumns();
  }
}

void MonitorPageWidget::slotItemDoubleClicked()
{
  int64_t procid = ui_->tableProcs->item(ui_->tableProcs->currentRow(), 4)->text().toLongLong();
  if ( false == statemap_.contains(procid) ) {
    return;
  }

  if ( statemap_[procid] == app::AppState::PROC_RUNNING ) {
    emit stop(procid, managerId_);
  }
  else if( statemap_[procid] == app::AppState::PROC_STOPPED  ) {
    emit start(procid, managerId_);
  }
}

void MonitorPageWidget::slotUpdate(const app::AppState::Proc& proc)
{
  if ( false == itemmap_.contains(proc.id()) ) {
    return;
  }

  int row = itemmap_[proc.id()]->row();

  setCellPixmap(icoProcState_.value(proc.state()), row, 1);
  ui_->tableProcs->item(row, 1)->setToolTip(toolTipFromState(proc.state()));

  setCellPixmap(icoProcStatus_.value(proc.status().state()), row, 2);
  ui_->tableProcs->item(row, 2)->setToolTip(toolTipFromStatus(proc.status().state()));

  QString args(QObject::tr("Параметры запуска: "));
  if( proc.app().arg_size() == 0 ){
    args += QObject::tr("не заданы");
  }
  for ( int i = 0, sz = proc.app().arg_size(); i < sz; ++i ){
    args += "\n arg: " + QString::fromUtf8(proc.app().arg(i).c_str());
  }
  ui_->tableProcs->item(row, 0)->setToolTip(args);

  if ( statemap_.contains(proc.id()) ) {
    statemap_.insert(proc.id(), proc.state());
  }
}

void MonitorPageWidget::slotColumnsClicked()
{
  _settingsMenu->exec( QCursor::pos() );
}

void MonitorPageWidget::slotColumnCheckToggled(bool on)
{
  QCheckBox* check = static_cast<QCheckBox*>(sender());
  if( on ){
    ui_->tableProcs->showColumn( checkmap_.key(check) );
    if (0 == ui_->tableProcs->columnWidth(checkmap_.key(check))) {
      ui_->tableProcs->setColumnWidth(checkmap_.key(check),41);
    }
  }
  else {
    ui_->tableProcs->hideColumn( checkmap_.key(check) );
  }
}

void MonitorPageWidget::slotHeaderCheckToggled(bool on)
{
  if( on ){
    header_->hide();
    ui_->tableProcs->verticalHeader()->hide();
  }
  else {
    header_->show();
    ui_->tableProcs->verticalHeader()->show();
  }
  slotSaveButtonState();
}

void MonitorPageWidget::slotContextMenu(QPoint point)
{
  QAction* start = menu_->actions().at(0);
  QAction* stop = menu_->actions().at(1);
  int64_t id = ui_->tableProcs->item(ui_->tableProcs->currentRow(), 4)->text().toLongLong();
  if( false == procwidgetmap_.contains(id) ){
    return;
  }
  if( procwidgetmap_[id]->isRunning() ){
    if( start != 0 && stop!= 0 ){
      start->setDisabled(true);
      stop->setDisabled(false);
    }
  }
  if( !procwidgetmap_[id]->isRunning() ){
    if( start != 0 && stop!= 0 ){
      start->setDisabled(false);
      stop->setDisabled(true);
    }
  }
  QAction* act = menu_->exec(ui_->tableProcs->mapToGlobal(point));
  if( 0 == act ){
    return ;
  }
}

ProcMonitorWidget* MonitorPageWidget::procw(int64_t id)
{
  return procwidgetmap_.value(id, nullptr);
}

void MonitorPageWidget::setConnectionLost(bool lost)
{
  if ( lost ) {
    ui_->tableProcs->hide();
    ui_->stackedWidget->setCurrentIndex(0);
    ui_->verticalLayout->addWidget(notFound_);
    notFound_->show();
  }
  else {
    ui_->tableProcs->show();
    ui_->verticalLayout->removeWidget(notFound_);
    notFound_->hide();
    slotItemClicked();
  }
}

bool MonitorPageWidget::isConnectionLost() const
{ return notFound_->isVisible(); }

void MonitorPageWidget::setCellPixmap(const QPixmap& pix, int row, int col)
{
  QLabel* label = qobject_cast<QLabel*>(ui_->tableProcs->cellWidget(row, col));
  if ( 0 == label ) {
    label = new QLabel(this);
    ui_->tableProcs->setCellWidget(row, col, label);
  }
  label->setPixmap(pix);
  label->setAlignment(Qt::AlignCenter);
}

QString MonitorPageWidget::toolTipFromState(app::AppState::ProcState state)
{
  switch( state ){
    case app::AppState::PROC_RUNNING : return tr("Состояние: запущен");
    case app::AppState::PROC_STOPPED : return tr("Состояние: остановлен");
    default : return QString();
  }
}

void MonitorPageWidget::createHeader()
{
  header_ = new bank::ViewHeader(this);
  header_->setObjectName(QString("%1_header").arg("currentTableName()"));
  QList<int> list;
  list.append(5);
  QObject::connect(header_, SIGNAL(activate(int,QAction*)), SLOT(slotMenuActivated(int,QAction*)));
  header_->setFilterColumns(list);
  header_->setMinimumSectionSize(26);
  ui_->tableProcs->setHorizontalHeader(header_);
  header_->init();
  header_->fixComboPositions();
}

void MonitorPageWidget::createAdditionalMenu()
{
  if( 0 == header_ ){
    return;
  }
  QMenu* menu = new QMenu(header_);
  QAction* act;
  act = menu->addAction(QIcon(":/meteo/icons/no-conditions.png"),QObject::tr("(Все)"));
  act->setData("no_conditions");
  act->setIconVisibleInMenu(true);
  foreach (QString str, spoList_) {
    act = menu->addAction(str);
    act->setIconVisibleInMenu(true);
    act->setData(str);
  }
  header_->addMenu(menu, 5);
}

void MonitorPageWidget::slotMenuActivated(int col ,QAction* act)
{
  Q_UNUSED(col);
  spoFilter_ = act->data().toString();
  settings_->setValue("spoFilter_"+managerId_, spoFilter_);
  filterProcsList();
}

void MonitorPageWidget::filterProcsList()
{
  if ("no_conditions" == spoFilter_ ) {
   ui_->tableProcs->horizontalHeaderItem(5)->setText("СПО Все");
    for(int i = 0; i < ui_->tableProcs->rowCount(); ++i) {
      if(0 == ui_->tableProcs->rowHeight(i)) {
        ui_->tableProcs->setRowHeight(i,40);
      }
      ui_->tableProcs->showRow(i);
    }
  }
  else {
    ui_->tableProcs->horizontalHeaderItem(5)->setText("СПО "+spoFilter_);
    for(int i = 0; i < ui_->tableProcs->rowCount(); ++i) {
      if (ui_->tableProcs->item(i,5)->text() == spoFilter_ ) {
        if(0 == ui_->tableProcs->rowHeight(i)) {
          ui_->tableProcs->setRowHeight(i,40);
        }
        ui_->tableProcs->showRow(i);
      }
      else {
        ui_->tableProcs->hideRow(i);
      }
    }
  }
}

QString MonitorPageWidget::toolTipFromStatus(app::OperationState state)
{
  switch( state ){
    case app::OperationState_NONE : return tr("Статус: не определен");
    case app::OperationState_NORM : return tr("Статус: нормальный");
    case app::OperationState_WARN : return tr("Статус: предупреждение");
    case app::OperationState_ERROR :return tr("Статус: ошибка");
    default : return QString();
  }
}

} // meteo
