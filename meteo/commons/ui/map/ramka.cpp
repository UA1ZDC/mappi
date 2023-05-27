#include "ramka.h"

#include <qpainter.h>
#include <qbrush.h>
#include <qpoint.h>
#include <qsize.h>
#include <qrect.h>
#include <qsettings.h>
#include <qvariant.h>
#include <qregion.h>
#include <qline.h>

#include "document.h"
#include "object.h"
#include "geopolygon.h"
#include "geotext.h"
#include "layergrid.h"

#include <meteo/commons/global/common.h>
#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace map {

Ramka::Ramka( Document* doc, int hw )
  : document_(doc),
    halfwidth_(hw)
{
  pen_ = kMAP_RAMKAPEN_DEFAULT;
}

Ramka::~Ramka()
{
}

bool Ramka::drawOnDocument( QPainter* painter )
{
  Projection* proj = document_->projection();
  const GeoVector& rmk = proj->ramka();
  QPolygon mappoly = document_->coord2screen(rmk);
  if ( 3 > mappoly.size() ) {
    return true;;
  }

  QPolygon intrsct = calcRamka( -halfwidth_, -halfwidth_ );

  QPen pen = pen_;
  pen.setWidth(halfwidth_*2);

  pen.setJoinStyle(Qt::MiterJoin);
  pen.setMiterLimit(0.2);

  painter->setPen(pen);
  painter->drawPolyline(intrsct);
  pen.setColor( Qt::white );
  int w = halfwidth_*2-2;
  if (w > 0) {
    pen.setWidth(w);
    painter->setPen(pen);
    painter->drawPolyline(intrsct);
  }
  drawGridLabels(painter);

  return true;
}

QPolygon Ramka::calcRamka( int mapindent, int docindent ) const
{
  Projection* proj = document_->projection();
  const GeoVector& rmk = proj->ramka();

  QPolygon mappoly = document_->coord2screen(rmk);
  if ( 3 > mappoly.size() ) {
    return QPolygon();
  }
  QRect dr = document_->documentRect();
  if ( 0 != docindent ) {
    dr.setTopLeft( dr.topLeft() - QPoint( docindent, docindent ) );
    dr.setBottomRight( dr.bottomRight() + QPoint( docindent, docindent ) );
  }
  QPolygon docpoly = QPolygon( dr, true );

  if ( 0 == mapindent ) {
    return docpoly.intersected(mappoly);
  }

  QPolygonF newpoly;
  QPolygon tmp(mappoly);
  tmp.append(tmp[1]);
  tmp.append(tmp[2]);
  for ( int i = 0, sz = tmp.size() - 2; i < sz; ++i ) {
    QPoint pnt1 = tmp[i];
    QPoint pnt2 = tmp[i+2];

    QPoint node = tmp[i+1];
    QPoint dist1 = node - pnt1;
    QPoint dist2 = pnt2 - node;
    float gipot1 = dist1.x()*dist1.x() + dist1.y()*dist1.y();
    float gipot2 = dist2.x()*dist2.x() + dist2.y()*dist2.y();
    if ( gipot1 > 8  && gipot2 > 8 )
    {
      if ( gipot1 > gipot2 ) {
        float angle = ::atan2( dist1.y(), dist1.x() );
        float gipot2sqrt = ::sqrt(dist2.x()*dist2.x() + dist2.y()*dist2.y());
        float x = ::cos(angle)*gipot2sqrt;
        float y = ::sin(angle)*gipot2sqrt;
        pnt1 = node - QPoint(x,y);
      }
      else {
        float angle = ::atan2( dist2.y(), dist2.x() );
        float gipot1sqrt = ::sqrt(dist1.x()*dist1.x() + dist1.y()*dist1.y());
        float x = ::cos(angle)*gipot1sqrt;
        float y = ::sin(angle)*gipot1sqrt;
        pnt2 = node + QPoint(x,y);
      }
    }

    QPoint dist = pnt2 - pnt1;
    float angle = ::atan2( dist.y(), dist.x() );
    angle -= M_PI_2;
    float x = ::cos(angle)*mapindent;
    float y = ::sin(angle)*mapindent;
    QPointF pnt = QPointF(tmp[i+1]) + QPointF(x,y);
    newpoly.append(pnt);
  }

  QPolygon intrsct = docpoly.intersected(newpoly.toPolygon());
  return intrsct;
}

void Ramka::drawGridLabels( QPainter* painter )
{
  if ( 0 == document_->gridLayer() ) {
    return;
  }
  if ( false == document_->gridLayer()->visible() ) {
    return;
  }

  painter->save();

  QFont font = painter->font();
  font.setPointSize(7);
  painter->setFont(font);
  painter->setRenderHint( QPainter::TextAntialiasing, false );

  QPolygon poly = calcRamka( -halfwidth_, -halfwidth_);
  lblrects_.clear();
  drawLongitudes( painter, poly );
  drawLatitides( painter, poly );

  painter->restore();
}

