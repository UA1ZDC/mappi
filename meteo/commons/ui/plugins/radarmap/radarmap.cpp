#include "radarmap.h"

#include <qmenu.h>

#include <cross-commons/debug/tmap.h>

#include <sql/psql/psqlquery.h>

#include <meteo/commons/global/dateformat.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/ui/map/layermrl.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/weather.h>

#include "ui_radarmap.h"

namespace meteo {
namespace map {

const int kClmnTime     = 0;
const int kClmnCount    = 1;

QMap< QAbstractButton*, QPair<int,int> > initLayers( Ui::RadarMap* ui )
{
  QMap< QAbstractButton*, QPair<int,int> > map = TMap<QAbstractButton*,QPair<int,int> >()
    << qMakePair( (QAbstractButton*)ui->layer_0_0, qMakePair(0,0) )
    << qMakePair( (QAbstractButton*)ui->layer_0_1000, qMakePair(0,1000) )
    << qMakePair( (QAbstractButton*)ui->layer_1000_2000, qMakePair(1000,2000) )
    << qMakePair( (QAbstractButton*)ui->layer_2000_3000, qMakePair(2000,3000) )
    << qMakePair( (QAbstractButton*)ui->layer_3000_4000, qMakePair(3000,4000) )
    << qMakePair( (QAbstractButton*)ui->layer_4000_5000, qMakePair(4000,5000) )
    << qMakePair( (QAbstractButton*)ui->layer_5000_6000, qMakePair(5000,6000) )
    << qMakePair( (QAbstractButton*)ui->layer_6000_7000, qMakePair(6000,7000) )
    << qMakePair( (QAbstractButton*)ui->layer_7000_8000, qMakePair(7000,8000) )
    << qMakePair( (QAbstractButton*)ui->layer_8000_9000, qMakePair(8000,9000) )
    << qMakePair( (QAbstractButton*)ui->layer_9000_10000, qMakePair(9000,10000) )
    << qMakePair( (QAbstractButton*)ui->layer_10000_11000, qMakePair(10000,11000) );
  return map;
}

RadarMap::RadarMap( MapWindow* w )
  : MapWidget(w),
  ui_(new Ui::RadarMap),
  menu_(new QMenu(this) ),
  radartypes_( WeatherLoader::instance()->radarlibrary() )
{
  ui_->setupUi(this);
  ui_->dateEdit->setDisplayFormat(meteo::dtHumanFormatDateOnly);
  kLayers = initLayers(ui_);
  QMapIterator<QString, proto::RadarColor> it(radartypes_);
  while ( true == it.hasNext() ) {
    it.next();
    ui_->typecmb->addItem( it.key() );
  }
  ui_->radartree->setContextMenuPolicy(Qt::CustomContextMenu);
  ui_->radartree->setSortingEnabled(true);
  QObject::connect( ui_->typecmb, SIGNAL( currentIndexChanged( const QString& ) ), this, SLOT( slotCurrentCmbChanged( const QString& ) ) );
  QObject::connect( ui_->layergroup, SIGNAL( buttonClicked( QAbstractButton* ) ), this, SLOT( slotCurrentLayerChanged( QAbstractButton* ) ) );
  QObject::connect( ui_->dateEdit, SIGNAL( dateChanged( const QDate& ) ), this, SLOT( slotDateChanged( const QDate& ) ) );
  QObject::connect( ui_->btnadd, SIGNAL( clicked() ), this, SLOT( slotAddRadarBtn() ) );
  QObject::connect( ui_->btndel, SIGNAL( clicked() ), this, SLOT( slotDelRadarBtn() ) );
  QObject::connect( ui_->btnclose, SIGNAL( clicked() ), this, SLOT( slotCloseBtn() ) );
  QObject::connect( ui_->radartree, SIGNAL( itemSelectionChanged() ), this, SLOT( slotSelectionChanged() ) );
  QObject::connect( ui_->radartree, SIGNAL(customContextMenuRequested( const QPoint& ) ), this, SLOT(slotContextMenu( const QPoint& ) ) );
  QObject::connect( ui_->radartree, SIGNAL( itemDoubleClicked(QTreeWidgetItem* , int) ), this, SLOT( slotDoubleClick() ) );

  ui_->dateEdit->setDate( QDateTime::currentDateTime().date() );
  if ( nullptr != mapdocument() ) {
    if ( nullptr != mapdocument()->eventHandler() ){
      mapdocument()->eventHandler()->installEventFilter(this);
    }
  }
  QAction* add = new QAction(QIcon(":/meteo/icons/tools/plus.png"),"Добавить слой", menu_);
  QObject::connect( add, SIGNAL(triggered()), SLOT(slotAddRadarBtn()));
  add->setIconVisibleInMenu(true);
  menu_->addAction(add);

  QAction* del = new QAction(QIcon(":/meteo/icons/tools/minus.png"),"Удалить слой", menu_);
  QObject::connect( del, SIGNAL(triggered()), SLOT(slotDelRadarBtn()));
  del->setIconVisibleInMenu(true);
  menu_->addAction(del);
}

RadarMap::~RadarMap()
{
  delete menu_; menu_ = nullptr;
  delete ui_; ui_ = nullptr;
}

void RadarMap::setCurrentMap( const QString& title )
{
  if ( false == radartypes_.contains(title) ) {
    error_log.msgBox() << QObject::tr("Не найден тип радарных карт %1")
      .arg(title);
    return;
  }
  current_ = radartypes_[title];
  if ( ui_->typecmb->currentText() != title ) {
    int indx = currentIndex(title);
    if ( -1 == indx ) {
      error_log << QObject::tr("Не найден элемент списка карт %1")
        .arg(title);
      return;
    }
    ui_->typecmb->setCurrentIndex( indx );
  }
  loadAvailableMaps();
}

bool RadarMap::eventFilter(QObject *watched, QEvent *event)
{
  if ( meteo::map::LayerEvent::LayerChanged == event->type() ) {
    LayerEvent* e = reinterpret_cast<LayerEvent*>(event);
    if ( LayerEvent::Added == e->changeType() ||  LayerEvent::Deleted == e->changeType() ) {
      updateItems();
    }
  }
  return MapWidget::eventFilter(watched, event);
}

QPair< int, int > RadarMap::currentLayer() const
{
  QPair<int,int> lvl(-1,-1);
  if ( true == kLayers.contains( ui_->layergroup->checkedButton() ) ) {
    lvl = kLayers[ui_->layergroup->checkedButton()];
  }
  else {
    warning_log.msgBox() << QObject::tr("Не выбран слой");
  }
  return lvl;
}

int RadarMap::currentIndex( const QString& title ) const
{
  QString t(title);
  if ( true == t.isEmpty() ) {
    t = ui_->typecmb->currentText();
  }
  return ui_->typecmb->findText(t);
}

QDateTime RadarMap::currentDateTime() const
{
  QDateTime dt;
  QTreeWidgetItem* item = currentItem();
  if ( 0 == item ) {
    return dt;
  }
  dt = QDateTime( ui_->dateEdit->date(), QTime::fromString( item->text(kClmnTime),"hh:mm:ss" ) );
  return dt;
}

QTreeWidgetItem* RadarMap::currentItem() const
{
  QList<QTreeWidgetItem*> list = ui_->radartree->selectedItems();
  if ( 0 == list.size() ) {
    return 0;
  }
  return list[0];
}

void RadarMap::loadAvailableMaps()
{
  QPair<int,int> layer = currentLayer();
  if ( -1 == layer.first || -1 == layer.second ) {
    error_log.msgBox() << QObject::tr("Неверно указан слой!");
    return;
  }

  QDateTime dtbeg(ui_->dateEdit->date(), QTime(0,0,0) );
  QDateTime dtend(ui_->dateEdit->date(), QTime(23,59,59) );
  surf::DataRequest request;
  request.add_type( surf::kRadarMapType );
  request.set_date_start( dtbeg.toString(Qt::ISODate ).toStdString() );
  request.set_date_end( dtend.toString(Qt::ISODate ).toStdString() );
  request.add_meteo_descr( current_.descr() );
  request.set_level_h( layer.first );
  request.set_level_h2( layer.second );

  surf::DataDescResponse* response ;
  {
      meteo::rpc::Channel* ctrl = global::serviceChannel( meteo::settings::proto::kSrcData );
      qApp->setOverrideCursor( Qt::WaitCursor );

      if ( nullptr == ctrl ) {
        error_log.msgBox() << QObject::tr("Не удалось подключиться к сервису данных");
        qApp->setOverrideCursor(Qt::ArrowCursor);
        return;
      }
      response = ctrl->remoteCall(&surf::SurfaceService::GetAvailableRadar, request, 50000);
      qApp->restoreOverrideCursor();
      delete ctrl;
  }
  if ( 0 == response ) {
    error_log.msgBox() << QObject::tr("Запрос количества данных не выполонен");
    return;
  }
  if ( false == response->result() ) {
    error_log << QObject::tr("Ошибка в ответе о количестве данных %1")
      .arg( QString::fromStdString( response->comment() ) );
  }
  else {
    loadResponseTree(response);
  }
  delete response;
}

void RadarMap::loadResponseTree( surf::DataDescResponse* response )
{
  ui_->radartree->clear();
  proto::WeatherLayer info;
  info.set_type(kLayerMrl);
  info.set_mode(proto::kRadar);
  info.set_source(proto::kSurface);
  info.set_center( global::kCenterWeather );
  info.set_model( global::kModelSurface );
  info.set_template_name(current_.name());
  info.set_layer_name_pattern( LayerMrl::layerNamePattern().toStdString() );
  //info.set_data_type(surf::kRadarMapType);
  info.add_data_type(surf::kRadarMapType);
  info.set_center_name( global::kCenterWeatherName.toStdString() );
  QPair<int,int> cl = currentLayer();
  info.set_h1(cl.first);
  info.set_h2(cl.second);
  info.set_hour(0);
  for ( int i = 0, sz = response->descr_size(); i < sz; ++i ) {
    const surf::DataDesc& desc = response->descr(i);
    QDateTime dt = PsqlQuery::datetimeFromString( desc.date() );
    if ( false == dt.isValid() ) {
      error_log << QObject::tr("Неверный формат даты %1")
        .arg( QString::fromStdString( desc.date() ) );
      continue;
    }
    proto::WeatherLayer k;
    k.CopyFrom(info);
    k.set_datetime( dt.toString(Qt::ISODate).toStdString() );
    QTreeWidgetItem* item = new QTreeWidgetItem( ui_->radartree );
    item->setText( kClmnTime, dt.toString("hh:mm:ss") );
    item->setText( kClmnCount, QString::number( desc.count() ) );
    QByteArray arr;
    global::protomsg2arr( k, &arr );
    item->setData( kClmnTime, Qt::UserRole, arr );
  }
  ui_->radartree->sortItems(kClmnTime, Qt::AscendingOrder);
  updateItems();
}

void RadarMap::loadRadar()
{
  QTreeWidgetItem* item = currentItem();
  if ( 0 == item ) {
    error_log.msgBox() << QObject::tr("Не выбран срок");
    return;
  }
  if ( 0 == mapdocument() ) {
    error_log.msgBox() << QObject::tr("Документ не установлен");
    return;
  }
  proto::WeatherLayer info;
  QByteArray arr = item->data( kClmnTime, Qt::UserRole ).toByteArray();
  global::arr2protomsg( arr, &info );
  Weather weather;
  qApp->setOverrideCursor( Qt::WaitCursor );
  Layer* l = weather.buildMrlLayer( mapdocument(), info );
  if ( 0 == l ) {
    error_log.msgBox() << QObject::tr("Не удалось создать слой с радиолокационными данными");
  }
  qApp->restoreOverrideCursor();
}

bool RadarMap::hasLayer(QTreeWidgetItem* item)
{
  if ( 0 == mapdocument() ) {
    return false;
  }
  if( 0 == item ){
    return false;
  }
  proto::WeatherLayer info;
  QByteArray arr = item->data( kClmnTime, Qt::UserRole ).toByteArray();
  global::arr2protomsg( arr, &info );
  QList<meteo::map::Layer*> list = mapdocument()->layers();
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    Layer* l = list[i];
    if ( true == l->isEqual(info) ) {
      return true;
    }
  }
  return false;
}

void RadarMap::updateItems()
{
  if( 0 == ui_){
    return;
  }
  if ( true == hasLayer( currentItem() ) ){
    ui_->btnadd->setDisabled(true);
    ui_->btndel->setDisabled(false);
  }
  else{
    ui_->btnadd->setDisabled(false);
    ui_->btndel->setDisabled(true);
  }
  QTreeWidgetItem* item = 0;
  for( int i = 0, sz = ui_->radartree->topLevelItemCount(); i < sz; ++i ){
    item = ui_->radartree->topLevelItem(i);
    if( 0 == item ){
      continue;
    }
    if( hasLayer(item) ){
      for ( int j = 0, jsz = ui_->radartree->columnCount(); j < jsz; ++j ){
        item->setBackgroundColor(j, Qt::yellow);
      }
    }
    else{
      for ( int j = 0, jsz = ui_->radartree->columnCount(); j < jsz; ++j ){
        item->setBackgroundColor(j, Qt::white);
      }
    }
  }
}

Layer* RadarMap::layerByItem(QTreeWidgetItem* item)
{
  if( 0 == mapdocument() ){
    return 0;
  }
  if( 0 == item ){
    return 0;
  }
  proto::WeatherLayer info;
  QByteArray arr = item->data( kClmnTime, Qt::UserRole ).toByteArray();
  global::arr2protomsg( arr, &info );
  QList<Layer*> list = mapdocument()->layers();
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    Layer* l = list[i];
    if ( true == l->isEqual(info) ) {
      return l;
    }
  }
  return 0;
}

