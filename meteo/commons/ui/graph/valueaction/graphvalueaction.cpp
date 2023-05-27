#include "graphvalueaction.h"

#include <qcursor.h>
#include <qevent.h>
#include <qtoolbutton.h>

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/graphitems/markeritem.h>
#include <meteo/commons/ui/map/graph/layergraph.h>
#include <meteo/commons/ui/graph/lineitem.h>
#include <meteo/commons/ui/graph/coordaction/graphcoordaction.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/global/common.h>
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

#include "combolabelitem.h"

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

const QString GraphValueAction::kName = "graph_value_action";

GraphValueAction::GraphValueAction(MapScene* scene, GraphType type)
  : Action(scene, GraphValueAction::kName),
  lineItem_( nullptr ),
  labelItem_( nullptr ),
  closeBtnItem_( nullptr ),
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
  auto title = QPair<QString, QString> ("graph_value_action", "Значение под курсором (вместе)");

  menuAct_ = scene_->mapview()->window()->addActionToMenu(title, path);
  if ( nullptr != menuAct_ ) {
    menuAct_->setCheckable(true);
    connect( menuAct_, SIGNAL(toggled(bool)), SLOT(slotActionToggled(bool)) );
  }
}
GraphValueAction::~GraphValueAction()
{
  if ( nullptr != this->lineItem_ ){
    delete this->lineItem_;
    this->lineItem_ = nullptr;
  }
  if ( nullptr != labelItem_ ){
    delete this->labelItem_;
    this->labelItem_ = nullptr;
  }
  if ( nullptr != this->closeBtnItem_ ){
    delete this->closeBtnItem_;
    this->closeBtnItem_ = nullptr;
  }
}

