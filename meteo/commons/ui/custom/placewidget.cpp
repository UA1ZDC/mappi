#include "placewidget.h"
#include "ui_placewidget.h"

#include <qdir.h>
#include <qmenu.h>
#include <qsettings.h>

Q_DECLARE_METATYPE( QAction* )

static const QString kSettingsFile = QDir::homePath() + "/.meteo/placewidget.ini";

PlaceWidget::PlaceWidget(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui::PlaceWidget)
{
  stationWidget_ = 0;

  ui_->setupUi(this);

  model_ = 0;
  ui_->table->setModel(model_);

  minVisibleItems_ = 3;

  placeTypeButtons_ += ui_->departBtn;
  placeTypeButtons_ += ui_->routeBtn;
  placeTypeButtons_ += ui_->reserveBtn;
  placeTypeButtons_ += ui_->landBtn;

  setExternalStationWidget(0);

  connect( ui_->addBtn, SIGNAL(clicked(bool)), SLOT(slotAddClicked()) );
  connect( ui_->delBtn, SIGNAL(clicked(bool)), SLOT(slotDelClicked()) );
  connect( ui_->upBtn,  SIGNAL(clicked(bool)), SLOT(slotUpClicked()) );
  connect( ui_->downBtn,SIGNAL(clicked(bool)), SLOT(slotDownClicked()) );

  connect( this, SIGNAL(placeAdded()),   SLOT(slotCalcTreeMinSize()) );
  connect( this, SIGNAL(placeRemoved()), SLOT(slotCalcTreeMinSize()) );
  connect( this, SIGNAL(placeAdded()),   SLOT(slotAdjustColumns()) );
  connect( this, SIGNAL(placeRemoved()), SLOT(slotAdjustColumns()) );

  for ( int i=0,isz=placeTypeButtons_.size(); i<isz; ++i ) {
    connect( placeTypeButtons_.at(i), SIGNAL(toggled(bool)), SLOT(slotPlaceTypeToggled(bool)) );
  }

  ui_->newBtn->setChecked(false);
  loadHistory();
  slotCalcTreeMinSize();
}

PlaceWidget::~PlaceWidget()
{
  delete ui_;
}

QString PlaceWidget::departureIndex() const
{
  if ( 0 == model_ ) { return QString(); }

  int row = model_->findRow(tr("вылет"), PlaceModel::kTypeColumn);
  if ( -1 == row ) {
    return QString();
  }

  return model_->index(row, PlaceModel::kIndexColumn).data().toString();
}

QString PlaceWidget::landIndex() const
{
  if ( 0 == model_ ) { return QString(); }

  int row = model_->findRow(tr("посадка"), PlaceModel::kTypeColumn);
  if ( -1 == row ) {
    return QString();
  }

  return model_->index(row, PlaceModel::kIndexColumn).data().toString();
}

QList<int> PlaceWidget::routeIndexes() const
{
  if ( 0 == model_ ) { return QList<int>(); }

  QList<int> list;
  int row = model_->findRow(tr("на маршруте"), PlaceModel::kTypeColumn);
  while ( row != -1 ) {
    list += model_->index(row, PlaceModel::kIndexColumn).data().toInt();

    row = model_->findRow(tr("на маршруте"), PlaceModel::kTypeColumn, row + 1);
  }
  return list;
}

QList<int> PlaceWidget::reserveIndexes() const
{
  if ( 0 == model_ ) { return QList<int>(); }

  QList<int> list;
  int row = model_->findRow(tr("запасной"), PlaceModel::kTypeColumn);
  while ( row != -1 ) {
    list += model_->index(row, PlaceModel::kIndexColumn).data().toInt();

    row = model_->findRow(tr("запасной"), PlaceModel::kTypeColumn, row + 1);
  }
  return list;
}

PlaceInfoList PlaceWidget::placeList() const
{
  if ( 0 == model_ ) { return PlaceInfoList(); }

  return model_->places();
}

StationWidget* PlaceWidget::stationWidget() const
{
  return stationWidget_;
}

void PlaceWidget::setExternalStationWidget(StationWidget* w)
{
  if ( 0 != stationWidget_ ) {
    disconnect( stationWidget_ );
  }

  if ( 0 == w ) {
    ui_->stationWidget->show();
    stationWidget_ = ui_->stationWidget;
  }
  else {
    ui_->stationWidget->hide();
    stationWidget_ = w;
  }
}

