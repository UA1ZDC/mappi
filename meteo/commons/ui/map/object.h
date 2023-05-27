#ifndef METEO_COMMONS_UI_MAP_OBJECT_H
#define METEO_COMMONS_UI_MAP_OBJECT_H

#include <unordered_set>

#include <qmap.h>
#include <qpen.h>
#include <qbrush.h>
#include <qfont.h>
#include <qpainterpath.h>
#include <qdatastream.h>

#include <cross-commons/includes/tcommoninclude.h>
#include <commons/geobasis/geovector.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/ui/map/ptkppformat.h>

#include <meteo/commons/global/common.h>

namespace meteo {

class Projection;

namespace map {

class Ramka;

class Layer;
class Property;
class Document;
class GeoPolygon;
class GeoText;
class Label;

class Object
{
  public:
    static int OBJECT_COUNTER;

    Object( Layer* layer );
    Object( Object* parent );
    Object( Projection* proj );
    Object( const meteo::Property& prop = meteo::Property() );
    virtual ~Object();

    enum {
      Type = 0,
      UserType = 65536
    };
    virtual int type() const { return Type; }

    virtual bool isLabel() const { return false; }

    virtual Object* copy( Layer* l ) const = 0;
    virtual Object* copy( Object* o ) const = 0;
    virtual Object* copy( Projection* proj ) const = 0;

    void updateProjection();

    const QString& uuid() const { return uuid_; }

    bool hasChilds() const { return (objects_.size() != 0 ); }
    const std::unordered_set<Object*>& childs() const { return objects_; }
    template <class T> std::unordered_set<T> childsByType( bool childs = false ) const ;
    QList<Label*> labels( bool childs = false ) const ;

    virtual bool visible( float scalepower ) const ;
    bool onDocument() const ;

    bool render( QPainter* painter, const QRect& target, Document* document );
    virtual bool render( QPainter* painter, const QRect& target, const QTransform& transform ) = 0;
    virtual QList<QRect> boundingRect( const QTransform& transform ) const = 0 ;

    bool containingInRect( const QRect& rect, const QTransform& transform ) const ;
    virtual QList<GeoVector> skeletInRect( const QRect& rect, const QTransform& transform ) const  = 0;

    virtual void setSkelet( const GeoVector& skel );
    virtual void setSkelet( const GeoPoint& gp );
    const GeoVector& skelet() const { return skelet_; }

    const QVector<QPolygon>& cartesianSkelet() const { return cartesian_points_; }

    virtual void setProperty( const meteo::Property& prop );
    const meteo::Property& property() const { return property_; }

    Document* document() const ;
    Layer* layer() const ;
    Object* parent() const { return parent_; }

    void setParent( Object* parent );
    void setParent( Layer* parent );

    virtual bool underGeoPoint( const GeoPoint& gp ) const ;
    virtual int minimumScreenDistance( const QPoint& pos, QPoint* cross = 0 ) const = 0;

    virtual bool hasValue() const = 0;
    virtual float value() const = 0;
    virtual void setValue( float val, const QString& format = "4' '.2'0'", const QString& unit = QString()  ) = 0;
    virtual void removeValue() {}

    std::unordered_set<Object*> objectsByValue( float val ) const ;
    std::unordered_set<GeoPolygon*> polygonsByValue( float val ) const ;
    std::unordered_set<GeoText*> textsByValue( float val ) const ;
    std::unordered_set<Object*> objectsByType( int val ) const ;

    Projection* projection() const ;
    bool isClosed()const;
    bool isObrez()const;//если на обрезе

    void setVisible( bool fl );
    void setGeneral( const General& general );
    void setGeneral( const meteo::Generalization& general );
    void setZlevel( int32_t zlevel );
    void setPen( const Pen& pen );
    void setPen( const QPen& pen );
    void setPenStyle( Qt::PenStyle style );
    void setPenWidth( int width );
    void setPenColor( const QColor& color );
    void setBrush( const Brush& brush );
    void setBrush( const QBrush& brush );
    void setBrushStyle( Qt::BrushStyle style );
    void setBrushColor( const QColor& color );
    void setSplineFactor( int32_t splinefactor );
    void setClosed( bool fl );
    void setDoubleLine( bool fl );
    void setArrowPlace( ArrowPlace arrowplace );
    void setArrowType( ArrowType arrowtype );
    void setPos( Position pos );
    void setPos( Qt::AlignmentFlag pos );
    void setPosOnParent( Position pos );
    void setPosOnParent( Qt::AlignmentFlag pos );
    void setDrawOnOnEnds( bool fl );
    void setDrawAlways( bool fl );
    void setFont( const Font& font );
    void setFont( const QFont& font );
    void setFontFamily( const QString& family );
    void setFontPointSize( int pointsize );
    void setFontWeight( int fonweight );
    void setFontBold( bool bold );
    void setFontItalic( bool italic );
    void setFontUnderline( bool underline );
    void setRamka( TextRamka ramka );
    void setRotateAngle( float angle );
    void setScaleXy( const PointF& scale );
    void setScaleXy( const QPointF& scale );
    void setScale( float dx, float dy );
    void setScale( float dxy );
    void setTranslateXy( const Point& pnt );
    void setTranslateXy( const QPoint& pnt );
    void setTranslate( int dx, int dy );
    void setTranslate( int dxy );
    void setTextScale( TextScale scale );
    void setOriginScale( float scale );
    void setOutlineColor( uint32_t color );
    void setOutlineColor( const QColor& color );
    void setAlphaPercent( int alphapercent );
    void setPriority( int priority );
    void setOrnament( const QString& name );
    void setOrnamentMirror( bool mirror );
    void setDrawOnRamka( bool fl );
    void setGeolineStartShift( int pixnum );
    void setGeolineEndShift( int pixnum );
    bool setCartesianSkelet(const QPolygon&);

