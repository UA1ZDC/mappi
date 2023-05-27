#include "advectaction.h"
#include "prepeartransferwidget.h"

#include <cross-commons/debug/tlog.h>
#include <commons/obanal/tfield.h>
#include <commons/geobasis/geopoint.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/geogroup.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/widgetitem.h>
#include <meteo/commons/ui/map/view/menu.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/drawtools/drawtoolsaction.h>

const QString kDrawToolsAct = QString("drawtoolsaction");

namespace meteo {
namespace map {

AdvectAction ::AdvectAction ( MapScene* scene )
  : TraceAction(scene, "advectaction" ),
    dlg_(nullptr)
{
  deactivate();
  setTraceEditable(false);
  if( 0 == traces_.count() ){
      addTrace();
    }
  QObject::connect(this, SIGNAL(currentTraceChange(int)), SLOT(slotCurrentTraceChange(int)));
  QList<Action*> actions = scene->mapview()->actions();
  foreach ( Action* action, actions ) {
      if ( kDrawToolsAct == action->name() ) {
          drawTools_ = qobject_cast<DrawToolsAction*>( action );
          break;
        }
    }
  slotRemoveTraces();
}

AdvectAction ::~AdvectAction (  ){
  delete dlg_;
  dlg_ = nullptr;
}

void AdvectAction::addActionsToMenu( Menu* menu ) const
{
  if(nullptr == menu ) {
      return;
    }
  QMenu* m =  menu->addMenu(QObject::tr("Анализ"));
  if(nullptr != m ){
      m->addSeparator();
      m->addAction(QObject::tr("Перенос частицы"), this, SLOT(slotTransfer()));
      QPoint pnt = view()->mapFromGlobal(QCursor::pos());
      QPoint pos = scene_->mapview()->mapToScene(pnt).toPoint();
      QGraphicsItem* itemUnderMouse = scene_->itemAt(pos, view()->transform() );
      TraceItem* item = nullptr;
      if( nullptr != itemUnderMouse ){
          if( itemUnderMouse->type() == TraceItem::Type ){
              item = qgraphicsitem_cast<TraceItem*>(itemUnderMouse);
              emit currentTraceChange(item->data(0).toInt());
              m->addAction(QObject::tr("Удалить траекторию"), this, SLOT(slotRemoveTrace()));
            }
        }
      if ( nullptr != drawTools_ ) {
          objUUIDList_.clear();
          objUUIDList_ = drawTools_->getSelectedObjects();
          QString uuid = drawTools_->getHighLightObject();
          if ( false == uuid.isEmpty()) {
              objUUIDList_.append(uuid);
            }
          if ( false == objUUIDList_.isEmpty() ) {
              m->addAction(QObject::tr("Перенос объектов"), this, SLOT(slotTransferObjects()));
            }
        }
      if ( false == traces_.isEmpty() ) {
          m->addAction(QObject::tr("Удалить все траектории"), this, SLOT(slotRemoveTraces()));
        }
      if ( false == traces_.isEmpty() ) {
          m->addAction(QObject::tr("Скрыть все метки"), this, SLOT(slotHideTraces()));
        }
      if ( false == traces_.isEmpty() ) {
          m->addAction(QObject::tr("Показать все метки"), this, SLOT(slotShowTraces()));
        }
      if ( false == traces_.isEmpty() ) {
          m->addAction(QObject::tr("Скрыть все стрелки"), this, SLOT(slotHideArrows()));
        }
      if ( false == traces_.isEmpty() ) {
          m->addAction(QObject::tr("Показать все стрелки"), this, SLOT(slotShowArrows()));
        }
      menu->addAnalyseAction(m->menuAction());
    }
}

void AdvectAction::mouseReleaseEvent(QMouseEvent* e)
{
  TraceAction::mouseReleaseEvent(e);
}

void AdvectAction::mousePressEvent(QMouseEvent* e)
{
  if( nullptr == scene_->mapview() ){
      TraceAction::mousePressEvent(e);
      return;
    }
  point_ = scene_->mapview()->mapToScene( e->pos() ).toPoint();
  TraceAction::mousePressEvent(e);
}

void AdvectAction::slotTryTransfer()
{
  if(nullptr == dlg_) {
      return;
    }
  meteo::field::AdvectObjectReply* reply = dlg_->advectObjectReply();
  bool napr = (1 == dlg_->trajectory())? true:false;
  bool markers = dlg_->markersIsVisible();
  if(nullptr == reply ||
     1 > reply->skelets_size() ||
     1 > reply->skelets(0).dots_size()) {
      return;
    }
  addTrace();
  const meteo::field::Skelet_Dot &dot = reply->skelets(0).dots(0);
  int kol_koord = dot.coord_size();
  QDateTime dt;
  for(int i= 0; kol_koord > i; ++i)  {
      const meteo::surf::Point &p = dot.coord(i);
      meteo::GeoPoint agp(p.fi(),p.la(),p.height());
      TraceItem* item = addNode(agp, !napr, TraceItem::HideNodeAndMark);
      item->setFlag(QGraphicsItem::ItemIsMovable, false);
      item->setFlag(QGraphicsItem::ItemIsSelectable, false);
      dt = dt.fromString(QString::fromStdString(p.date_time()),Qt::ISODate);
      item->setShowDistance(false);
      item->updateText(dt.toString("yyyy-MM-dd HH:mm"));
      if( dlg_->getStartGeoPoint() == agp ) {
          item->setBrush(Qt::green);
        }
      if( false == markers ) {
          item->setMarkVisible(false);
        }
    }
}

void AdvectAction::slotTryTransferObjects()
{
  if ( nullptr == dlg_ || nullptr == drawTools_ ) {
      error_log << QObject::tr("Ошибка. Невозможно получить данные для переноса");
      return;
    }
  Layer* layer = drawTools_->getLayer();
  if ( nullptr == layer ) {
      error_log << QObject::tr("Ошибка. Невозможно получить слой для переноса");
      return;
    }
  QMap< QString, QList<meteo::field::Skelet_Dot > > skeletGroupMap;
  meteo::field::AdvectObjectReply* reply = dlg_->advectObjectReply();
  if (nullptr == reply || (reply->skelets_size() == 0)||
      (reply->skelets(0).dots_size() == 0)) {
      error_log << QObject::tr("Ошибка. Невозможно получить кординаты для переноса");
      return;
    }
  int countOfSkelet = 0, sizeOfSkelet = reply->skelets_size();//reply->skelet().dots_size();
  for (; countOfSkelet < sizeOfSkelet; ++countOfSkelet ) {
      const meteo::field::Skelet &skelet = reply->skelets(countOfSkelet);
      transferOneObject(skelet);
    }
}

void AdvectAction::transferOneObject(const meteo::field::Skelet &skelet)
{
  Layer* layer = drawTools_->getLayer();
  if ( nullptr == layer ) {
      error_log << QObject::tr("Ошибка. Невозможно получить слой для переноса");
      return;
    }

  if (false == skelet.has_uuid()) {
      error_log << QObject::tr("Ошибка. У скелета не установлен uuid объекта.");
      return;
    }
  if ( skelet.dots_size() == 0 || skelet.dots(0).coord_size() == 0) {
      error_log << QObject::tr("Ошибка. Нет кординат для переноса");
      return;
    }
  QString uuid = QString::fromStdString(skelet.uuid());

  Object* object = layer->objectByUuid(uuid);
  if ( nullptr == object ) {
      return;
    }
  bool napr = (1 == dlg_->trajectory())? true:false;
  bool markers = dlg_->markersIsVisible();

  addTrace();
  GeoVector gskelet;
  QDateTime dt;
  int sizeOfDots = skelet.dots_size();
  if( sizeOfDots < 1 ) return;
  int sizeOfPoint = skelet.dots(0).coord_size();
  int countOfPoint = 0;
  for (; countOfPoint < sizeOfPoint; ++countOfPoint) {
      int countOfDots = 0;
      gskelet.clear();
    for (; countOfDots < sizeOfDots; ++countOfDots) {
        const meteo::field::Skelet_Dot& skeletDot = skelet.dots(countOfDots);
        sizeOfPoint = skeletDot.coord_size();
        if(countOfPoint >= sizeOfPoint) {
            debug_log << "Stranno!!!";
            break;
          }
        GeoPoint gp;
        meteo::surf::Point coord = skeletDot.coord(countOfPoint);
        gp.setFi(coord.fi());
        gp.setLa(coord.la());
        gp.setAlt(coord.height());
        gskelet.append(gp);
        dt = dt.fromString(QString::fromStdString(skelet.dots(countOfDots).coord(countOfPoint).date_time()),Qt::ISODate);
    }
    if(gskelet.size() == 0) {
        debug_log << "Stranno!!!";
        return;
      }
    object->setSkelet(gskelet);
    TraceItem* item = addNode(gskelet.first(), !napr, TraceItem::HideNodeAndMark);
    item->setFlag(QGraphicsItem::ItemIsMovable, false);
    item->setFlag(QGraphicsItem::ItemIsSelectable, false);
    item->setShowDistance(false);
    if( 0 == countOfPoint ) {
        item->setBrush(Qt::green);
      }
    item->updateText(dt.toString("yyyy-MM-dd HH:mm"));
    item->setMarkVisible(markers);
    if(countOfPoint+1 < sizeOfPoint) object =  object->copy(layer);
    }
}

void AdvectAction::slotCurrentTraceChange(int index)
{
  current_ = index;
}

void AdvectAction::slotRemoveTraces()
{
  foreach (int key, traces_.keys()) {
      foreach( TraceItem* item, traces_[key] ){
          if( nullptr != item ){
              item->removeArrows();
              scene_->removeItem(item);
              delete item;
            }
        }
    }
  traces_.clear();
  emit removeTrace();
}

void AdvectAction::slotHideTraces()
{
  foreach (int key, traces_.keys()) {
      foreach( TraceItem* item, traces_[key] ){
          if( nullptr != item ){
              item->hide();
            }
        }
    }
}

void AdvectAction::slotShowTraces()
{
  foreach (int key, traces_.keys()) {
      foreach( TraceItem* item, traces_[key] ){
          if( nullptr != item ){
              item->show();
            }
        }
    }
}

void AdvectAction::slotHideArrows()
{
  foreach (int key, traces_.keys()) {
      foreach( TraceItem* item, traces_[key] ){
          if( 0 != item ){
              item->hideArrows();
            }
        }
    }
}

void AdvectAction::slotShowArrows()
{
  foreach (int key, traces_.keys()) {
      foreach( TraceItem* item, traces_[key] ){
          if( nullptr != item ){
              item->showArrows();
            }
        }
    }
}

void AdvectAction::slotTransfer()
{

  QAction* act = static_cast<QAction*>(sender());
  if( nullptr == act ){
      return;
    }

  if(nullptr != dlg_){
      delete dlg_;
      dlg_ = nullptr;
    }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  dlg_ = new PrepearTransferWidget( PrepearTransferWidget::Mode::kPoint, scene_->mapview());
  QObject::connect( dlg_, SIGNAL(applyTraj()), SLOT(slotTryTransfer()) );
  GeoPoint gp = scene_->screen2coord(point_);
  dlg_->setStartGeoPoint(gp);
  QApplication::restoreOverrideCursor();

  dlg_->show();
}

void AdvectAction::slotTransferObjects()
{
  if (nullptr == drawTools_) {
      error_log << QObject::tr("Невозможно получить слой с объектами");
      return;
    }
  Layer* layer = drawTools_->getLayer();
  if (nullptr == layer) {
      error_log << QObject::tr("Cлой с объектами получен неправильно");
      return;
    }
  geoGroupMap_.clear();
  QMap<QString, GeoVector> skeletMap;
  foreach (QString uuid, objUUIDList_) {
      Object* object = layer->objectByUuid(uuid);
      if (nullptr == object) {
          continue;
        }
      if ( PrimitiveTypes::kGroup == object->type() ) {
          std::unordered_set<Object*> list = object->childs();
          QList<QString> groupList;
          foreach (Object* o, list) {
              skeletMap.insert(o->uuid(),o->skelet());
              groupList.append(o->uuid());
            }
          geoGroupMap_.insert(uuid,groupList);
          continue;
        }
      skeletMap.insert(uuid,object->skelet());
    }
  if(nullptr != dlg_){
      delete dlg_;
      dlg_ = nullptr;
    }
  dlg_ = new PrepearTransferWidget( PrepearTransferWidget::Mode::kObjects, scene_->mapview());
  QObject::connect( dlg_, SIGNAL(applyObjectsTraj()), SLOT(slotTryTransferObjects()) );
  dlg_->setSkeletMap(skeletMap);
  dlg_->setWindowTitle("Перенос объектов");
  dlg_->show();
}

}
}
