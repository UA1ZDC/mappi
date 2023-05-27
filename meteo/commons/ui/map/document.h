#ifndef METEO_COMMONS_UI_MAP_DOCUMENT_H
#define METEO_COMMONS_UI_MAP_DOCUMENT_H

#include <qimage.h>
#include <qtransform.h>
#include <qpoint.h>
#include <qsize.h>

#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/coords.h>
#include <meteo/commons/proto/map_document.pb.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/global/common.h>

#include "map.h"
#include "incut.h"
#include "event.h"
#include "object.h"
//#include "legend.h"
#include "layer.h"

#include "freezedlabel.h"

class QGraphicsItem;

namespace meteo {
namespace map {

class Legend;

enum DocumentTypes {
  kFormal   = 1,
  kVProfile = 2,
  kAero     = 3,
  kPos      = 4
};

class LayerItems;

const int kMaxDistForNear = 20; //!< Максимальное расстояние для поиска ближайших объектов

class Ramka;
class LayerGrid;
class MapScene;

class Document
{
public:
  static int DOCUMENT_COUNTER;
  Document( const GeoPoint& mapcenter,
            const GeoPoint& doc_center,
            proto::DocumentType type,
            ProjectionType proj_type );
  Document( const proto::Document& doc );
  Document();

protected:
  Document( Document* doc, const GeoPoint& center, const QSize& docsize );

public:
  virtual ~Document();

  enum {
    Type = 0,
    UserType = 65536
  };
  virtual int type() const { return Type; }

  void setProperty( const proto::Document& doc );

  bool init( const proto::Document& doc );
  virtual bool init();
  void clearDocument();
  void clearDataLayers();

  void setName( const QString& n );
  //    void setProjection( proto::Projection type );
  //   void setLoader( const QString& loader_id );
  //изменить размер документа
  void resizeDocument( const QSize& size );
  void resizeMap( const QSize& size );
  //управление системой координат
  void setScale(double scale );
  void setMinScale( double scale );
  void setMaxScale( double scale );
  void setRotateAngle( double ang );
  void setScreenCenter( const QPointF& center );
  void setScreenCenter( const QPoint& center );
  void setScreenCenter( int x, int y );
  void setScreenCenter( const GeoPoint& center );
  void setRamkaWidth( int width );
  void setIncutProperty( const proto::IncutParams& params );
  void setMap( const proto::Map& i ) ;
  void setPuansonOrientaton( proto::PunchOrient po );

  Document* stub( const GeoPoint& center, const QSize& docsize );

  QString uuid() const { return QString::fromStdString( property_.uuid() ); }
  Projection* projection() const { return projection_; }

  QString name() const ;//{ return QString::fromStdString( info().title() ); }
  //размер карты
  QSize mapsize() const { return size2qsize( property_.mapsize() ); }
  //размер документа
  QSize documentsize() const { return size2qsize( property_.docsize() ); }
  QRect documentRect() const;
  QRect mapRect() const ;
  QRect contentRect() const ;

  //координаты углов документа на карте
  QPoint documentTopLeft() const ;
  QPoint documentTopRight() const ;
  QPoint documentBottomLeft() const ;
  QPoint documentBottomRight() const ;
  QPoint documentTop() const ;
  QPoint documentBottom() const ;
  QPoint documentLeft() const ;
  QPoint documentRight() const ;

  // функции для получения масштаба на указанном отрезке
  int mapScaleDenominatorPixels( const QPoint& pnt1, const QPoint& pnt2, int pixelcount ) const ;
  int mapScaleDenominatorMeters( const QPoint& pnt1, const QPoint& pnt2 ) const ;
  int mapScaleDenominatorPixels( const GeoPoint& pnt1, const GeoPoint& pnt2, int pixelcount ) const ;
  int mapScaleDenominatorMeters( const GeoPoint& pnt1, const GeoPoint& pnt2 ) const ;
  int santimeteresScale() const ;
  int santimeteresScaleProjCenter() const ;

  // прямоугольник указанной врезки
  QRectF incutBoundingRect( Incut* i ) const { return i->boundingRect(); }
  QRectF incutBoundingRect( int num ) const ;
  QRectF incutBoundingRect( const QString& uuid ) const ;

  //смещение центра документа от центра карты
  const QPoint& offcentermap() const { return offcentermap_; }

