#include "layervprofile.h"

#include <qdebug.h>
#include <quuid.h>
#include <qpainter.h>

#include <cross-commons/debug/tlog.h>

#include <commons/obanal/tfield.h>
#include <commons/geobasis/projection.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/proto/puanson.pb.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/global/weatherloader.h>

namespace meteo {
namespace map {

namespace {
  Layer* createLayer( Document* d )
  {
    return new LayerVProfile(d);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerLayerHandler( LayerVProfile::Type, createLayer );
}

LayerVProfile::LayerVProfile( Document* d, const QString& n )
  : LayerPunch(d)
{
  QMap< QString, meteo::puanson::proto::Puanson > map = WeatherLoader::instance()->punchlibraryspecial();
  setPunch(map["vcutl_levels"]);
  setName(n);
}

void LayerVProfile::setPlaceData(const QList<zond::PlaceData> &apd, const QVector<float> &levels){
//  places_ = apd;

  QMap< QString, meteo::puanson::proto::Puanson > map = WeatherLoader::instance()->punchlibraryspecial();
  const meteo::puanson::proto::Puanson& tmpl = map["vcutl_levels"];

  for ( int x = 0, sx = apd.count(); x < sx; ++x ) {
    for ( int z = 0, sz = levels.count(); z < sz; ++z ) {
      float aval = levels.at(z);
      TMeteoData md;
      if(!apd.at(x).getMeteoData(aval,&md)) { continue;}
     // md.printData();
      meteo::map::Puanson* p = new meteo::map::Puanson(this);
      p->setPunch(tmpl);
       // debug_log<<"не найден шаблон наноски по уровням для  вертикального разреза";
      p->setMeteodata(md);
      meteo::GeoVector gv;
      gv.append(GeoPoint(apd.at(x).xPos(), aval, 0.,meteo::LA_GENERAL));
      // debug_log<<gv;
      // md.printData();

      p->setSkelet(gv);
    }
  }
}

void LayerVProfile::addWind(const QList<zond::PlaceData>& data, const QVector<float>& levels)
{
  for ( int x=0,xsz=data.count(); x<xsz; ++x ) {
    for ( int z=0,zsz=levels.count(); z<zsz; ++z ) {
      float P = levels.at(z);

      zond::Uroven ur;
      if ( !data.at(x).zond().getUrPoP(P,&ur) ) { continue; }

      TMeteoData md;

      TMeteoParam paramdd("", ur.value(zond::UR_dd), static_cast<control::QualityControl>(ur.quality(zond::UR_dd)));
      md.set("dd", paramdd);

      TMeteoParam paramff("", ur.value(zond::UR_ff), static_cast<control::QualityControl>(ur.quality(zond::UR_ff)));
      md.set("ff", paramff);

      QMap< QString, meteo::puanson::proto::Puanson > map = WeatherLoader::instance()->punchlibraryspecial();
      const meteo::puanson::proto::Puanson& tmpl = map["vcutl_levels"];

      meteo::map::Puanson* p = new meteo::map::Puanson(this);
      p->setDrawAlways(false);
      p->setPunch(tmpl);
      p->setMeteodata(md);
      meteo::GeoVector gv;
      gv.append(GeoPoint(data.at(x).xPos(), P, 0, meteo::LA_GENERAL));
      p->setSkelet(gv);
    }
  }
}

void LayerVProfile::addWind(LayerIso* uuLayer, LayerIso* vvLayer, const GeoPoint& coord, float angle)
{
  if ( 0 == uuLayer || 0 == vvLayer ) { return; }
  if ( 0 == uuLayer->field() || 0 == vvLayer->field() ) { return; }

  bool ddOk = false;
  bool ffOk = false;

  float uu = uuLayer->field()->pointValue(coord, &ddOk);
  float vv = vvLayer->field()->pointValue(coord, &ffOk);

  if ( !ddOk || !ffOk ) { return; }
  float dd,ff;
  MnMath::preobrUVtoDF(uu, vv, &dd, &ff);
  dd += angle;

  TMeteoData md;

  TMeteoParam ddParam("", dd, control::RIGHT);
  md.set("dd", ddParam);

  TMeteoParam ffParam("", ff, control::RIGHT);
  md.set("ff", ffParam);

  QMap< QString, meteo::puanson::proto::Puanson > map = WeatherLoader::instance()->punchlibraryspecial();
  const meteo::puanson::proto::Puanson& tmpl = map["vcutl_levels"];

  meteo::map::Puanson* p = new meteo::map::Puanson(this);
  p->setPunch(tmpl);
  p->setMeteodata(md);

  meteo::GeoVector gv;
  gv << coord;
  p->setSkelet(gv);
}

} // map
} // meteo
