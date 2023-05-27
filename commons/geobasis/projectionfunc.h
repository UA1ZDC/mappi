#ifndef COMMONS_GEOBASIS_PROJECTIONFUNC_H
#define COMMONS_GEOBASIS_PROJECTIONFUNC_H

#include "projection.h"

namespace meteo
{
int peresech(const QPolygon& scrPt);
int nearPointPos(const Projection &aprj, const QPolygon &ar, const QPoint &pos, GeoPoint *fl, int *st);
int provGeo(const Projection& projection, const QPolygon &ar) ;

  //!преобразование геодезических координат в плоские прямоугольные (проекция Гаусса-Крюгера, СК-95)
  void latlon2xy_gkgost( float lat, float lon, int n, float* x, float* y );
  //!преобразование плоских прямоугольных координат (проекция Гаусса-Крюгера, СК-95) в геодезические
  void xy2latlon_gkgost( float x, float y, float* lat, float* lon );


  //-----
  //TODO это, возможно, должен быть свой класс от Projection
  struct ProjectionParams {
    float a;			//!< Большая полуось эллипсоида
    float e;			//!< Первый эксцентриситет
    float la0;			//!< Долгота центрального меридиана
    float fi0;			//!< Широта центрального меридиана
    float k0;			//!< Масштабный коэффициент у экватора или центральной праллели
    float fe;			//!< Ложный сдвиг на восток
    float fn;			//!< Ложный сдвиг на север
    ProjectionParams():a(0), e(0), la0(0), fi0(0), k0(0), fe(0), fn(0) {
    }
  };

  //!  Mercator (1SP)
  void xy2f_merc1sp(const ProjectionParams& pr, float east, float north, float* lat, float* lon);
  //!  Mercator (1SP)
  void f2xy_merc1sp(const ProjectionParams& pr, float lat, float lon, float* east, float* north);

  //!  Polar Stereographic
  void f2xy_stereo_polar(const ProjectionParams& pr, float lat, float lon, float* east, float* north);


}

#endif