void RadarMap::slotCurrentCmbChanged( const QString& title )
{
  if ( QString::fromStdString( current_.name() ) != title ) {
    setCurrentMap(title);
  }
}

void RadarMap::slotCurrentLayerChanged( QAbstractButton* btn )
{
  Q_UNUSED(btn);
  loadAvailableMaps();
}

void RadarMap::slotDateChanged( const QDate& date )
{
  Q_UNUSED(date);
  loadAvailableMaps();
}

void RadarMap::slotSelectionChanged()
{
  updateItems();
}

void RadarMap::slotAddRadarBtn()
{
  loadRadar();
}

void RadarMap::slotDelRadarBtn()
{
  Layer* l = layerByItem( currentItem() );
  if( 0 == l ){
    return;
  }
  delete l;
}

void RadarMap::slotCloseBtn()
{
  QWidget::close();
}

void RadarMap::slotDoubleClick()
{
  if ( hasLayer( currentItem() )) {
    slotDelRadarBtn();
  }
  else {
    slotAddRadarBtn();
  }
  updateItems();
}

void RadarMap::slotContextMenu( const QPoint& pnt )
{
  QAction* add = menu_->actions()[0];
  QAction* del = menu_->actions()[1];
  QTreeWidgetItem* item = currentItem();
  if( 0 == item || 0 == del || 0 == add ){
    return;
  }
  if( hasLayer(item) ){
    add->setDisabled(true);
    del->setDisabled(false);
  }
  else{
    add->setDisabled(false);
    del->setDisabled(true);
  }
  menu_->exec(ui_->radartree->mapToGlobal(pnt));
}

}
}
