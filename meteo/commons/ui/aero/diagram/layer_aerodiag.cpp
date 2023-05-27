#include "layer_aerodiag.h"

#include <qdebug.h>
#include <quuid.h>
#include <qpainter.h>

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/zond/placedata.h>

//#include "map.h"
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/geotext.h>

namespace meteo {
namespace map {

LayerAeroDiag::LayerAeroDiag( Document* d, const QString& n )
: Layer(d, n)
{


}

LayerAeroDiag::~LayerAeroDiag()
{

}

void LayerAeroDiag::setPlaceData(const zond::PlaceData &apd,  const QVector<double> &levels){
  place_ = apd;
  TMeteoData md;
  for ( int z = 0, sz = levels.count(); z < sz; ++z ) {
    double aval = levels.at(z);
    if(!place_.getMeteoData(aval,&md)) { continue;}
    //md.printData();
    meteo::map::Puanson* p = new meteo::map::Puanson(this);
    if(!p->setPunch(":/verticalcut/level.puan")){
      debug_log<<"не найден шаблон :/verticalcut/level.puan";
      continue;
    }
    p->setMeteodata(md);
    p->setSkelet(GeoPoint(place_.xPos(), aval, 0.,meteo::LA_GENERAL));
  }
}

int32_t LayerAeroDiag::dataSize() const
{
  int32_t pos = Layer::dataSize();

  pos += place_.dataSize();

  return pos;
}

int32_t LayerAeroDiag::serializeToArray( char* arr ) const
{
  int32_t pos = Layer::serializeToArray(arr);
  pos += place_.serializeToArray( arr + pos );
  return pos;
}

int32_t LayerAeroDiag::parseFromArray( const char* data )
{
  int32_t pos = Layer::parseFromArray(data);
  pos += place_.parseFromArray( data + pos );
  return pos;
}

}
}
