#ifndef COMMONS_GEOM_GEOM_H
#define COMMONS_GEOM_GEOM_H

#include <qrect.h>
#include <qpolygon.h>
#include <qline.h>

namespace meteo {
namespace geom {

const float kErrorCalcDistanceValue(-999999999);

//! Расстояние от точки pnt до line.
//! Если cross != 0, тогда в нее запишется точка линии, до которой посчиталось расстояние
//! Если полигон poly замкнутый, а точка pnt внутри него, то расстояние отрицательное
int distance( const QPoint& pnt, const QLine& line, QPoint* cross = 0 );

//! Расстояние от точки pnt до полигона poly.
//! Если cross != 0, тогда в нее запишется точка полигона, до которой посчиталось расстояние
//! Если полигон poly замкнутый, а точка pnt внутри него, то расстояние отрицательное
int distance( const QPoint& pnt, const QPolygon& poly, QPoint* cross = 0 );

//! Расстояние от точки pnt до прямоугольника rect.
//! Если cross != 0, тогда в нее запишется точка прямоугольника, до которой посчиталось расстояние
//! Если точка pnt внутри прямоугольника rect, то расстояние отрицательное
int distance( const QPoint& pnt, const QRect& rect, QPoint* cross = 0 );

//! Определяет точку пересечения pnt двух отрезков l1 и l2
//! Если пересекаются, вернется true, иначе - false
bool crossPoint( const QLine& l1, const QLine& l2, QPoint* pnt = 0 );

//! Определяет точки пересечения отрезка l1 полигона poly
//! anglelist - список углов наклона отрезков poly, которые пересекаются с l1
//! Вернется список точек пересечения
QList<QPoint> crossPoint( const QLine& l1, const QPolygon& poly, QList<float>* anglelist = 0 );

//! Определяет точки пересечения полигонов poly1 и poly2
//! Вернется список точек пересечения
//! ang1list и ang2list - список углов наклона пересекающихся отрезков poly1 и poly2
QList<QPoint> crossPoint( const QPolygon& poly1, const QPolygon& poly2, QList<float>* ang1list = 0, QList<float>* ang2list = 0 );

}
}

#endif