  //центр карты
  const GeoPoint center() const { return pbgeopoint2geopoint( property_.doc_center() ); }

  QColor backgroundColor() const ;
  void setBackgroundColor(QColor color);

  //нарисовать всю карту полностью
  bool drawDocument( QPainter* painter, const QRect& target = QRect(), const QRect& source = QRect() );
  bool drawDocument( QPainter* painter, const QList<QString>& layers, const QRect& target = QRect(), const QRect& source = QRect() );

  void drawDocTile( QPainter* pntr, const QStringList& uuids, const QRect& source );

  QString cacheHash() const ;
  QStringList cacheUuids() const ;

  //нарисовать кусочек слоя, видимый в документе c подгонкой размера (размеры картинки pixmap не изменятся)
  bool drawLayer( const QString& uuid, QPainter* painter, const QRect& target = QRect() );
  //нарисовать рамку документа
  bool drawRamka( QPainter* painer );
  //нарисовать врезку
  bool drawIncut( QPainter* painter );
  bool drawIncut( QPainter* painter, Incut* i );

  bool drawLegend( QPainter* painter );

  //установить смещение центра документа относительно центра карты
  void setOffCenterMap( const QPoint& point ) { offcentermap_ = point; }

  Ramka* ramka() const { return ramka_; }
  void setRamka( Ramka* r );

  Legend* legend() const { return legend_; }
  Legend* setLegend(Legend* legend) { Legend* l = legend_; legend_ = legend; return l; }

  //управление параметрами врезки
  Incut* incut( Position pos ) const ;
  QList<Incut*> incuts() const { return incuts_; }

  void addIncut( Incut* i ) { incuts_.append(i); }
  Incut* addIncut( Position );
  Incut* addIncut( const QPoint& incut );

  void removeIncut( Incut* i ) { incuts_.removeAll(i); }
  void removeIncut( int num );
  void removeIncut( const QString& uuid );

  void setIncutPosition( Incut* i, Position pos ) { i->setPositionOnDocument(pos); }
  void setIncutPosition( int num, Position pos );
  void setIncutPosition( const QString& uuid, Position pos );

  void setIncutPosition( Incut* i, const QPoint& pos ) { i->setPositionOnDocument(pos); }
  void setIncutPosition( int num, const QPoint& pos );
  void setIncutPosition( const QString& uuid, const QPoint& pos );

  void addLabelToIncut( Incut* incut, const QString& key, const QString& value );
  void addLabelToIncut( int num, const QString& key, const QString& value );
  void addLabelToIncut( const QString& uuid, const QString& key, const QString& value );

  //доступ к значениям преобразований системы координат
  double scale() const { return property_.scale(); }
  double rotateAngle() const { return property_.rot_angle(); }
  GeoPoint screenCenter() const ;
  const QTransform& transform() const { return transform_; }
  const QTransform& absolutetransform() const { return absolutetransform_; }

  int ramkaWidth() const { return ramkawidth_; }

  proto::PunchOrient puansonOrientation() const { return property_.punchorient(); }

  //преобразования координат ( экранные<->географические )
  QPoint coord2screen( const GeoPoint& coord, bool* fl = nullptr ) const ;
  QPointF coord2screenf( const GeoPoint& coord, bool* fl = nullptr ) const ;
  GeoPoint screen2coord( const QPoint& point, bool* fl = nullptr ) const ;
  GeoPoint screen2coord( const QPointF& point, bool* fl = nullptr ) const ;

  QPolygon coord2screen( const GeoVector& poly ) const ;
  GeoVector screen2coord( const QPolygon& poly ) const ;

  double deviationFromNorth( const QPoint& pnt ) const ; //отклонение от севера в радианах
  double deviationFromNorth( const GeoPoint& pnt ) const ; //отклонение от севера в радианах

  double deviationFromCenter( const QPoint& pnt ) const ; //отклонение от центра в радианах
  double deviationFromCenter( const GeoPoint& pnt ) const ; //отклонение от центра в радианах

  //доступ к слоям карты через документ
  int layersSize() const { return layers_.size(); }
  QList<Layer*> layers() const { return layers_; }

  LayerGrid* gridLayer() const ;
  LayerItems* itemsLayer() const ;

