#include "document.h"

#include <qmath.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qpixmapcache.h>
#include <qsettings.h>
#include <qnumeric.h>
#include <qdebug.h>
#include <quuid.h>
#include <qcryptographichash.h>
#include <qbuffer.h>
#include <qpainterpath.h>
#include <qprocess.h>
#include <qfile.h>
#include <qtemporaryfile.h>
#include <qfileinfo.h>

#include <meteo/commons/global/dateformat.h>
#include <cross-commons/debug/tlog.h>
#include <sql/dbi/gridfs.h>
#include <sql/dbi/dbiquery.h>
#include <sql/dbi/dbientry.h>
#include <commons/mathtools/mnmath.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/dataexchange/sxfexchange.h>
#include <meteo/commons/ui/map/dataexchange/passportbasis.h>

#include "layer.h"
#include "layergrid.h"
#include "layeriso.h"
#include "layerpunch.h"
#include "layermrl.h"
#include "layeritems.h"
#include "object.h"
#include "incut.h"
#include "ramka.h"
#include "geopolygon.h"
#include "geotext.h"
#include "ramka.h"
#include "loader.h"
#include "legend.h"
#include <meteo/commons/global/weatherloader.h>
#include "formaldoc.h"
//#include <meteo/commons/ui/map/loader/ptkpp.geo/ptkppgeo.h>
//#include <meteo/commons/ui/map/loader/old.geo/oldgeo.h>
#include "commongrid.h"

#include "tileimage/basiscache.h"

namespace meteo {
namespace map {

static const QString kDocumentdb = QObject::tr("documentdb");

namespace {

QByteArray typepostfix( proto::DocumentType t )
{
  QByteArray arr;
  switch (t) {
    case proto::kGeoMap:
      arr = global::kMapPtkppPostfix;
    break;
    case proto::kVerticalCut:
      arr = global::kCutPtkppPostfix;
    break;
    case proto::kAeroDiagram:
      arr = global::kAeroPtkppPostfix;
    break;
    case proto::kOceanDiagram:
      arr = global::kOceanPtkppPostfix;
    break;
    case proto::kMeteogram:
      arr = global::kMgPtkppPostfix;
    break;
    case proto::kFormalDocument:
      arr = global::kFormalPtkppPostfix;
    break;
  }
  return arr;
}

}
//
//int Document::DOCUMENT_COUNTER = 0;
//static QMutex gMutex(QMutex::Recursive);

static sxf::PassportBasis passportBasisFromDocument(Document* doc);

Document::Document( const GeoPoint& mapcenter,
                    const GeoPoint& doc_center,
                    proto::DocumentType type,
                    ProjectionType proj_type )
  : original_(this),
    projection_(nullptr),
    activeLayer_(nullptr),
    offcentermap_( QPoint(0,0) ),
    ramkawidth_(20),
    ramka_(nullptr),
    handler_(nullptr),
    stub_(false),
    xfactor_(1.0),
    yfactor_(1.0),
    items_(nullptr)
{
  //  QMutexLocker loc(&gMutex);
  //  DOCUMENT_COUNTER += 1;
  ramka_ = new Ramka(this, ramkawidth_/2);

  property_.mutable_map_center()->CopyFrom( geopoint2pbgeopoint(mapcenter) );
  property_.mutable_doc_center()->CopyFrom( geopoint2pbgeopoint(doc_center) );
  property_.set_doctype(type);
  property_.set_projection(proj_type);
  property_.set_uuid( QUuid::createUuid().toString().toStdString() );
  addIncut( kTopCenter );
  legend_ =  new Legend(this);

}

Document::Document( const proto::Document& doc )
  : original_(this),
    projection_(nullptr),
    activeLayer_(nullptr),
    offcentermap_( QPoint(0,0) ),
    ramkawidth_(20),
    ramka_(nullptr),
    handler_(nullptr),
    stub_(false),
    xfactor_(1.0),
    yfactor_(1.0),
    items_(nullptr)
{
  //  QMutexLocker loc(&gMutex);
  //  DOCUMENT_COUNTER += 1;
  ramka_ = new Ramka(this, ramkawidth_/2);

  property_.CopyFrom(doc);
  if ( false == property_.has_uuid() ) {
    property_.set_uuid( QUuid::createUuid().toString().toStdString() );
  }
  Incut* inc = addIncut( kTopCenter );
  if ( true == doc.has_incut() ) {
    inc->setProto(doc.incut());
  }
  legend_ =  new Legend(this);
  if ( true == doc.has_legend() ) {
    legend_->setPositionOnDocument(doc.legend().pos());
  }
}

Document::Document()
  : original_(this),
    projection_(nullptr),
    activeLayer_(nullptr),
    offcentermap_( QPoint(0,0) ),
    ramkawidth_(20),
    ramka_(nullptr),
    handler_(nullptr),
    stub_(false),
    xfactor_(1.0),
    yfactor_(1.0),
    items_(nullptr)

{
  //  QMutexLocker loc(&gMutex);
  //  DOCUMENT_COUNTER += 1;
  ramka_ = new Ramka(this, ramkawidth_/2);

  property_.set_uuid( QUuid::createUuid().toString().toStdString() );
  addIncut( kTopCenter );
  legend_ = new Legend(this);
}

bool Document::init( const proto::Document& doc )
{
  if ( &doc != &property_ ) {
    property_.CopyFrom(doc);
  }
  return init();
}

void Document::clearDataLayers(){

  QList<Layer*> layers = layers_;

  for(int i =0 ; i <  layers.size();++i){
    if(false == layers[i]->isBase()){
      delete layers[i];
      layers[i] = nullptr;
    }
  }


}

void Document::clearDocument()
{
  while ( 0 < layers_.size() ) {
    delete layers_[0];
  }
  delete projection_; projection_ = nullptr;
}

bool Document::init()
{
  clearDocument();
  ProjType proj_type = GENERAL;
  if ( true == property_.has_projection() ) {
    proj_type = proto2projection( property_.projection() );
  }
  GeoPoint mc( 0.0, 0.0, 0.0 );
  if ( GENERAL == proj_type ) {
    mc = GeoPoint( 0.0, 0.0, 0.0, LA_GENERAL );
  }
  if ( true == property_.has_map_center() ) {
    mc = pbgeopoint2geopoint( property_.map_center() );
  }
  projection_ = Projection::createProjection( proj_type, mc );
  if ( nullptr == projection_ ) {
    error_log << QObject::tr("Не удалось создать проекцию с типом = %1")
                 .arg(proj_type);
    return false;
  }
  if ( false == property_.has_geoloader() && proto::kGeoMap == property_.doctype() ) {
    property_.set_geoloader( "geo.old" );
  }

  QMap< QString, meteo::Property > setts = meteo::mapSettings();
  if ( true == setts.contains( kMAPCOLORSETTINGS_IDENT ) ) {
    meteo::Property prop;
    prop.CopyFrom( setts[kMAPCOLORSETTINGS_IDENT] );
    backgroundColor_ = prop.brush().color();
  }
  else {
    backgroundColor_ = QColor( kMAP_ISLAND_COLOR );
  }

  GeoPoint sc = mc;
  if ( true == property_.has_doc_center() ) {
    sc = pbgeopoint2geopoint( property_.doc_center() );
  }
  setScreenCenter(sc);
  if ( false == property_.has_mapsize() ) {
    property_.mutable_mapsize()->CopyFrom( qsize2size( QSize( 3072, 3072 ) ) );
  }
  if ( false == property_.has_docsize() ) {
    property_.mutable_docsize()->CopyFrom( property_.mapsize() );
  }
  if ( true == property_.cache() /*&& 0 != cacheHash().length()*/ ) {
    return true;
  }

//  meteo::map::Loader::instance()->registerDataHandler("geo.old", &meteo::map::oldgeo::loadGeoData );
 // meteo::map::Loader::instance()->registerDataHandler("ptkpp", &meteo::map::ptkppgeo::loadGeoData );
  meteo::map::Loader::instance()->registerCitiesHandler(meteo::global::kCitiesLoaderCommon, &meteo::map::loadCitiesLayer);//TODO?

  // meteo::map::oldgeo::loadGeoData();
  //  debug_log << "ZAGRUZHAYU OSNOVU!!!";
  return loadGeoLayers(this);
}

Document::~Document()
{
  //  QMutexLocker loc(&gMutex);
  //  DOCUMENT_COUNTER -= 1;
  delete legend_; legend_ = nullptr;
  delete ramka_; ramka_ = nullptr;
  while ( 0 != incuts_.size() ) {
    delete incuts_[0];
  }
  incuts_.clear();
  if ( true == stub_ ) {
    QList<Layer*> list;
    for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
      if ( 0 != original_ && false == original_->layers_.contains(layers_[i]) ) {
        list.append(layers_[i]);
      }
    }
    for ( int i = 0, sz = list.size(); i < sz; ++i ) {
      delete list[i];
    }
    return;
  }
  if ( nullptr != handler_ ) {
    handler_->deleteLater(); handler_ = nullptr;
  }
  activeLayer_ = nullptr;
  QList<Layer*> list = layers_;
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    delete list[i];
  }
  delete projection_; projection_ = nullptr;
  items_ = nullptr;
}

Document::Document( Document* doc, const GeoPoint& c, const QSize& docsize )
  : original_(doc),
    layers_(doc->layers_),
    activeLayer_(doc->activeLayer_),
    offcentermap_( QPoint(0,0) ),
    ramkawidth_(doc->ramkaWidth()),
    ramka_(nullptr),
    handler_(nullptr),
    stub_(true),
    xfactor_(doc->xfactor_),
    yfactor_(doc->yfactor_),
    items_(nullptr)
{
  ramka_ = doc->ramka()->copy(this);

  projection_ = doc->projection();
  property_.CopyFrom( doc->property() );
  property_.set_uuid( QUuid::createUuid().toString().toStdString() );
  property_.mutable_doc_center()->CopyFrom( geopoint2pbgeopoint(c) );
  property_.mutable_docsize()->CopyFrom( qsize2size(docsize) );
  property_.mutable_mapsize()->CopyFrom( qsize2size(docsize) );

  backgroundColor_ = doc->backgroundColor_;

  setScreenCenter(c);
  for ( int i = 0, sz = doc->incuts().size(); i < sz; ++i ) {
    Incut* inct = addIncut( doc->incuts()[i]->position() );
    inct->copyFrom(doc->incuts()[i]);
    inct->setPositionOnDocument( doc->incuts()[i]->position() );
  }
  legend_ =  new Legend(this);
}

