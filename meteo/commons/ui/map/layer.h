#ifndef METEO_COMMONS_UI_MAP_LAYER_H
#define METEO_COMMONS_UI_MAP_LAYER_H

#include <qstring.h>
#include <qimage.h>

#include <cross-commons/includes/tcommoninclude.h>
#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/ui/map/object.h>


namespace obanal {
class TField;
}

namespace meteo {

class Projection;

namespace map {

class Map;
class GeoPolygon;
class GeoText;
class Property;
class Document;
class Legend;
class Label;

struct valLabel{
  valLabel(){
    name = QString();
    val  = QString();
    unit = QString();
    level = QString();
    date = QString();
    center = QString();
    hour = QString();
  }

  QString name;
  QString level;
  QString date;
  QString val;
  QString unit;
  QString center;
  QString hour;
};

class LayerMenu;

enum LayerDataType{
  noData,
  fieldData,
  srcData
};

class Layer
{
public:
  static int LAYER_COUNTER;
  Layer( Document* map, const QString& name = QString(), int customtype = -1 );
  virtual ~Layer();

  enum {
    Type = kLayerCommon,
    UserType = 65536
  };
  virtual int type() const { return Type; }

  bool copyFrom( Layer* layer );

  void updateProjection();

  virtual void render( QPainter* painter, const QRect& target, Document* document );

  Document* document() const { return document_; }

  bool isBase() const ;
  void setBase( bool fl ) { isbase_ = fl; }

  const QString& uuid() const { return uuid_; }
  QString name() const;
  //const QString& shortName() const { return shortName_; }

  const std::unordered_set<Object*>& objects() const { return objects_; }
  Object* objectByUuid( const QString& uuid ) const ;

  QList<Label*> labels( bool childs = false ) const ;

  template <class T> std::unordered_set<T> objectsByType( bool in_childs = false ) const ;

  int size() const { return objects_.size(); }
  Projection* projection() const;
  bool visible() const { return visible_; }

  obanal::TField* field() const { return field_; }
  bool hasField() const { return (nullptr != field_); }
  bool hasData() const ;
  virtual float fieldValue( const GeoPoint& gp, bool* ok = nullptr ) const ;
  virtual QString valuePodpis( float value, bool* ok = nullptr ) const ;

  QStringList uuidsUnderGeoPoint( const GeoPoint& gp ) const ;
  QList<Object*> objectsUnderGeoPoint( const GeoPoint& gp ) const ;

  QList<Object*> objectsByValue( float val ) const ;
  QList<GeoPolygon*> polygonsByValue( float val ) const ;
  QList<GeoText*> textsByValue( float val ) const ;

  void setName( const QString& n );
  // void setShotName(const QString& n ) {shortName_ = n;}
  void setVisisble( bool fl );
  void setActive();
  virtual void setField( obanal::TField* f );

  void show() { visible_ = true; }
  void hide() { visible_ = false; }

  bool isObjectInDrawedList( const Object* o ) const ;
  void addToDrawedLabels( Object* o );

  void setProperty( const QString& name, const QVariant& value );
  QVariant property( const QString& name );

  void setInfo( const proto::WeatherLayer& info );
  const proto::WeatherLayer& info() const { return info_; }

  void setAlphaPercent( int prcnt );
  int alphaPercent() const { return alphapercent_; }
  int alpha() const { return ((100. - alphapercent_/100.0)*255.); }

  void repaint();

  virtual bool hasValue() const { return false; }
  virtual valLabel valueLabel() const { return valLabel(); }

  bool isEqual( Layer* l ) const ;
  bool isEqual( const proto::WeatherLayer& info ) const ;
  bool isEqual( const field::DataDesc& info ) const ;

  virtual int32_t dataSize() const ;
  virtual int32_t serializeToArray( char* data ) const ;
  virtual int32_t parseFromArray( const char* data );

  int32_t childDataSize() const ;
  int32_t serializeChildsToArray( char* data ) const ;
  int32_t parseChildsFromArray( const char* data );

  virtual LayerMenu* layerMenu();

  bool cache() const { return cache_; }
  void setCache( bool fl );

  void setCustomType( int ct ) { customtype_ = ct; }
  int customType() const { return customtype_; }

  void setLegendImage( const QImage& image ) { legend_ = image; }
  QImage legendImage() const { return legend_; }
  bool hasLegendImage() const { return false == legend_.isNull(); }

  void removeAllObjects();
  template <class T> void removeObjectsByType(  );

protected:
  Document* document_;
  QString uuid_;
  QString name_;
  std::unordered_set<Object*> objects_;

  // QList<Object*> objects_;
  bool visible_;
  obanal::TField* field_;
  QString shortName_;
  bool isbase_;
  QMap<QString, QVariant> property_;

  int32_t alphapercent_;

  proto::WeatherLayer info_;

  std::unordered_set<const Object*> drawedlabels_;
  //QList<const Object*> drawedlabels_;

  LayerMenu* menu_;

  bool cache_ = false;
  int customtype_ = -1;
  LayerDataType layer_data_type_;
  QImage legend_;
  virtual void addObject( Object* object );
  virtual void rmObject( Object* object );

private:
  DISALLOW_EVIL_CONSTRUCTORS(Layer);

  friend class Object;
  friend class Document;
  friend class Map;
};

}
}

template <class T> inline T maplayer_cast( meteo::map::Layer* l )
{
  return int(static_cast<T>(0)->Type) == int(meteo::map::Layer::Type)
      || ( nullptr != l && int(static_cast<T>(0)->Type) == l->type() ) ? static_cast<T>(l) : nullptr;
}

template <class T> inline T maplayer_cast(const meteo::map::Layer* l )
{
  return int(static_cast<T>(0)->Type) == int(meteo::map::Layer::Type)
      || ( nullptr != l && int(static_cast<T>(0)->Type) == l->type() ) ? static_cast<T>(l) : nullptr;
}

namespace meteo {
namespace map {

template <class T> std::unordered_set<T> Layer::objectsByType( bool in_childs ) const
{
  std::unordered_set<T> list;
  for (auto o = objects_.begin(); o != objects_.end(); ++o) {
  //foreach(Object* o, objects_){
    // for ( int i = 0, sz = objects_.size(); i < sz; ++i ) {
    //   Object* o = objects_[i];
    T t = mapobject_cast<T>(*o);
    if ( 0 != t ) {
      list.insert(t);
    }
    if ( true == in_childs ) {
      //list.insert( o->childsByType<T>(in_childs) );
      list.insert( (*o)->childsByType<T>(in_childs).begin(),(*o)->childsByType<T>(in_childs).end() );
    }
  }
  return list;
}

template <class T> void Layer::removeObjectsByType(  )
{
  std::unordered_set<Object*>::iterator it = objects_.begin();
  std::unordered_set<Object*>::iterator eit = objects_.end();

  while (it != eit) {
    T t = mapobject_cast<T>(*it);
    if ( nullptr != t ) {
      it = objects_.erase(it);
      delete t;
    } else{
      ++it;
    }
  }


}
}
}

#endif
