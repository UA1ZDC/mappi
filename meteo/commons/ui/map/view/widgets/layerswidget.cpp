#include "layerswidget.h"
#include "ui_layers.h"
#include "../mapview.h"
#include "../mapscene.h"
#include "../actions/action.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>

#include <qevent.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsproxywidget.h>
#include <qgraphicslinearlayout.h>
#include <qobject.h>
#include <qtoolbutton.h>
#include <qdebug.h>
#include <qpalette.h>
#include <qtreewidget.h>
#include <qheaderview.h>
#include <qmessagebox.h>
#include <qgraphicseffect.h>
#include <qwidgetaction.h>

#include <cross-commons/debug/tmap.h>
#include <commons/textproto/tprototext.h>
#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/ui/custom/alphaedit.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/layermenu.h>
#include <meteo/commons/ui/map/event.h>
#include <meteo/commons/ui/custom/gradienteditor.h>
#include <meteo/commons/ui/custom/doublespinaction.h>



namespace meteo {
namespace map {

static const QList< float > kStepFixed = QList< float >()
     <<  0.25 << 0.5 << 1 << 2 << 3  << 4 << 5 << 8 << 10 << 16;

QString baseLayersVisibilityMapOldFile() { return QDir::homePath() + "/.meteo/baselayers.visibility.map.old.conf";}
QString baseLayersVisibilityMapPtkppFile() { return QDir::homePath() + "/.meteo/baselayers.visibility.map.ptkpp.conf";}
QString baseLayersVisibilityVPFile() { return QDir::homePath() + "/.meteo/baselayers.visibility.vprofile.conf";}
QString baseLayersVisibilityADFile() { return QDir::homePath() + "/.meteo/baselayers.visibility.ad.conf";}
QString baseLayersVisibilityODFile() { return QDir::homePath() + "/.meteo/baselayers.visibility.od.conf";}
QString baseLayersVisibilityMeteogramFile() { return QDir::homePath() + "/.meteo/baselayers.visibility.meteogram.conf";}


LayersWidget::LayersWidget( MapWindow* w ) :
  MapWidget(w),
  ui_(new Ui::Layers),
  deletevisible_(true),
  stepedit_(0),
  lastselected_(0),
  lastpos_(-1)
{
  ui_->setupUi(this);
  setTitle(QObject::tr("Слои"));
  setPixmap(QPixmap(":/meteo/icons/map/layers.png"));
  ui_->layertree->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui_->layertree, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotContextMenu(QPoint)));
  ui_->layertree->setColumnCount(2);
  ui_->layertree->setColumnWidth(0, 60);
  connect(ui_->layertree, SIGNAL(itemSelectionChanged()), SLOT(slotItemSelected()));
  connect(ui_->layertree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(slotItemSelected()));

  connect(ui_->btnUp, SIGNAL(clicked()), SLOT(slotLayerUp()));
  connect(ui_->btnDown, SIGNAL(clicked()), SLOT(slotLayerDown()));
  connect(ui_->btnRemove, SIGNAL(clicked()), SLOT(slotLayerRemove()));
  connect(ui_->btnActive, SIGNAL(clicked()), SLOT(slotLayerActive()));
  ui_->layertree->setStyleSheet(
        "QToolButton{"
        "background-color: white;"
        "border: 1px solid rgb(56,123,248);"
        "image: url(:/meteo/icons/tools/layer_hidden.png);"
        "}"
        "QToolButton:checked {"
        "image: url(:/meteo/icons/tools/layer_visible.png);"
        "}"
        );
  connect(ui_->closeBtn, SIGNAL(clicked()), SLOT(close()));
  if( true == hasDocument() ){
    if( 0 != mapview()->mapscene()->document()->eventHandler() ){
      mapview()->mapscene()->document()->eventHandler()->installEventFilter(this);
    }
  }
}

LayersWidget::~LayersWidget()
{
  delete ui_; ui_ = 0;
}

void LayersWidget::showEvent( QShowEvent* ev )
{
  updateLayerItems();
  MapWidget::showEvent(ev);
}

bool LayersWidget::eventFilter( QObject* watched, QEvent* ev )
{
  if( false == hasDocument() ){
    return MapWidget::eventFilter(watched, ev);
  }
  if ( watched == mapview()->mapscene()->document()->eventHandler() ) {
    if ( meteo::map::LayerEvent::LayerChanged == ev->type()) {
      LayerEvent* lev = static_cast<LayerEvent*>(ev);
      if ( lev->changeType() != LayerEvent::ObjectChanged) {
	updateLayerItems();
      }
    }
  }
  return MapWidget::eventFilter(watched, ev);
}