Document* Document::stub( const GeoPoint& cntr, const QSize& docsize )
{
  Document* doc = new Document( this, cntr, docsize );
  return doc;
}

QRect Document::documentRect() const
{
  Size ms = property_.mapsize();
  Size ds = property_.docsize();
  QPoint mapcenter = QPoint( ms.width(), ms.height() )/2.;
  QPoint doccenter = QPoint( ds.width(), ds.height() )/2.;
  return QRect( QPoint(mapcenter - doccenter + offcentermap_), documentsize() ) ;
}

QRect Document::mapRect() const
{
  return QRect( QPoint(0,0), mapsize() );
}

QRect Document::contentRect() const
{
  QRect r;
  if ( nullptr == this->projection() ) {
    return r;
  }
  const GeoVector& rmk = projection()->ramka();
  QPolygon p = this->coord2screen(rmk);
  if ( 3 > p.size() ) {
    return r;
  }
  return p.boundingRect();
}

QPoint Document::documentTopLeft() const
{
  Size ms = property_.mapsize();
  Size ds = property_.docsize();
  QPoint mapcenter = QPoint( ms.width(), ms.height() )/2.;
  QPoint doccenter = QPoint( ds.width(), ds.height() )/2.;
  return (mapcenter - doccenter + offcentermap_);
}

QPoint Document::documentTopRight() const
{
  Size ms = property_.mapsize();
  Size ds = property_.docsize();
  QPoint mapcenter = QPoint( ms.width(), ms.height() )/2.;
  QPoint doccenter = QPoint( ds.width(), ds.height() )/2.;
  return (mapcenter + QPoint( doccenter.x(), -doccenter.y() ) + offcentermap_);
}

QPoint Document::documentBottomLeft() const
{
  Size ms = property_.mapsize();
  Size ds = property_.docsize();
  QPoint mapcenter = QPoint( ms.width(), ms.height() )/2.;
  QPoint doccenter = QPoint( ds.width(), ds.height() )/2.;
  return (mapcenter + QPoint( -doccenter.x(), doccenter.y() ) + offcentermap_);
}

QPoint Document::documentBottomRight() const
{
  Size ms = property_.mapsize();
  Size ds = property_.docsize();
  QPoint mapcenter = QPoint( ms.width(), ms.height() )/2.;
  QPoint doccenter = QPoint( ds.width(), ds.height() )/2.;
  return (mapcenter + doccenter + offcentermap_);
}

QPoint Document::documentTop() const
{
  Size ms = property_.mapsize();
  Size ds = property_.docsize();
  QPoint mapcenter = QPoint( ms.width(), ms.height() )/2.;
  QPoint doccenter = QPoint( ds.width(), ds.height() )/2.;
  return (mapcenter - QPoint( 0, doccenter.y() ) + offcentermap_);
}

QPoint Document::documentBottom() const
{
  Size ms = property_.mapsize();
  Size ds = property_.docsize();
  QPoint mapcenter = QPoint( ms.width(), ms.height() )/2.;
  QPoint doccenter = QPoint( ds.width(), ds.height() )/2.;
  return (mapcenter + QPoint( 0, doccenter.y() ) + offcentermap_);
}

QPoint Document::documentLeft() const
{
  Size ms = property_.mapsize();
  Size ds = property_.docsize();
  QPoint mapcenter = QPoint( ms.width(), ms.height() )/2.;
  QPoint doccenter = QPoint( ds.width(), ds.height() )/2.;
  return (mapcenter - QPoint( doccenter.x(), 0 ) + offcentermap_);
}

QPoint Document::documentRight() const
{
  Size ms = property_.mapsize();
  Size ds = property_.docsize();
  QPoint mapcenter = QPoint( ms.width(), ms.height() )/2.;
  QPoint doccenter = QPoint( ds.width(), ds.height() )/2.;
  return (mapcenter + QPoint( doccenter.x(), 0 ) + offcentermap_);
}

int Document::mapScaleDenominatorPixels( const QPoint& pnt1, const QPoint& pnt2, int pixelcount ) const
{
  GeoPoint gp1 = screen2coord( pnt1 );
  GeoPoint gp2 = screen2coord( pnt2 );
  int64_t meters = gp1.calcDistance(gp2) * 1000;
  double p1_p2_dist  = ::sqrt( (pnt1.x() - pnt2.x())*(pnt1.x() - pnt2.x()) + (pnt1.y() - pnt2.y())*(pnt1.y() - pnt2.y()) );
  if (MnMath::isZero(p1_p2_dist) ) {
    warning_log << QObject::tr("Расстояние между указанными точками == 0");
    return 0;
  }
  double koef = pixelcount/p1_p2_dist;
  meters *= koef;
  return meters;
}

int Document::mapScaleDenominatorMeters( const QPoint& pnt1, const QPoint& pnt2 ) const
{
  GeoPoint gp1 = screen2coord( pnt1 );
  GeoPoint gp2 = screen2coord( pnt2 );
  int64_t meters = gp1.calcDistance(gp2) * 1000;
  double p1_p2_dist  = ::sqrt( (pnt1.x() - pnt2.x())*(pnt1.x() - pnt2.x()) + (pnt1.y() - pnt2.y())*(pnt1.y() - pnt2.y()) );
  if (MnMath::isZero(p1_p2_dist) ) {
    warning_log << QObject::tr("Расстояние между указанными точками == 0");
    return 0;
  }
  QImage pix( QSize( p1_p2_dist, p1_p2_dist ), QImage::Format_ARGB32 );
  pix.fill( Qt::black );
  int widthMM = pix.widthMM();
  if ( 0 == widthMM ) {
    warning_log << QObject::tr("Расстояние в миллиметрах между указанными точками == 0");
    return 0;
  }
  double koef = 1000.0/widthMM;
  meters *= koef;
  return meters;
}

int Document::mapScaleDenominatorPixels( const GeoPoint& gp1, const GeoPoint& gp2, int pixelcount ) const
{
  QPoint pnt1 = coord2screen( gp1 );
  QPoint pnt2 = coord2screen( gp2 );
  int64_t meters = gp1.calcDistance(gp2) * 1000;
  double p1_p2_dist  = ::sqrt( (pnt1.x() - pnt2.x())*(pnt1.x() - pnt2.x()) + (pnt1.y() - pnt2.y())*(pnt1.y() - pnt2.y()) );
  if (MnMath::isZero(p1_p2_dist) ) {
    warning_log << QObject::tr("Расстояние между указанными точками == 0");
    return 0;
  }
  double koef = pixelcount/p1_p2_dist;
  meters *= koef;
  return meters;
}

int Document::mapScaleDenominatorMeters( const GeoPoint& gp1, const GeoPoint& gp2 ) const
{
  QPoint pnt1 = coord2screen( gp1 );
  QPoint pnt2 = coord2screen( gp2 );
  int64_t meters = gp1.calcDistance(gp2) * 1000;
  double p1_p2_dist  = ::sqrt( (pnt1.x() - pnt2.x())*(pnt1.x() - pnt2.x()) + (pnt1.y() - pnt2.y())*(pnt1.y() - pnt2.y()) );
  if (MnMath::isZero(p1_p2_dist) ) {
    warning_log << QObject::tr("Расстояние между указанными точками == 0");
    return 0;
  }
  QImage pix( QSize( p1_p2_dist, p1_p2_dist ), QImage::Format_ARGB32 );
  pix.fill( Qt::black );
  int widthMM = pix.widthMM();
  if ( 0 == widthMM ) {
    warning_log << QObject::tr("Расстояние в миллиметрах между указанными точками == 0");
    return 0;
  }
  double koef = 1000.0/widthMM;
  meters *= koef;
  return meters;
}

int Document::santimeteresScale() const
{
  QImage pix( QSize(10,10), QImage::Format_ARGB32 );
  int dpix = pix.logicalDpiX();
  dpix /= 2.54;
  QPoint pnt1( documentRect().center() );
  QPoint pnt2( pnt1.x() + dpix, pnt1.y() );
  return mapScaleDenominatorMeters( pnt1, pnt2 );//1000;
}

int Document::santimeteresScaleProjCenter() const
{
  QImage pix( QSize(10,10), QImage::Format_ARGB32);
  int dpix = pix.logicalDpiX();
  dpix /= 2.54;
  QPoint pnt1( coord2screen( projection_->getProjCenter() ) );
  QPoint pnt2( pnt1.x() + dpix, pnt1.y() );
  return mapScaleDenominatorMeters( pnt1, pnt2 );//1000;
}


QRectF Document::incutBoundingRect(int num ) const
{
  if ( 0 > num || incuts_.size() <= num ) {
    error_log << QObject::tr("Врезка №%1 не найдена")
                 .arg(num);
    return QRectF();
  }
  return incutBoundingRect( incuts_[num] );
}

QRectF Document::incutBoundingRect( const QString& iuuid ) const
{
  for ( int i = 0, sz = incuts_.size(); i < sz; ++i ) {
    if ( iuuid == incuts_[i]->uuid() ) {
      return incutBoundingRect(incuts_[i]);
    }
  }
  error_log << QObject::tr("Врезка с uuid = %1 не найдена")
               .arg(iuuid);
  return QRectF();
}

QColor Document::backgroundColor() const
{
  return backgroundColor_;
}

void Document::setBackgroundColor(QColor color)
{
  backgroundColor_ = color;
}

bool Document::drawDocument( QPainter* p, const QRect& target, const QRect& source )
{
  return drawDocument( p, QList<QString>(), target, source );
}