  bool hasLayer( const proto::WeatherLayer& layer ) const ;
  bool hasLayer( const QString& uuid ) const ;
  QString layerName( const QString& uuid ) const ;
  int layerZOrder( const QString& uuid ) const ;
  bool layerVisible( const QString& uuid ) const ;
  QString activeLayerUuid() const ;
  Layer* activeLayer() const { return activeLayer_; }
  bool isLayerActive( const QString& uuid ) const { return ( uuid == Document::activeLayerUuid() ); }
  Layer* layerByUuid( const QString& uuid ) const ;
  Layer* layerByCustomType( int t ) const ;

  bool moveLayer( const QString& uuid, int delta ) ;
  void rmLayer( const QString& uuid ) ;
  void showLayer( const QString& uuid ) ;
  void hideLayer( const QString& uuid ) ;
  void setActiveLayer(const QString& uuid) ;
  Layer* layer( const QString& uuid ) const ;

  QStringList uuidsUnderGeoPoint( const GeoPoint& gp, Layer* layer ) const ;
  QStringList uuidsUnderGeoPoint( const GeoPoint& gp, const QString& layeruuid ) const ;
  QStringList uuidsUnderGeoPoint( const GeoPoint& gp ) const ;
  QList<Object*> objectsUnderGeoPoint( const GeoPoint& gp, Layer* layer ) const ;
  QList<Object*> objectsUnderGeoPoint( const GeoPoint& gp, const QString& layeruuid ) const ;
  QList<Object*> objectsUnderGeoPoint( const GeoPoint& gp ) const ;

  QStringList uuidsUnderScreenPoint( const QPoint& point, Layer* layer ) const ;
  QStringList uuidsUnderScreenPoint( const QPoint& point, const QString& layeruuid ) const ;
  QStringList uuidsUnderScreenPoint( const QPoint& point ) const ;
  QList<Object*> objectsUnderScreenPoint( const QPoint& point, Layer* layer ) const ;
  QList<Object*> objectsUnderScreenPoint( const QPoint& point, const QString& layeruuid ) const ;
  QList<Object*> objectsUnderScreenPoint( const QPoint& point ) const ;

  //! Возвращает ближайший к gp найденный объект слоя l
  //! cross - ближайшая экранная координата объекта
  //! onlyvisible - true - искать только видимые, false - искать среди всех
  Object* objectNearGeoPoint( const GeoPoint& gp, Layer* l, QPoint* cross = nullptr, bool onlyvisible = true ) const ;
  Object* objectNearScreenPoint( const QPoint& pnt, Layer* l, QPoint* cross = nullptr, bool onlyvisible = true ) const ;
  template <class T> T* objectNearScreenPoint( const QPoint& pnt, Layer* l, QPoint* cross = nullptr, bool onlyvisible = true ) const ;
  template <class T> QList<T*> objectsNearScreenPoint( const QPoint& pnt, Layer* l, bool onlyvisible = true ) const ;

  int currentSearchDistance() const { if ( 0 == searchdistancestack_.size() ) { return kMaxDistForNear; } return searchdistancestack_.last(); }
  void setSearchDistance( int distance ) { searchdistancestack_.push_back(distance); }
  void restoreSearchDistance() { if ( 0 != searchdistancestack_.size() ) { searchdistancestack_.pop_back(); } }

  QVector<QPolygon> objectScreenSkelet( const QString& uuid ) const ;
  QVector<QPolygon> objectScreenSkelet( const QString& uuid, const QString& layeruuid ) const ;
  QVector<QPolygon> objectScreenSkelet( const QString& uuid, Layer* l ) const ;

  EventHandler* eventHandler() const { return handler_; }

  void turnEvents() { if ( nullptr == handler_ ) { handler_ = new EventHandler(); } handler_->turnEvents(); }
  void muteEvents() { if ( nullptr != handler_ ) { handler_->muteEvents(); } }
  bool muted() const { if ( nullptr == handler_ ) { return true; } return handler_->muted(); }

  QPoint cartesianToScreen( const QPoint& point ) const ;
  QPoint screenToCartesian( const QPoint& point ) const ;
  QPointF cartesianToScreen( const QPointF& point ) const ;
  QPointF screenToCartesian( const QPointF& point ) const ;

  QPolygon cartesianToScreen( const QPolygon& poly ) const ;
  QPolygon screenToCartesian( const QPolygon& poly ) const ;

  const proto::Document& property() const ;

