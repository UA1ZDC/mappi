#include "graphcoordaction.h"

#include <qcursor.h>
#include <qevent.h>

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/graphitems/markeritem.h>
#include <meteo/commons/ui/map/graph/layergraph.h>
#include <meteo/commons/ui/graph/lineitem.h>
#include <meteo/commons/ui/graph/labelitem.h>
#include <meteo/commons/ui/graph/labellayout.h>
#include <meteo/commons/ui/graph/valueaction/graphvalueaction.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/layeritems.h>
#include <meteo/commons/ui/map/ramka.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/menu.h>
#include <meteo/commons/ui/map/view/widgetitem.h>

bool fSortByPos(const meteo::graph::LabelItem* l1, const meteo::graph::LabelItem* l2)
{
  return l2->geoPos().lon() < l1->geoPos().lon();
}

namespace meteo {
namespace graph {

struct MarkerData {
  GeoPoint pos;
  QString  text;
  QColor   color;
};

} // graph
} // meteo

namespace meteo {
namespace map {

const QString GraphCoordAction::kName = "graph_coord_action";

GraphCoordAction::GraphCoordAction(MapScene* scene, GraphType type)
  : Action(scene, GraphCoordAction::kName),
    dtItem_(nullptr),
    lineItem_(nullptr),
    closeBtnItem_(nullptr),
    type_(type)
{
  QToolButton* close = new QToolButton;
  close->setMaximumSize(QSize(16,16));
  close->setMinimumSize(QSize(16,16));
  close->setIcon(QIcon(":/meteo/icons/map/close.png"));
  close->setIconSize(QSize(8,8));
  closeBtnItem_ = new map::WidgetItem(close);

  deleteAct_ = new QAction(tr("Удалить"), this);
  connect( deleteAct_, SIGNAL(triggered(bool)), SLOT(slotDeleteTriggered()) );

  auto path = QList<QPair<QString, QString>> ({QPair<QString, QString>("view", "Вид")});
  auto title = QPair<QString, QString> ("graph_coord_action", "Значение под курсором (раздельно)");

  menuAct_ = scene_->mapview()->window()->addActionToMenu(title, path);

  if ( nullptr == menuAct_ ) {
    menuAct_ = new QAction(tr("Значение под курсором"), this);
  }
  menuAct_->setCheckable(true);
  connect( menuAct_, SIGNAL(toggled(bool)), SLOT(slotActionToggled(bool)) );
}

GraphCoordAction::~GraphCoordAction()
{
}

void GraphCoordAction::mouseMoveEvent(QMouseEvent* e)
{
  if ( nullptr == scene_ ) { return; }

  // при наведении указателя мыши на подпись добавляем кнопку "закрыть"
  {
    QPoint pos = view()->mapFromGlobal(QCursor::pos());
    pos = view()->mapToScene(pos).toPoint();

    QGraphicsItem* item = scene_->itemAt(pos, view()->transform() );
    graph::LabelItem* label = qgraphicsitem_cast<graph::LabelItem*>(item);
    if ( nullptr != label ) {
      for (int i = 0, sz = labels_.size(); i<sz;++i) {
        if ( label == labels_[i] ){ return; }
      }
      if (label == dtItem_) { return; }
      if ( nullptr == closeBtnItem_->parentItem() ) {
        if ( nullptr == closeBtnItem_->scene() ) {
          closeBtnItem_->setParentItem(label);
        }
      }
      QPoint pos = label->boundingRect().topRight().toPoint();
      pos.rx() -= closeBtnItem_->boundingRect().width() + 3;
      pos.ry() += 3;
      closeBtnItem_->setPos(pos);
    }

    // когда указатель покидает пределы подписи, скрываем кнопку "закрыть"
    QGraphicsItem* parent = closeBtnItem_->parentItem();
    if ( nullptr != parent ) {
      QRectF r = parent->boundingRect();
      QPointF p = parent->mapToScene(r.topLeft());
      r.moveTopLeft(p);
      if ( !r.contains(pos) ) {
        closeBtnItem_->setParentItem(nullptr);
        scene_->removeItem(closeBtnItem_);
      }
    }
  }

  if ( !isActive() ) { return; }

  QPoint cursorPos = view()->mapToScene(e->pos()).toPoint();
  GeoPoint cursorGp = scene_->screen2coord(cursorPos);

  map::Document* doc = scene_->document();
  if ( nullptr == doc ) { return; }

  LayerItems* layer = doc->itemsLayer();
  QList<double> existsValues;
  if ( nullptr != layer ) {
    foreach ( const QGraphicsItem* i, layer->items() ) {
      const graph::LineItem* item = qgraphicsitem_cast<const graph::LineItem*>(i);
      if ( nullptr != item && item != lineItem_ ) {
        existsValues << item->geoPos().lat();
      }
    }
  }

  QList<graph::MarkerData> xMarkers;
  QList<graph::MarkerData> yMarkers;

  // ищем значения X и Y в точке пересечения графика с координатой X курсора
  foreach ( map::Layer* layer, doc->layers() ) {
    map::LayerGraph* l = maplayer_cast<map::LayerGraph*>(layer);

    if ( nullptr == l || !l->visible() ) { continue; }
    QList<GeoPoint> points;
    switch (type_) {
      case GraphType::kGraph:
        points = l->crossByX(cursorGp.lat());
      break;
      case GraphType::kOcean:
        points = l->crossByY(cursorGp.lon());
      break;
      default:
        break;
    }

    AxisGraph* x = l->xAxis();
    AxisGraph* y = l->yAxis();
    graph::MarkerData md;
    for ( const GeoPoint& point : points ) {
      md.pos = point;
      md.color = pen2qpen(l->graphProperty().pen()).color();

      if ( y->property().visible() ) {
        if ( AxisGraph::kltNumber == y->tickLabelType() ) {
          md.text = (y->label() + ": " + y->tickLabel(y->coord2scale(point.lon())));
        }
        else {
          md.text = QDateTime::fromTime_t(y->coord2scale(point.lon())).toString("hh:mm \"dd\" MMMM yyyy");
        }
        yMarkers << md;
      }

      if ( x->property().visible() ) {
        if ( AxisGraph::kltNumber == x->tickLabelType() ) {
          md.text = (x->label() + ": " + x->tickLabel(x->coord2scale(point.lat())));
        }
        else {
          md.text = QDateTime::fromTime_t(x->coord2scale(point.lat())).toString("hh:mm \"dd\" MMMM yyyy");
        }
        xMarkers << md;
      }
    }
  }

  if ( nullptr == lineItem_ ) {
    lineItem_ = new graph::LineItem(nullptr,nullptr,nullptr, graph::LineItem::LineType(type_));
    lineItem_->setZValue(10);
    view()->mapscene()->addItem(lineItem_);
  }

  if ( nullptr == dtItem_ ) {
    dtItem_ = new graph::LabelItem;
    dtItem_->setZValue(18);
    view()->mapscene()->addItem(dtItem_);
    dtItem_->setAlignment(Qt::AlignCenter);
  }

  qDeleteAll(markers_);
  markers_.clear();

  for ( int i=0,isz=yMarkers.size(); i<isz; ++i ) {
    MarkerItem* mark = new MarkerItem;
    scene_->addItem(mark);
    mark->setZValue(15);
    mark->setGeoPos(yMarkers[i].pos);
    QPen pen(kMAP_ISLAND_COLOR, 3);
    QBrush brush(yMarkers[i].color);
    mark->setStyle(MarkerItem::kNormalStyleRole, pen, brush);
    mark->setStyle(MarkerItem::kHoverStyleRole, pen, brush);
    mark->setStyle(MarkerItem::kSelectedStyleRole, pen, brush);
    markers_ << mark;

    graph::LabelItem* lbl = nullptr;
    if ( labels_.size() <= i ) {
      lbl = new graph::LabelItem;
      lbl->setZValue(18);
      scene_->addItem(lbl);
      labels_ << lbl;
    }
    else {
      lbl = labels_[i];
    }
    lbl->setGeoPos(yMarkers[i].pos);
    lbl->setPos(lbl->calcScenePoint());
    switch (type_) {
      case GraphType::kGraph:
        lbl->setText(yMarkers[i].text);
      break;
      case GraphType::kOcean:
        lbl->setText(xMarkers[i].text);
      break;
      default:
        break;
    }
    QColor c = yMarkers[i].color;
    c.setAlpha(220);
    lbl->setBorderColor(c);
  }

  // удалям item'ы , оставшиеся от скрытых и удалённых слоёв
  while ( yMarkers.size() < labels_.size() ) {
    delete labels_.takeLast();
  }

  QRect ramkaRect = doc->ramka()->calcRamka(0,0).boundingRect();

  QList<graph::LabelItem*> leftSide;
  QList<graph::LabelItem*> rightSide;
  for ( int i=0,isz=labels_.size(); i<isz; ++i ) {
    if ( labels_[i]->alignment().testFlag(Qt::AlignLeft) ) {
      leftSide << labels_[i];
    }
    else if ( labels_[i]->alignment().testFlag(Qt::AlignRight) ) {
      rightSide << labels_[i];
    }
    else {
      debug_log << tr("Ошибка: подписи должны располагаться либо слева, либо справа.");
    }
  }

  qSort(leftSide.begin(), leftSide.end(), fSortByPos);
  qSort(rightSide.begin(), rightSide.end(), fSortByPos);

  LabelLayout layout;
  layout.setRamkaPos(ramkaRect.top(), ramkaRect.bottom());
  foreach ( graph::LabelItem* item, leftSide ) {
    layout.addLabel(item->calcScenePoint().y(), item->textRect().height() + 4);
  }
  QVector<double> list = layout.optimPosition();
  for( int i=0,isz=list.size(); i<isz; ++i ) {
    leftSide[i]->setOffset(10, list[i]);
  }

  layout = LabelLayout();
  layout.setRamkaPos(ramkaRect.top(), ramkaRect.bottom());
  list.clear();
  foreach ( graph::LabelItem* item, rightSide ) {
    layout.addLabel(item->calcScenePoint().y(), item->textRect().height() + 4);
  }
  list = layout.optimPosition();
  for( int i=0,isz=list.size(); i<isz; ++i ) {
    rightSide[i]->setOffset(10, list[i]);
  }

  // сдвигаем подпись относительно центра, если она выходит за рамку
  dtItem_->setAlignment(Qt::AlignCenter);
  QRect r = dtItem_->boundingRect().toRect();
  QPoint p = dtItem_->calcScenePoint();
  r.setLeft(r.left() + p.x());
  r.setRight(r.right() + p.x());
  r.setTop(r.top() + p.y());
  r.setBottom(r.bottom() + p.y());

  setAlignment(ramkaRect, r, dtItem_);

  QList<QRect> processed;
  for ( int i=0,isz=labels_.size(); i<isz; ++i ) {
    graph::LabelItem* lbl = labels_[i];
    lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QRect r = lbl->boundingRect().toRect();
    QPoint p = lbl->calcScenePoint();
    r.setLeft(r.left() + p.x());
    r.setRight(r.right() + p.x());
    r.setTop(r.top() + p.y());
    r.setBottom(r.bottom() + p.y());

    setAlignment(ramkaRect, r, lbl);
  }

  GeoPoint gp;
  switch (type_) {
    case GraphType::kGraph:
      if (xMarkers.size()>0) {
        gp = xMarkers[0].pos;
        gp.setLon(cursorGp.lon());
        dtItem_->setGeoPos(gp);
        dtItem_->setText(xMarkers[0].text);
      }
    break;
    case GraphType::kOcean:
      if (yMarkers.size() > 0) {
        gp = yMarkers[0].pos;
        gp.setLat(cursorGp.lat());
        dtItem_->setGeoPos(gp);
        dtItem_->setText(yMarkers[0].text);
      }
    break;
    default:
      break;
  }
  dtItem_->setPos(dtItem_->calcScenePoint() - QPoint(0,25));

  lineItem_->setGeoPos(gp);

  bool show = true;
  foreach ( float v, existsValues ) {
    if ( MnMath::isEqual(v, gp.lat()) ) {
      show = false;
      break;
    }
  }

  if ( !show ) {
    removeItems();
  }
}

void GraphCoordAction::mousePressEvent(QMouseEvent* e)
{
  if ( !moveDistance_.isNull() ) { moveDistance_.setP1(e->pos()); } else { moveDistance_ = QLine(e->pos(), e->pos()); }
}

void GraphCoordAction::mouseReleaseEvent(QMouseEvent* e)
{
  if ( nullptr == scene_ ) { return; }

  moveDistance_.setP2(e->pos());

  // удаляем элемент "подпись" если под курсором мыши была кнопка "закрыть"
  if ( closeBtnItem_->isUnderMouse() ) {
    QGraphicsItem* parent = closeBtnItem_->parentItem();
    if ( nullptr != parent ) {
      closeBtnItem_->setParentItem(nullptr);
      parent->setVisible(false);

    }
    scene_->removeItem(closeBtnItem_);
    return;
  }

  // обновляем (при необходимости) позицию кнопки, на случай если была передвинута карта
  if ( 0 != closeBtnItem_->parentItem() ) {
    graph::LabelItem* label = qgraphicsitem_cast<graph::LabelItem*>(closeBtnItem_->parentItem());
    if ( nullptr != label ) {
      QPoint pos = label->boundingRect().topRight().toPoint();
      pos.rx() -= closeBtnItem_->boundingRect().width() + 3;
      pos.ry() += 3;
      closeBtnItem_->setPos(pos);
    }
  }

  if ( moveDistance_.length() > 5) { return; }
  if ( !isActive() ) { return; }
  if ( isWidgetUnderMouse(e->pos()) ) { return; }

  if ( nullptr != dtItem_ ) {
    scene_->document()->addItem(dtItem_);
    dtItem_->setLayer(scene_->document()->itemsLayer());
    dtItem_->setParentItem(lineItem_);

    GeoPoint gp = dtItem_->geoPos();
    dtItem_->setPos(0,0);
    dtItem_->setGeoPos(gp);
  }
  dtItem_ = nullptr;

  for ( int i=0,isz=markers_.size(); i<isz; ++i ) {
    MarkerItem* mark = markers_[i];
    mark->setParentItem(lineItem_);
    mark->setLayer(scene_->document()->itemsLayer());
    scene_->document()->addItem(mark);
  }
  markers_.clear();

  for ( int i=0,isz=labels_.size(); i<isz; ++i ) {
    graph::LabelItem* lbl = labels_[i];
    lbl->setLayer(scene_->document()->itemsLayer());
    lbl->setParentItem(lineItem_);

    GeoPoint gp = lbl->geoPos();
    lbl->setPos(0,0);
    lbl->setGeoPos(gp);

    scene_->document()->addItem(lbl);
  }
  labels_.clear();

  if ( nullptr != lineItem_ ) {
    scene_->document()->addItem(lineItem_);
    lineItem_->setZValue(0);
    lineItem_->setLayer(scene_->document()->itemsLayer());
  }
  lineItem_ = nullptr;
}

void GraphCoordAction::addActionsToMenu(Menu* menu) const
{
  if ( nullptr == menu ) { return; }

  if ( !hasLayerGraph() ) { return; }

  menu->addLayerAction(menuAct_);

  if ( nullptr == scene_ ) { return; }

  QPoint pos = view()->mapFromGlobal(QCursor::pos());
  pos = view()->mapToScene(pos).toPoint();

  QGraphicsItem* item = scene_->itemAt(pos, view()->transform() );

  if ( nullptr == item ) { return; }

  while ( item->parentItem() != nullptr ) {
    item = item->parentItem();
  }

  graph::LabelItem* label = nullptr;

  graph::LineItem* line = qgraphicsitem_cast<graph::LineItem*>(item);
  if ( nullptr != line ) {
    foreach ( QGraphicsItem* c, line->childItems() ) {
      label = qgraphicsitem_cast<graph::LabelItem*>(c);

      if ( 0 != label ) { break; }
    }
  }

  if ( nullptr == label ) {
    label = qgraphicsitem_cast<graph::LabelItem*>(item);
  }

  if ( nullptr != label ) {
    deleteAct_->setData(QVariant::fromValue(item));
    deleteAct_->setText(tr("Удалить метку за '%1'").arg(label->text()));
    menu->addAction(deleteAct_);
  }
}

void GraphCoordAction::deactivate()
{
  if ( nullptr == menuAct_ ) { return; }

  menuAct_->setChecked(false);
}

bool GraphCoordAction::isActive() const
{
  if ( nullptr != menuAct_ ) {
    return menuAct_->isChecked();
  }
  return false;
}

bool GraphCoordAction::hasLayerGraph() const
{
  if ( nullptr == scene_ ) { return false; }

  foreach ( map::Layer* l, scene_->document()->layers() ) {
    map::LayerGraph* lg = maplayer_cast<map::LayerGraph*>(l);
    if ( nullptr != lg ) {
      return true;
    }
  }

  return false;
}

bool GraphCoordAction::isWidgetUnderMouse(const QPointF& screenPos) const
{
  QPointF scenePos = view()->mapToScene(screenPos.toPoint());
  QList<QGraphicsItem*> items = scene_->items(scenePos);

  for ( int i=0,isz=items.size(); i<isz; ++i ) {
    if ( nullptr != qgraphicsitem_cast<QGraphicsProxyWidget*>(items.at(i)) ) {
      return true;
    }
  }

  return false;
}

void GraphCoordAction::removeItems()
{
  delete lineItem_;
  lineItem_ = nullptr;

  delete dtItem_;
  dtItem_ = nullptr;

  qDeleteAll(markers_);
  markers_.clear();

  qDeleteAll(labels_);
  labels_.clear();
}

void GraphCoordAction::setAlignment(QRect ramkaRect, QRect r, graph::LabelItem* lbl)
{
  switch (type_) {
    case GraphType::kGraph:
      if ( r.left() < ramkaRect.left() ) {
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      }
      else if ( r.right() > ramkaRect.right() ) {
        lbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      }
    break;
    case GraphType::kOcean:
      if (r.bottom() > ramkaRect.bottom()){
        lbl->setAlignment(Qt::AlignTop);
        if ( r.left() < ramkaRect.left() ) {
          lbl->setAlignment(Qt::AlignRight | Qt::AlignTop);
        }
        else if ( r.right() > ramkaRect.right() ) {
          lbl->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        }
      }
      else if ( r.top() < ramkaRect.top()) {
        lbl->setAlignment(Qt::AlignBottom);
        if ( r.left() < ramkaRect.left() ) {
          lbl->setAlignment(Qt::AlignRight | Qt::AlignBottom);
        }
        else if ( r.right() > ramkaRect.right() ) {
          lbl->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
        }
      }
      else if ( r.left() < ramkaRect.left() ) {
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      }
      else if ( r.right() > ramkaRect.right() ) {
        lbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      }
    break;
    default:
      break;
  }
}

void GraphCoordAction::slotActionToggled(bool toggled)
{
  if ( false == toggled ) {
    removeItems();
  }
  else {
    GraphValueAction* a = qobject_cast<GraphValueAction*>(scene_->getAction(GraphValueAction::kName));
    if ( nullptr != a ) {
      a->deactivate();
    }
  }
}

void GraphCoordAction::slotDeleteTriggered()
{
  if ( nullptr == scene_ ) { return; }

  QGraphicsItem* parent = closeBtnItem_->parentItem();
  if ( nullptr != parent ) {
    closeBtnItem_->setParentItem(nullptr);
    scene_->removeItem(closeBtnItem_);
  }

  QGraphicsItem* l = deleteAct_->data().value<QGraphicsItem*>();
  if ( nullptr != l ) {
    delete l;
  }
  deleteAct_->setData(QVariant());
}

} // map
} // meteo