bool Document::drawDocument( QPainter* p, const QList<QString>& list, const QRect& target, const QRect& source )
{
  if ( nullptr == p ) {
    error_log << QObject::tr("Нулевой указатель pixmap");
    return false;
  }
  if ( 1 > property_.mapsize().width() ) {
    error_log << QObject::tr("Ширина картинки некорректна = %1").arg( property_.mapsize().width() );
    return false;
  }
  if ( 1 > property_.mapsize().height() ) {
    error_log << QObject::tr("Высота картинки некорректна = %1").arg( property_.mapsize().height() );
    return false;
  }

  QRect loctarget = target;
  if ( true == loctarget.isNull() ) {
    loctarget = QRect( QPoint(0,0), documentRect().size() );
  }

  QRect locsource = source;
  if ( true == locsource.isNull() ) {
    //    warning_log << QObject::tr("Попытка отрисовать нулевой участок карты");
    locsource = documentRect();
  }
  if ( locsource.width() > loctarget.width() ) {
    locsource.setWidth( loctarget.width() );
  }
  if ( locsource.height() > loctarget.height() ) {
    locsource.setHeight( loctarget.height() );
  }

  p->save();

  QPoint shift = loctarget.topLeft() - locsource.topLeft();

  isolineramka_ = calcIsolineRamka( -80, -60 );

  drawedlabels_.clear();

  mapramka_ = ramka()->calcRamka(0,0);

  mapramka_ = mapramka_.intersected( QPolygon( locsource, true ) );


  QRect maprect = mapramka_.boundingRect();
  locsource = locsource.intersected(maprect);

  QPainterPath path;
  path.addPolygon(mapramka_);
  p->translate( shift.x(), shift.y() );
  p->save();
  p->setClipPath(path);

  bool drawitems = false;
  if ( nullptr != original_ && nullptr != original_->items_ ) {
    if ( true == list.isEmpty() || true == list.contains( original_->items_->uuid() ) ) {
      drawitems = true;
    }
  }
  if ( true == property_.cache() ) {
    BasisCache bcache;
    if ( false == bcache.renderBasis( this, p ) ) {
      if ( false == hasGeoBasis() ) {
        bcache.loadGeoBasis(this);
      }
      bcache.flushBasis(this);
      bcache.renderBasis( this, p );
    }
  }
  freezeText( p, locsource );

  for ( auto l : layers_ ) {
    if ( l == original_->items_ ) {
      continue;
    }
    if ( false == list.isEmpty() && false == list.contains( l->uuid() ) ) {
      continue;
    }
    if ( true == property_.cache() && true == l->cache() ) {
      continue;
    }
    if ( false == drawLayer( l->uuid(), p, locsource ) ) {
      error_log << QObject::tr("Ошибка отрисовки слоя %1").arg( layerName(l->uuid()) );
      continue;
    }
  }
  defreezeText(p);
  if ( true == drawitems ) {
    drawLayer( original_->items_->uuid(), p, locsource );
  }
  p->restore();
  if ( false == drawRamka(p) ) {
    p->restore();
    return false;
  }
  if ( false == drawLegend(p) ) {
    p->restore();
    return false;
  }
  if ( false == drawIncut(p) ) {
    p->restore();
    return false;
  }
  p->restore();
  return true;
}

void Document::drawDocTile( QPainter* pntr, const QStringList& uuids, const QRect& source )
{
  pntr->save();
  pntr->translate( -source.x(), -source.y() );
  QListIterator<Layer*> it(layers_);
  while ( true == it.hasNext() ) {
    Layer* l = it.next();
    if ( false == uuids.contains( l->uuid() ) ) {
      continue;
    }
    if ( false == drawLayer( l->uuid(), pntr, source ) ) {
      error_log << QObject::tr("Ошибка отрисовки слоя %1").arg( layerName(l->uuid()) );
      continue;
    }
  }
  pntr->restore();
}

QString Document::cacheHash() const
{
  if ( "ptkpp" == property_.geoloader() && kMercat == property_.projection() ) { //кейс для подложки, используемой в интерактивной карте
    return "";
  }
  QByteArray data;
  for ( auto l : layers_ ) {
    if ( true == l->cache() && true == l->visible() ) {
      data += l->name().toUtf8();
    }
  }
  return QCryptographicHash::hash( data, QCryptographicHash::Sha1 ).toHex();
}

QStringList Document::cacheUuids() const
{
  QStringList list;
  for ( auto l : layers_ ) {
    if ( true == l->cache() && true == l->visible() ) {
      list << l->uuid();
    }
  }
  return list;
}

bool Document::drawLayer( const QString& uuid, QPainter* painter, const QRect& target )
{
  QRect t = target;
  if ( true == t.isNull() ) {
    t = QRect( QPoint(0,0), size2qsize(property_.mapsize()) );
  }
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    if ( layers_[i]->uuid() == uuid ) {
      layers_[i]->render( painter, t, this );
      return true;
    }
  }
  error_log << QObject::tr("Слой %1 не найден")
               .arg(uuid);

  return false;
}

bool Document::drawRamka( QPainter* painter )
{
  if ( nullptr == ramka_ ) {
    return true;
  }
  painter->save();
  bool res = ramka_->drawOnDocument(painter);
  painter->restore();
  return res;
}

bool Document::drawIncut( QPainter* painter )
{
  painter->setRenderHint( QPainter::Antialiasing, true );
  painter->setRenderHint( QPainter::SmoothPixmapTransform, true );
  painter->setCompositionMode( QPainter::CompositionMode_SourceOver );
  for ( int i = 0, sz = incuts_.size(); i < sz; ++i ) {
    incuts_[i]->drawOnDocument(painter);
  }
  return true;
}

bool Document::drawLegend( QPainter* painter )
{
  painter->setRenderHint( QPainter::Antialiasing, true );
  painter->setRenderHint( QPainter::SmoothPixmapTransform, true );
  painter->setCompositionMode( QPainter::CompositionMode_SourceOver );
  if ( nullptr != legend_ ) {
    legend_->drawOnDocument(painter);
  }
  return true;
}

bool Document::drawIncut( QPainter* painter, Incut* i )
{
  painter->setRenderHint( QPainter::Antialiasing, true );
  painter->setRenderHint( QPainter::SmoothPixmapTransform, true );
  painter->setCompositionMode( QPainter::CompositionMode_SourceOver );
  i->drawOnDocument(painter);
  return true;
}

QString Document::name() const
{
  return QString::fromStdString( info().title() );
}

void Document::setMap( const proto::Map& i ) {
  info_.CopyFrom(i);
}

void Document::setName( const QString& n )
{
  info_.set_title( n.toStdString() );
  if ( nullptr != handler_ ) {
    handler_->notifyDocumentChanges( DocumentEvent::NameChanged );
  }
}

void Document::resizeDocument( const QSize& size )
{
  property_.mutable_docsize()->CopyFrom( qsize2size(size) );
  QSize sz = size2qsize( property_.docsize() );
  if ( sz != size ) {
    if ( nullptr != handler_ ) {
      handler_->notifyDocumentChanges( DocumentEvent::Changed );
    }
  }
}

void Document::resizeMap( const QSize& size )
{
  property_.mutable_mapsize()->CopyFrom( qsize2size(size) );
  QSize sz = size2qsize( property_.mapsize() );
  if ( sz != size ) {
    if ( nullptr != handler_ ) {
      handler_->notifyDocumentChanges( DocumentEvent::Changed );
    }
  }
}

void Document::setMinScale(double scale)
{
  minScale_ = scale;
}

void Document::setMaxScale( double scale )
{
  maxScale_ = scale;
}

void Document::setScale( double scale )
{
  if ( scale < minScale_) {
    property_.set_scale( minScale_ );
    performTransform();
    if ( nullptr != handler_ ) {
      handler_->notifyMapChanges();
    }
  }
  else if ( scale > maxScale_) {
    property_.set_scale( maxScale_ );
    performTransform();
    if ( nullptr != handler_ ) {
      handler_->notifyMapChanges();
    }
  }
  else {
    property_.set_scale(scale);
    performTransform();
    if ( nullptr != handler_ ) {
      handler_->notifyMapChanges();
    }
  }
}

void Document::setRotateAngle( double ang )
{
  double newlon = center().lon() + DEG2RAD*(ang - property_.rot_angle() )*projection_->hemisphere();
  property_.set_rot_angle(ang);
  setScreenCenter( GeoPoint( center().lat(), newlon ));
}

GeoPoint Document::screenCenter() const
{
  GeoPoint gp = pbgeopoint2geopoint( property_.doc_center() );
  QPointF pnt = coord2screen(gp);
  gp = screen2coord( pnt + offcentermap_ );
  return gp;
}

QStringList Document::uuidsUnderGeoPoint( const GeoPoint& gp, Layer* l ) const
{
  if ( nullptr == l ) {
    error_log << QObject::tr("Нулевой указатель на слой");
    return QStringList();
  }
  return l->uuidsUnderGeoPoint(gp);
}

QStringList Document::uuidsUnderGeoPoint( const GeoPoint& gp, const QString& layeruuid ) const
{
  for ( int i = 0, sz = layers_.size(); i < sz; ++i  ) {
    Layer* l = layers_[i];
    if ( layeruuid != l->uuid() ) {
      continue;
    }
    return l->uuidsUnderGeoPoint(gp);
  }
  return QStringList();
}

QStringList Document::uuidsUnderGeoPoint( const GeoPoint& gp ) const
{
  QStringList uuids;
  for ( int i = 0, sz = layers_.size(); i < sz; ++i  ) {
    Layer* l = layers_[i];
    uuids.append( l->uuidsUnderGeoPoint(gp) );
  }
  return uuids;
}

QList<Object*> Document::objectsUnderGeoPoint( const GeoPoint& gp, Layer* l ) const
{
  if ( nullptr == l ) {
    error_log << QObject::tr("Нулевой указатель на слой");
    return QList<Object*>();
  }
  return l->objectsUnderGeoPoint(gp);
}

QList<Object*> Document::objectsUnderGeoPoint( const GeoPoint& gp, const QString& layeruuid ) const
{
  for ( int i = 0, sz = layers_.size(); i < sz; ++i  ) {
    Layer* l = layers_[i];
    if ( layeruuid != l->uuid() ) {
      continue;
    }
    return l->objectsUnderGeoPoint(gp);
  }
  return QList<Object*>();
}

QList<Object*> Document::objectsUnderGeoPoint( const GeoPoint& gp ) const
{
  QList<Object*> olist;
  for ( int i = 0, sz = layers_.size(); i < sz; ++i  ) {
    Layer* l = layers_[i];
    olist.append( l->objectsUnderGeoPoint(gp) );
  }
  return olist;
}

QStringList Document::uuidsUnderScreenPoint( const QPoint& point, Layer* layer ) const
{
  GeoPoint gp = screen2coord(point);
  return uuidsUnderGeoPoint( gp, layer );
}

QStringList Document::uuidsUnderScreenPoint( const QPoint& point, const QString& layeruuid ) const
{
  GeoPoint gp = screen2coord(point);
  return uuidsUnderGeoPoint( gp, layeruuid );
}

