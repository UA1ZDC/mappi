#include "tgeometrymath.h"

#include "mnmath.h"

//int MnMath::oprTochPeresech (GeoPoint *kr1,int size_kr1,GeoPoint *kr2,int size_kr2,GeoPoint *&pointPeresech, QPointArray *numOtrPeresech, int *kol_peresech )
bool MnMath::oprTochPeresech( const QPolygonF& kr1, const QPolygonF& kr2, QPolygonF* pointPeresech ) {
  if ( 0 == pointPeresech ) {
    return false;
  }

  QPointF p11,p12,p21,p22;

  for( int i = 0, sz1 = kr1.size() - 1; i < sz1; ++i ) { //идем по кривой состояния
    
    p11 = kr1[i];
    p12 = kr1[i+1];
    for ( int j = 0, sz2 = kr2.size() - 1; j < sz2; ++j ) { //идем по кривой стратификации
      p21 = kr2[j];
      p22 = kr2[j+1];
      QPointF resPoint;
      CrossType res = crossing( p11, p12, p21, p22, &resPoint ); //проверяем на пересечение
      if ( res == ONBOUNDS || res == INBOUNDS ) { //если внутри отрезка или на концах его
        pointPeresech->append(resPoint);
      }
    }
  }
  return pointPeresech->size();
}

MnMath::CrossType MnMath::crossing( const QPointF& p11, const QPointF& p12,
      const QPointF& p21, const QPointF& p22, QPointF* resPoint )
{
  // знаменатель
  double Z  = (p12.y()-p11.y())*(p21.x()-p22.x())-(p21.y()-p22.y())*(p12.x()-p11.x());
  // числитель 1
  double Ca = (p12.y()-p11.y())*(p21.x()-p11.x())-(p21.y()-p11.y())*(p12.x()-p11.x());
  // числитель 2
  double Cb = (p21.y()-p11.y())*(p21.x()-p22.x())-(p21.y()-p22.y())*(p21.x()-p11.x());

  // если числители и знаменатель = 0, прямые совпадают
  if ( isZero(Z) ) {
    if ( isZero(Ca) && isZero(Cb) ) {
      return SAMELINE;
    }
    return PARALLEL;
  }

  if ( 0 == resPoint ) {
    return ERROR_CROSS_PARAM;
  }

  double Ua = Ca/Z;
  double Ub = Cb/Z;
  resPoint->setX(p11.x() + (p12.x() - p11.x()) * Ub);
  resPoint->setY(p11.y() + (p12.y() - p11.y()) * Ub);
  // если 0<=Ua<=1 и 0<=Ub<=1, точка пересечения в пределах отрезков
  if( ( 0. <= Ua)&&(Ua <= 1.)&&( 0. <= Ub)&&(Ub <= 1.) ) {
    if ( (Ua < 1.e-9)||(Ua == 1.)||(Ub < 1.e-9)||(Ub == 1.) ) {
      return ONBOUNDS;
    }
    return INBOUNDS;
  }
  return OUTBOUNDS;
}

bool MnMath::crossing( const QPointF& p11, const QPointF& p12,
      const QPointF& p21, const QPointF& p22 ) {
  double v1 = ( p22.x() - p21.x() ) * ( p11.y() - p21.y() )
    - ( p22.y() - p21.y() ) * ( p11.x() - p21.x() );
  double v2 = ( p22.x() - p21.x() ) * ( p12.y() - p21.y() )
    - ( p22.y() - p21.y() ) * ( p12.x() - p21.x() );
  if ( v1*v2 >= 0 ) {
    return false;
  }

  double v3 = ( p12.x() - p11.x() ) * ( p21.y() - p11.y() )
    - ( p12.y() - p11.y() ) * ( p21.x() - p11.x() );
  double v4 = ( p12.x() - p11.x() ) * ( p22.y() - p11.y() )
    - ( p12.y() - p11.y() ) * ( p22.x() - p11.x() );
  if ( v3*v4 >= 0 ) {
    return false;
  }
  return true;
}

int MnMath::rectCrossCode( const QRect& rect, const QPoint& p ) {
  return ( ( p.x() < rect.left() ) ? MnMath::LEFT_CROSS : 0 )
    + ( ( p.x() > rect.right() ) ? MnMath::RIGHT_CROSS : 0 )
    + ( ( p.y() < rect.top() ) ? MnMath::TOP_CROSS : 0 )
    + ( ( p.y() > rect.bottom() ) ? MnMath::BOT_CROSS : 0 );       
}

/*!
 * \brief Алгоритм Коэна-Сазерленда
 *
 * Определение координат пересечения отрезка (beg,end)
 * с прямоугольником rect
 * \param rect Прямоугольник, точки пересечения с которым ищутся
 * \param beg Координаты начала отрезка
 * \param end Координаты окончания отрезка
 * \param cp набор координат пересечения rect и (beg,end)
 * \return -1 - если не пересекаются,
 *  0 - если пересекаются,
 *  1 - если линия внутри прямоугольника
 */
int MnMath::rectCrossing( const QRect& rect, const QPoint& b, const QPoint& e, CrossPoints* cp ) {
  QPoint beg(b);
  QPoint end(e);
  int begCC = rectCrossCode( rect, beg );
  int endCC = rectCrossCode( rect, end );
  if ( 0 == begCC + endCC ) {
    return 1;
  }
  QPoint* pnt = 0;
  int cc = 0;
  while ( begCC || endCC ) {
    if ( begCC & endCC ) {
      return -1;
    }
    if ( begCC ) {
      pnt = &beg;
      cc = begCC;
    }
    else {
      pnt = &end;
      cc = endCC;
    }
    if ( beg.x() != end.x() ) {
      if ( cc & MnMath::LEFT_CROSS ) {
        pnt->setY( pnt->y() + ( beg.y() - end.y() ) * ( rect.left() - pnt->x() ) / ( beg.x() - end.x() ) );
        pnt->setX( rect.left() );
        if ( 0 != cp ) {
          cp->insert( MnMath::LEFT_CROSS, *pnt );
        }
      }
      else if ( cc & MnMath::RIGHT_CROSS ) {
        pnt->setY( pnt->y() + ( beg.y() - end.y() ) * ( rect.right() - pnt->x() ) / ( beg.x() - end.x() ) );
        pnt->setX( rect.right() );
        if ( 0 != cp ) {
          cp->insert( MnMath::RIGHT_CROSS, *pnt );
        }
      }
    }
    if ( beg.y() != end.y() ) {
      if ( cc & MnMath::BOT_CROSS ) {
        pnt->setX( pnt->x() + ( beg.x() - end.x() ) * ( rect.bottom() - pnt->y() ) / ( beg.y() - end.y() ) );
        pnt->setY( rect.bottom() );
        if ( 0 != cp ) {
          cp->insert( MnMath::BOT_CROSS, *pnt );
        }
      }
      else if ( cc & MnMath::TOP_CROSS ) {
        pnt->setX( pnt->x() + ( beg.x() - end.x() ) * ( rect.top() - pnt->y() ) / ( beg.y() - end.y() ) );
        pnt->setY( rect.top() );
        if ( 0 != cp ) {
          cp->insert( MnMath::TOP_CROSS, *pnt );
        }
      }
    }
    if ( cc == begCC ) {
      begCC = rectCrossCode( rect, beg );
    }
    else {
      endCC = rectCrossCode( rect, end );
    }
  }
  return 0;
}