    bool visible() const ;
    const Generalization& general() const ;
    int32_t zlevel() const ;
    Pen pen() const ;
    QPen qpen() const ;
    Brush brush() const ;
    QBrush qbrush() const ;
    int32_t spinefactor() const ;
    bool closed() const ;
    bool doubleLine() const ;
    ArrowPlace arrowPlace() const ;
    ArrowType arrowType() const ;
    Position pos() const ;
    int qpos() const ;
    Position posOnParent() const ;
    int qposOnParent() const ;
    bool drawOnOnEnds() const ;
    bool drawAlways() const ;
    const Font& font() const ;
    QFont qfont() const ;
    TextRamka ramka() const ;
    float rotateAngle() const ;
    const PointF& scaleXy() const ;
    QPointF qscaleXy() const ;
    const Point& translateXy() const ;
    QPoint qtranslateXy() const ;
    TextScale textScale() const ;
    float originScale() const ;
    uint32_t outlineColor() const ;
    QColor qoutlineColor() const ;
    int geolineStartShift() const ;
    int geolineEndShift() const ;

    int alphapercent() const { return property_.alphapercent(); }
    int alpha() const { return (100 - property_.alphapercent()/100.0)*255; }

    int priority() const ;

    bool drawOnRamka() const ;

    bool hasOrnament() const ;
    QString ornament() const ;
    bool ornamentMirror() const ;

    bool wasdrawed() const { return wasdrawed_; }

    void setProperty( const QString& name, const QString& value );
    QString property( const QString& name ) const ;

    const QVector<QPolygon>& cachedScreenPoints() const { return cached_screen_points_; }

    virtual int32_t dataSize() const ;
    int32_t data( char* arr ) const ;
    int32_t setData( const char* arr );

    int32_t childDataSize() const ;

    virtual int32_t serializeToArray( char* data ) const ;
    virtual int32_t parseFromArray( const char* data );

    int32_t serializeChildsToArray( char* data ) const ;
    int32_t parseChildsFromArray( const char* data );

    const std::string& propertyString() const ;

    virtual void prepareChange() {}

  protected:
    QString uuid_;
    meteo::Property property_;
    meteo::Generalization generalization_;
    Layer* layer_;
    Object* parent_;
    std::unordered_set<Object*> objects_;

    mutable QVector<QPolygon> cached_screen_points_; //!< векторы экранных координат, полученных при отрисовке на посл. документе

    GeoVector skelet_; //!< векторы географических координат
    QVector<QPolygon> cartesian_points_; //!< векторы прямоугольных координат

    Projection* projection_;
    Document* curdoc_;
    bool wasdrawed_;
    mutable std::string cached_property_string_;

    bool calcCartesianPoints();

  private:
    void addObject( Object* o );
    void rmObject( Object* o );

    DISALLOW_EVIL_CONSTRUCTORS(Object);

  friend class Layer;
  friend class Ramka;
  friend class Document;
};

}
}

template <class T> inline T mapobject_cast( meteo::map::Object* o )
{
  return int(static_cast<T>(0)->Type) == int(meteo::map::Object::Type)
    || ( 0 != o && int(static_cast<T>(0)->Type) == o->type() ) ? static_cast<T>(o) : 0;
}

template <class T> inline T mapobject_cast(const meteo::map::Object* o )
{
  return int(static_cast<T>(0)->Type) == int(meteo::map::Object::Type)
    || ( 0 != o && int(static_cast<T>(0)->Type) == o->type() ) ? static_cast<T>(o) : 0;
}

namespace meteo {
namespace map {

template <class T> std::unordered_set<T> Object::childsByType( bool in_childs ) const
{
  std::unordered_set<T> list;
  for (auto itr = objects_.begin(); itr != objects_.end(); ++itr) {
  Object* o = (*itr);
//  foreach(, objects_){
    T t = mapobject_cast<T>(o);
    if ( 0 != t ) {
      list.insert(t);
    }
    if ( true == in_childs ) {
      list.insert( o->childsByType<T>(in_childs).begin(),o->childsByType<T>(in_childs).end() );
    }
  }
  return list;
}

}
}

#endif