QStringList Document::uuidsUnderScreenPoint( const QPoint& point ) const
{
  GeoPoint gp = screen2coord(point);
  return uuidsUnderGeoPoint(gp);
}

QList<Object*> Document::objectsUnderScreenPoint( const QPoint& point, Layer* layer ) const
{
  GeoPoint gp = screen2coord(point);
  return objectsUnderGeoPoint( gp, layer );
}

QList<Object*> Document::objectsUnderScreenPoint( const QPoint& point, const QString& layeruuid ) const
{
  GeoPoint gp = screen2coord(point);
  return objectsUnderGeoPoint( gp, layeruuid );
}

QList<Object*> Document::objectsUnderScreenPoint( const QPoint& point ) const
{
  GeoPoint gp = screen2coord(point);
  return objectsUnderGeoPoint(gp);
}


Object* Document::objectNearGeoPoint( const GeoPoint& gp, Layer* l, QPoint* cross, bool onlyvisible ) const
{
  QPoint pnt = coord2screen(gp);
  return objectNearScreenPoint( pnt, l, cross, onlyvisible );
}

Object* Document::objectNearScreenPoint( const QPoint& point, Layer* l, QPoint* cross, bool onlyvisible ) const
{
  QPoint res;
  QPair< Object*, int > minpair( 0, 1000000 );
  std::unordered_set<Object*> olist = l->objects();
  foreach(Object* o, olist){
    if ( true == onlyvisible && false == o->visible( property_.scale() ) ) {
      continue;
    }
   // if ( false == o->onDocument() ) {   // TODO этот метод очень медленный....
    //  continue;
   // }
    if ( true == onlyvisible && false == o->wasdrawed() ) {
      continue;
    }
    int mindist = o->minimumScreenDistance( point, &res );
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
    return minpair.first;
  }
  return nullptr;
}

QVector<QPolygon> Document::objectScreenSkelet( const QString& uuid ) const
{
  QVector<QPolygon> screen;

  for ( int i = 0, sz = layers_.size(); i < sz; ++i  ) {
    Layer* l = layers_[i];
    screen += objectScreenSkelet( uuid, l );
  }
  return screen;
}

QVector<QPolygon> Document::objectScreenSkelet( const QString& uuid, const QString& layeruuid ) const
{
  QVector<QPolygon> screen;

  for ( int i = 0, sz = layers_.size(); i < sz; ++i  ) {
    Layer* l = layers_[i];
    if ( layeruuid == l->uuid() ) {
      screen = objectScreenSkelet(uuid, l);
      break;
    }
  }
  return screen;
}

QVector<QPolygon> Document::objectScreenSkelet( const QString& uuid, Layer* l ) const
{
  QVector<QPolygon> cartesian;

  std::unordered_set<Object*> olist = l->objects();
  foreach(Object* o, olist){
    if ( o->uuid() == uuid ) {
      cartesian = o->cartesianSkelet();
    }
  }
  if ( true == cartesian.isEmpty() ) {
    return cartesian;
  }

  QVector<QPolygon> screen;
  for ( int i = 0, sz = cartesian.size(); i < sz; ++i ) {
    QPolygon p = transform_.map( cartesian[i] );
    screen.append(p);
  }
  return screen;
}

void Document::setScreenCenter( const GeoPoint& center )
{
  property_.mutable_doc_center()->CopyFrom( geopoint2pbgeopoint(center) );
  //  if ( true == MnMath::isEqual( M_PI_2, center_.lat() ) ) {
  //    center_.setLatDeg(85.0);
  //  }
  //  else if ( true == MnMath::isEqual( -M_PI_2, center_.lat() ) ) {
  //    center_.setLatDeg(-85.0);
  //  }
  performTransform();
  if ( nullptr != handler_ ) {
    handler_->notifyMapChanges();
  }
}

void Document::setScreenCenter( const QPoint& center )
{
  setScreenCenter( screen2coord(center) );
}

void Document::setScreenCenter( const QPointF& center )
{
  setScreenCenter( screen2coord(center) );
}

void Document::setScreenCenter( int x, int y )
{
  setScreenCenter( screen2coord( QPoint(x,y) ) );
}

void Document::setPuansonOrientaton( proto::PunchOrient po )
{
  property_.set_punchorient(po);
}

void Document::setRamka( Ramka* r )
{
  delete ramka_;
  ramka_ = r;
  if ( nullptr != ramka_ ) {
    ramkawidth_ = ramka_->halfwidth();
  }
}

void Document::setRamkaWidth( int width )
{
  ramkawidth_ = width;
  if ( nullptr != ramka_ ) {
    ramka_->setHalfWidth( ramkawidth_/2 );
  }
}

Incut* Document::incut( Position pos ) const
{
  for ( int i = 0, sz = incuts_.size(); i < sz; ++i) {
    if ( pos == incuts_[i]->position() ) {
      return incuts_[i];
    }
  }
  return nullptr;
}


Incut* Document::addIncut( Position pos )
{
  Incut* incut = new Incut(this);
  incut->setPositionOnDocument(pos);
  return incut;
}

Incut* Document::addIncut( const QPoint& pos )
{
  Incut* incut = new Incut(this);
  incut->setPositionOnDocument(pos);
  return incut;
}

void Document::removeIncut( int num )
{
  if ( 0 > num || incuts_.size() <= num ) {
    return;
  }
  delete incuts_[num];
}

void Document::removeIncut( const QString& uuid )
{
  for ( int i = 0, sz = incuts_.size(); i < sz; ++i ) {
    if ( incuts_[i]->uuid() == uuid ) {
      delete incuts_[i];
      return;
    }
  }
}

void Document::setIncutPosition( int num, Position pos )
{
  if ( 0 > num || incuts_.size() <= num ) {
    return;
  }
  setIncutPosition( incuts_[num], pos );
}

void Document::setIncutPosition( const QString& uuid, Position pos )
{
  for ( int i = 0, sz = incuts_.size(); i < sz; ++i ) {
    if ( incuts_[i]->uuid() == uuid ) {
      setIncutPosition( incuts_[i], pos );
      return;
    }
  }
}
void Document::setIncutPosition( int num, const QPoint& pos )
{
  if ( 0 > num || incuts_.size() <= num ) {
    return;
  }
  setIncutPosition( incuts_[num], pos );
}

void Document::setIncutPosition( const QString& uuid, const QPoint& pos )
{
  for ( int i = 0, sz = incuts_.size(); i < sz; ++i ) {
    if ( incuts_[i]->uuid() == uuid ) {
      setIncutPosition( incuts_[i], pos );
      return;
    }
  }
}


void Document::addLabelToIncut( Incut* i, const QString& key, const QString& value )
{
  if ( nullptr != i ) {
    i->setLabel( key, value );
  }
}

void Document::addLabelToIncut( int num, const QString& key, const QString& value )
{
  if ( 0 > num || incuts_.size() <= num ) {
    error_log << QObject::tr("Врезка №%1 не найдена")
                 .arg(num);
    return;
  }
  addLabelToIncut( incuts_[num], key, value );
}
void Document::addLabelToIncut( const QString& uuid, const QString& key, const QString& value )
{
  for ( int i = 0, sz = incuts_.size(); i < sz; ++i ) {
    if ( uuid == incuts_[i]->uuid() ) {
      addLabelToIncut( incuts_[i], key, value );
      return;
    }
  }
  error_log << QObject::tr("Врезка с uuid = %1 не найдена")
               .arg(uuid);
}

QPoint Document::coord2screen( const GeoPoint& coord, bool* fl ) const
{
  QPoint point;
  bool res = projection_->F2X_one( coord, &point );
  if ( nullptr != fl ) {
    *fl = res;
  }
  return cartesianToScreen(point);
}

QPointF Document::coord2screenf( const GeoPoint& coord, bool* fl ) const
{
  QPointF point;
  bool res = projection_->F2X_one( coord, &point );
  if ( nullptr != fl ) {
    *fl = res;
  }
  return cartesianToScreen(point);
}

QPolygon Document::coord2screen( const GeoVector& poly ) const
{
  QPolygon res;
  bool fl;
  for ( int i = 0, sz = poly.size(); i < sz; ++i ) {
    QPoint pnt = coord2screen( poly[i], &fl );
    if ( true == fl ) {
      res.append(pnt);
    }
    //    if ( 0  != res.size() && res[ res.size() - 1 ] != pnt ) {
    //      res[i] = pnt;
    //    }
  }
  return res;
}

GeoVector Document::screen2coord( const QPolygon& poly ) const
{
  GeoVector res;
  bool fl;
  for ( int i = 0, sz = poly.size(); i < sz; ++i ) {
    GeoPoint gp = screen2coord( poly[i], &fl );
    if ( true == fl ) {
      res.append(gp);
    }
  }
  return res;
}

GeoPoint Document::screen2coord( const QPoint& point, bool* fl ) const
{
  GeoPoint coord;
  QPoint pnt(point);
  pnt = screenToCartesian(point);
  bool res = projection_->X2F_one( pnt, &coord );
  if ( nullptr != fl ) {
    *fl = res;
  }
  return coord;
}

GeoPoint Document::screen2coord( const QPointF& point, bool* fl ) const
{
  GeoPoint coord;
  QPointF pnt(point);
  pnt = screenToCartesian(point);
  bool res = projection_->X2F_one( pnt, &coord );
  if ( nullptr != fl ) {
    *fl = res;
  }
  return coord;
}

double Document::deviationFromNorth( const QPoint& pnt ) const
{
  switch ( property_.projection() ) {
    case kMercat:
    return 0.0;
    break;
    case kGeneral:
    return 0.0;
    break;
    default:
    break;
  }
  GeoPoint gp = screen2coord(pnt);
  double northlat = gp.lat() + M_PI_4;
  if ( northlat > M_PI_2 ) {
    northlat = M_PI_2;
  }
  GeoPoint north( northlat, gp.lon() );
  QPoint northpnt = coord2screen(north);
  QPoint delta = pnt - northpnt;
  return ::atan2( delta.x(), delta.y() );
}

