#include "projection.h"

#include <qmath.h>

#include "stereoproj.h"
#include "merkatproj.h"
#include "generalproj.h"
#include "conichproj.h"
#include "obliquemerkator.h"

namespace meteo
{

Projection* Projection::createProjection( ProjType t )
{
  return createProjection( t, GeoPoint( 0.0, 0.0 ) );
}

Projection* Projection::createProjection( ProjType t, const GeoPoint& center )
{
  switch (t) {
    case MERCAT:
      return new meteo::MerkatProj(center);
    case CONICH:
      return new meteo::ConichProj(center);
    case GENERAL:
      return new meteo::GeneralProj(center);
    case STEREO:
      return new meteo::StereoProj(center);
    case OBLIQUEMERKAT: {
      ObliqueMerkat* proj = new meteo::ObliqueMerkat;
      proj->setMapCenter( center,  0.0 );
      return proj;
    }
    case NONE_PROJ:
    default:
      return nullptr;
  }

  return nullptr;
}





Projection::Projection( const GeoPoint& agpoint )
  : xfactor_(1.0),
  yfactor_(1.0)
{
  Projection::init();
  centerMap_ = agpoint;
  if ( centerMap_.lat() < 0.0f ) {
    hemisphere_ = Southern;
  }
  else {
    hemisphere_ = Northern;
  }
}

Projection::Projection()
  : xfactor_(1.0),
  yfactor_(1.0)
{
  Projection::init();
  centerMap_ = GeoPoint( .0, .0, .0, LA180 );
}

bool Projection::X2F( const QPolygon& poly, GeoVector* gv ) const
{
  for ( int i = 0, sz = poly.size(); i < sz; ++i ) {
    GeoPoint gp;
    if ( true == X2F_one( poly[i], &gp ) ) {
      gv->append(gp);
    }
  }
  return true;
}

void Projection::init()
{
  type_ = NONE_PROJ;
  hemisphere_ = Northern;
  projRadius = kRVS;
  projRadius_1 = 1.f/projRadius;         //!< Радиус сферы (м-1)
  projRadius_MPI2 = projRadius * M_PI_2f;         //!< Радиус сферы (м)* M_PI_2

}

bool Projection::isEqual( const Projection& p ) const
{
  if ( centerMap_   != p.centerMap_
    || hemisphere_  != p.hemisphere_
    || type_        != p.type_
    || projRadius   != p.projRadius
    || ramka_       != p.ramka_
    || xfactor_     != p.xfactor_
    || yfactor_     != p.yfactor_
    || start_       != p.start_
    || end_         != p.end_
    || funcF2X_one_ != p.funcF2X_one_
    || funcX2F_one_ != p.funcX2F_one_
    || funcF2X_onef_ != p.funcF2X_onef_
    || funcX2F_onef_ != p.funcX2F_onef_ ) {
    return false;
  }
  return true;
}

Projection* Projection::copyTo( Projection* proj ) const
{
  if ( nullptr == proj ) {
    return nullptr;
  }
  proj->centerMap_   = centerMap_;
  proj->hemisphere_  = hemisphere_;
  proj->type_        = type_;
  proj->projRadius   = projRadius;
  proj->projRadius_1   = projRadius_1;
  proj->projRadius_MPI2  = projRadius_MPI2;
  proj->ramka_       = ramka_;
  proj->xfactor_     = xfactor_;
  proj->yfactor_     = yfactor_;
  proj->start_       = start_;
  proj->end_         = end_;
  proj->funcF2X_one_ = funcF2X_one_;
  proj->funcX2F_one_ = funcX2F_one_;
  proj->funcF2X_onef_ = funcF2X_onef_;
  proj->funcX2F_onef_ = funcX2F_onef_;
  return proj;
}

bool Projection::isEqual( Projection* proj ) const
{
  return isEqual(*proj);
}

void Projection::setMapCenter( const GeoPoint& gpnt )
{
  centerMap_ = gpnt;
  if ( centerMap_.lat() < 0.0f ) {
    hemisphere_ = Southern;
  }
  else {
    hemisphere_ = Northern;
  }
  initRamka();
}

bool Projection::isInMap(const GeoPoint&){
  return true;
}


}