void GraphValueAction::mouseMoveEvent(QMouseEvent* e)
{
  if ( nullptr == scene_ ) { return; }

  // при наведении указателя мыши на подпись добавляем кнопку "закрыть"
  {
    QPoint pos = view()->mapFromGlobal(QCursor::pos());
    pos = view()->mapToScene(pos).toPoint();

    QGraphicsItem* item = scene_->itemAt(pos, view()->transform() );
    graph::ComboLabelItem* label = qgraphicsitem_cast<graph::ComboLabelItem*>(item);
    if ( nullptr != label ) {
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

  QPoint pos = view()->mapToScene(e->pos()).toPoint();
  GeoPoint cursorGp = scene_->screen2coord(pos);

  map::Document* doc = scene_->document();

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
  QList<graph::MarkerData> puanMarkers;

  // ищем ближайшие точки со значениями
  for( map::Layer* layer : doc->layers() ) {
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

    for ( const GeoPoint& point : points ) {
      graph::MarkerData md;
      md.pos = point;

      AxisGraph* x = l->xAxis();
      AxisGraph* y = l->yAxis();

      if ( y->property().visible() ) {
        if ( AxisGraph::kltNumber == y->tickLabelType() ) {
          md.text = (y->label() + ":" + y->tickLabel(y->coord2scale(point.lon())));
          md.color = pen2qpen(l->graphProperty().pen()).color();
        }
        else {
          md.text = QDateTime::fromTime_t(y->coord2scale(point.lon())).toString("hh:mm \"dd\" MMM yyyy");
          md.color = pen2qpen(l->graphProperty().pen()).color();
        }
        yMarkers << md;
      }

      if ( x->property().visible() ) {
        if ( AxisGraph::kltNumber == x->tickLabelType() ) {
          md.text = (x->label() + ":" + x->tickLabel(x->coord2scale(point.lat())));
          md.color = pen2qpen(l->graphProperty().pen()).color();
        }
        else {
          md.text = QDateTime::fromTime_t(x->coord2scale(point.lat())).toString("hh:mm \"dd\" MMM yyyy");
          md.color = pen2qpen(l->graphProperty().pen()).color();
        }
        xMarkers << md;
      }

      for ( const QString& text : l->puansonLabels(x->coord2scale(point.lat())) ) {
        md.text = text;
        md.color = QColor();
        puanMarkers << md;
      }
    }
  }

  bool show = false;
  QList< QPair<double,graph::MarkerData> > markers;

  QString dtText;

  // определяем минимальное расстояние
  double minDist = std::numeric_limits<double>::max();
  for ( int i=0,isz=xMarkers.size(); i<isz; ++i ) {
    double dist = qAbs(cursorGp.lat() - xMarkers.at(i).pos.lat());

    if ( dist > minDist ) {
      continue;
    }

    dtText = xMarkers[i].text;

    minDist = dist;
  }
  for ( int i=0,isz=yMarkers.size(); i<isz; ++i ) {
    double dist = qAbs(cursorGp.lat() - yMarkers.at(i).pos.lat());

//    if ( dist > minDist ) { continue; }

    markers << qMakePair(dist,yMarkers.at(i));

    minDist = dist;
  }
  for ( int i=0,isz=puanMarkers.size(); i<isz; ++i ) {
    double dist = qAbs(cursorGp.lat() - puanMarkers.at(i).pos.lat());

    if ( dist > minDist ) { continue; }

    markers << qMakePair(dist,puanMarkers.at(i));

    minDist = dist;
  }


  QSet<QString> uniqTexts;
  for ( int i=0,isz=markers.size(); i<isz; ++i ) {
    if ( uniqTexts.contains(markers.at(i).second.text) ) {
      markers.removeAt(i);
      --i;
      --isz;
    }
    uniqTexts.insert(markers.at(i).second.text);
  }

  if ( nullptr == lineItem_ ) {
    lineItem_ = new graph::LineItem( nullptr, nullptr, nullptr, graph::LineItem::LineType(type_) );
    view()->mapscene()->addItem(lineItem_);

    lineItem_->setZValue(10);
  }
  if ( nullptr == labelItem_ ) {
    labelItem_ = new graph::ComboLabelItem;
    view()->mapscene()->addItem(labelItem_);

    labelItem_->setZValue(20);
  }

  labelItem_->clearTexts();

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
  }

  QStringList arrText;
  switch (type_) {
    case GraphType::kGraph:
      labelItem_->setTitle(dtText.mid(6));
      labelItem_->addText(tr("Срок"), ": " + dtText.left(5));
      for ( int i=0,isz=markers.size(); i<isz; ++i ) {
//        if ( !MnMath::isEqual(markers.at(i).first, minDist) ) {
//          continue;
//        }
        QString s = markers.at(i).second.text;
        labelItem_->addText(s.section(":", 0, 0), ": " + s.section(":", 1, 1));
        show = true;
      }
    break;
    case GraphType::kOcean:
      if (0 < yMarkers.size()) {
        arrText = yMarkers[0].text.split(":");
        if ( 1 < arrText.size()) {
          labelItem_->addText(arrText[0], ": " + arrText[1]);
        }
        arrText.clear();
      }
      for (int i=0, sz=xMarkers.size(); i<sz;++i) {
        arrText = xMarkers[i].text.split(":");
        if (1< arrText.size()) {
          labelItem_->addText(arrText[0], ": " + arrText[1]);
        }
        arrText.clear();
        show = true;
      }

    break;
    default:
      break;
  }

  GeoPoint gp = markers.value(0).second.pos;
  if ( std::isnan(gp.lat()) ) { gp.setLat(cursorGp.lat()); }
  if ( std::isnan(gp.lon()) ) { gp.setLon(cursorGp.lon()); }

  foreach ( float v, existsValues ) {
    if ( MnMath::isEqual(v, gp.lat()) ) {
      show = false;
      break;
    }
  }


  // сдвигаем подпись относительно центра, если она выходит за рамку
  QRect ramkaRect = doc->ramka()->calcRamka(0,0).boundingRect();
  labelItem_->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
  QRect r = labelItem_->boundingRect().toRect();
  QPoint p = labelItem_->calcScenePoint();
  r.setLeft(r.left() + p.x());
  r.setRight(r.right() + p.x());
  r.setTop(r.top() + p.y());
  r.setBottom(r.bottom() + p.y());
  //labelItem_->setAlignment(Qt::AlignCenter );
  labelItem_->setAlignment(Qt::AlignTop /*| Qt::AlignLeft*/);
  switch (type_) {
    case GraphType::kGraph:
      if ( r.left() < ramkaRect.left() ) {
        labelItem_->setAlignment(Qt::AlignTop | Qt::AlignRight);
      }
      else if ( r.right() > ramkaRect.right() ) {
        labelItem_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
      }
    break;
    case GraphType::kOcean:
      if ( r.bottom() > ramkaRect.bottom() ) {
        labelItem_->setAlignment(Qt::AlignTop);
        if ( r.left() < ramkaRect.left() ) {
          labelItem_->setAlignment(Qt::AlignTop | Qt::AlignRight);
        }
        else if ( r.right() > ramkaRect.right() ) {
          labelItem_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        }
      }
      else if ( r.top() < ramkaRect.top() ) {
        labelItem_->setAlignment(Qt::AlignBottom);
        if ( r.left() < ramkaRect.left() ) {
          labelItem_->setAlignment(Qt::AlignBottom | Qt::AlignRight);
        }
        else if ( r.right() > ramkaRect.right() ) {
          labelItem_->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
        }
      }
      else if ( r.left() < ramkaRect.left() ) {
        labelItem_->setAlignment(Qt::AlignTop | Qt::AlignRight);
      }
      else if ( r.right() > ramkaRect.right() ) {
        labelItem_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
      }
    break;
    default:
      break;
  }

  lineItem_->setGeoPos(gp);
  GeoPoint lblGp = cursorGp;
  switch (type_) {
    case GraphType::kGraph:
      lblGp.setLat(gp.lat());
    break;
    case GraphType::kOcean:
      lblGp.setLon(gp.lon());
    break;
    default:
      break;
  }
  labelItem_->setGeoPos(lblGp);
  labelItem_->setPos(labelItem_->calcScenePoint());


  if ( !show ) {
    view()->mapscene()->removeItem(lineItem_);
    delete lineItem_;
    lineItem_ = nullptr;

    view()->mapscene()->removeItem(labelItem_);
    closeBtnItem_->setParentItem(nullptr);
    scene_->removeItem(closeBtnItem_);
    delete labelItem_;
    labelItem_ = nullptr;

    qDeleteAll(markers_);
    markers_.clear();
  }
}

void GraphValueAction::mousePressEvent(QMouseEvent* e)
{
  if ( !moveDistance_.isNull() ) { moveDistance_.setP1(e->pos()); } else { moveDistance_ = QLine(e->pos(), e->pos()); }
}

void GraphValueAction::mouseReleaseEvent(QMouseEvent* e)
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
  if ( nullptr != closeBtnItem_->parentItem() ) {
    graph::ComboLabelItem* label = qgraphicsitem_cast<graph::ComboLabelItem*>(closeBtnItem_->parentItem());
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

  if ( nullptr != lineItem_ ) {
    scene_->document()->addItem(lineItem_);
    lineItem_->setLayer(scene_->document()->itemsLayer());
    lineItem_->setFlag(QGraphicsItem::ItemStacksBehindParent);
    lineItem_->setParentItem(labelItem_);

    GeoPoint gp = lineItem_->geoPos();
    lineItem_->setPos(0,0);
    lineItem_->setGeoPos(gp);
  }
  if ( nullptr != labelItem_ ) {
    scene_->document()->addItem(labelItem_);
    labelItem_->setLayer(scene_->document()->itemsLayer());
    labelItem_->setZValue(0);
  }
  for ( int i=0,isz=markers_.size(); i<isz; ++i ) {
    MarkerItem* mark = markers_[i];
    mark->setParentItem(labelItem_);
    mark->setFlag(QGraphicsItem::ItemStacksBehindParent);
    mark->setLayer(scene_->document()->itemsLayer());

    GeoPoint gp = mark->geoPos();
    mark->setPos(0,0);
    mark->setGeoPos(gp);

    scene_->document()->addItem(mark);
  }
  markers_.clear();

  lineItem_ = nullptr;
  labelItem_ = nullptr;
}

void GraphValueAction::addActionsToMenu(Menu* menu) const
{
  if ( nullptr == menu || nullptr == menuAct_ ) { return; }

  if ( !hasLayerGraph() ) { return; }

  menu->addLayerAction(menuAct_);

  if ( nullptr == scene_ ) { return; }

  QPoint pos = view()->mapFromGlobal(QCursor::pos());
  pos = view()->mapToScene(pos).toPoint();

  graph::ComboLabelItem* label = qgraphicsitem_cast<graph::ComboLabelItem*>(scene_->itemAt(pos, view()->transform() ));
  if ( 0 != label && label != labelItem_ ) {
    menu->addLayerAction(deleteAct_);
    deleteAct_->setData(QVariant::fromValue(label));
  }
}

void GraphValueAction::deactivate()
{
  if ( nullptr == menuAct_ ) { return; }

  menuAct_->setChecked(false);
}

bool GraphValueAction::isActive() const
{
  return nullptr != menuAct_ && menuAct_->isChecked();
}

bool GraphValueAction::hasLayerGraph() const
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

bool GraphValueAction::isWidgetUnderMouse(const QPointF& screenPos) const
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

void GraphValueAction::slotActionToggled(bool toggled)
{
  if ( false == toggled ) {
    delete lineItem_;
    lineItem_ = nullptr;
    closeBtnItem_->setParentItem(nullptr);
    scene_->removeItem(closeBtnItem_);
    delete labelItem_;
    labelItem_ = nullptr;
    qDeleteAll(markers_);
    markers_.clear();
  }
  else {
    GraphCoordAction* a = qobject_cast<GraphCoordAction*>(scene_->getAction(GraphCoordAction::kName));
    if ( nullptr != a ) {
      a->deactivate();
    }
  }
}

void GraphValueAction::slotDeleteTriggered()
{
  if ( nullptr == scene_ ) { return; }

  QGraphicsItem* parent = closeBtnItem_->parentItem();
  if ( nullptr != parent ) {
    closeBtnItem_->setParentItem(nullptr);
    scene_->removeItem(closeBtnItem_);
  }

  graph::ComboLabelItem* l = deleteAct_->data().value<graph::ComboLabelItem*>();
  if ( nullptr != l ) {
    closeBtnItem_->setParentItem(nullptr);
    scene_->removeItem(closeBtnItem_);
    delete l;
  }

  deleteAct_->setData(QVariant());
}

} // map
} // meteo
