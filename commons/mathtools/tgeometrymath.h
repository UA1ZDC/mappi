#ifndef TGEOMETRYMATH
#define TGEOMETRYMATH

#include <qpolygon.h>
#include <qpoint.h>
#include <qhash.h>

namespace MnMath {

  enum CrossType {      //!< Тип пересечения линий
    PARALLEL    = 0, //!< отрезки лежат на параллельных прямых
    SAMELINE    = 1, //!< отрезки лежат на одной прямой
    ONBOUNDS    = 2, //!< прямые пересекаются в конечных точках отрезков
    INBOUNDS    = 3, //!< прямые пересекаются в   пределах отрезков
    OUTBOUNDS   = 4,  //!< прямые пересекаются вне пределов отрезков
    ERROR_CROSS_PARAM = 5 //!< в функцию crossing переданы неверные параметры
  };

  enum RectCrossCode {  //!< Тип пересчения прямоугольника
    LEFT_CROSS  = 1,    //!< пересечение слева
    RIGHT_CROSS = 2,    //!< пересечение справа
    TOP_CROSS   = 4,    //!< пересечение сверху
    BOT_CROSS   = 8     //!< пересечение снизу
  };

  typedef QHash< RectCrossCode, QPoint > CrossPoints;

  bool oprTochPeresech( const QPolygonF& kr1, const QPolygonF& kr2, QPolygonF* pointPeresech );
  CrossType crossing( const QPointF& p11, const QPointF& p12,
      const QPointF& p21, const QPointF& p22, QPointF* resPoint );

  bool crossing( const QPointF& p11, const QPointF& p12,
      const QPointF& p21, const QPointF& p22 );

  int rectCrossing( const QRect& rect, const QPoint& beg, const QPoint& end, CrossPoints* cp = 0 );
  int rectCrossCode( const QRect& rect, const QPoint& p );

}


#endif