int PlaceWidget::maxEchelon() const
{
  if ( 0 == model_ ) { return -1; }

  int max = 0;
  QList<PlaceInfo> list = model_->places();
  // эшелон последнего пункта не проверяем
  for ( int i=0,isz=list.size()-1; i<isz; ++i ) {
    max = qMax(max, list[i].echelon);
  }
  return max;
}

int PlaceWidget::minEchelon() const
{
  if ( 0 == model_ ) { return -1; }

  QList<PlaceInfo> list = model_->places();

  if ( list.size() == 0 ) { return -1; }

  int min = list.first().echelon;
  for ( int i=0,isz=list.size(); i<isz; ++i ) {
    min = qMin(min, list[i].echelon);
  }
  return min;
}

void PlaceWidget::setModel(PlaceModel* model)
{
  if ( 0 != model_ ) {
    disconnect(model_, 0, 0, 0);
  }

  model_ = model;
  ui_->table->setModel(model);

  if ( 0 != model_ ) {
    connect( model_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(slotAdjustColumns()) );
  }
}

void PlaceWidget::slotClear()
{
  if ( 0 == model_ ) { return; }

  model_->clear();

  emit placeRemoved();
}

void PlaceWidget::slotLoadHistory()
{
  loadHistory();
}

void PlaceWidget::slotAddClicked()
{
  if ( 0 == stationWidget_ ) { return; }

  QString type = selectedType();
  QString name = stationWidget_->ruName();
  QString index = stationWidget_->stationIndex();
  meteo::GeoPoint c = stationWidget_->coord();

  slotAddPlace(type, name, index, c);

  saveHistory();
  loadHistory();
}

void PlaceWidget::slotDelClicked()
{
  if ( 0 == model_ ) { return; }

  foreach ( const QModelIndex& i, ui_->table->selectionModel()->selectedRows() ) {
    model_->removeRow(i.row());
    ui_->table->selectionModel()->clear();

    int row = i.row() - 1;
    selectRow(row < 0 ? 0 : row);
  }

  emit placeRemoved();
}

void PlaceWidget::slotUpClicked()
{
  if ( 0 == model_ ) { return; }

  QMap<int,bool> sort;
  foreach ( const QModelIndex& i, ui_->table->selectionModel()->selectedRows() ) {
    sort.insert(i.row(), true);
  }

  ui_->table->selectionModel()->clear();

  foreach ( int row, sort.keys() ) {
    model_->swapRows(row, row - 1);
    selectRow(row - 1);
  }

  if ( !ui_->table->selectionModel()->hasSelection() ) {
    selectRow(0);
  }
}

void PlaceWidget::slotDownClicked()
{
  if ( 0 == model_ ) { return; }

  QMap<int,bool> sort;
  foreach ( const QModelIndex& i, ui_->table->selectionModel()->selectedRows() ) {
    sort.insert(i.row(), true);
  }

  ui_->table->selectionModel()->clear();

  foreach ( int row, sort.keys() ) {
    if ( row + 1 >= model_->rowCount() ) { continue; }

    model_->swapRows(row, row + 1);

    selectRow(row + 1);
  }

  if ( !ui_->table->selectionModel()->hasSelection() ) {
    selectRow(model_->rowCount() - 1);
  }
}

void PlaceWidget::slotPlaceTypeToggled(bool toggle)
{
  QToolButton* btn = qobject_cast<QToolButton*>(sender());
  if ( 0 == btn ) {
    return;
  }

  if ( false == toggle ) {
    btn->setChecked(true);
    return;
  }

  for ( int i=0,isz=placeTypeButtons_.size(); i<isz; ++i ) {
    if ( btn != placeTypeButtons_.at(i) ) {
      placeTypeButtons_[i]->blockSignals(true);
      placeTypeButtons_[i]->setChecked(!toggle);
      placeTypeButtons_[i]->blockSignals(false);
    }
  }

  if ( 0 == model_ ) { return; }

  int row = model_->findLastRow(btn->text(), PlaceModel::kTypeColumn);

  if ( -1 == row ) { return; }

  ui_->table->selectionModel()->clear();
  selectRow(row);
}

void PlaceWidget::slotClearHistory()
{
  QSettings opt(kSettingsFile, QSettings::IniFormat);
  opt.remove("history/" + settingsGroup_);
  loadHistory();
}