double Document::deviationFromNorth( const GeoPoint& gp ) const
{
  switch ( property_.projection() ) {
    case kMercat:
    return 0.0;
    break;
    case kGeneral:
    return 0.0;
    break;
    default:
    break;
  }
  double northlat = gp.lat() + M_PI_4;
  if ( northlat > M_PI_2 ) {
    northlat = M_PI_2;
  }
  GeoPoint north( northlat, gp.lon() );
  QPoint pnt = coord2screen(gp);
  QPoint northpnt = coord2screen(north);
  QPoint delta = pnt - northpnt;
  return ::atan2( delta.x(), delta.y() );
}

double Document::deviationFromCenter( const QPoint& pnt ) const
{
  if ( kMercat == property_.projection() ) {
    return 0.0;
  }
  GeoPoint cntr( pbgeopoint2geopoint(property_.map_center() ) );
  QPoint cntrpnt = coord2screen(cntr);
  QPoint delta = pnt - cntrpnt;
  return ::atan2( delta.x(), delta.y() );
}

double Document::deviationFromCenter( const GeoPoint& gp ) const
{
  if ( kMercat == property_.projection() ) {
    return 0.0;
  }
  GeoPoint cntr( pbgeopoint2geopoint(property_.map_center() ) );
  QPoint pnt = coord2screen(gp);
  QPoint cntrpnt = coord2screen(cntr);
  QPoint delta = pnt - cntrpnt;
  return ::atan2( delta.x(), delta.y() );
}

LayerGrid* Document::gridLayer() const
{
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    if ( layers_[i]->type() == static_cast<LayerGrid*>(0)->Type ) {
      return maplayer_cast<LayerGrid*>(layers_[i]);
    }
  }
  return nullptr;
}

LayerItems* Document::itemsLayer() const
{
  return original_->items_;
}

bool Document::hasLayer( const proto::WeatherLayer& layer ) const
{
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    Layer* l = layers_[i];
    if ( true == l->isEqual(layer) ) {
      return true;
    }
  }
  return false;
}

bool Document::hasLayer( const QString& uuid ) const {
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    if ( layers_[i]->uuid() == uuid ) {
      return true;
    }
  }
  return false;
}

QString Document::layerName( const QString& uuid ) const
{
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    if ( layers_[i]->uuid() == uuid ) {
      return layers_[i]->name();
    }
  }
  error_log << QObject::tr("Слой с идентифкатором %1 не найден")
               .arg(uuid);
  return QString();
}

int Document::layerZOrder( const QString& uuid ) const
{
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    if ( layers_[i]->uuid() == uuid ) {
      return i;
    }
  }
  error_log << QObject::tr("Слой с идентифкатором %1 не найден")
               .arg(uuid);
  return -1;
}

bool Document::layerVisible( const QString& uuid ) const
{
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    if ( layers_[i]->uuid() == uuid ) {
      return layers_[i]->visible();
    }
  }
  error_log << QObject::tr("Слой с идентифкатором %1 не найден. Документ %2")
               .arg(uuid)
               .arg( QString::fromStdString( property_.uuid() ) );
  return false;
}

QString Document::activeLayerUuid() const
{
  if ( nullptr == activeLayer_ ) {
    return QString();
  }
  return activeLayer_->uuid();
}

Layer* Document::layerByUuid( const QString& id ) const
{
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    if ( layers_[i]->uuid() == id ) {
      return layers_[i];
    }
  }
  return nullptr;
}

Layer* Document::layerByCustomType( int t ) const
{
  for ( auto l : layers_ ) {
    if ( t == l->customType() ) {
      return l;
    }
  }
  return nullptr;
}

bool Document::moveLayer( const QString& uuid, int delta )
{
  if ( 0 == delta ) {
    error_log << QObject::tr("Нет указания, куда перемещать слой по оси Z");
    return false;
  }
  Layer* l = nullptr;
  for ( auto ll : layers_ ) {
    if ( ll->uuid() == uuid ) {
      l = ll;
      break;
    }
  }
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой %1")
                 .arg(uuid);
    return false;
  }
  if ( kLayerItems == l->type() ) {
    return false;
  }
  if ( true == l->cache() ) {
    return false;
  }
  int indx = layers_.indexOf(l) + delta;
  if ( 0 > indx ) {
    indx = 0;
  }
  if ( nullptr != original_->items_ && indx == layers_.indexOf(items_) ) {
    return  false;
  }
  if ( true == layers_[indx]->cache() ) {
    return false;
  }
  layers_.removeAll(l);
  if ( layers_.size() <= indx ) {
    layers_.append(l);
  }
  else {
    layers_.insert( indx, l );
  }
  layerorder_.clear();
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    layerorder_.insert( layers_[i], i );
  }
  if ( nullptr != handler_ ) {
    handler_->notifyLayerChanges( l, LayerEvent::Moved );
  }
  return true;
}

void Document::moveCachedLayer( Layer* l )
{
  if ( nullptr == l ) {
    error_log << QObject::tr("Нулевой указатель");
    return;
  }
  if ( false == l->cache() ) {
    return;
  }
  layers_.removeAll(l);
  layerorder_.clear();
  for ( auto ll : layers_ ) {
    if ( false == ll->cache() ) {
      int indx = layers_.indexOf(ll);
      layers_.insert( indx, l );
      break;
    }
  }
  if ( -1 == layers_.indexOf(l) ) {
    layers_.append(l);
  }

  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    layerorder_.insert( layers_[i], i );
  }
  return;
}

void Document::rmLayer( const QString& uuid )
{
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    if ( layers_[i]->uuid() == uuid ) {
      Layer* l = layers_[i];
      layers_.removeAll(l);
      if ( activeLayer_ == l ) {
        if ( 0 != layers_.size() ) {
          setActiveLayer( layers_[ layers_.size() - 1 ]->uuid() );
        }
        else {
          activeLayer_ = nullptr;
        }
      }
      if ( nullptr != handler_ ) {
        handler_->notifyLayerChanges( l, LayerEvent::Deleted );
      }
      delete l;
      return;
    }
  }
  error_log << QObject::tr("Слой с идентифкатором %1 не найден")
               .arg(uuid);
}
const proto::Document& Document::property() const
{
  return property_;
}

void Document::setProperty( const proto::Document& proto )
{
  property_.CopyFrom(proto);
  setScreenCenter( pbgeopoint2geopoint( property_.doc_center() ) );
  if ( 0 != incuts_.size() ) {
    incuts_[0]->setProto(property_.incut());
  }
  if ( nullptr != legend_ && true == property_.legend().has_pos() ) {
    legend_->setPositionOnDocument( property_.legend().pos() );
  }
  info_.mutable_document()->CopyFrom(property_);
  performTransform();
}

void Document::setIncutProperty( const proto::IncutParams& inc )
{
  property_.mutable_incut()->CopyFrom(inc);
  if ( 0 != incuts_.size() ) {
    incuts_[0]->setProto(property_.incut());
  }
}

void Document::freezeText( QPainter* painter, const QRect& target )
{
  freezetext_ = true;
  QMap< int , QMap< int, Label* > > list; //с учетом приоритета
  int i = 0;
  for ( auto l : layers_ ) {
    if ( false == l->visible() ) {
      continue;
    }
    if ( proto::kCrossLayer == property_.cross_type() ) {
      ++i;
    }
    auto lblist = l->labels(true);
    auto& priorlabels = list[i];
    for ( auto lbl : lblist ) {
      priorlabels.insertMulti( 10000000 - lbl->priority(), lbl );
    }
  }

  i = 0;
  for ( auto it = list.begin(), end = list.end(); it != end; ++it ) {
    auto priorlabels = it.value();
    for ( auto pit = priorlabels.begin(), pend = priorlabels.end(); pit != pend; ++pit ) {
      auto lbl = pit.value();
      ((Object*)(lbl))->render( painter, target, this );
      ++i;
    }
  }
}

bool Document::addToFreeze( const QRect& target, Label* lbl, const QRect& r, const QPoint& pnt, const QPoint& oldtopleft, float angle )
{
  int indx = 0;
  if ( proto::kCrossLayer == property_.cross_type() ) {
    Layer* l = lbl->layer();
    indx = layerorder_[l];
  }
  if ( false == freezed_.contains(indx) ) {
    FreezedLabelList list( this, target );
    freezed_.insert( indx, list );
  }
  FreezedLabelList& list = freezed_[indx];
  return list.addRect( lbl, r, pnt, oldtopleft, angle );
}

void Document::defreezeText( QPainter* painter )
{
  freezetext_ = false;
  auto it = freezed_.begin(), end = freezed_.end();
  for ( ; it != end; ++it ) {
    FreezedLabelList& l = it.value();
    l.render(painter);
  }
  freezed_.clear();
}

bool Document::isObjectInDrawedList( const Object* o ) const
{
  if ( 0 != drawedlabels_.count(o) ) {
    return true;
  }
  return false;
}

void Document::addToDrawedLabels( Object* o )
{
  if ( 0 == drawedlabels_.count(o) ) {
    drawedlabels_.insert(o);
  }
}

void Document::setProjScaleFactorToSize( const QSize& size )
{
  if ( nullptr == projection() ) {
    return;
  }
  const GeoVector& gv = projection()->ramka();
  QVector<QPolygon> list;
  if ( false == projection()->F2X( gv, &list, false ) ) {
    return;
  }
  if ( 0 == list.size() ) {
    return;
  }
  QRect rect = list[0].boundingRect();
  for ( int i = 1, sz = list.size(); i < sz; ++i ) {
    rect = rect.united( list[i].boundingRect() );
  }
  QRect target = transform().inverted().mapRect( QRect( QPoint(0,0), size ) );
  double xf = double(target.width())/double(rect.width());
  double yf = double(target.height())/double(rect.height());
  projection()->setXfactor(xf);
  projection()->setYfactor(yf);
}

QString Document::T1() const
{
  return "P";
}

QString Document::T2() const
{
  return "W";
}

QString Document::A1() const
{
  return "S";
}

QString Document::A2() const
{
  return "S";
}

int Document::ii() const
{
  return 98;
}

QString Document::TT() const
{
  return T1()+T2();
}

QString Document::AA() const
{
  return A1() + A2();
}

QString Document::YYGGgg() const
{
  QDateTime dt = DbiQuery::datetimeFromString( info().datetime() );
  if ( false == dt.isValid() ) {
    dt = QDateTime::currentDateTimeUtc();
  }
  return dt.toString("ddhhmm");
}

QString Document::CCCC() const
{
  return global::wmoId();
}

