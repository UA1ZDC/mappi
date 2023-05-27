#ifndef METEO_COMMONS_UI_MAP_PTKPPFORMAT_H
#define METEO_COMMONS_UI_MAP_PTKPPFORMAT_H

#include <qmap.h>

#include <cross-commons/singleton/tsingleton.h>

namespace meteo {
namespace map {

class Object;
class Layer;
class Document;
class Map;

class PtkppFormat
{
  typedef Object* (*CreateChild)(Object*);
  typedef Object* (*CreateObject)(Layer*);
  typedef Layer* (*CreateLayer)(Document*);
  typedef Layer* (*CreateBaseLayer)(Map*);

  public:
    PtkppFormat();
    ~PtkppFormat();

    bool registerChildHandler( int type, CreateChild func );
    bool registerObjectHandler( int type, CreateObject func );
    bool registerLayerHandler( int type, CreateLayer func );
    bool registerLayerHandler( int type, CreateBaseLayer func );

    Object* createObject( int type, Layer* l ) const ;
    Object* createChild( int type, Object* o ) const ;
    Layer* createLayer( int type, Document* d ) const ;
    Layer* createLayer( int type, Map* d ) const ;

  private:
    QMap< int, CreateChild > chandlers_;
    QMap< int, CreateObject > ohandlers_;
    QMap< int, CreateLayer> lhandlers_;
    QMap< int, CreateBaseLayer> blhandlers_;
};

namespace singleton {
typedef TSingleton<meteo::map::PtkppFormat> PtkppFormat;
}

}
}

#endif
