#include "spectrvalueaction.h"

#include <qcursor.h>
#include <qevent.h>
#include <qtoolbutton.h>

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/graphitems/markeritem.h>
#include <mappi/ui/plugins/uhdcontrol/layerspectr.h>
#include <mappi/ui/plugins/uhdcontrol/lineitem.h>
#include <mappi/ui/plugins/uhdcontrol/coordaction/spectrcoordaction.h>
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

const QString SpectrValueAction::kName = "graph_value_action";

SpectrValueAction::SpectrValueAction(MapScene* scene)
  : Action(scene, SpectrValueAction::kName)
{
  lineItem_ = 0;
  labelItem_ = 0;
  closeBtnItem_ = 0;

  QToolButton* close = new QToolButton;
  close->setMaximumSize(QSize(16,16));
  close->setMinimumSize(QSize(16,16));
  close->setIcon(QIcon(":/meteo/icons/map/close.png"));
  close->setIconSize(QSize(8,8));
  closeBtnItem_ = new map::WidgetItem(close);

  deleteAct_ = new QAction(tr("Удалить"), this);
  connect( deleteAct_, SIGNAL(triggered(bool)), SLOT(slotDeleteTriggered()) );

  menuAct_ = scene_->mapview()->window()->addActionToMenu({"graph_value_action", tr("Значение под курсором")});
  //  if ( 0 != menuAct_ ) {
  //    menuAct_->setCheckable(true);
  //    connect( menuAct_, SIGNAL(toggled(bool)), SLOT(slotActionToggled(bool)) );
  //  }
  if ( 0 == menuAct_ ) {
    menuAct_ = new QAction(tr("Значение под курсором"), this);
  }
  menuAct_->setCheckable(true);
  connect( menuAct_, SIGNAL(toggled(bool)), SLOT(slotActionToggled(bool)) );
  map::Document* doc = scene_->document();
  if ( !doc->isStub() ) {
    doc->turnEvents();
    if ( doc->eventHandler() != 0 ) {
      doc->eventHandler()->installEventFilter(this);
    }
  }

}