bool LayersWidget::hasDocument()
{
  if( 0 == mapview() ){
    return false;
  }
  if ( false == mapview()->hasMapscene() ) {
    return false;
  }
  if( 0 == mapview()->mapscene()->document() ){
    return false;
  }
  return true;
}

void LayersWidget::updateActiveLayer()
{
  if( false == hasDocument() ){
    return;
  }
  for( int i = 0; i < ui_->layertree->topLevelItemCount(); i++ ){
      ui_->layertree->topLevelItem(i)->setIcon(1, QIcon());
  }
  Layer* l = mapview()->mapscene()->document()->activeLayer();
  if( 0 == l ){
    return;
  }
  QTreeWidgetItem* itm = layerItem(l);
  if( 0 != itm ){
    itm->setIcon(1, QIcon(":/meteo/icons/tools/star_active.png"));
  }
}

void LayersWidget::setSelectedLastLayer()
{
  if ( -1 == mapdocument()->layers().indexOf(lastselected_) ) {
    lastselected_ = 0;
  }
  if ( 0 == lastselected_ ) {
    return;
  }
  QTreeWidgetItem* item = layerItem(lastselected_);
  if ( 0 != item ) {
    item->setSelected(true);
    if ( -1 != lastpos_ ) {
      ui_->layertree->verticalScrollBar()->setValue(lastpos_);
    }
  }
}

void LayersWidget::saveVisibilitySettings()
{
  QString fileName;
  switch (mapview()->mapscene()->document()->property().doctype()) {
    case meteo::map::proto::kGeoMap:
      if ( "geo.old" == mapview()->mapscene()->document()->property().geoloader() ) {
        fileName = baseLayersVisibilityMapOldFile();
      }
      else if ( "ptkpp" == mapview()->mapscene()->document()->property().geoloader() ) {
        fileName = baseLayersVisibilityMapPtkppFile();
      }
      else {
        fileName = QString();
      }
      break;
    case meteo::map::proto::kVerticalCut:
      fileName = baseLayersVisibilityVPFile();
      break;
    case meteo::map::proto::kAeroDiagram:
      fileName = baseLayersVisibilityADFile();
      break;
    case meteo::map::proto::kOceanDiagram:
      fileName = baseLayersVisibilityODFile();
      break;
    case meteo::map::proto::kMeteogram:
      fileName = baseLayersVisibilityMeteogramFile();
      break;
    case meteo::map::proto::kFormalDocument:
    default:
      fileName = QString();
      break;
  }

  meteo::Layers setts;
  for ( int i = 0, sz = ui_->layertree->topLevelItemCount(); i < sz; ++i ) {
    auto item = ui_->layertree->topLevelItem(i);
    auto w = qobject_cast<internal::LayerItemWidget*>(ui_->layertree->itemWidget(item, 0));
    if ( false == w->layer()->isBase() ) {
      continue;
    }
    auto l = setts.add_layer();
    l->set_visible(w->layer()->visible());
    l->set_name(w->layer()->name().toStdString());
  }

  if ( false == QFile::exists(fileName) ) {
    QFile f(fileName);
    f.open(QFile::WriteOnly);
    f.close();
  }

  if ( false == TProtoText::toFile(setts, fileName) ) {
    debug_log << QObject::tr("Не удалось сохранить настройки видимости базовых слоев");
  }
}

QTreeWidgetItem* LayersWidget::layerItem( Layer* l ) const
{
  for ( int i = 0, sz = ui_->layertree->topLevelItemCount(); i < sz; ++i ) {
    QTreeWidgetItem* item = ui_->layertree->topLevelItem(i);
    QString uuid = item->data( 0, Qt::UserRole ).toString();
    if ( l->uuid() == uuid ) {
      return item;
    }
  }
  return 0;
}

Layer* LayersWidget::itemLayer( QTreeWidgetItem* item ) const
{
  if ( 0 == mapdocument() ) {
    return 0;
  }
  QString uuid = item->data( 0, Qt::UserRole ).toString();
  QList<Layer*> list = mapdocument()->layers();
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    Layer* l = list[i];
    if ( l->uuid() == uuid ) {
      return l;
    }
  }
  return 0;
}