void Ramka::drawLongitudes( QPainter* painter, const QPolygon& ramka )
{
  LayerGrid* l = maplayer_cast<LayerGrid*>( document_->gridLayer() );
  if ( 0 == l ) {
    return;
  }
  QFontMetrics fm(painter->font());
  foreach(Object* o, l->longitudes()){
    GeoPolygon* geopol = mapobject_cast<GeoPolygon*>(o);
    if ( 0 == geopol ) {
      error_log << QObject::tr("Не удалсоь выполнить приведение тип meteo::map::Object* к meteo::map::GeoPolygon*");
      continue;
    }
    if ( false == geopol->visible( document_->scale() ) ) {
      continue;
    }

    int left = document_->documentRect().left() + halfwidth_;
    int right = document_->documentRect().right() - halfwidth_;
    
    const QVector<QPolygon>& scrv = geopol->cached_screen_points_;
    for ( int j = 0, jsz = scrv.size(); j < jsz; ++j ) {
      QPolygon screen = scrv[j];
      screen.prepend( screen[0] - (screen[1]-screen[0])*halfwidth_);
      screen.append( screen[ screen.size()-1 ] + (screen[ screen.size() - 1 ]-screen[ screen.size() - 2 ])*halfwidth_);
      QList<QPoint> intersections;
      intersectionPoints( screen, ramka, &intersections );
      for ( int k = 0, ksz = intersections.size(); k < ksz; ++k ) {
        const QPoint& pnt = intersections[k];
        if ( ::abs(pnt.x() - left) < 2 || ::abs(pnt.x() - right) < 2 ) {
          continue;
        }
        GeoText* lbl = mapobject_cast<GeoText*>(*geopol->childs().begin());
        QString text ;
        if(lbl != nullptr){
          text = lbl->text();
        }
        QRect lblrect = fm.boundingRect(text);
        lblrect.setTopLeft( lblrect.topLeft() - QPoint(1,1) );
        lblrect.setBottomRight( lblrect.bottomRight() + QPoint(1,1) );
        lblrect.moveCenter(pnt);
        if ( true == rectIntersectOneOfRects( lblrect, lblrects_ ) ) {
          continue;
        }
        lblrects_.append(lblrect);
        painter->setPen( geopol->qpen() );
        painter->drawText( lblrect, Qt::AlignCenter, text );
      }
    }
  }
}

void Ramka::drawLatitides( QPainter* painter, const QPolygon& ramka )
{
  LayerGrid* l = document_->gridLayer();
  if ( 0 == l ) {
    return;
  }
  QFontMetrics fm(painter->font());
  foreach(Object* o, l->latitudes()){
    GeoPolygon* geopol = mapobject_cast<GeoPolygon*>(o);
    if ( 0 == geopol ) {
      error_log << QObject::tr("Не удалсоь выполнить приведение тип meteo::map::Object* к meteo::map::GeoPolygon*");
      continue;
    }
    if ( false == geopol->visible( document_->scale() ) ) {
      continue;
    }

    int top = document_->documentRect().top() + halfwidth_;
    int bottom  = document_->documentRect().bottom() - halfwidth_;
    int left = document_->documentRect().left() + halfwidth_;
    int right = document_->documentRect().right() - halfwidth_;
    
    const QVector<QPolygon>& scrv = geopol->cached_screen_points_;
    for ( int j = 0, jsz = scrv.size(); j < jsz; ++j ) {
      QPolygon screen = scrv[j];
      screen.prepend( screen[0] - (screen[1]-screen[0])*halfwidth_);
      screen.append( screen[ screen.size()-1 ] + (screen[ screen.size() - 1 ]-screen[ screen.size() - 2 ])*halfwidth_);
      QList<QPoint> intersections;
      intersectionPoints( screen, ramka, &intersections );
      for ( int k = 0, ksz = intersections.size(); k < ksz; ++k ) {
        const QPoint& pnt = intersections[k];
        if ( STEREO == document_->projection()->type() ) {
          if ( ::abs(pnt.x() - left) > 2 && ::abs(pnt.x() - right) > 2 ) {
            continue;
          }
        }
        if ( ::abs(pnt.y() - top) < 2 || ::abs(pnt.y() - bottom ) < 2 ) {
          continue;
        }
        GeoText* lbl = mapobject_cast<GeoText*>((*geopol->childs().begin()));
        QString text ;
        if(lbl != nullptr){
          text = lbl->text();
        }
        QRect lblrect = fm.boundingRect(text);
        lblrect.setTopLeft( lblrect.topLeft() - QPoint(1,1) );
        lblrect.setBottomRight( lblrect.bottomRight() + QPoint(1,1) );
        lblrect.moveCenter(pnt);
        if ( true == rectIntersectOneOfRects( lblrect, lblrects_ ) ) {
          continue;
        }
        lblrects_.append(lblrect);
        painter->setPen( geopol->qpen() );
        painter->drawText( lblrect, Qt::AlignCenter, text );
      }
    }
  }
}

void Ramka::intersectionPoints( const QPolygon& first, const QPolygon& second, QList<QPoint>* intersections )
{
  for ( int k = 0, ksz = first.size()-1; k < ksz; ++k ) {
    bool cont_k_1 = second.containsPoint(first[k], Qt::WindingFill);
    bool cont_k_2 = second.containsPoint(first[k+1], Qt::WindingFill);
    if ( ( true == cont_k_1 && true == cont_k_2 ) || ( false == cont_k_1 && false == cont_k_2 ) ) {
      continue;
    }
    QLineF l1( first[k], first[k+1] );
    for ( int l = 0, lsz = second.size()-1; l < lsz; ++l ) {
      QPointF pnt;
      if ( QLineF::BoundedIntersection == l1.intersect( QLineF(second[l],second[l+1]), &pnt ) ) {
        intersections->append(pnt.toPoint());
      }
    }
  }
}

void Ramka::setHalfWidth( int w )
{
  halfwidth_ = w;
  document_->ramkawidth_ = halfwidth_*2;
}

Ramka* Ramka::copy( Document* d ) const
{
  Ramka* r = new Ramka( d, halfwidth_ );
  r->pen_ = pen_;
  return r;
}

bool Ramka::rectIntersectOneOfRects( const QRect& rect, const QList<QRect>& rects )
{
  for ( int i = 0, sz = rects.size(); i < sz; ++i ) {
    if ( true == rect.intersects( rects[i] ) ) {
      return true;
    }
  }

  return false;
}

}
}