void SpectrValueAction::mouseMoveEvent(QMouseEvent* e)
{
  if ( 0 == scene_ ) { return; }

  // при наведении указателя мыши на подпись добавляем кнопку "закрыть"
  {
    QPoint pos = view()->mapFromGlobal(QCursor::pos());
    pos = view()->mapToScene(pos).toPoint();

    QGraphicsItem* item = scene_->itemAt(pos, view()->transform() );
    spectr::ComboLabelItem* label = qgraphicsitem_cast<spectr::ComboLabelItem*>(item);
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

  QPoint pos = view()->mapToScene(e->pos()).toPoint();
  GeoPoint cursorGp = scene_->screen2coord(pos);

  map::Document* doc = scene_->document();

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


  QList<spectr::MarkerData> xMarkers;
  QList<spectr::MarkerData> yMarkers;
  QList<spectr::MarkerData> puanMarkers;

  // ищем ближайшие точки со значениями
  foreach ( map::Layer* layer, doc->layers() ) {
    map::LayerSpectr* l = maplayer_cast<map::LayerSpectr*>(layer);

    if ( 0 == l || !l->visible() ) { continue; }

    QList<GeoPoint> points = l->crossByX(cursorGp.lat());

    foreach ( const GeoPoint& point, points ) {
      spectr::MarkerData md;
      md.pos = point;

      AxisSpectr* x = l->xAxis();
      AxisSpectr* y = l->yAxis();

      if ( y->property().visible() ) {
        if ( AxisSpectr::kltNumber == y->tickLabelType() ) {
          //          md.text = (y->label() + ":" + y->tickLabel(y->coord2scale(point.lon())));
          md.text = (y->label() + ":" + y->tickLabel(point.lon()));
          md.color = pen2qpen(l->grProperty().pen()).color();
        }
        else {
          //md.text = QDateTime::fromTime_t(y->coord2scale(point.lon())).toString("hh:mm \"dd\" MMM yyyy");
          md.text = QDateTime::fromTime_t(point.lon()).toString("hh:mm \"dd\" MMM yyyy");
          md.color = pen2qpen(l->grProperty().pen()).color();
        }
        yMarkers << md;
      }

      if ( x->property().visible() ) {
        if ( AxisSpectr::kltNumber == x->tickLabelType() ) {
          //          md.text = (x->label() + ":" + x->tickLabel(x->coord2scale(point.lat())));
          md.text = (x->label() + ":" + x->tickLabel(point.lat()));
          md.color = pen2qpen(l->grProperty().pen()).color();
        }
        else {
          //          md.text = QDateTime::fromTime_t(x->coord2scale(point.lat())).toString("hh:mm \"dd\" MMM yyyy");
          md.text = QDateTime::fromTime_t(point.lat()).toString("hh:mm \"dd\" MMM yyyy");
          md.color = pen2qpen(l->grProperty().pen()).color();
        }
        xMarkers << md;
      }

//      foreach ( const QString& text, l->puansonLabels(x->coord2scale(point.lat())) ) {
//        md.text = text;
//        md.color = QColor();
//        puanMarkers << md;
//      }
    }
  }

  bool show = false;
  QList< QPair<double,spectr::MarkerData> > markers;

  QString dtText;

  // определяем минимальное расстояние
  double minDist = std::numeric_limits<double>::max();
  for ( int i=0,isz=xMarkers.size(); i<isz; ++i ) {
    double dist = qAbs(cursorGp.lat() - xMarkers.at(i).pos.lat());

    if ( dist > minDist ) { continue; }

    dtText = xMarkers[i].text;

    minDist = dist;
  }
  for ( int i=0,isz=yMarkers.size(); i<isz; ++i ) {
    double dist = qAbs(cursorGp.lat() - yMarkers.at(i).pos.lat());

    if ( dist > minDist ) { continue; }

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

  if ( 0 == lineItem_ ) {
    lineItem_ = new spectr::LineItem;
    view()->mapscene()->addItem(lineItem_);

    lineItem_->setZValue(10);
  }
  if ( 0 == labelItem_ ) {
    labelItem_ = new spectr::ComboLabelItem;
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
    QPen pen(meteo::kMAP_ISLAND_COLOR, 3);
    QBrush brush(yMarkers[i].color);
    mark->setStyle(MarkerItem::kNormalStyleRole, pen, brush);
    mark->setStyle(MarkerItem::kHoverStyleRole, pen, brush);
    mark->setStyle(MarkerItem::kSelectedStyleRole, pen, brush);
    markers_ << mark;
  }

  //labelItem_->setTitle(dtText.mid(6));
  labelItem_->addText(tr("Частота"), ": " + dtText.mid(6) + trUtf8(" МГц"));//dtText.left(5));
  //var(dtText);
  for ( int i=0,isz=markers.size(); i<isz; ++i ) {
    if ( !MnMath::isEqual(markers.at(i).first, minDist) ) {
      continue;
    }
    QString s = markers.at(i).second.text;
    //labelItem_->addText(s.section(":", 0, 0), ": " + s.section(":", 1, 1));
    labelItem_->addText(tr("Амплитуда"), ": " + s.section(":", 1, 1) + trUtf8(" дБ"));
    //var(s.section(":",1,1).toDouble());
//    map::LayerSpectr* l = maplayer_cast<map::LayerSpectr*>(doc->activeLayer());
//    QString phase=QString::number(RAD2DEG*(l->getPhaseOnFreq(dtText.section(":",1,1).toDouble())));
//    labelItem_->addText(tr("Фаза"),": " + phase );
    show = true;
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
  if ( r.left() < ramkaRect.left() ) {
    labelItem_->setAlignment(Qt::AlignTop | Qt::AlignRight);
  }
  else if ( r.right() > ramkaRect.right() ) {
    labelItem_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  }

  lineItem_->setGeoPos(gp);
  GeoPoint lblGp = cursorGp;
  lblGp.setLat(gp.lat());
  labelItem_->setGeoPos(lblGp);
  labelItem_->setPos(labelItem_->calcScenePoint());


  if ( !show ) {
    view()->mapscene()->removeItem(lineItem_);
    delete lineItem_;
    lineItem_ = 0;

    view()->mapscene()->removeItem(labelItem_);
    delete labelItem_;
    labelItem_ = 0;

    qDeleteAll(markers_);
    markers_.clear();
  }
}

void SpectrValueAction::mousePressEvent(QMouseEvent* e)
{
  if ( !moveDistance_.isNull() ) { moveDistance_.setP1(e->pos()); } else { moveDistance_ = QLine(e->pos(), e->pos()); }
}

void SpectrValueAction::mouseReleaseEvent(QMouseEvent* e)
{
  if ( 0 == scene_ ) { return; }

  moveDistance_.setP2(e->pos());

  // удаляем элемент "подпись" если под курсором мыши была кнопка "закрыть"
  if ( closeBtnItem_->isUnderMouse() ) {
    QGraphicsItem* parent = closeBtnItem_->parentItem();
    spectr::ComboLabelItem* delParent=qgraphicsitem_cast<spectr::ComboLabelItem*>(parent);
    comboItemsData_.remove(delParent);
    if ( 0 != parent ) {
      closeBtnItem_->setParentItem(0);
      parent->setVisible(false);
    }
    scene_->removeItem(closeBtnItem_);
    return;
  }

  // обновляем (при необходимости) позицию кнопки, на случай если была передвинута карта
  if ( 0 != closeBtnItem_->parentItem() ) {
    spectr::ComboLabelItem* label = qgraphicsitem_cast<spectr::ComboLabelItem*>(closeBtnItem_->parentItem());
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

  if ( 0 != lineItem_ ) {
    scene_->document()->addItem(lineItem_);
    lineItem_->setLayer(scene_->document()->itemsLayer());
    lineItem_->setFlag(QGraphicsItem::ItemStacksBehindParent);
    lineItem_->setParentItem(labelItem_);

    GeoPoint gp = lineItem_->geoPos();
    lineItem_->setPos(0,0);
    lineItem_->setGeoPos(gp);
  }
  if ( 0 != labelItem_ ) {
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

  spectr::ComboItemsData buff;
  buff.comboLabelItem = labelItem_;
  buff.lineItem = lineItem_;
  buff.markers = markers_;
  comboItemsData_.insert(labelItem_,buff);

  markers_.clear();
  lineItem_ = 0;
  labelItem_ = 0;

}

void SpectrValueAction::addActionsToMenu(Menu* menu) const
{
  if ( 0 == menu || 0 == menuAct_ ) { return; }

  if ( !hasLayerSpectr() ) { return; }

  menu->addLayerAction(menuAct_);

  if ( 0 == scene_ ) { return; }

  QPoint pos = view()->mapFromGlobal(QCursor::pos());
  pos = view()->mapToScene(pos).toPoint();

  spectr::ComboLabelItem* label = qgraphicsitem_cast<spectr::ComboLabelItem*>(scene_->itemAt(pos, view()->transform() ));
  if ( 0 != label && label != labelItem_ ) {
    menu->addLayerAction(deleteAct_);
    deleteAct_->setData(QVariant::fromValue(label));
  }
}

void SpectrValueAction::deactivate()
{
  if ( 0 == menuAct_ ) { return; }

  menuAct_->setChecked(false);
}

bool SpectrValueAction::isActive() const
{
  return 0 != menuAct_ && menuAct_->isChecked();
}

bool SpectrValueAction::hasLayerSpectr() const
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

bool SpectrValueAction::isWidgetUnderMouse(const QPointF& screenPos) const
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

void SpectrValueAction::slotActionToggled(bool toggled)
{
  if ( false == toggled ) {
    delete lineItem_;
    lineItem_ = 0;
    delete labelItem_;
    labelItem_ = 0;
    qDeleteAll(markers_);
    markers_.clear();
  }
  else {
    SpectrCoordAction* a = qobject_cast<SpectrCoordAction*>(scene_->getAction(SpectrCoordAction::kName));
    if ( 0 != a ) {
      a->deactivate();
    }
  }
}

void SpectrValueAction::slotDeleteTriggered()
{
  if ( 0 == scene_ ) { return; }

  QGraphicsItem* parent = closeBtnItem_->parentItem();
  spectr::ComboLabelItem* delParent=qgraphicsitem_cast<spectr::ComboLabelItem*>( closeBtnItem_->parentItem());
  comboItemsData_.remove(delParent);
  if ( 0 != parent ) {
    closeBtnItem_->setParentItem(0);
    scene_->removeItem(closeBtnItem_);
  }

  spectr::ComboLabelItem* l = deleteAct_->data().value<spectr::ComboLabelItem*>();
  if ( 0 != l ) { delete l; }

  deleteAct_->setData(QVariant());
}


bool SpectrValueAction::eventFilter(QObject* obj, QEvent* event)
{
  Q_UNUSED( obj );
  map::Document* doc = scene_->document();
  if ( 0 == doc ) {
    return false;
  }
  if ( map::LayerEvent::LayerChanged != event->type() ) {
    return false;
  }

  map::LayerEvent* ev = static_cast<map::LayerEvent*>(event);
  if ( false == doc->hasLayer(ev->layer()) && map::LayerEvent::Deleted != ev->changeType() ) {
//  debug_log << QObject::tr("Ошибка. Слой %1 не найден. Событие %2").arg(ev->layer()).arg(ev->changeType());
    return false;
  }

  if (ev->changeType() == map::LayerEvent::ObjectChanged ){
//    var(obj);
//    std::cout << "test filter" << std::endl;
    updateComboLabel();
    return false;
  }

  return false;
}

void SpectrValueAction::updateComboLabel()
{
  QMap<spectr::ComboLabelItem*,spectr::ComboItemsData>::iterator itMap;
  for (itMap=comboItemsData_.begin();itMap!=comboItemsData_.end();itMap++) {
    spectr::ComboItemsData comboItem=itMap.value();
    GeoPoint linePos = comboItem.lineItem->geoPos();
    map::Document* doc = scene_->document();
    QList<double> existsValues;
    existsValues << linePos.lat();
    QList<spectr::MarkerData> xMarkers;
    QList<spectr::MarkerData> yMarkers;

    // ищем ближайшие точки со значениями
    foreach ( map::Layer* layer, doc->layers() ) {
      map::LayerSpectr* l = maplayer_cast<map::LayerSpectr*>(layer);

      if ( 0 == l || !l->visible() ) { continue; }

      QList<GeoPoint> points = l->crossByX(linePos.lat());
      foreach ( const GeoPoint& point, points ) {
        spectr::MarkerData md;
        md.pos = point;
        AxisSpectr* x = l->xAxis();
        AxisSpectr* y = l->yAxis();
        if ( y->property().visible() ) {
          if ( AxisSpectr::kltNumber == y->tickLabelType() ) {
            md.text = (y->label() + ":" + y->tickLabel(point.lon()));
            md.color = pen2qpen(l->grProperty().pen()).color();
          }
          else {
            md.text = QDateTime::fromTime_t(point.lon()).toString("hh:mm \"dd\" MMM yyyy");
            md.color = pen2qpen(l->grProperty().pen()).color();
          }
          yMarkers << md;
        }

        if ( x->property().visible() ) {
          if ( AxisSpectr::kltNumber == x->tickLabelType() ) {
            md.text = (x->label() + ":" + x->tickLabel(point.lat()));
            md.color = pen2qpen(l->grProperty().pen()).color();
          }
          else {
            md.text = QDateTime::fromTime_t(point.lat()).toString("hh:mm \"dd\" MMM yyyy");
            md.color = pen2qpen(l->grProperty().pen()).color();
          }
          xMarkers << md;
        }
      }
    }

    QList< QPair<double,spectr::MarkerData> > markers;

    QString dtText;

    // определяем минимальное расстояние
    double minDist = std::numeric_limits<double>::max();
    for ( int i=0,isz=xMarkers.size(); i<isz; ++i ) {
      double dist = qAbs(linePos.lat() - xMarkers.at(i).pos.lat());

      if ( dist > minDist ) { continue; }

      dtText = xMarkers[i].text;

      minDist = dist;
    }
    for ( int i=0,isz=yMarkers.size(); i<isz; ++i ) {
      double dist = qAbs(linePos.lat() - yMarkers.at(i).pos.lat());

      if ( dist > minDist ) { continue; }

      markers << qMakePair(dist,yMarkers.at(i));

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
    comboItem.comboLabelItem->clearTexts();
    for (int i=0, isz=comboItem.markers.size();i<isz;i++) {
      if ( 0 != yMarkers.size() ) {
        comboItem.markers[i]->setGeoPos(yMarkers[i].pos);
      }
    }
    comboItem.comboLabelItem->addText(tr("Частота"), ": " + dtText.mid(6) + trUtf8(" МГц"));
    for ( int i=0,isz=markers.size(); i<isz; ++i ) {
      if ( !MnMath::isEqual(markers.at(i).first, minDist) ) {
        continue;
      }
      QString s = markers.at(i).second.text;
      comboItem.comboLabelItem->addText(tr("Амплитуда"), ": " + s.section(":", 1, 1) + trUtf8(" дБ"));
//      map::LayerSpectr* l = maplayer_cast<map::LayerSpectr*>(doc->activeLayer());
//      QString phase=QString::number(RAD2DEG*(l->getPhaseOnFreq(dtText.section(":",1,1).toDouble())));
//      comboItem.comboLabelItem->addText(tr("Фаза"),": " + phase);
    }
  }
}

} // map
} // meteo