void LayersWidget::updateLayerItems()
{
  if( false == hasDocument() ){
    return;
  }
  QTreeWidgetItem* item = 0;
  QList<Layer*> layers =  mapview()->mapscene()->document()->layers();

  while ( 0 != ui_->layertree->topLevelItemCount() ) {
    QTreeWidgetItem* i = ui_->layertree->topLevelItem(0);
    delete ui_->layertree->itemWidget( i, 0 );
    delete i;
  }

  ui_->layertree->clear();
  for ( int i =  layers.size() - 1, stop = -1; i > stop; --i ) {
    Layer* l =  layers[i];
    item = new QTreeWidgetItem( ui_->layertree, item );
    item->setData( 0, Qt::UserRole, l->uuid() );
    item->setText( 1, l->name() );
    item->setSizeHint(0, QSize(0, 36));
    internal::LayerItemWidget* w = new internal::LayerItemWidget( l, item, ui_->layertree );
    w->setChecked(l->visible());
    ui_->layertree->setItemWidget( item, 0, w );
    ui_->layertree->setItemExpanded(item, true);
    connect(w, SIGNAL(toggled(bool)), SLOT(slotLayerVisibility(bool)));
  }
  updateActiveLayer();
  setSelectedLastLayer();
  if( 0 == ui_->layertree->selectedItems().count() ){
    ui_->layertree->setItemSelected(ui_->layertree->topLevelItem(0), true);
    slotItemSelected();
  }
}

void LayersWidget::slotItemSelected()
{
  if( false == hasDocument() ){
    return;
  }
  if( 0 == ui_->layertree->selectedItems().count() ){

    ui_->btnUp->setDisabled(true);
    ui_->btnDown->setDisabled(true);
    ui_->btnActive->setDisabled(true);
    ui_->btnRemove->setDisabled(true);
    return;
  }
  QTreeWidgetItem* item = ui_->layertree->selectedItems()[0];
  Layer* l = itemLayer(item);
  if ( 0 != l ) {
    lastselected_ = l;
    setDeleteVisible( false == l->isBase() );
  }

  ui_->btnUp->setEnabled(true);
  ui_->btnDown->setEnabled(true);
  ui_->btnActive->setEnabled(true);

  int index = ui_->layertree->indexOfTopLevelItem(item);
  if( 0 == index ){
    ui_->btnUp->setDisabled(true);
  }
  if( index == ui_->layertree->topLevelItemCount()-1 ){
    ui_->btnDown->setDisabled(true);
  }
  if( ui_->layertree->selectedItems().count() > 1 ){
    ui_->btnUp->setDisabled(true);
    ui_->btnDown->setDisabled(true);
    ui_->btnActive->setDisabled(true);
  }
  if ( 0 != l ){
    if( l == mapview()->mapscene()->document()->activeLayer() ){
      ui_->btnActive->setDisabled(true);
    }
  }
}

void LayersWidget::slotLayerUp()
{
  if( false == hasDocument() ){
    return;
  }
  if( 0 == ui_->layertree->selectedItems().count() ){
    return;
  }
  QTreeWidgetItem* item = ui_->layertree->selectedItems().at(0);
  lastpos_ = ui_->layertree->verticalScrollBar()->value();
  Layer* l = itemLayer(item);
  lastselected_ = l;
  int index = ui_->layertree->indexOfTopLevelItem(item);
  if( 0 == index || 0 == item){
    return;
  }
  QString uuid = item->data( 0, Qt::UserRole ).toString();
  if( true == uuid.isEmpty() ){
    return;
  }
  bool fl = mapview()->mapscene()->document()->moveLayer( uuid, 1 );
  if ( false == fl ) {
    return;
  }
}

void LayersWidget::slotLayerDown()
{
  if( false == hasDocument() ){
    return;
  }
  if( 0 == ui_->layertree->selectedItems().count() ){
    return;
  }
  QTreeWidgetItem* item = ui_->layertree->selectedItems().at(0);
  lastpos_ = ui_->layertree->verticalScrollBar()->value();
  Layer* l = itemLayer(item);
  lastselected_ = l;
  int index = ui_->layertree->indexOfTopLevelItem(item);
  if( index == ui_->layertree->topLevelItemCount()-1 || 0 == item){
    return;
  }
  QString uuid = item->data( 0, Qt::UserRole ).toString();
  if( true == uuid.isEmpty() ){
    return;
  }
  bool fl = mapview()->mapscene()->document()->moveLayer( uuid, -1 );
  if ( false == fl ) {
    return;
  }
}

