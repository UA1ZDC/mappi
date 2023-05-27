#include "puansonitem.h"

#include <qpixmap.h>
#include <qpainter.h>

#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/ui/map/wind.h>
#include <meteo/commons/ui/map/layeritems.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/view/mapscene.h>

namespace meteo {
namespace map {

const QSize kPixSize( 150, 150 );

PuansonItem::PuansonItem( Puanson* punch, MapScene* sc )
  : QGraphicsItem(),
    punch_(punch),
    scene_(sc),
    copy_( mapobject_cast<Puanson*>(punch_->copy( punch_->projection() ) ) )
{
  copy_->setScreenPos( QPoint(0,0) );
  puanson::proto::Puanson p = copy_->punch();
  p.clear_scale();
  copy_->setPunch(p);
  copy_->setMeteodata( punch_->meteodata() );
  scene_->addItem(this);
  scene_->document()->addItem(this);
}

PuansonItem::~PuansonItem()
{
  delete copy_; copy_ = 0;
  scene_->document()->removeItem(this);
  punch_ = 0;
}

QRectF PuansonItem::boundingRect() const
{
  QRectF r = punchrect();
  r.setBottom( r.bottom() + 20 );
  return r;
}

QRectF PuansonItem::punchrect() const
{
  QRectF rect;
  if ( 0 == punch_ ) {
    return rect;
  }
  if ( 0 == copy_->skelet().size() ) {
    return rect;
  }

  if ( 0 == scene_ || 0 == scene_->document() ) {
    return rect;
  }
  if ( false == reinterpret_cast<Object*>(punch_)->visible()
       || 0 == punch_->layer()
       || false == punch_->layer()->visible()
       || punch_->layer() != scene_->document()->activeLayer() ) {
    return rect;
  }
  LayerItems* layer = scene_->document()->itemsLayer();
  if ( 0 == layer ) {
    return rect;
  }
  Document* document = layer->document();
  if ( 0 == document ) {
    return rect;
  }
  rect = QRectF( QPointF(0,0), kPixSize );
  QPoint pnt = document->coord2screen( punch_->skelet()[0] );
  pnt.setY( pnt.y() - kPixSize.height()/2 - 20 );
  rect.moveCenter(pnt);
  return rect;
}

void PuansonItem::paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w )
{
  Q_UNUSED(o);
  Q_UNUSED(w);
  if ( 0 == punch_ ) {
    return;
  }
  if ( 0 == scene_ || 0 == scene_->document() ) {
    return;
  }
  if ( false == reinterpret_cast<Object*>(punch_)->visible()
       || 0 == punch_->layer()
       || false == punch_->layer()->visible()
       || punch_->layer() != scene_->document()->activeLayer() ) {
    return;
  }
  LayerItems* layer = scene_->document()->itemsLayer();
  if ( 0 == layer ) {
    return;
  }

  QRectF prect = punchrect();

  QSize sz(kPixSize);
  QPixmap pix(sz);
  pix.fill( Qt::white );
  QPainter pntr(&pix);
  QPoint pnt( pix.rect().center() );
  QTransform tr;
  tr.translate( pnt.x(), pnt.y() );
  tr.scale(1.4,1.4);
  pntr.setTransform(tr);
  copy_->render( &pntr, copy_->boundingRect(), QTransform() );
  p->save();
  p->setPen( QPen( kMAP_RAMKAPEN_DEFAULT ) );
  p->drawPixmap( prect, pix, pix.rect() );
  p->save();
  p->setBrush( QBrush(Qt::NoBrush) );
  p->drawRect(prect);
  p->restore();
  int indx = TMeteoDescriptor::instance()->station( copy_->meteodata() );
  QString station;
  if( BAD_METEO_ELEMENT_VAL != indx){
    station = QObject::tr("Индекс: %1").arg( indx, 5, 10, QChar('0') );
  }
  QRectF r = p->boundingRect( QRect(), Qt::AlignCenter, station );
  r = QRectF( prect.topLeft(), QSizeF(prect.size().width(), r.height() ) );
  p->setPen( QPen( Qt::black ) );
  p->drawText( r, Qt::AlignCenter, station );
  p->setPen( QPen( kMAP_RAMKAPEN_DEFAULT ) );
  p->setBrush( QBrush(Qt::NoBrush) );
  p->drawRect(r);
  QPolygonF poly;
  poly.append( prect.bottomLeft() );
  poly.append( prect.bottomRight() );
  poly.append( QPointF( prect.center().x(), prect.bottom() + 20 ) );
  poly.append( prect.bottomLeft() );
  p->setBrush( QBrush( QColor( 127, 127, 127, 127 ) ) );
  p->drawPolygon(poly);
  p->restore();
}

}
}