  const QPolygon& isolineRamka() const { return isolineramka_; }
  QPolygon calcIsolineRamka( int mapindent, int docindent ) const ;
  void setIsolineRamka( const QPolygon& poly ) { isolineramka_ = poly; }

  void freezeText( QPainter* painter, const QRect& target );
  bool textFreezed() const { return freezetext_; }
  bool addToFreeze( const QRect& target, Label* l, const QRect& r, const QPoint& pnt, const QPoint& oldtopleft, float angle );
  void defreezeText( QPainter* p );

  bool isObjectInDrawedList( const Object* o ) const ;
  void addToDrawedLabels( Object* o );

  void setProjScaleFactorToSize( const QSize& size );

  QString T1() const ;
  QString T2() const ;
  QString A1() const ;
  QString A2() const ;
  QString TT() const ;
  QString AA() const ;
  int ii() const ;
  QString YYGGgg() const ;
  QString CCCC() const ;
  QDateTime date() const ;
  QString dateString() const ;
  int term() const ;
  QString termString() const ;

  QString wmoHeader() const ;

  QString author() const ;

  proto::Map info() const ;

  void setXfactor( double f ) { xfactor_ = f; }
  void setYfactor( double f ) { yfactor_ = f; }

  double xfactor() const { return xfactor_; }
  double yfactor() const { return yfactor_; }

  const QPolygon& mapramka() const { return mapramka_; }

  template <class T> bool postEvent();

  void addItem( QGraphicsItem* item );
  void removeItem( QGraphicsItem* item );
  bool hasItem( QGraphicsItem* item ) const ;

  bool isStub() const { return stub_; }

  virtual int32_t dataSize( const QStringList& layers = QStringList() ) const ;
  virtual int32_t serializeToArray( char* data, const QStringList& layers = QStringList() ) const ;
  virtual int32_t parseFromArray( const char* data );

  const QMap< Layer*, int >& layerOrder() const { return layerorder_; }

  const QString& defaulthash() const { return defaulthash_; }
  bool hasGeoBasis() const ;


  /*!
     * \brief Экспорт документа в каком-либо формате, указаном в map
     * \param map - параметры документа, в т.ч. формат для экспорта ( jpeg, png, bmp, sxf )
     * \param arr - в это массив кладется документ в указанном формате
     * \return true - в случае успешного выполнения функции
     */
  virtual bool exportDocument( const meteo::map::proto::Map& map, QByteArray* arr );

  /*!
     * \brief Возвращает рекомендованное имя файла с документом
     * \param name Имя документа
     * \param format Формат экспорта
     * \return Имя файла
     */
  QString fileName( const QString& name, proto::DocumentFormat format );

  /*!
     * \brief Получить суффикс расширения для формата format
     * \return Суффикс расширения
     */
  static QString extension( proto::DocumentFormat format );

  /*!
     * \brief Сохраняет данные документа в БД Монго
     * \param arr - данные документа (это результат выполнения функции Document::exportDocument )
     * \return true - в случае успешного сохранения документа в БД
     */
  bool saveDocumentToMongo( const QByteArray& arr, const QString& filename, proto::DocumentFormat format );
  static bool saveDocumentToMongo( const proto::Map& info, const QByteArray& arr, const QString& filename, proto::DocumentFormat format );

  /*!
     * \brief Сохранение документа в БД документов (gridfs)
     * \param fileName имя файла
     * \param data сохраняемые данные
     * \param id Идентификатор записи, добавленной в gridfs
     * \return true - в случае успешного сохранения
     */
  static bool saveDocumentToGridFs( const QString& fileName, const QByteArray& data, QString* id );

  bool hasGradient() const ;
  bool hasGradientWithWhiteSea() const ;

protected:
  void setProjection( Projection* proj ) { projection_ = proj; }

protected:
  Document* original_;
  Projection* projection_;

  QColor backgroundColor_;

  proto::Document property_;

private:
  QList<Incut*> incuts_;

  QList<Layer*> layers_;
  QMap<Layer*,int> layerorder_;
  Layer* activeLayer_;
  QPoint offcentermap_;
  int32_t ramkawidth_;
  Ramka* ramka_;
  Legend* legend_;

  QTransform transform_;
  QTransform absolutetransform_;

  EventHandler* handler_;
  const bool stub_;

  QList<int> searchdistancestack_;

