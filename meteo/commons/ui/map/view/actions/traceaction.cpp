#include "traceaction.h"
#include "labelitem.h"
#include "arrow.h"

#include <qinputdialog.h>

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geopoint.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/event.h>
#include <meteo/commons/ui/map/layeritems.h>

#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/widgetitem.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/global/global.h>
#include <commons/geom/geom.h>

namespace meteo {
namespace map {

namespace {
ArrowItem* arrow_click = nullptr;
TraceItem* after = nullptr;
TraceItem* before = nullptr;
}


TraceAction ::TraceAction (MapScene* scene, const QString& name)
  : Action(scene, name ),
    interactionMode_(InsertItem),
    workmode_(TraceMode),
    shape_(TraceItem::Round),
    press_(false),
    editable_(true)
{  
  if( 0 == traces_.count() ){
    addTrace();
  }

  if(nullptr != scene_){
    scene_->document()->eventHandler()->installEventFilter(this);   
    connect(scene_, &QObject::destroyed, this, &TraceAction::slotSceneToNull);
  }    
}

TraceAction::~TraceAction()
{
  blockSignals(true);
  slotRemoveTrace();
  blockSignals(false);

  removeFromScene();  
}


void TraceAction::installScene( MapScene* sc ) {  
  scene_ = sc;
  if(nullptr != scene_){
    scene_->document()->eventHandler()->installEventFilter(this);   
    connect(scene_, &QObject::destroyed, this, &TraceAction::slotSceneToNull);
  }
}

void TraceAction::slotChangeNode(int num, const GeoPoint& point, const QString& text){
  if ( num >= traces_[current_].size() ) {
    return;
  }
  traces_[current_][num]->setGeoPoint(point);
  traces_[current_][num]->updateText(text);
}

void TraceAction::mousePressEvent(QMouseEvent* e)
{
  press_ = true;
  if (e->button() != Qt::LeftButton){
    return;
  }
  if(nullptr == scene_){return;}

  QGraphicsItem* itemUnderMouse = scene_->itemAt(scene_->mapview()->mapToScene(e->pos()), view()->transform() );
  if( nullptr != itemUnderMouse ){
    if( itemUnderMouse->type() == TraceItem::Type ){
      bool movable = itemUnderMouse->flags() & QGraphicsItem::ItemIsMovable;
      setInteractionMode(movable ? MoveItem : DoNothing);
    }
    if( itemUnderMouse->type() == ArrowItem::Type ){
      return;
    }
  }
}

void TraceAction::mouseReleaseEvent(QMouseEvent* e)
{
  press_ = false;
  if(nullptr == scene_){return;}

  QPointF pnt = scene_->mapview()->mapToScene(e->pos());
  GeoPoint gp = scene_->document()->screen2coord(pnt);
  QGraphicsItem* itemUnderMouse = scene_->itemAt(scene_->mapview()->mapToScene(e->pos()), view()->transform() );
  switch( interactionMode_ ){
    case InsertItem :
      if( nullptr != itemUnderMouse || false == activate_){
        return;
      }
      addNode(gp);
      break;
    case MoveItem :
      if( nullptr != itemUnderMouse ){
        if( itemUnderMouse->type() == TraceItem::Type ){
          TraceItem* item = qgraphicsitem_cast<TraceItem*>(itemUnderMouse);
          item->setGeoPoint(gp);
          emit changeItem(item->number(), gp);
        }
      }
      break;
    default :
      break;
  }
  setInteractionMode(InsertItem);
}

void TraceAction::mouseMoveEvent(QMouseEvent* e)
{
  Q_UNUSED(e)
  if( true == press_ && MoveItem != interactionMode_ ){
    setInteractionMode(MoveMap);
    return;
  }
}

void TraceAction::addActionsToMenu(Menu* menu) const
{
  if(nullptr == menu || false == editable_) {
    return;
  }
  if(nullptr == scene_){return;}

  if( !menu->isEmpty()) {
    menu->addSeparator();
  }

  if( false == traces_.contains(current_) ){
    return;
  }

  QList<TraceItem*> nodes = traces_[current_];

  if( nodes.count() <= 0){
    return;
  }
  QPoint pnt = view()->mapFromGlobal(QCursor::pos());
  QPoint pos = scene_->mapview()->mapToScene(pnt).toPoint();
  QPoint* cross = new QPoint;
  QPoint min_cross;
  int min = -1;
  QMap<int, ArrowItem*> list_dist;
  foreach( TraceItem* item, nodes ){
    QList<ArrowItem*> list = item->arrows();
    foreach( ArrowItem* arrow, list ){
      if( nullptr == arrow ){
        continue;
      }
      int dist = meteo::geom::distance(pos, arrow->line().toLine(), cross);
      if( -1 == min ){
        min = dist;
        min_cross = *cross;
      }
      if( dist < min ){
        min = dist;
        min_cross = *cross;
      }
      if( false == list_dist.keys().contains(min) ){
        list_dist.insert(min, arrow);
      }
    }
  }
  arrow_click = list_dist[min];
  if( nullptr != arrow_click && min < 12 ){

    QAction* act = menu->addAction(QObject::tr("Вставить узел"));
    GeoPoint gp = scene_->document()->screen2coord(min_cross);
    connect( act, SIGNAL(triggered(bool)), SLOT(slotItemInsert()) );
    after = arrow_click->startItem();
    before = arrow_click->endItem();
    act->setProperty("latdeg", gp.latDeg());
    act->setProperty("londeg", gp.lonDeg());
    act->setProperty("pos", min_cross);
  }
  delete cross;
  QGraphicsItem* itemUnderMouse = scene_->itemAt(pos, view()->transform() );
  TraceItem* item = nullptr;
  if( nullptr != itemUnderMouse ){
    if( itemUnderMouse->type() == TraceItem::Type ){
      item = qgraphicsitem_cast<TraceItem *>(itemUnderMouse);
      if( false == item->itemRemovable() ){
        return;
      }
      if( nodes.contains(item) ){
        QAction* act = menu->addAction(QObject::tr("Удалить узел"));
        connect( act, SIGNAL(triggered(bool)), SLOT(slotProcessMenuAction()) );
        act->setProperty("node", item->number());
      }
    }
  }
}

void TraceAction::mouseDoubleClickEvent(QMouseEvent* e)
{
  if(nullptr == scene_){return;}

  QGraphicsItem* itemUnderMouse = scene_->itemAt(scene_->mapview()->mapToScene(e->pos()), view()->transform() );
  if( nullptr != itemUnderMouse  ){
    if( itemUnderMouse->type() == QGraphicsTextItem::Type ){
      QGraphicsTextItem* textitem = qgraphicsitem_cast<QGraphicsTextItem*>(itemUnderMouse);
      ArrowItem* arrow = qgraphicsitem_cast<ArrowItem*>(textitem->parentItem());

      bool arrowOwner = false;
      if( false == traces_.contains(current_) ){
        return;
      }
      for ( int i=0,isz=traces_[current_].size(); i<isz; ++i ) {
        if ( traces_[current_].at(i)->arrows().contains(arrow) ) {
          arrowOwner = true;
          break;
        }
      }

      if ( arrowOwner ) {
        QInputDialog* dlg = new QInputDialog;
        QString text = dlg->getText(nullptr,
                                    tr("Редактирование надписи"),
                                    tr("Введите текст:"),
                                    QLineEdit::Normal,
                                    textitem->toPlainText());
        if ( false == text.isEmpty() ) {
          textitem->setPlainText(text);
        }

        if( nullptr != arrow ){
          arrow->setText(text);
        }
        delete dlg;
      }
    }
  }

  Action::mouseDoubleClickEvent(e);
}

int TraceAction::nodeCount(int num_trace) const{
  if(traces_.size() < (num_trace-1)) return 0;
  return traces_[num_trace].size();
}

TraceItem *TraceAction::addNode(const GeoPoint &gp, bool reverse, TraceItem::CollideItemOption collide)
{
  Q_UNUSED(reverse)
  Q_UNUSED(collide)
  if(nullptr == scene_){return nullptr;}

  if( 0 == traces_.count() ) {
    addTrace();
  }
  if( false == traces_.contains(current_) ){
    return nullptr;
  }
  if ( DotMode == workmode_) {
    if ( 1 <= traces_[current_].count() ) {
      foreach (TraceItem *item, traces_[current_]) {
        int num_item  = item->number();
        deleteItem(item);
        emit itemRemoved(num_item);
      }
    }
  }
  QList<TraceItem*> nodes = traces_[current_];
  TraceItem* last = nullptr;
  if( nodes.count() > 0 ){
    last = nodes.last();
  }
  TraceItem* item = new TraceItem(shape_, gp, last, nullptr, scene_);
  item->setData(0, current_);
  nodes.append(item);
  traces_[current_].append(item);
  item->setBrush(Qt::white);
  item->setPen(QPen(Qt::black, 2));
  if ( nodes.count() > 1 )
  {
    if( false == reverse ){
      createArrow(nodes[nodes.count()-2],nodes.last());
    }else{
      item->setOrientation(TraceItem::Backward);
      createArrow( nodes.last(),nodes[nodes.count()-2]);
    }
  }
  item->setMarkVisible(true);
  item->setCollideOption(collide);
  emit itemAdded(item->number(), gp);
  return item;
}

TraceItem* TraceAction::insertNode(const GeoPoint& gp, TraceItem* after, TraceItem* before, TraceItem::CollideItemOption collide)
{
  if( nullptr == before || nullptr == after || after == before ){
    return nullptr;
  }
  if(nullptr == scene_){return nullptr;}

  TraceItem* item = new TraceItem(shape_, gp, nullptr, nullptr, scene_);
  item->setBrush(Qt::white);
  item->setPen(QPen(Qt::black, 2));
  int idx = before->number();
  item->setPreviousItem(after);
  item->setNextItem(before);
  after->setNextItem(item);
  before->setPreviousItem(item);
  if( false == traces_.contains(current_) ){
    return nullptr;
  }
  traces_[current_].insert(idx, item);
  createArrow(after, item);
  createArrow(item, before);
  item->setMarkVisible(true);
  item->setCollideOption(collide);
  emit itemAdded(item->number(), gp);
  return item;
}

int TraceAction::addTrace()
{
  QList<TraceItem*> track;
  int idx = traces_.count();
  traces_.insert(idx, track);
  current_ = idx;
  return current_;
}

void TraceAction::setInteractionMode(TraceAction::InteractionMode mode)
{
  interactionMode_ = mode;
}

void TraceAction::setWorkMode(WorkMode mode)
{
  workmode_ = mode;
}

ArrowItem* TraceAction::createArrow(TraceItem* itemStart, TraceItem* itemEnd)
{
  if(nullptr == scene_){return nullptr;}
  ArrowItem *arrow = nullptr;
  if( nullptr != itemStart && nullptr != itemEnd ){
    if( itemStart->arrowOut()!= nullptr && itemEnd->arrowIn() !=nullptr){
      arrow = itemStart->arrowOut();
      arrow->setStartItem(itemStart);
      arrow->setEndItem(itemEnd);
      arrow->updatePosition();
      arrow->show();
    }
    else{
      arrow = new ArrowItem(itemStart, itemEnd, nullptr, scene_);
      arrow->setColor(Qt::black);
      itemStart->setArrowOut(arrow);
      itemEnd->setArrowIn(arrow);
      arrow->setZValue(-1000.0);
    }
    arrow->updatePosition();
  }
  return arrow;
}

Layer* TraceAction::activeLayer()
{
  if( nullptr == scene_ ){
    return nullptr;
  }
  if( nullptr == scene_->document() ){
    return nullptr;
  }
  return  scene_->document()->activeLayer();
}

void TraceAction::deleteItem()
{
  if( nullptr == scene_ ){
    return;
  }

  foreach (QGraphicsItem *item, scene_->selectedItems()) {
    if (item->type() == TraceItem::Type) {
      TraceItem * itemcast = qgraphicsitem_cast<TraceItem *>(item);
      int num_item  = itemcast->number();

      deleteItem(itemcast);
      emit itemRemoved(num_item);
    }else{
      scene_->removeItem(item);
      delete item;
    }
  }
}

void TraceAction::slotProcessMenuAction()
{
  QAction* act = qobject_cast<QAction*>(sender());
  if ( nullptr == act ) {
    return;
  }
  if( false == traces_.contains(current_) ){
    return;
  }
  int index = act->property("node").toInt();
  if( traces_[current_].count() >= index ){
    TraceItem* item = traces_[current_][index];
    if( nullptr != item ){
      int num_item  = item->number();
      deleteItem(item);
      emit itemRemoved(num_item);
    }
  }
}

void TraceAction::deleteItem(TraceItem* item)
{
  if( nullptr == item ){
    return;
  }
  if( nullptr == scene_ ){
    return;
  }
  if( false == traces_.contains(current_) ){
    return;
  }
  int index = traces_[current_].indexOf(item);
  if (index != -1){
    traces_[current_].removeAt(index);
  }else{
    return;
  }

  TraceItem* pItem = item->findNearestPreviousPaintedItem();
  TraceItem* nItem = item->findNearestNextPaintedItem();
  item->removeArrows();
  if( nullptr != item->arrowIn()  ){
    scene_->removeItem(item->arrowIn());
    delete item->arrowIn();
    if( nullptr != pItem ){
      pItem->setArrowOut(nullptr);
    }
  }
  if( nullptr != item->arrowOut()  ){
    scene_->removeItem(item->arrowOut());
    delete item->arrowOut();
    if( nullptr != nItem ){
      nItem->setArrowIn(nullptr);
    }
  }
  if( TraceItem::Forward == item->orientation() ){
    createArrow(pItem, nItem);
  }
  if( TraceItem::Backward == item->orientation() ){
    createArrow(nItem, pItem);
  }
  if( nullptr != item->prev() ){
    item->prev()->setNextItem(item->next());
  }
  if( nullptr != item->next() ){
    item->next()->setPreviousItem(item->prev());
  }
  scene_->removeItem(item);
  delete item;
  if( nullptr != pItem ){
    pItem->updateDistance();
  }
}

void TraceAction::slotRemoveTrace()
{  
  if( false == traces_.contains(current_) ){    
    return;
  }
  if( nullptr == scene_ ){
    return;
  }
  foreach( TraceItem* item, traces_[current_] ){
    if( nullptr != item ){
      item->removeArrows();
      scene_->removeItem(item);
      delete item;
      item = nullptr;
    }
  }
  traces_.remove(current_);
  emit removeTrace();
}

void TraceAction::slotItemRemove(int num)
{
  if( false == traces_.contains(current_) ){
    return;
  }
  if( num > traces_[current_].size()-1 || num < 0) {
    return;
  }
  TraceItem* item = traces_[current_][num];
  deleteItem(item);
}

void TraceAction::slotItemAdd(int, const GeoPoint & gp, const QString & text)
{
  TraceItem* item  = addNode(gp);
  if(!text.isEmpty()){
    item->updateText(text);
  }
}

void TraceAction::slotItemInsert()
{
  if( nullptr == scene_ ){
    return;
  }
  QAction* act = qobject_cast<QAction*>(sender());
  if ( nullptr == act ) {
    return;
  }
  double lat = act->property("latdeg").toDouble();
  double lon = act->property("londeg").toDouble();
  GeoPoint gp;
  gp.setLatDeg(lat);
  gp.setLonDeg(lon);
  insertNode(gp, after, before);
  scene_->removeItem(arrow_click);
  delete arrow_click;
  arrow_click = nullptr;
}

void TraceAction::slotItemChange(int num, const GeoPoint & geo, const QString & text)
{
  if( false == traces_.contains(current_) ){
    return;
  }
  if( num > traces_[current_].size()-1 || num < 0) {
    return;
  }
  TraceItem* item = traces_[current_][num];
  if( nullptr != item ){
    item->setGeoPoint(geo);
  }
  if(!text.isEmpty()) {
    item->updateText(text);
  }
}

void TraceAction::setTraceEditable(bool st)
{
  editable_ = st;
}

bool TraceAction::eventFilter( QObject* o, QEvent* e )
{
  if( nullptr == scene_ ){
    return false;
  }
  if ( o == scene_->document()->eventHandler() && LayerEvent::LayerChanged  == e->type() ) {
    LayerEvent* ev = reinterpret_cast<LayerEvent*>(e);
    if ( LayerEvent::Deleted == ev->changeType() ) {
      if ( nullptr == scene_->document()->itemsLayer() || ev->layer() == scene_->document()->itemsLayer()->uuid() ) {
        traces_.clear();
      }
    }
  }
  return false;
}

void TraceAction::slotSceneToNull(QObject*){      
  scene_ = nullptr;  
  traces_.remove(current_);
}

void TraceAction::appendPlace(const GeoPoint& gp, bool move )
{
  this->slotAppendSingleNode(gp, makePlaceName(gp, search_near_), move );
}

QString TraceAction::makePlaceName(const meteo::GeoPoint& point, bool search_near)
{
  sprinf::Station station;
  if ((false == search_near) || (false == meteo::global::loadStationByCoord(point, &station)) ){
    return QObject::tr("( %1 )").arg(point.toString(false));
  }

  QString stName = QString::fromStdString(station.name().rus());
  QString stCoord = point.toString(true);
  QString stIndex = QString::number(station.index());
  if ( true == stName.isEmpty() && true == stIndex.isEmpty() ){
    return QObject::tr("( %1 )").arg(stCoord);
  }
  if ( true == stName.isEmpty() ){
    return  QObject::tr("%1 ( %2 )").arg(stIndex).arg(stCoord);
  }
  if ( true == stIndex.isEmpty() ){
    return  QObject::tr("%1 ( %2 )").arg(stName).arg(stCoord);
  }
  return QObject::tr("%1 ( %2, %3 )").arg(stName)
      .arg(stIndex)
      .arg(stCoord);
}

void TraceAction::slotAppendSingleNode(const GeoPoint& point, const QString& title, bool move)
{
  if(nullptr == scene_){return;}
  if(0 == nodeCount(0)) {
      meteo::map::TraceItem* item  = addNode(point);
      if(nullptr == item) return;
      item->updateText(title);
      item->setFlag(QGraphicsItem::ItemIsMovable, move);
      item->setShowDistance(false);
    } else {
      slotChangeNode(0,  point, title);
    }
}

void TraceAction::removeFromScene(){  
  Action::removeFromScene();
  scene_ = nullptr;
  this->slotSceneToNull(nullptr);
  this->slotRemoveTrace();
}


}
}