QDateTime Document::date() const
{
  QDateTime dt = DbiQuery::datetimeFromString( info().datetime() );
  if ( false == dt.isValid() ) {
    dt = QDateTime::currentDateTimeUtc();
  }
  return dt;
}

QString Document::dateString() const
{
  QDateTime dt = DbiQuery::datetimeFromString( info().datetime() );
  if ( false == dt.isValid() ) {
    dt = QDateTime::currentDateTimeUtc();
  }
  return meteo::dateToHumanTimeShort(dt);
}

int Document::term() const
{
  return info().hour();
}

QString Document::termString() const
{
  if ( 0 == info().hour() ) {
    return "";
  }
  return QObject::tr("Прогноз на %1 часов")
      .arg( info().hour() );
}

QString Document::wmoHeader() const
{
  return QString("%1%2%3 %4 %5")
      .arg( TT() )
      .arg( AA() )
      .arg( ii() )
      .arg( CCCC() )
      .arg( YYGGgg() );
}

QString Document::author() const
{
  QString user = meteo::global::currentUserShortName();
  if ( false == user.isEmpty() ) {
    // user = QObject::tr("Пользователь не указан");
    user = meteo::global::currentUserRank() + " " + user;
  }

  return user;
}


QString Document::searchLayerName() const
{
  QString mapname;
  Layer* l = activeLayer();
  if ( nullptr == l || true == l->isBase() ) {
    for ( int i = layers().size() - 1; i > -1; --i ) {
      if ( false == layers()[i]->isBase() ) {
        l = layers()[i];
        break;
      }
    }
  }
  if ( nullptr == l || true == l->isBase() ) {
    return QString();// QObject:: tr("Пустой документ");
  }

  if ( false == l->info().has_template_name() ) {
    mapname = l->name();
  }
  else {
    internal::WeatherLoader* wl = WeatherLoader::instance();
    bool ok = false;
    switch ( l->info().mode() ) {
      case proto::kEmptyLayer:
        mapname = l->name();
      break;
      case proto::kPuanson: {
        mapname = l->name();
        /*
      puanson::proto::Puanson punch = wl->punchparams( QString::fromStdString( l->info().template_name() ), &ok );
      if ( false == ok ) {
        mapname = QObject::tr("Наноска: неизвестный шаблон");
      }
      else {
        mapname = QObject::tr("Наноска %1").arg( QString::fromStdString( punch.name() ) );
      }*/
      }
      break;
      case proto::kIsoline:
      case proto::kGradient:
      case proto::kIsoGrad: {
        proto::FieldColor fc = wl->isoparams( QString::fromStdString( l->info().template_name() ), &ok );
        if ( false == ok ) {
          mapname = QObject::tr("Неизвестные изолинии");
        }
        else {

          mapname = QObject::tr("%1")
                    .arg( QString::fromStdString( fc.name() ) );

          int ntypel = l->info().type_level();
          QString ltype = QString::number(ntypel);
          QMap< int, QString > ltypes = global::kLevelTypes();
          if(ltypes.contains(ntypel)) {
            ltype = ltypes[ntypel];
          }
          mapname += " "+ ltype;
          switch(ntypel){
            case 100:
            case 210:
              mapname += QObject::tr(" %1 мбар")
                         .arg(l->info().level());
            break;
            case 20:
              mapname += QObject::tr(" %1 C")
                         .arg(l->info().level());
            break;
            case 102:
            case 103:
            case 106:
            case 160:
              mapname += QObject::tr(" %1 м")
                         .arg(l->info().level());
            break;
          }
        }
      }
      break;
      case proto::kRadar: {
        proto::RadarColor rc = wl->radarparams( QString::fromStdString( l->info().template_name() ), &ok );
        if ( false == ok ) {
          mapname = QObject::tr("Неизвестные радиолокационные данные");
        }
        else {
          mapname = QObject::tr("%1").arg( QString::fromStdString( rc.name() ) );
        }
      }
      break;
      case proto::kSigwx:
        mapname = QObject::tr("Карта SIGWX");
      break;
    }
  }
  return mapname;
}

proto::Map Document::info() const
{

  proto::Map map;
  map.CopyFrom(info_);
  map.clear_data();
  map.mutable_document()->CopyFrom(property_);
  Layer* datalayer = nullptr;
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    proto::WeatherLayer* l = map.add_data();
    l->CopyFrom( layers_[i]->info() );
    if ( false == layers_[i]->isBase() ) {
      datalayer = layers_[i];
    }
    /*  if ( true == l->has_level() ) {
  map.set_level( l->level() );
}
if ( true == l->has_type_level() ) {
  map.set_type_level( l->type_level() );
}
if ( true == l->has_datetime() ) {
  map.set_datetime( l->datetime() );
}
if ( true == l->has_hour() ) {
  map.set_hour( l->hour() );
}*/
  }
  Layer* al = activeLayer();
  if ( nullptr != datalayer && ( nullptr == al ||  true == al->isBase() ) ) {
    al = datalayer;
  }
  if ( nullptr != al ) {
    if ( true == al->info().has_level() ) {
      map.set_level( al->info().level() );
    }
    if ( true == al->info().has_type_level() ) {
      map.set_type_level( al->info().type_level() );
    }
    if ( true == al->info().has_datetime() ) {
      map.set_datetime( al->info().datetime() );
    }
    if ( true == al->info().has_hour() ) {
      map.set_hour( al->info().hour() );
    }
  }
  QString mapname;
  if ( true == info_.has_title() ) {
    mapname = QString::fromStdString( info_.title() );
  }
  else
  {
    mapname = searchLayerName();
    if(false == mapname.isEmpty()){
      map.set_title( mapname.toStdString() );
    }
  }
  /*  if (nullptr != al &&
      true == al->info().has_center_name() &&
      0 != al->info().center_name().size() ) {
    mapname += QObject::tr( " %1")
        .arg( QString::fromStdString( al->info().center_name() ) );
    map.set_title( mapname.toStdString() );
  }*/
  return map;
}

void Document::addItem( QGraphicsItem* item )
{
  if ( nullptr == item ) {
    return;
  }
  if ( nullptr == original_->items_ ) {
    Layer* l = original_->activeLayer();
    original_->items_ = new LayerItems(this);
    if ( nullptr != l ) {
      original_->setActiveLayer( l->uuid() );
    }
  }
  original_->items_->addItem(item);
}

void Document::removeItem( QGraphicsItem* item )
{
  if ( nullptr == original_->items_ ) {
    return;
  }
  original_->items_->removeItem(item);
}

bool Document::hasItem( QGraphicsItem* item ) const
{
  if ( nullptr == items_ ) {
    return false;
  }
  return items_->hasItem(item);
}

QPolygon Document::calcIsolineRamka( int mapindent, int docindent ) const
{
  QPolygon p;
  if ( nullptr != ramka_ ) {
    p = ramka_->calcRamka( mapindent, docindent );
  }
  return p;
}

void Document::showLayer( const QString& uuid )
{
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    if ( layers_[i]->uuid() == uuid ) {
      layers_[i]->setVisisble(true);
      if ( nullptr != handler_ ) {
        handler_->notifyLayerChanges( layers_[i], LayerEvent::Visibility );
      }
      return;
    }
  }
  error_log << QObject::tr("Слой %1 не найден")
               .arg(uuid);
}

void Document::hideLayer( const QString& uuid )
{
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    if ( layers_[i]->uuid() == uuid ) {
      layers_[i]->setVisisble(false);
      if ( nullptr != handler_ ) {
        handler_->notifyLayerChanges( layers_[i], LayerEvent::Visibility );
      }
      return;
    }
  }
  error_log << QObject::tr("Слой %1 не найден")
               .arg(uuid);
}

void Document::setActiveLayer( const QString& uuid )
{
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    if ( layers_[i]->uuid() == uuid ) {
      if ( layers_[i] == original_->items_ ) {
        return;
      }
      activeLayer_ = layers_[i];
      if ( nullptr != handler_ ) {
        handler_->notifyLayerChanges( layers_[i], LayerEvent::Activity );
      }
      return;
    }
  }
  error_log << QObject::tr("Слой %1 не найден")
               .arg(uuid);
}

Layer* Document::layer( const QString& uuid ) const
{
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    if ( layers_[i]->uuid() == uuid ) {
      return layers_[i];
    }
  }
  return nullptr;
}

void Document::addLayer( Layer* l )
{
  if ( nullptr == l ) {
    error_log << QObject::tr("Нулевой указатель");
    return;
  }
  if ( true == layers_.contains(l) ) {
    error_log << QObject::tr("Слой уже добавлен. uuid = %1")
                 .arg(l->uuid());
    return;
  }
  if ( nullptr == itemsLayer() ) {
    layers_.append(l);
  }
  else {
    int indx = layers_.indexOf( itemsLayer() );
    layers_.insert( indx, l );
  }
  layerorder_.clear();
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    layerorder_.insert( layers_[i], i );
  }
  setActiveLayer(l->uuid());
  if ( nullptr != handler_ ) {
    handler_->notifyLayerChanges( l, LayerEvent::Added );
  }
}

void Document::rmLayer( Layer* l )
{
  if ( nullptr == l ) {
    error_log << QObject::tr("Нулевой указатель");
    return;
  }
  if ( false == layers_.contains(l) ) {
    return;
  }
  if ( original_->items_ == l ) {
    original_->items_ = nullptr;
  }
  layers_.removeAll(l);
  layerorder_.clear();
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    layerorder_.insert( layers_[i], i );
  }
  if ( activeLayer_ == l ) {
    if ( 0 != layers_.size() ) {
      Layer* newact = nullptr;
      QListIterator<Layer*> it(layers_);
      it.toBack();
      while ( true == it.hasPrevious() ) {
        Layer* prev = it.previous();
        if ( 0 != prev && prev != original_->items_ ) {
          newact = prev;
          break;
        }
      }
      if ( nullptr != newact ) {
        setActiveLayer( newact->uuid() );
      }
      else {
        activeLayer_ = nullptr;
      }
    }
    else {
      activeLayer_ = nullptr;
    }
  }
  if ( nullptr != handler_ ) {
    handler_->notifyLayerChanges( l, LayerEvent::Deleted );
  }
}

