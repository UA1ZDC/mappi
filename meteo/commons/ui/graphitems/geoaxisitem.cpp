#include "geoaxisitem.h"

#include <qpixmap.h>
#include <qpainter.h>
#include <qfontmetrics.h>
#include <qstyleoption.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/layeritems.h>
#include <meteo/commons/ui/map/ramka.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/geoaxis.h>
#include <meteo/commons/ui/map/profile/layerprofile.h>
#include <meteo/commons/global/weatherloader.h>

namespace meteo {
namespace graph {

static const bool drawDebug = false;

GeoAxisItem::GeoAxisItem(QGraphicsItem* parent, QGraphicsScene* scene)
  : QGraphicsItem(parent),
    axis_(new map::AxisProfile)
{
  Q_UNUSED(scene);
  axis_->setAxisType(map::AxisProfile::kCenter);

  setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

GeoAxisItem::~GeoAxisItem()
{
  map::Document* doc = document();
  if ( nullptr != doc ) {
    map::LayerItems* l = nullptr;
    for ( map::Layer* layer : doc->layers() ) {
      l = maplayer_cast<map::LayerItems*>(layer);
      if ( nullptr != l ) {
        break;
      }
    }

    if ( nullptr != l ) { l->removeItem(this); }
  }
  if ( nullptr != this->axis_ ){
    delete this->axis_;
    this->axis_ = nullptr;
  }
}

GeoPoint GeoAxisItem::geoPos() const
{
  return axis_->geoPos();
}

void GeoAxisItem::setGeoPos(const GeoPoint& gp)
{
  map::Document* doc = document();
  if ( nullptr == doc ) {
    debug_log << QObject::tr("Невозможно обновить позицию, т.к. нулевой указатель на Document");
    return;
  }

  prepareGeometryChange();
  setPos(doc->coord2screenf(gp));
  axis_->setGeoPos(gp);
  axis_->updateData(gp, doc);
}

void GeoAxisItem::setLength(double length)
{
  prepareGeometryChange();
  axis_->setLength(length);
}

void GeoAxisItem::setRange(double lower, double upper)
{
  prepareGeometryChange();
  axis_->setRange(lower,upper);
}

void GeoAxisItem::setTickVector(const QVector<float>& ticks, int level)
{
  prepareGeometryChange();
  axis_->setTickVector(ticks,level);
}

void GeoAxisItem::setAxisType(map::AxisSimple::AxisType type)
{
  prepareGeometryChange();
  axis_->setAxisType(type);
}

void GeoAxisItem::setPuanson(const puanson::proto::Puanson& puanson)
{
  prepareGeometryChange();
  axis_->setPuanson(puanson);
}

QRectF GeoAxisItem::boundingRect() const
{
  if ( nullptr == axis_ ) {
    error_log << QObject::tr("Невозможно вычислить размер: объект не добавлен на сцену.");
    return QRectF();
  }

  QRectF r = axis_->boundingRect(document());
  return r;
}

void GeoAxisItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  Q_UNUSED( option );
  Q_UNUSED( widget );

  map::Document* doc = document();
  if ( nullptr == doc ) {
    debug_log << QObject::tr("Невозможно нарисовать объект: нулевой указатель на Document");
    return;
  }

  bool antialiasing = painter->testRenderHint(QPainter::Antialiasing);
  painter->setRenderHint(QPainter::Antialiasing, false);
//  painter->setRenderHint(QPainter::TextAntialiasing, true);

  QRect r = boundingRect().toRect();
  QPoint beg;
  QPoint end;
  if ( map::AxisSimple::kLeft == axis_->axisType() || map::AxisSimple::kCenter == axis_->axisType() ) {
    beg = r.bottomRight() + QPoint(1,1);
    end = r.topRight() + QPoint(1,1);
  }
  else if ( map::AxisSimple::kRight == axis_->axisType() ) {
    beg = r.bottomLeft();
    end = r.topLeft();
  }

  axis_->updateData(axis_->geoPos(), doc);
  axis_->paint(painter, beg, end, doc);

  if ( true == drawDebug )
  {
    painter->save();

    QPen pen(QBrush(Qt::blue), 1, Qt::DotLine);
    painter->setPen(pen);

    QRect rr = boundingRect().toRect();
    rr.setWidth(rr.width() - 1);
    rr.setHeight(rr.height() - 1);
    painter->drawRect(rr);

    painter->restore();
  }

  painter->setRenderHint(QPainter::Antialiasing, antialiasing);
}

QVariant GeoAxisItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
  if ( QGraphicsItem::ItemScenePositionHasChanged == change ) {
    map::Document* doc = document();
    if ( nullptr != doc ) {
      axis_->setGeoPos(doc->screen2coord(scenePos()));
    }
    else {
      error_log << QObject::tr("Невозможно рассчитать географическую координату: нулевой указатель на Document");
    }
  }
  else if ( QGraphicsItem::ItemSceneChange == change ) {
    map::MapScene* mapScene = qobject_cast<map::MapScene*>(value.value<QGraphicsScene*>());
    if ( nullptr == mapScene ) {
      error_log << QObject::tr("Невозможно инициализировать объект: нулевой указатель на MapScene");
      return value;
    }

    map::Document* doc = mapScene->document();
    if ( nullptr == doc ) {
      error_log << QObject::tr("Невозможно инициализировать объект: нулевой указатель на Document");
      return value;
    }

    map::LayerProfile* l = nullptr;
    for ( map::Layer* layer : doc->layers() ) {
      l = maplayer_cast<map::LayerProfile*>(layer);
      if ( nullptr != l ) {
        break;
      }
    }

    if ( nullptr == l ) {
      error_log << QObject::tr("Невозможно инициализировать объект: не найден LayerProfile");
      return value;
    }

    axis_->setLayer(l);
    axis_->setMapSize(l->mapRect().size());
    axis_->setOffset(l->yAxis()->offset());
  }

  return value;
}

map::Document* GeoAxisItem::document() const
{
  map::MapScene* mapScene = qobject_cast<map::MapScene*>(scene());
  if ( nullptr != mapScene ) {
    return mapScene->document();
  }

  return nullptr;
}

} // graph
} // meteo
