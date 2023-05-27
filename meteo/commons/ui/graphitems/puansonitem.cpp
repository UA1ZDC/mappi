#include "puansonitem.h"

#include <qpixmap.h>
#include <qpainter.h>
#include <qfontmetrics.h>
#include <qstyleoption.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/layeritems.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/profile/layerprofile.h>
#include <meteo/commons/ui/map/puanson.h>


namespace meteo {
namespace graph {

static const bool drawDebug = false;

PuansonItem::PuansonItem(QGraphicsItem* parent, QGraphicsScene* scene)
  : QGraphicsItem(parent)
{
  Q_UNUSED(scene);

  setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

PuansonItem::~PuansonItem()
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
}

GeoPoint PuansonItem::geoPos() const
{
  return geoPos_;
}

void PuansonItem::setGeoPos(const GeoPoint& gp)
{
  map::Document* doc = document();
  if ( 0 == doc ) {
    debug_log << QObject::tr("Невозможно обновить позицию, т.к. нулевой указатель на Document");
    return;
  }

  prepareGeometryChange();
  setPos(doc->coord2screenf(gp));
  geoPos_ = gp;
}

void PuansonItem::setPuanson(const puanson::proto::Puanson& puanson)
{
  puanson_ = puanson;
}

void PuansonItem::setBorderStyle(const QColor& color, int width)
{
  borderColor_ = color;
  borderWidth_ = width;
}

void PuansonItem::setBackgroundStyle(const QColor& color, Qt::BrushStyle style)
{
  bgColor_ = color;
  bgStyle_ = style;
}

QRectF PuansonItem::boundingRect() const
{
  map::LayerProfile* l = layer();
  if ( nullptr == l ) { return QRectF(); }

  auto mdVec = l->getDataByCoords({ geoPos_ });
  if ( mdVec.isEmpty() ) { return QRectF(); }

  map::Puanson p;
  p.setScreenPos(QPoint(0,0));
  p.setPunch(puanson_);
  p.setMeteodata(mdVec.first());

  QRect r = p.boundingRect();
  r += QMargins(5,5,5,5);
  return r;
}

void PuansonItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  Q_UNUSED( option );
  Q_UNUSED( widget );
  Q_UNUSED( painter );

  map::Document* doc = document();
  if ( 0 == doc ) {
    debug_log << QObject::tr("Невозможно нарисовать объект: нулевой указатель на Document");
    return;
  }

  bool antialiasing = painter->testRenderHint(QPainter::Antialiasing);
  painter->setRenderHint(QPainter::Antialiasing, false);

  map::LayerProfile* l = layer();
  if ( nullptr == l ) { return; }

  auto mdVec = l->getDataByCoords({ geoPos_ });
  if ( mdVec.isEmpty() ) { return; }

  QRect r = boundingRect().toRect();

  map::Puanson p;
  p.setScreenPos(QPoint(0,0));
  p.setPunch(puanson_);
  p.setMeteodata(mdVec.first());

  QTransform tr;
  tr.translate(r.left(),r.top());
  tr = tr.inverted();

  painter->save();

  if ( Qt::NoBrush != bgStyle_ ) {
    QBrush b(bgColor_,bgStyle_);
    painter->setBrush(b);
    painter->fillRect(r,bgColor_);
  }
  if ( borderWidth_ > 0 ) {
    QPen pen;
    pen.setColor(borderColor_);
    pen.setWidth(borderWidth_);
    painter->setPen(pen);
    painter->drawRect(r);
  }

  painter->restore();

  p.render(painter, r, QTransform());

//  if ( true == drawDebug )
//  {
//    painter->save();

//    QPen pen(QBrush(Qt::blue), 1, Qt::DotLine);
//    painter->setPen(pen);

//    QRect rr = boundingRect().toRect();
//    rr.setWidth(rr.width() - 1);
//    rr.setHeight(rr.height() - 1);
//    painter->drawRect(rr);

//    painter->restore();
//  }

  painter->setRenderHint(QPainter::Antialiasing, antialiasing);
}

QVariant PuansonItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
  if ( QGraphicsItem::ItemScenePositionHasChanged == change ) {
    map::Document* doc = document();
    if ( 0 != doc ) {
      geoPos_ = doc->screen2coord(scenePos());
    }
    else {
      error_log << QObject::tr("Невозможно рассчитать географическую координату: нулевой указатель на Document");
    }
  }

  return value;
}

map::Document* PuansonItem::document() const
{
  map::MapScene* mapScene = qobject_cast<map::MapScene*>(scene());
  if ( nullptr != mapScene ) {
    return mapScene->document();
  }

  return nullptr;
}

map::LayerProfile*PuansonItem::layer() const
{
  map::Document* doc = document();
  if ( nullptr == doc ) { return nullptr; }

  map::LayerProfile* l = nullptr;
  for ( map::Layer* layer : doc->layers() ) {
    l = maplayer_cast<map::LayerProfile*>(layer);
    if ( nullptr != l ) {
      break;
    }
  }
  return l;
}

} // graph
} // meteo