void Document::performTransform()
{
  if ( nullptr == projection() ) {
    return;
  }
  transform_.reset();

  double scaleKoef = 1.0/( qPow( 2.0, property_.scale() ) );

  transform_.scale( scaleKoef*xfactor_, scaleKoef*yfactor_ );
  //  GeoPoint rotatecenter( M_PI_2*projection_->hemisphere(), 0 );
  GeoPoint rotatecenter( M_PI_2*projection_->hemisphere(), projection_->getMapCenter().lon() );
  if ( nullptr != projection() && MERCAT == projection()->type() ) {
    rotatecenter = GeoPoint( 0, 0 );
  }
  double angle = property_.rot_angle()*projection_->lonfactor();
  QPointF zeropoint;
  QPointF cartcenter;
  projection_->F2X_one( pbgeopoint2geopoint( property_.doc_center() ), &cartcenter );
  projection_->F2X_one( rotatecenter, &zeropoint );
  QPointF offset( cartcenter - zeropoint );
  transform_.translate( -offset.x(), -offset.y() );
  transform_.rotate(angle);
  transform_.translate( offset.x(), offset.y() );

  GeoPoint mapcenter = projection_->getMapCenter();
  projection_->F2X_one( mapcenter, &zeropoint );
  offset = QPointF( cartcenter - zeropoint );

  QPointF screencenter = screenToCartesian( QPointF(mapsize().width()/2.0,mapsize().height()/2.0) );
  offset = offset - screencenter;

  absolutetransform_ = transform_;
  transform_.translate( -offset.x(), - offset.y() );
  absolutetransform_ = transform_.inverted()*absolutetransform_;
  if ( nullptr != eventHandler() ) {
    eventHandler()->notifyMapChanges();
  }
}

QPoint Document::cartesianToScreen( const QPoint& point ) const
{
  QPoint newpnt = transform_.map(point);
  if ( newpnt.x() == -newpnt.x() || newpnt.y() == -newpnt.y() ) {
    //    warning_log << QObject::tr("Переполнение int");
  }
  return newpnt;
}

QPoint Document::screenToCartesian( const QPoint& point ) const
{
  QPoint newpnt = transform_.inverted().map(point);
  if ( newpnt.x() == -newpnt.x() || newpnt.y() == -newpnt.y() ) {
    //    warning_log << QObject::tr("Переполнение int");
  }
  return newpnt;
}

QPointF Document::cartesianToScreen( const QPointF& point ) const
{
  QPointF newpnt = transform_.map(point);
  return newpnt;
}

QPointF Document::screenToCartesian( const QPointF& point ) const
{
  QPointF newpnt = transform_.inverted().map(point);
  return newpnt;
}

QPolygon Document::cartesianToScreen( const QPolygon& poly ) const
{
  return transform_.map(poly);
}

QPolygon Document::screenToCartesian( const QPolygon& poly ) const
{
  return transform_.inverted().map(poly);
}

int32_t Document::dataSize( const QStringList& layeruuids ) const
{
  if ( true == stub_ ) {
    return -1;
  }
  int32_t pos = 0;
  pos += global::kBeginPtkppDocument.size();
  pos += sizeof(int32_t); //размер документа

  pos += typepostfix( property_.doctype() ).size();

  std::string str;
  property_.SerializeToString(&str);
  int32_t sz = str.size();
  pos += sizeof(sz);
  pos += sz;

  pos += sizeof(ramkawidth_);

  str.clear();
  info_.SerializeToString(&str);
  sz = str.size();
  pos += sizeof(sz);
  pos += sz;

  pos += sizeof(xfactor_);
  pos += sizeof(yfactor_);

  pos += sizeof(sz);

  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    Layer* l = layers_[i];
    if ( 0 != layeruuids.size() && -1 == layeruuids.indexOf( l->uuid() ) ) {
      continue;
    }
    pos += l->dataSize();
    pos += l->childDataSize();
  }

  pos += global::kFinishPtkppDocument.size();
  return pos;
}

int32_t Document::serializeToArray( char* arr, const QStringList& layeruuids ) const
{
  if ( nullptr == arr ) {
    error_log << QObject::tr("Нулевой указатель");
    return -1;
  }
  if ( true == stub_ ) {
    error_log << QObject::tr("Нельзя сохранять копию документа");
    return -1;
  }
  int32_t pos = 0;
  ::memcpy( arr + pos, global::kBeginPtkppDocument.data(), global::kBeginPtkppDocument.size() );
  pos += global::kBeginPtkppDocument.size();

  QByteArray loc = typepostfix( property_.doctype() );

  ::memcpy( arr + pos, loc.data(), loc.size() );

  pos += loc.size();

  int32_t sz = dataSize(layeruuids);


  int32_t sizeLayers = 0;
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    Layer* l = layers_[i];
    if ( 0 != layeruuids.size() && -1 == layeruuids.indexOf( l->uuid() ) ) {
      continue;
    }
    sizeLayers += l->dataSize();
    sizeLayers += l->childDataSize();
  }
  QByteArray arrlayers( sizeLayers, '\0' );
  int32_t layerpos = 0;
  for ( int i = 0, sz = layers_.size(); i < sz; ++i ) {
    Layer* l = layers_[i];
    if ( 0 != layeruuids.size() && -1 == layeruuids.indexOf( l->uuid() ) ) {
      continue;
    }
    layerpos += l->serializeToArray( arrlayers.data() + layerpos );
    layerpos += l->serializeChildsToArray( arrlayers.data() + layerpos );
  }

  QByteArray cmparr = qCompress(arrlayers);
  int32_t cmpsz = cmparr.size();
  int32_t diffsz = sizeLayers - cmpsz;


  sz = sz - diffsz + sizeof(sz);

  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);

  std::string str;
  property_.SerializeToString(&str);
  sz = str.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, str.data(), sz );
  pos += sz;

  ::memcpy( arr + pos, &ramkawidth_, sizeof(ramkawidth_) );
  pos += sizeof(ramkawidth_);

  str.clear();
  info_.SerializeToString(&str);
  sz = str.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, str.data(), sz );
  pos += sz;

  ::memcpy( arr + pos, &xfactor_, sizeof(xfactor_) );
  pos += sizeof(xfactor_);
  ::memcpy( arr + pos, &yfactor_, sizeof(yfactor_) );
  pos += sizeof(yfactor_);

  sz = layeruuids.size();
  if ( 0 == sz ) {
    sz = layers_.size();
  }
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);

  sz = cmparr.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, cmparr.data(), cmparr.size() );
  pos += cmparr.size();

  ::memcpy( arr + pos, global::kFinishPtkppDocument.data(), global::kFinishPtkppDocument.size() );
  pos += global::kFinishPtkppDocument.size();
  return pos;
}

int Document::parseFromArray( const char* arr )
{
  if ( true == stub_ ) {
    error_log << QObject::tr("Нельзя открыть документ в копии документа");
    return -1;
  }
  QByteArray loc( arr, global::kBeginPtkppDocument.size() );
  if ( loc != global::kBeginPtkppDocument ) {
    error_log << QObject::tr("Ошибка открытия документа");
    return -1;
  }
  int32_t pos = global::kBeginPtkppDocument.size();
  loc = QByteArray( arr + pos, 2 );
  if ( typepostfix(proto::kGeoMap) == loc
       || typepostfix(proto::kVerticalCut) == loc
       || typepostfix(proto::kAeroDiagram) == loc
       || typepostfix(proto::kOceanDiagram) == loc
       || typepostfix(proto::kMeteogram) == loc
       ) {
    pos += 2;
  }
  int32_t sz;
  global::fromByteArray( arr + pos, &sz );      //размер всего документа
  int32_t docsize = sz;
  pos += sizeof(sz);
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  if ( 0 != sz ) {
    proto::Document prop;
    if ( false == prop.ParseFromArray( arr + pos, sz ) ) {
      error_log << QObject::tr("Не удалось прочитать заголовок документа!");
      return -1;
    }
    if ( prop.uuid() == property_.uuid() ) {
      error_log << QObject::tr("Нельзя открыть документ в самом себе");
      return false;
    }
  }
  pos += sz;
  global::fromByteArray( arr + pos, &ramkawidth_ );
  pos += sizeof(ramkawidth_);
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  if ( 0 != sz ) {
    info_.ParseFromArray( arr + pos, sz );
    property_.set_raw_data( info_.document().raw_data() );
  }
  pos += sz;
  global::fromByteArray( arr + pos, &xfactor_ );
  pos += sizeof(xfactor_);
  global::fromByteArray( arr + pos, &yfactor_ );
  pos += sizeof(yfactor_);

  int32_t sizelayers;
  global::fromByteArray( arr + pos, &sizelayers );
  pos += sizeof(sizelayers);
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  QByteArray arrlayers;
  arrlayers.setRawData( arr + pos, sz );
  int32_t cmpsz = sz;
  QByteArray locbuf = qUncompress(arrlayers);
  if ( 0 != locbuf.size() ) {
    arrlayers = locbuf;
  }
  else {
    pos -= sizeof(sz);
    arrlayers.setRawData( arr + pos, docsize - pos );
    cmpsz = 0;
  }
  int32_t lpos = 0;
  for ( int i = 0; i < sizelayers; ++i ) {
    int32_t type;
    global::fromByteArray( arrlayers.data() + lpos, &type );
    Layer* l = singleton::PtkppFormat::instance()->createLayer( type, this );
    if ( nullptr == l ) {
      error_log << QObject::tr("Не удалось создать слой с типом %1").arg(type);
      return -1;
    }
    int32_t locpos = l->parseFromArray( arrlayers.data() + lpos );
    if ( -1 == locpos ) {
      error_log << QObject::tr("Не удалось открыть документ");
      return -1;
    }
    lpos += locpos;
    locpos = l->parseChildsFromArray( arrlayers.data() + lpos );
    if ( -1 == locpos ) {
      error_log << QObject::tr("Не удалось открыть документ");
      return -1;
    }
    lpos += locpos;
  }
  if ( 0 == cmpsz ) {
    cmpsz = lpos;
  }
  pos += cmpsz;
  if ( QByteArray( arr + pos, global::kFinishPtkppDocument.size() ) != global::kFinishPtkppDocument ) {
    warning_log << QObject::tr("Не ожиданный конец документа");
    return pos;
  }
  pos += global::kFinishPtkppDocument.size();
  if ( nullptr != eventHandler() ) {
    DocumentEvent* e = new DocumentEvent( DocumentEvent::DocumentLoaded );
    eventHandler()->postEvent(e);
  }
  return pos;
}

