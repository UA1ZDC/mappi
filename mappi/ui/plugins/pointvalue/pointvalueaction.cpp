#include "pointvalueaction.h"

#include <qcursor.h>
#include <qevent.h>
#include <qtoolbutton.h>
#include <qgraphicsitem.h>
#include <qcryptographichash.h>

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/graphitems/markeritem.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/layeritems.h>
#include <meteo/commons/ui/map/ramka.h>
#include <meteo/commons/ui/map/view/actionbutton.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/menu.h>
#include <meteo/commons/ui/map/view/widgetitem.h>

#include <mappi/ui/satelliteimage/satelliteimage.h>
#include <mappi/ui/satelliteimage/georastr.h>
#include <mappi/ui/satlayer/satlayer.h>

#include "balloonitem.h"

namespace meteo {
namespace map {

const QString PointValueAction::kName = "pointvalueaction";

PointValueAction::PointValueAction(MapScene* scene)
  : Action(scene, PointValueAction::kName)
{
  QToolButton* close = new QToolButton;
  close->setMaximumSize(QSize(16,16));
  close->setMinimumSize(QSize(16,16));
  close->setIcon(QIcon(":/meteo/icons/map/close.png"));
  close->setIconSize(QSize(8,8));
  closeBtnItem_ = new map::WidgetItem(close);
  connect(close, &QToolButton::clicked, this, &PointValueAction::slotDeleteTriggered);

  deleteAct_ = new QAction(tr("Удалить"), this);
  connect(deleteAct_, &QAction::triggered, this, &PointValueAction::slotDeleteTriggered);

  auto path = QList<QPair<QString, QString>>({QPair<QString, QString>("view", QObject::tr("Вид"))});
  auto title = QPair<QString, QString>(kName, QObject::tr("Значение под курсором"));
  
  menuAct_ = scene_->mapview()->window()->addActionToMenu(title, path);
  if ( nullptr == menuAct_ ) {
    menuAct_ = new QAction(tr("Значение под курсором"), this);
  }
  menuAct_->setCheckable(true);
  connect(menuAct_, &QAction::toggled, this, &PointValueAction::slotActionToggled);

  map::Document* doc = scene_->document();
  if ( !doc->isStub() ) {
    doc->turnEvents();
    if ( doc->eventHandler() != 0 ) {
//      doc->eventHandler()->installEventFilter(this);
      connect( doc->eventHandler(), &EventHandler::layerChanged, this, &PointValueAction::slotLayerChanged );
    }
  }
}

void PointValueAction::mouseMoveEvent(QMouseEvent* e)
{
  if ( 0 == scene_ ) { return; }

  // при наведении указателя мыши на подпись добавляем кнопку "закрыть"
  {
    QPoint pos = view()->mapToScene(e->pos()).toPoint();

    QGraphicsItem* item = scene_->itemAt(pos, view()->transform());
    BalloonItem* b = qgraphicsitem_cast<BalloonItem*>(item);
    if ( 0 != b && b != balloon_ ) {
      if ( item != closeBtnItem_->parentItem() || (0 == closeBtnItem_->parentItem() && 0 == closeBtnItem_->scene()) ) {
        closeBtnItem_->setParentItem(b);
        deleteAct_->setData(QVariant::fromValue(b));
      }
      QPoint p = b->boundingRect().topRight().toPoint();
      p.rx() -= closeBtnItem_->boundingRect().width() + 3;
      p.ry() += 3;
      closeBtnItem_->setPos(p);
    }

    // когда указатель покидает пределы подписи, скрываем кнопку "закрыть"
    QGraphicsItem* parent = closeBtnItem_->parentItem();
    if ( 0 != parent ) {
      QRectF r = parent->boundingRect();
      QPointF p = parent->mapToScene(r.topLeft());
      r.moveTopLeft(p);
      if ( !r.contains(pos) ) {
        closeBtnItem_->setParentItem(0);
        scene_->removeItem(closeBtnItem_);
        deleteAct_->setData(QVariant());
      }
    }
  }

  if ( !isActive() ) { return; }

  QPoint pos = view()->mapToScene(e->pos()).toPoint();
  GeoPoint cursorGp = scene_->screen2coord(pos);

  if ( nullptr == balloon_ ) {
    balloon_ = new BalloonItem;
    balloon_->setZValue(1000);
    balloon_->setOffset(15,100);
    balloon_->setBorder(1);
    balloon_->setBorderColor(QColor(220,220,220));
    balloon_->setTextColor(QColor(20,20,20));
    balloon_->setBackgroundColor(Qt::white);
    view()->mapscene()->addItem(balloon_);
  }

  balloon_->clearTexts();

  bool hasValue = false;
  foreach ( GeoRastr* sat, imageObjects() ) {
    QStringList list = imageTexts(sat, cursorGp);
    if ( list.isEmpty() ) {
      continue;
    }

    balloon_->addText(list.at(0) + "  " + list.at(1) + "   ", list.at(2));
    hasValue = true;
  }

  if ( !hasValue ) {
    balloon_->addText(tr("Нет данных"), "");
  }
  balloon_->setPos(view()->mapToScene(e->pos()));
  balloon_->setVisible(true);
}

void PointValueAction::mousePressEvent(QMouseEvent* e)
{
  if ( !moveDistance_.isNull() ) { moveDistance_.setP1(e->pos()); } else { moveDistance_ = QLine(e->pos(), e->pos()); }
}

void PointValueAction::mouseReleaseEvent(QMouseEvent* e)
{
  if ( 0 == scene_ ) { return; }

  moveDistance_.setP2(e->pos());

  // удаляем элемент "подпись" если под курсором мыши была кнопка "закрыть"
  if ( closeBtnItem_->isUnderMouse() ) {
    slotDeleteTriggered();
    return;
  }

  updateCloseBtnPos();

  if ( moveDistance_.length() > 5) { return; }
  if ( !isActive() ) { return; }
  if ( isWidgetUnderMouse(e->pos()) ) { return; }

  if ( nullptr != balloon_ ) {
    scene_->document()->addItem(balloon_);
    balloon_->setLayer(scene_->document()->itemsLayer());
    balloon_->setFlag(QGraphicsItem::ItemStacksBehindParent);

    GeoPoint gp = balloon_->geoPos();
    balloon_->setPos(0,0);
    balloon_->setGeoPos(gp);
  }

  balloon_ = nullptr;
}

void PointValueAction::wheelEvent(QWheelEvent* e)
{
  Q_UNUSED( e );

  updateCloseBtnPos();
}

void PointValueAction::addActionsToMenu(Menu* menu) const
{
  if ( 0 == menu || 0 == menuAct_ ) { return; }

  menu->addLayerAction(menuAct_);

  if ( 0 == scene_ ) { return; }

  QPoint pos = view()->mapFromGlobal(QCursor::pos());
  pos = view()->mapToScene(pos).toPoint();

  BalloonItem* item = qgraphicsitem_cast<BalloonItem*>(scene_->itemAt(pos, view()->transform()));
  if ( 0 != item && item != balloon_ ) {
    menu->addLayerAction(deleteAct_);
    deleteAct_->setData(QVariant::fromValue(item));
  }
}

void PointValueAction::deactivate()
{
  if ( 0 == menuAct_ ) { return; }

  menuAct_->setChecked(false);
}

bool PointValueAction::isActive() const
{
  return 0 != menuAct_ && menuAct_->isChecked();
}

bool PointValueAction::isWidgetUnderMouse(const QPointF& screenPos) const
{
  QPointF scenePos = view()->mapToScene(screenPos.toPoint());
  QList<QGraphicsItem*> items = scene_->items(scenePos);

  for ( int i=0,isz=items.size(); i<isz; ++i ) {
    if ( 0 != qgraphicsitem_cast<QGraphicsProxyWidget*>(items.at(i)) ) {
      return true;
    }
  }

  return false;
}

QList<GeoRastr*> PointValueAction::imageObjects() const
{
  if ( layers_.isEmpty() ) { return QList<GeoRastr*>(); }

  QList<GeoRastr*> list;
  foreach ( Layer* layer, layers_ ) {
    SatLayer* l = maplayer_cast<SatLayer*>(layer);
    if ( nullptr == l || !l->visible() ) {
      continue;
    }
    GeoRastr* sat = mapobject_cast<GeoRastr*>(l->currentObject());
    if ( nullptr == sat ) {
      continue;
    }
    list << sat;
  }
  return list;
}

QStringList PointValueAction::imageTexts(const GeoRastr* o, const GeoPoint& pos) const
{
  if ( nullptr == o || !o->visible() ) { return QStringList(); }

  QStringList list;

  QDateTime dt = QDateTime::fromString(QString::fromStdString(o->protoData().date_start()), Qt::ISODate);
  list << dt.toLocalTime().toString("hh:mm dd.MM") +" " + QString::fromStdString(o->protoData().satellite_name());
  bool ok = false;
  if(o->getPretName().isEmpty()){
    list << "";
  } else {
    list << o->getPretName() +","+o->getPretUnit();
  }

  double v = o->coordsValue(pos, &ok);
  if ( ok ) {
    list << QString::number(v, 'f', 2).rightJustified(12, ' ');
  }
  else {
    list << tr("нет данных");
  }

  return list;
}

void PointValueAction::slotActionToggled(bool toggled)
{
  if ( nullptr == balloon_ ) { return; }

  balloon_->setPos(view()->mapToScene(QCursor::pos()));
  balloon_->setVisible(toggled);
}

void PointValueAction::slotDeleteTriggered()
{
  if ( 0 == scene_ ) { return; }

  BalloonItem* item = qgraphicsitem_cast<BalloonItem*>(closeBtnItem_->parentItem());
  if ( 0 != item ) {
    closeBtnItem_->setParentItem(0);
    scene_->removeItem(closeBtnItem_);
  }

  item = deleteAct_->data().value<BalloonItem*>();
  if ( 0 != item ) { delete item; }

  deleteAct_->setData(QVariant());
}

void PointValueAction::slotLayerChanged(Layer* layer, int ch)
{
  switch ( ch ) {
    case LayerEvent::Added:
      layers_.insert(layer);
      break;
    case LayerEvent::Deleted:
      layers_.remove(layer);
      break;
  }
  updateContent();
}

void PointValueAction::updateCloseBtnPos()
{
  BalloonItem* b = qgraphicsitem_cast<BalloonItem*>(closeBtnItem_->parentItem());
  if ( 0 != b ) {
    QPointF pos = b->boundingRect().topRight();
    pos.rx() -= closeBtnItem_->boundingRect().width() + 3;
    pos.ry() += 3;
    closeBtnItem_->setPos(pos);
  }
}

void PointValueAction::updateContent()
{
  Document* doc = scene_->document();

  if ( 0 == doc ) { return; }

  map::LayerItems* l = doc->itemsLayer();
  if ( nullptr == l ) {
    return;
  }

  foreach ( QGraphicsItem* item, l->items() ) {
    BalloonItem* b = qgraphicsitem_cast<BalloonItem*>(item);
    if ( nullptr == b ) {
      continue;
    }

    bool hasData = false;
    b->clearTexts();
    foreach ( GeoRastr* o, imageObjects() ) {
      QStringList list = imageTexts(o, b->geoPos());
      if ( list.isEmpty() ) {
        continue;
      }

      b->addText(list.at(0) + "  " + list.at(1) + "   ", list.at(2));
      hasData = true;
    }
    if ( !hasData ) {
      b->addText(tr("Нет данных"), "");
    }
  }
}

bool PointValueAction::eventFilter(QObject* obj, QEvent* event)
{
  Q_UNUSED( obj );

  if ( map::LayerEvent::LayerChanged != event->type() ) { return false; }

  Document* doc = scene_->document();
  if ( 0 == doc ) {
    return false;
  }

  LayerEvent* ev = static_cast<map::LayerEvent*>(event);

  SatLayer* l = maplayer_cast<SatLayer*>(doc->layerByUuid(ev->layer()));
  if ( nullptr == l ) {
    return false;
  }

  return false;
}

} // map
} // meteo


