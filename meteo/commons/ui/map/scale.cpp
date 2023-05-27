#include "scale.h"

#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/geopolygon.h>

namespace meteo {
namespace map {

Scale::Scale(const ScaleOptions& opt)
{
  opt_ = opt;
}

GeoVector Scale::body() const
{
  GeoPoint p = opt_.point;
  if ( Qt::Horizontal == opt_.orientation ) {
    p.setLat(p.lat() + opt_.length);
  }
  else {
    p.setLon(p.lon() + opt_.length);
  }

  GeoVector gv;
  gv << opt_.point << p;
  return gv;
}

GeoPoint Scale::tickPoint(float value) const
{
  return getPoint(value);
}

GeoPoint Scale::tickLabel(float value, float padding) const
{
  if ( opt_.tickSide & ScaleOptions::kLeftSide )  { padding = -padding; }

  GeoPoint gp = getPoint(value);
  if ( Qt::Horizontal == opt_.orientation ) {
    gp.setLon(gp.lon() + padding);
  }
  else {
    gp.setLat(gp.lat() + padding);
  }
  return gp;
}

GeoVector Scale::tick(float value, float size) const
{
  GeoPoint p1 = tickPoint(value);

  float lOffset = 0;
  float rOffset = 0;

  if ( opt_.tickSide & ScaleOptions::kLeftSide )  { lOffset = -size; }
  if ( opt_.tickSide & ScaleOptions::kRightSide ) { rOffset = size; }

  GeoPoint gp1 = p1;
  GeoPoint gp2 = p1;

  if ( Qt::Horizontal == opt_.orientation ) {
    gp1.setLon(gp1.lon() + lOffset);
    gp2.setLon(gp2.lon() + rOffset);
  }
  else {
    gp1.setLat(gp1.lat() + lOffset);
    gp2.setLat(gp2.lat() + rOffset);
  }

  GeoVector gv;
  gv << gp1 << gp2;
  return gv;
}

map::GeoGroup* Scale::createObject(map::Layer* layer) const
{
  if ( 0 == layer ) { return 0; }

  map::GeoGroup* gr = new map::GeoGroup(layer);
  map::GeoPolygon* bodyObj = new map::GeoPolygon(gr);
  bodyObj->setSkelet(body());

  for ( int i=0,isz=opt_.tickValues.size(); i<isz; ++i )
  {
    if ( ScaleOptions::kNoneSide != opt_.tickSide ) {
      map::GeoPolygon* t = new map::GeoPolygon(gr);
      t->setSkelet(tick(opt_.tickValues[i]));
    }

    if ( opt_.showLabel & ScaleOptions::kLeftSide ) {
      GeoVector gv;
      gv << tickLabel(opt_.tickValues[i], 15);

      Position pos = isHorizontal() ? kTopCenter : kLeftCenter;

      map::GeoText* text = new map::GeoText(gr);
      text->setText(opt_.tickLabels[i]);
      text->setDrawAlways(true);
      text->setSkelet(gv);
      text->setPos(pos);
      text->setFontPointSize(8);
    }
    if ( opt_.showLabel & ScaleOptions::kRightSide ) {
      GeoVector gv;
      gv << tickLabel(opt_.tickValues[i], 15);

      Position pos = isHorizontal() ? kBottomCenter : kRightCenter;

      map::GeoText* text = new map::GeoText(gr);
      text->setText(opt_.tickLabels[i]);
      text->setDrawAlways(true);
      text->setSkelet(gv);
      text->setPos(pos);
      text->setFontPointSize(8);
    }
  }

  return gr;
}

GeoPoint Scale::getPoint(float value) const
{
  GeoPoint p = opt_.point;
  if ( Qt::Horizontal == opt_.orientation ) {
    p.setLat(value);
  }
  else {
    p.setLon(value);
  }
  return p;
}

} // cut
} // meteo