void PlaceWidget::slotHistorySelected()
{
  if ( 0 == stationWidget_ ) { return; }

  QAction* act = qobject_cast<QAction*>(sender());
  if ( 0 == act ) { return; }

  PlaceInfo info = act->data().value<PlaceInfo>();
  slotAddPlace(info.type, info.name, info.index, info.coord);
}

void PlaceWidget::slotAddPlace(const QString& type, const QString& name, const QString& index, const meteo::GeoPoint& coord, int echelon)
{
  if ( 0 == model_ ) { return; }

  PlaceInfo info;
  info.type = type;
  info.name = name;
  info.index = index;
  info.echelon = echelon;
  info.coord = coord;

  int row = model_->findRow(type, PlaceModel::kTypeColumn);

  if ( tr("вылет") == type ) {
    if ( -1 == row ) {
      row = 0;
      model_->insertRow(-1);
    }
  }
  else if ( tr("посадка") == type ) {
    if ( -1 == row ) {
      row = model_->rowCount();
      model_->insertRow(row);
    }
  }
  else {
    row = -1;
    int r = model_->findRow(tr("посадка"), PlaceModel::kTypeColumn);
    if ( -1 != r ) {
      row = r;
      model_->insertRow(row);
    }
  }

  if ( -1 == row ) {
    row = model_->rowCount();
    model_->insertRow(row);
  }

  model_->setData(model_->index(row,0), QVariant::fromValue(info));

  ui_->table->selectionModel()->clear();
  ui_->table->selectionModel()->selectedColumns(row);

  emit placeAdded();
}

void PlaceWidget::slotCalcTreeMinSize()
{
  ui_->table->setMinimumHeight(35 + 20 + minVisibleItems_ * 22);
}

void PlaceWidget::slotAdjustColumns()
{
  if ( 0 == model_ ) { return; }

  for ( int i=0,isz=model_->columnCount(); i<isz; ++i ) {
    ui_->table->resizeColumnToContents(i);
  }
}

QString PlaceWidget::selectedType() const
{
  for ( int i=0,isz=placeTypeButtons_.size(); i<isz; ++i ) {
    if ( placeTypeButtons_.at(i)->isChecked() ) {
      return placeTypeButtons_.at(i)->text();
    }
  }

  return QString();
}

void PlaceWidget::loadHistory()
{
  QSettings opt(kSettingsFile, QSettings::IniFormat);

  QStringList keys;
  for ( int i=0,isz=placeTypeButtons_.size(); i<isz; ++i ) {
    keys += placeTypeButtons_.at(i)->text();
  }

  QMenu* menu = new QMenu(this);

  QAction* act = menu->addAction(QIcon(":/meteo/icons/delete-16.png"), tr("очистить"));
  connect( act, SIGNAL(triggered(bool)), SLOT(slotClearHistory()) );

  foreach ( const QString& key, keys ) {
    QStringList hist = opt.value("history/" + settingsGroup_ + "/" + key).toStringList();

    if ( hist.isEmpty() ) { continue; }

    QAction* sp = menu->addSeparator();
    sp->setText(key);

    foreach ( const QString& h, hist ) {
      PlaceInfo info = PlaceInfo::fromString(h);

      QString txt = QString("%1 (%2)").arg(info.name).arg(info.index);

      QAction* act = menu->addAction(txt);
      act->setData(QVariant::fromValue(info));

      connect( act, SIGNAL(triggered(bool)), SLOT(slotHistorySelected()) );
    }
  }

  ui_->histBtn->setMenu(menu);
}

void PlaceWidget::saveHistory()
{
  QSettings opt(kSettingsFile, QSettings::IniFormat);

  QStringList keys;
  for ( int i=0,isz=placeTypeButtons_.size(); i<isz; ++i ) {
    keys += placeTypeButtons_.at(i)->text();
  }

  foreach ( const QString& key, keys ) {
    QStringList hist = opt.value("history/" + settingsGroup_ + "/" + key).toStringList();

    foreach ( const PlaceInfo& i, model_->findPlaces(key, PlaceModel::kTypeColumn) ) {
      hist += i.toString();
    }

    hist.removeDuplicates();

    if ( hist.isEmpty() ) { continue; }

    opt.setValue("history/" + settingsGroup_ + "/" + key, hist);
  }
}

void PlaceWidget::selectRow(int row)
{
  if ( 0 == model_ ) { return; }

  for ( int col=0,isz=model_->columnCount(); col<isz; ++col ) {
    ui_->table->selectionModel()->select(model_->index(row,col), QItemSelectionModel::Select);
  }
}
