#ifndef METEO_COMMONS_UI_MAP_LAYERGRID_H
#define METEO_COMMONS_UI_MAP_LAYERGRID_H

#include "layer.h"

namespace meteo {
namespace map {

class LayerGrid : public Layer
{
  public:
    LayerGrid( Document* map, const QString& name = QString() );
    ~LayerGrid();

    enum {
      Type = kLayerGrid
    };
    virtual int type() const { return Type; }

    std::unordered_set<Object*> latitudes() const { return latitudes_; }
    std::unordered_set<Object*> longitudes() const { return longitudes_; }

    void setLatitudes( const std::unordered_set<Object*> list ) { latitudes_ = list; }
    void setLongitudes( const std::unordered_set<Object*> list ) { longitudes_ = list; }

    void addLatitude( Object* o ) { if ( 0 == latitudes_.count(o) ) { latitudes_.insert(o); } }
    void addLongitude( Object* o ) { if ( 0 == longitudes_.count(o) ) { longitudes_.insert(o); } }

    int32_t dataSize() const ;
    int32_t serializeToArray( char* data ) const ;
    int32_t parseFromArray( const char* data );
    
  protected:
    LayerGrid( Map* map, const QString& name = QString() );

    void rmObject( Object* object );

  private:
    std::unordered_set<Object*> latitudes_;
    std::unordered_set<Object*> longitudes_;

    DISALLOW_EVIL_CONSTRUCTORS(LayerGrid);
};

}
}

#endif