bool Document::hasGeoBasis() const
{
  for ( auto l : layers_ ) {
    if ( true == l->cache() ) {
      return true;
    }
  }
  return false;
}

QString Document::fileName( const QString& name, proto::DocumentFormat format )
{
  return QObject::tr("%1_%2_%3.%4")
      .arg(name)
      .arg( date().toString("yyyy_MM-dd_hh_mm_ss") )
      .arg( info().hour() )
      .arg( extension( format ).toLower() );
}

QString Document::extension( proto::DocumentFormat format )
{
  switch ( format ) {
    case proto::kJpeg:
    return "JPEG";
    case proto::kBmp:
    return "BMP";
    case proto::kPng:
    return "PNG";
    case proto::kSxf:
    return "SXF";
    case proto::kOdt:
    return "ODT";
    case proto::kOds:
    return "ODS";
    case proto::kPdf:
    return "PDF";
    case proto::kPtkpp:
      error_log << QObject::tr("Сохранение в формате ptkpp не реализовано");
    return "UNK";
    default:
    return "UNK";
  }
  return "UNK";
}

bool Document::saveDocumentToMongo( const QByteArray& arr, const QString& filename, proto::DocumentFormat fmt )
{
  return Document::saveDocumentToMongo( info(), arr, filename, fmt );
}

bool Document::saveDocumentToMongo( const proto::Map& docinfo, const QByteArray& arr, const QString& filename, proto::DocumentFormat fmt )
{
  QString id;
  if ( false == Document::saveDocumentToGridFs( filename, arr, &id ) ) {
    error_log << QObject::tr("Ошибка сохранения в gridfs");
    return false;
  }
  QString format = extension(fmt);

  if ( true == format.isEmpty() ) {
    error_log << QObject::tr("Неверный формат документа");
    return false;
  }


  std::unique_ptr<Dbi> db(meteo::global::dbDocument());
  if ( nullptr == db.get() ) {
    error_log << QObject::tr("Не удалось подключиться к БД");
    return false;
  }
  auto query = db->queryptrByName("update_document");
  if(nullptr == query) {return false;}

  query->arg("mapid", docinfo.name() );
  query->arg("jobid", docinfo.job_name() );
  query->argDt("dt", docinfo.datetime() );
  query->arg("hour", docinfo.hour() * 3600 );
  query->arg("center", docinfo.center() );
  query->arg("model", docinfo.model() );
  query->arg("format", format );
  query->arg("path", filename );
  query->arg("idfile", id );
  query->arg("map_title", docinfo.title() );
  query->arg("job_title", docinfo.job_title() );
  query->arg("dt_write", QDateTime::currentDateTimeUtc() );

  bool res = query->exec();

  if ( false == res ) {
    error_log << QObject::tr("Не удалось выполнить запрос %1").arg(query->query());
    return false;
  }

  const DbiEntry& doc = query->result();
  if ( 0 == static_cast<int>(doc.valueDouble("ok") ) ) {
    error_log << QObject::tr("Не удалось выполнить запрос %1")
                 .arg(query->query());
    return false;
  }
  return true;
}

bool Document::saveDocumentToGridFs( const QString& fileName, const QByteArray& data, QString* id )
{
  GridFs gridfs;
  GridFile file;
  auto dbparams = meteo::global::mongodbConfDocument();
  if ( false == gridfs.connect( dbparams ) ) {
    error_log <<  QObject::tr("Не удалось подключиться к БД с параметрами подключения %1:%2")
                  .arg( dbparams.host() )
                  .arg( dbparams.port() );
    return false;
  }
  gridfs.use( kDocumentdb, "fs" );

  bool res = gridfs.put( fileName, data, &file );
  if ( false == res ) {
    error_log << QObject::tr("Не удалось сохранить файл в GRIDFS")
              << gridfs.lastError();
    return false;
  }
  *id = file.id();
  return true;
}

bool Document::hasGradient() const
{
  for ( auto l : layers_ ) {
    if ( true == l->isBase() ) {
      continue;
    }
    if ( false == l->visible() ) {
      continue;
    }
    LayerIso* liso = maplayer_cast<LayerIso*>(l);
    if ( nullptr != liso ) {
      if ( true == liso->hasGradient() ) {
        return true;
      }
    }
  }
  return false;
};

bool Document::hasGradientWithWhiteSea() const
{
  return false;	//!< задумка с бесцветным морем - неудачная
  for ( auto l : layers_ ) {
    if ( true == l->isBase() ) {
      continue;
    }
    if ( false == l->visible() ) {
      continue;
    }
    LayerIso* liso = maplayer_cast<LayerIso*>(l);
    if ( nullptr != liso ) {
      if ( true == liso->hasGradient() && true == liso->fieldColor().white_sea_under_gradient() ) {
        return true;
      }
    }
  }
  return false;
}

bool Document::exportDocument( const meteo::map::proto::Map& reqinfo, QByteArray* arr )
{
  if ( nullptr == arr ) {
    error_log << QObject::tr("Нулевой указатель!");
    return false;
  }

  bool res = false;
  switch ( reqinfo.format() ) {
    case proto::kJpeg:
    case proto::kBmp:
    case proto::kPng:
      res = exportDocumentImage( reqinfo, arr);
    break;
    case proto::kSxf:
      res = exportDocumentSxf(arr);
    break;
    case proto::kPtkpp:
      res = exportDocumentPtkpp(reqinfo, arr);
    break;
    case proto::kOdt:
    case proto::kOds:
    case proto::kOdp:
      res = exportDocumentFormal(arr);
    break;
    case proto::kPdf:
      res = exportDocumentPdf(arr);
    break;
    default:
      res = false;
    break;
  }

  if ( false == res ) {
    error_log << QObject::tr("Не удалось получить документ в формате %1").arg(reqinfo.format());
  }

  return res;
}

bool Document::exportDocumentImage( const proto::Map& req, QByteArray* arr)
{
  if ( nullptr == arr ) {
    error_log << QObject::tr("Нулевой указатель!");
    return false;
  }

  QString format;
  switch( req.format() ) {
    case proto::kJpeg:
      format = "JPEG";
    break;
    case proto::kBmp:
      format = "BMP";
    break;
    case proto::kPng:
      format = "PNG";
    break;
    default:
    return false;
  }
  QImage pix( documentsize(), QImage::Format_ARGB32 );
  pix.fill( backgroundColor() );

  QPainter pntr(&pix);
  drawDocument(&pntr);

  QBuffer buf(arr);
  buf.open(QIODevice::WriteOnly);

  bool res = pix.save(&buf, format.toUtf8().data(), 100);
  if ( false == res ) {
    error_log << QObject::tr("Не удалось создать документ.");
  }
  return res;
}

bool Document::exportDocumentSxf( QByteArray* arr )
{
  if ( nullptr == arr ) {
    error_log << QObject::tr("Нулевой указатель!");
    return false;
  }

  QSharedPointer<sxf::PassportBasis> basis( new sxf::PassportBasis );
  *basis = passportBasisFromDocument(this);

  Sxf sxf;
  sxf.setPassportBasis(basis);
  sxf.setCoordType(true);

  QStringList uuids;
  for ( const auto layer : layers() ) {
    if ( false == layer->isBase() ) {
      uuids << layer->uuid();
    }
  }

  QImage img( 10, 10, QImage::Format_ARGB32 );
  QPainter p(&img);
  drawDocument(&p);

  if ( false == property().sxfttfsupported() ) {
    sxf.setTTFSupported(false);
  }

  *arr = sxf.exportDocument(uuids, *this, true);
  return true;
}

bool Document::exportDocumentPtkpp( const proto::Map& map, QByteArray* arr)
{
  Q_UNUSED(map);
  Q_UNUSED(arr);
  error_log << QObject::tr("Получение документа по Ptkpp не реализовано");
  return false;
}

bool Document::exportDocumentFormal( QByteArray* arr )
{
  auto d = document_cast<FormalDoc*>(this);
  if ( nullptr == d ) {
    return false;
  }
  *arr = d->getFile();
  return true;
}

bool Document::exportDocumentPdf( QByteArray* arr ) {
  bool res = exportDocumentFormal(arr);
  if ( false == res ) {
    return false;
  }
  QProcess proc;

  QString temp_template("/tmp/odf_to_pdf_convertation");
  QString libre("libreoffice");

  QTemporaryFile tmpfile(temp_template);
  tmpfile.open();
  QString tmpfilename = tmpfile.fileName();
  tmpfilename.replace(temp_template+ ".",temp_template);
  QFile odtfile(tmpfilename);
  odtfile.open(QIODevice::WriteOnly);
  odtfile.write(*arr);
  odtfile.flush();
  odtfile.close();

  QFileInfo tmpfi(tmpfilename);
  QString outdir = tmpfi.absolutePath();

  QString pdf_name = tmpfilename + ".pdf";

  QStringList args;
  args << "--headless"
       << "--convert-to"
       << "pdf"
       << "--outdir"
       << outdir
       << tmpfilename;

  proc.start( libre, args );
  proc.waitForFinished();
  QFile pdfile(pdf_name);
  pdfile.open(QIODevice::ReadOnly);
  *arr = pdfile.readAll();
  pdfile.close();
  odtfile.remove();
  pdfile.remove();

  return res;
}

sxf::PassportBasis passportBasisFromDocument( Document* doc )
{
  sxf::PassportBasis basis;

  if ( nullptr == doc ) {
    QObject::tr("Нулевой указатель");
    return basis;
  }

  switch(doc->projection()->type()) {
    case STEREO:
      basis.projection = 23;
    break;
    case MERCAT:
      basis.projection = 36;
    break;
    case CONICH:
      basis.projection = 20;
    break;
    default:
      basis.projection = 36;
    break;
  }
  basis.northWest = doc->screen2coord(doc->documentTopLeft());
  basis.northEast = doc->screen2coord(doc->documentTopRight());
  basis.southEast = doc->screen2coord(doc->documentBottomRight());
  basis.southWest = doc->screen2coord(doc->documentBottomLeft());
  basis.band = doc->documentRect();

  basis.scale = doc->santimeteresScale();
  basis.mainPoint = doc->projection()->getMapCenter();
  basis.firstMajorParallel = doc->projection()->getMapCenter().lat();
  basis.secondMajorParallel = 0;
  basis.northOffset = 0;
  basis.eastOffset = 0;
  return basis;
}

}
}