  QPolygon isolineramka_;
  QPolygon mapramka_;
  bool freezetext_;

  QMap< int, FreezedLabelList > freezed_;

  std::unordered_set<const Object*> drawedlabels_;

  proto::Map info_;

  float xfactor_;
  float yfactor_;
  float minScale_ = 0.0;
  float maxScale_ = 20.0;

  LayerItems* items_;

  QString defaulthash_;

  void addLayer( Layer* layer );
  void rmLayer( Layer* layer );

  void performTransform();

  QString searchLayerName() const ;

  void moveCachedLayer( Layer* l );

  bool exportDocumentImage( const proto::Map& req, QByteArray* arr );
  bool exportDocumentSxf( QByteArray* arr );
  bool exportDocumentPtkpp( const proto::Map& req, QByteArray* arr );
  bool exportDocumentFormal( QByteArray* arr );
  bool exportDocumentPdf( QByteArray* arr );

  friend class Layer;
  friend class Object;
  friend class Ramka;
  friend class Incut;
  friend class Legend;
  friend class MapScene;
};

template<class T> T* Document::objectNearScreenPoint( const QPoint& pnt, Layer* l, QPoint* cross, bool onlyvisible ) const
{
  QPoint res;
  QPair< Object*, int > minpair( nullptr, 1000000 );

  // foreach(Object* o, l->objects()){
  for (auto itr = l->objects_.begin(); itr != l->objects_.end(); ++itr) {
    Object* o = (*itr);
    if ( int(static_cast<T*>(nullptr)->Type ) != o->type() ) {
      continue;
    }
    if ( true == onlyvisible && false == o->visible( property_.scale() ) ) {
      continue;
    }
    //    if ( false == o->onDocument() ) {   // TODO этот метод очень медленный....
    //      continue;
    //    }
    if ( true == onlyvisible && false == o->wasdrawed() ) {
      continue;
    }
    int mindist = o->minimumScreenDistance( pnt, &res );
    if ( ::abs(mindist) < ::abs(minpair.second) ) {
      minpair.first = o;
      minpair.second = mindist;
      if ( nullptr != cross ) {
        *cross = res;
      }
    }
  }
  int curdist = currentSearchDistance();
  if ( ::abs(minpair.second) <= curdist ) {
    return mapobject_cast<T*>(minpair.first);
  }
  return 0;
}


template<class T> QList<T*> Document::objectsNearScreenPoint( const QPoint& pnt, Layer* l, bool onlyvisible ) const
{
  QPoint res;
  QList<T*> ret_list;
  int curdist = currentSearchDistance();
  for (auto itr = l->objects().begin(); itr != l->objects().end(); ++itr) {
    Object* o = (*itr);
    //foreach(Object* o, l->objects()){
    // for ( int  i = 0, sz = l->size(); i < sz; ++i ) {
    //   const QList<Object*>& olist = l->objects();
    //   Object* o = olist[i];
    if ( int(static_cast<T*>(nullptr)->Type ) != o->type() ) {
      continue;
    }
    if ( true == onlyvisible && false == o->visible( property_.scale() ) ) {
      continue;
    }

    //    if ( false == o->onDocument() ) {   // TODO этот метод очень медленный....
    //    continue;
    //  }
    if ( true == onlyvisible && false == o->wasdrawed() ) {
      continue;
    }
    int mindist = o->minimumScreenDistance( pnt, &res );
    if ( ::abs(curdist) >= ::abs(mindist) ) {
      ret_list.append(mapobject_cast<T*>(o));
    }
  }
  return ret_list;
}

template <class T> bool Document::postEvent()
{
  if ( nullptr == handler_ ) {
    return false;
  }
  T* t = new T;
  handler_->postEvent(t);
}

}
}

template <class T> inline T document_cast( meteo::map::Document* d )
{
  return int(static_cast<T>(0)->Type) == int(meteo::map::Document::Type)
      || ( nullptr != d && int(static_cast<T>(nullptr)->Type) == d->type() ) ? static_cast<T>(d) : 0;
}

template <class T> inline T document_cast( const meteo::map::Document* d )
{
  return int(static_cast<T>(0)->Type) == int(meteo::map::Document::Type)
      || ( nullptr != d && int(static_cast<T>(nullptr)->Type) == d->type() ) ? static_cast<T>(d) : 0;
}

#endif