void LayersWidget::slotLayerRemove()
{
  if( false == hasDocument() ){
    return;
  }
  QTreeWidgetItem* item = 0;
  QList<QTreeWidgetItem*> items = ui_->layertree->selectedItems();
  if( 0 == items.count() ){
    return;
  }
  QString names;
  foreach(item, items){
    QString uuid = item->data( 0, Qt::UserRole ).toString();
    names += " -" +  mapview()->mapscene()->document()->layerName(uuid) + "\n";
  }
  int res = QMessageBox::question(
              0,
              QObject::tr("Удаление"),
              QObject::tr("Удалить следующие слои? \n%1").arg(names),
              QObject::tr("Да"),
              QObject::tr("Нет")
              );
  if( 1 == res ){
    return;
  }
//  bool ok = false;
  foreach(item, items){
    QString uuid = item->data( 0, Qt::UserRole ).toString();
    if( true == uuid.isEmpty() ){
      return;
    }
    Layer* l = mapview()->mapscene()->document()->layer(uuid);
    if( 0 == l ){
      return;
    }
    if( 0 != l ){
      if( true == l->isBase() ){
        debug_log << "cant remove base layer!";
        continue;
      }
      delete l;
    }
  }
}

void LayersWidget::slotLayerActive()
{
  if( false == hasDocument() ){
    return;
  }
  QTreeWidgetItem* item = ui_->layertree->currentItem();
  if( 0 == item ){
    return;
  }
  QString uuid = item->data( 0, Qt::UserRole ).toString();
  if( true == uuid.isEmpty() ){
    return;
  }
  lastpos_ = ui_->layertree->verticalScrollBar()->value();
  Layer* l = itemLayer(item);
  lastselected_ = l;
  mapview()->mapscene()->document()->setActiveLayer(uuid);
  ui_->btnActive->setDisabled(true);
}

void LayersWidget::slotLayerVisibility(bool on)
{
  if( false == hasDocument() ){
    return;
  }
  internal::LayerItemWidget* w = qobject_cast<internal::LayerItemWidget*>(sender());
  QTreeWidgetItem* item = w->item();
  if( 0 == item ){
    return;
  }
  QVariant vrnt = item->data( 0, Qt::UserRole );
  if ( true == vrnt.isNull() || false == vrnt.isValid() ) {
    return;
  }
  QString uuid = vrnt.toString();
  if( true == uuid.isEmpty() ){
    return;
  }
  lastpos_ = ui_->layertree->verticalScrollBar()->value();
  Layer* l = w->layer();
  lastselected_ = l;
  if ( false == on ) {
    mapview()->mapscene()->document()->hideLayer(uuid);
  }
  else {
    mapview()->mapscene()->document()->showLayer(uuid);
  }

  saveVisibilitySettings();
}

void LayersWidget::slotContextMenu(QPoint pnt )
{
  slotItemSelected();
  QTreeWidgetItem* item = ui_->layertree->itemAt(pnt);
  Layer* l = itemLayer(item);
  if ( 0 == l ) {
    return;
  }
  QMenu* menu = new QMenu(this);
  lastselected_ = l;
  lastpos_ = ui_->layertree->verticalScrollBar()->value();
  if ( 0 != l ) {
    LayerMenu* m = l->layerMenu();
    if ( 0 != m ) {
      QList<QTreeWidgetItem*> list = ui_->layertree->selectedItems();
      QList<Layer*> layers;
      for ( int i = 0, sz = list.size(); i < sz; ++i ) {
        Layer* ll = itemLayer(list[i]);
        layers.append(ll);
      }
      m->setLayersForRemove(layers);
      m->addActions(menu);
    }
  }
  for ( auto a : mapscene()->actions() ) {
    a->addActionsToMenu(menu);
  }
  menu->exec(QCursor::pos());
  delete menu;
}

//! Видимость кнопки и меню удаления слоев
void  LayersWidget::setDeleteVisible( bool fl )
{
  deletevisible_ = fl;

  ui_->btnRemove->setEnabled(fl);
}

namespace internal{

LayerItemWidget::LayerItemWidget( Layer* l, QTreeWidgetItem* i, QWidget* parent)
  : QWidget(parent),
  layer_(l),
  item_(i)
{
  btn_ = new QToolButton(this);
  QHBoxLayout* hbl = new QHBoxLayout(this);
  hbl->addWidget(btn_);
  hbl->setMargin(5);
  btn_->setAutoRaise(true);
  btn_->setCheckable(true);
  btn_->setChecked(true);
  connect(btn_, SIGNAL(toggled(bool)), this, SIGNAL(toggled(bool)));
}

LayerItemWidget::~LayerItemWidget()
{
  this->blockSignals(true);
  delete btn_; btn_ = 0;
  item_ = 0;
  layer_ = 0;
}

bool LayerItemWidget::isChecked()
{
  return btn_->isChecked();
}

void LayerItemWidget::setChecked(bool on)
{
  btn_->setChecked(on);
}

}

}
}
