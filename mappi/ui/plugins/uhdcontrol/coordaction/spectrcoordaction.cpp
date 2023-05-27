#include "spectrcoordaction.h"

#include <qcursor.h>
#include <qevent.h>

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/graphitems/markeritem.h>
#include <mappi/ui/plugins/uhdcontrol/layerspectr.h>
#include <mappi/ui/plugins/uhdcontrol/lineitem.h>
#include <mappi/ui/plugins/uhdcontrol/labelitem.h>
#include <meteo/commons/ui/graph/labellayout.h>
#include <mappi/ui/plugins/uhdcontrol/valueaction/spectrvalueaction.h>
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

bool fSortByPos(const meteo::spectr::LabelItem* l1, const meteo::spectr::LabelItem* l2)
{
  return l2->geoPos().lon() < l1->geoPos().lon();
}

namespace meteo {
namespace spectr {

struct MarkerData {
  GeoPoint pos;
  QString  text;
  QColor   color;
};

} // spectr
} // meteo

namespace meteo {
namespace map {

const QString SpectrCoordAction::kName = "graph_coord_action";

SpectrCoordAction::SpectrCoordAction(MapScene* scene)
  : Action(scene, SpectrCoordAction::kName)
{
  dtItem_ = 0;
  lineItem_ = 0;
  closeBtnItem_ = 0;

  QToolButton* close = new QToolButton;
  close->setMaximumSize(QSize(16,16));
  close->setMinimumSize(QSize(16,16));
  close->setIcon(QIcon(":/meteo/icons/map/close.png"));
  close->setIconSize(QSize(8,8));
  closeBtnItem_ = new map::WidgetItem(close);

  deleteAct_ = new QAction(tr("Удалить"), this);
  connect( deleteAct_, SIGNAL(triggered(bool)), SLOT(slotDeleteTriggered()) );

  //TODO в нужное место
  // menuAct_ = scene_->mapview()->window()->addActionToMenuFromFile("graph_coord_action");
  menuAct_ = scene_->mapview()->window()->addActionToMenu({"graph_coord_action", QObject::tr("Значение под курсором")});
  if ( 0 == menuAct_ ) {
    menuAct_ = new QAction(tr("Значение под курсором"), this);
  }
  menuAct_->setCheckable(true);
  connect( menuAct_, SIGNAL(toggled(bool)), SLOT(slotActionToggled(bool)) );
}

SpectrCoordAction::~SpectrCoordAction()
{
}

void SpectrCoordAction::mouseMoveEvent(QMouseEvent* e)
{
  if ( 0 == scene_ ) { return; }

  // при наведении указателя мыши на подпись добавляем кнопку "закрыть"
  {
    QPoint pos = view()->mapFromGlobal(QCursor::pos());
    pos = view()->mapToScene(pos).toPoint();

    QGraphicsItem* item = scene_->itemAt(pos, view()->transform() );
    spectr::LabelItem* label = qgraphicsitem_cast<spectr::LabelItem*>(item);
    if ( 0 != label ) {
      if ( 0 == closeBtnItem_->parentItem() ) {
        if ( 0 == closeBtnItem_->scene() ) {
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
    if ( 0 != parent ) {
      QRectF r = parent->boundingRect();
      QPointF p = parent->mapToScene(r.topLeft());
      r.moveTopLeft(p);
      if ( !r.contains(pos) ) {
        closeBtnItem_->setParentItem(0);
        scene_->removeItem(closeBtnItem_);
      }
    }
  }

  if ( !isActive() ) { return; }

  QPoint cursorPos = view()->mapToScene(e->pos()).toPoint();
  GeoPoint cursorGp = scene_->screen2coord(cursorPos);

  map::Document* doc = scene_->document();
  if ( 0 == doc ) { return; }

  LayerItems* layer = doc->itemsLayer();
  QList<double> existsValues;
  if ( 0 != layer ) {
    foreach ( const QGraphicsItem* i, layer->items() ) {
      const spectr::LineItem* item = qgraphicsitem_cast<const spectr::LineItem*>(i);
      if ( 0 != item && item != lineItem_ ) {
        existsValues << item->geoPos().lat();
      }
    }
  }

 spectr::MarkerData xMarker;
  QList<spectr::MarkerData> yMarkers;

  // ищем значения X и Y в точке пересечения графика с координатой X курсора
  foreach ( map::Layer* layer, doc->layers() ) {
    map::LayerSpectr* l = maplayer_cast<map::LayerSpectr*>(layer);

    if ( 0 == l || !l->visible() ) { continue; }

    QList<GeoPoint> points = l->crossByX(cursorGp.lat());

    AxisSpectr* x = l->xAxis();
    AxisSpectr* y = l->yAxis();
    spectr::MarkerData md;
    foreach ( const GeoPoint& point, points ) {
      md.pos = point;
      md.color = pen2qpen(l->grProperty().pen()).color();

      if ( y->property().visible() ) {
        if ( AxisSpectr::kltNumber == y->tickLabelType() ) {
          md.text = (y->label() + ": " + y->tickLabel(point.lon()));
        }
        else {
          md.text = QDateTime::fromTime_t(point.lon()).toString("hh:mm \"dd\" MMMM yyyy");
        }
        yMarkers << md;
      }

      if ( x->property().visible() ) {
        if ( AxisSpectr::kltNumber == x->tickLabelType() ) {
          md.text = (x->label() + ": " + x->tickLabel(point.lat()));
        }
        else {
          md.text = QDateTime::fromTime_t(point.lat()).toString("hh:mm \"dd\" MMMM yyyy");
        }
        xMarker = md;
      }
    }
  }

  if ( 0 == lineItem_ ) {
    lineItem_ = new spectr::LineItem;
    lineItem_->setZValue(10);
    view()->mapscene()->addItem(lineItem_);
  }

  if ( 0 == dtItem_ ) {
    dtItem_ = new spectr::LabelItem;
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
    QPen pen(meteo::kMAP_ISLAND_COLOR, 3);
    QBrush brush(yMarkers[i].color);
    mark->setStyle(MarkerItem::kNormalStyleRole, pen, brush);
    mark->setStyle(MarkerItem::kHoverStyleRole, pen, brush);
    mark->setStyle(MarkerItem::kSelectedStyleRole, pen, brush);
    markers_ << mark;

    spectr::LabelItem* lbl = 0;
    if ( labels_.size() <= i ) {
      lbl = new spectr::LabelItem;
      lbl->setZValue(18);
      scene_->addItem(lbl);
      labels_ << lbl;
    }
    else {
      lbl = labels_[i];
    }
    lbl->setGeoPos(yMarkers[i].pos);
    lbl->setPos(lbl->calcScenePoint());
    lbl->setText(yMarkers[i].text);
    QColor c = yMarkers[i].color;
    c.setAlpha(220);
    lbl->setBorderColor(c);
  }

  // удалям item'ы , оставшиеся от скрытых и удалённых слоёв
  while ( yMarkers.size() < labels_.size() ) {
    delete labels_.takeLast();
  }

  QRect ramkaRect = doc->ramka()->calcRamka(0,0).boundingRect();

  QList<spectr::LabelItem*> leftSide;
  QList<spectr::LabelItem*> rightSide;
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
  foreach ( spectr::LabelItem* item, leftSide ) {
    layout.addLabel(item->calcScenePoint().y(), item->textRect().height() + 4);
  }
  QVector<double> list = layout.optimPosition();
  for( int i=0,isz=list.size(); i<isz; ++i ) {
    leftSide[i]->setOffset(10, list[i]);
  }

  layout = LabelLayout();
  layout.setRamkaPos(ramkaRect.top(), ramkaRect.bottom());
  list.clear();
  foreach ( spectr::LabelItem* item, rightSide ) {
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
  if ( r.left() < ramkaRect.left() ) {
    dtItem_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  }
  else if ( r.right() > ramkaRect.right() ) {
    dtItem_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  }

  QList<QRect> processed;
  for ( int i=0,isz=labels_.size(); i<isz; ++i ) {
    spectr::LabelItem* lbl = labels_[i];
    lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QRect r = lbl->boundingRect().toRect();
    QPoint p = lbl->calcScenePoint();
    r.setLeft(r.left() + p.x());
    r.setRight(r.right() + p.x());
    r.setTop(r.top() + p.y());
    r.setBottom(r.bottom() + p.y());

    if ( r.left() < ramkaRect.left() ) {
      lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    else if ( r.right() > ramkaRect.right() ) {
      lbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }
  }

  GeoPoint gp = xMarker.pos;
  gp.setLon(cursorGp.lon());
  dtItem_->setGeoPos(gp);
  dtItem_->setText(xMarker.text);
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

void SpectrCoordAction::mousePressEvent(QMouseEvent* e)
{
  if ( !moveDistance_.isNull() ) { moveDistance_.setP1(e->pos()); } else { moveDistance_ = QLine(e->pos(), e->pos()); }
}

void SpectrCoordAction::mouseReleaseEvent(QMouseEvent* e)
{
  if ( 0 == scene_ ) { return; }

  moveDistance_.setP2(e->pos());

  // удаляем элемент "подпись" если под курсором мыши была кнопка "закрыть"
  if ( closeBtnItem_->isUnderMouse() ) {
    QGraphicsItem* parent = closeBtnItem_->parentItem();
    if ( 0 != parent ) {
      closeBtnItem_->setParentItem(0);
      parent->setVisible(false);

    }
    scene_->removeItem(closeBtnItem_);
    return;
  }

  // обновляем (при необходимости) позицию кнопки, на случай если была передвинута карта
  if ( 0 != closeBtnItem_->parentItem() ) {
   spectr::LabelItem* label = qgraphicsitem_cast<spectr::LabelItem*>(closeBtnItem_->parentItem());
    if ( 0 != label ) {
      QPoint pos = label->boundingRect().topRight().toPoint();
      pos.rx() -= closeBtnItem_->boundingRect().width() + 3;
      pos.ry() += 3;
      closeBtnItem_->setPos(pos);
    }
  }

  if ( moveDistance_.length() > 5) { return; }
  if ( !isActive() ) { return; }
  if ( isWidgetUnderMouse(e->pos()) ) { return; }

  if ( 0 != dtItem_ ) {
    scene_->document()->addItem(dtItem_);
    dtItem_->setLayer(scene_->document()->itemsLayer());
    dtItem_->setParentItem(lineItem_);

    GeoPoint gp = dtItem_->geoPos();
    dtItem_->setPos(0,0);
    dtItem_->setGeoPos(gp);
  }
  dtItem_ = 0;

  for ( int i=0,isz=markers_.size(); i<isz; ++i ) {
    MarkerItem* mark = markers_[i];
    mark->setParentItem(lineItem_);
    mark->setLayer(scene_->document()->itemsLayer());
    scene_->document()->addItem(mark);
  }
  markers_.clear();

  for ( int i=0,isz=labels_.size(); i<isz; ++i ) {
   spectr::LabelItem* lbl = labels_[i];
    lbl->setLayer(scene_->document()->itemsLayer());
    lbl->setParentItem(lineItem_);

    GeoPoint gp = lbl->geoPos();
    lbl->setPos(0,0);
    lbl->setGeoPos(gp);

    scene_->document()->addItem(lbl);
  }
  labels_.clear();

  if ( 0 != lineItem_ ) {
    scene_->document()->addItem(lineItem_);
    lineItem_->setZValue(0);
    lineItem_->setLayer(scene_->document()->itemsLayer());
  }
  lineItem_ = 0;
}

void SpectrCoordAction::addActionsToMenu(Menu* menu) const
{
  if ( 0 == menu ) { return; }

  if ( !hasLayerSpectr() ) { return; }

  menu->addLayerAction(menuAct_);

  if ( 0 == scene_ ) { return; }

  QPoint pos = view()->mapFromGlobal(QCursor::pos());
  pos = view()->mapToScene(pos).toPoint();

  QGraphicsItem* item = scene_->itemAt(pos, view()->transform() );

  if ( 0 == item ) { return; }

  while ( item->parentItem() != 0 ) {
    item = item->parentItem();
  }

  spectr::LabelItem* label = 0;

  spectr::LineItem* line = qgraphicsitem_cast<spectr::LineItem*>(item);
  if ( 0 != line ) {
    foreach ( QGraphicsItem* c, line->childItems() ) {
      label = qgraphicsitem_cast<spectr::LabelItem*>(c);

      if ( 0 != label ) { break; }
    }
  }

  if ( 0 == label ) {
    label = qgraphicsitem_cast<spectr::LabelItem*>(item);
  }

  if ( 0 != label ) {
    deleteAct_->setData(QVariant::fromValue(item));
    deleteAct_->setText(tr("Удалить метку за '%1'").arg(label->text()));
    menu->addAction(deleteAct_);
  }
}

void SpectrCoordAction::deactivate()
{
  if ( 0 == menuAct_ ) { return; }

  menuAct_->setChecked(false);
}

bool SpectrCoordAction::isActive() const
{
  if ( 0 != menuAct_ ) {
    return menuAct_->isChecked();
  }
  return false;
}

bool SpectrCoordAction::hasLayerSpectr() const
{
  if ( 0 == scene_ ) { return false; }

  foreach ( map::Layer* l, scene_->document()->layers() ) {
    map::LayerSpectr* lg = maplayer_cast<map::LayerSpectr*>(l);
    if ( 0 != lg ) {
      return true;
    }
  }

  return false;
}

bool SpectrCoordAction::isWidgetUnderMouse(const QPointF& screenPos) const
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

void SpectrCoordAction::removeItems()
{
  delete lineItem_;
  lineItem_ = 0;

  delete dtItem_;
  dtItem_ = 0;

  qDeleteAll(markers_);
  markers_.clear();

  qDeleteAll(labels_);
  labels_.clear();
}

void SpectrCoordAction::slotActionToggled(bool toggled)
{
  if ( false == toggled ) {
    removeItems();
  }
  else {
    SpectrValueAction* a = qobject_cast<SpectrValueAction*>(scene_->getAction(SpectrValueAction::kName));
    if ( 0 != a ) {
      a->deactivate();
    }
  }
}

void SpectrCoordAction::slotDeleteTriggered()
{
  if ( 0 == scene_ ) { return; }

  QGraphicsItem* parent = closeBtnItem_->parentItem();
  if ( 0 != parent ) {
    closeBtnItem_->setParentItem(0);
    scene_->removeItem(closeBtnItem_);
  }

  QGraphicsItem* l = deleteAct_->data().value<QGraphicsItem*>();
  if ( 0 != l ) {
    delete l;
  }
  deleteAct_->setData(QVariant());
}

} // map
} // meteo
