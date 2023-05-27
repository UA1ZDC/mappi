#ifndef METEO_MAP_COMMON_H
#define METEO_MAP_COMMON_H

#include <unordered_set>
#include <stdlib.h>

#include <qstring.h>
#include <qcolor.h>
#include <qpen.h>
#include <qhash.h>
#include <qmap.h>

#include <commons/geobasis/projection.h>
#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/sigwx.pb.h>

class QPainter;

namespace meteo {

enum PrimitiveTypes
{
  kPolygon  = 1,
  kText     = 2,
  kPixmap   = 3,
  kGroup    = 4,
  kGradient = 5,
  kPuanson  = 6,
  kWind     = 7,
  kIsoline  = 8,
  kRadar    = 9,
  kFlow     = 10,
  kFigure   = 11,
  kCloudWx  = 12,
  kLabel    = 13
};

class Generalization
{
  static const int LEVELSIZE;
  public:
    Generalization();
    Generalization( const General& proto );
    ~Generalization() {}

    void setLowLimit( int scale );
    void setHighLimit( int scale );
    void setLimits( int low, int high );
    void setScaleVisibility( int scale, bool value ) { if ( true == scales_.contains(scale) ) scales_.insert( scale, value ); }

    const QMap<int,bool>& scales() const { return scales_; }
    int lowLimit() const { return lowlimit_; }
    int highLimit() const { return highlimit_; }
    bool scaleVisible( int scale ) const { return ( true == scales_.contains(scale) ) ? scales_[scale] : false; }
    bool scaleVisible( float scale ) const
    {
      int sc = static_cast<int>(scale);
      if ( (scale - sc) >= 0.5 ) {
        sc += 1;
      }
      return scaleVisible(sc);
    }

    Generalization& loadProto( const General& proto );
    General proto() const ;

  private:
    QMap<int,bool> scales_;
    int lowlimit_;
    int highlimit_;
};

ProjectionType projection2proto( int type );
ProjectionType projection2proto( meteo::ProjType type );
meteo::ProjType proto2projection( ProjectionType type );

QPen pen2qpen( const Pen& pen );
Pen qpen2pen( const QPen& pen );

QBrush brush2qbrush( const Brush& brush );
Brush qbrush2brush( const QBrush& brush );

QFont font2qfont( const Font& font );
Font qfont2font( const QFont& font );

QPoint point2qpoint( const Point& point );
Point qpoint2point( const QPoint& point );

QPointF pointf2qpointf( const PointF& point );
PointF qpointf2pointf( const QPointF& point );

int position2qtalignmentflag( Position pos );
Position qtalignmentflag2position( int align );

QSize size2qsize( const Size& size );
Size qsize2size( const QSize& size );

QSizeF sizef2qsize( const SizeF& size );
SizeF qsize2size( const QSizeF& size );

GeoPoint pbgeopoint2geopoint( const GeoPointPb& gp );
GeoPointPb geopoint2pbgeopoint( const GeoPoint& gp );

GeoPoint surfGeopoint2geopoint( const meteo::surf::Point& gp );
meteo::surf::Point geopoint2SurfGeopoint( const GeoPoint& gp );


GeoVector pbgeovector2geovector( const GeoVectorPb& gv );
GeoVectorPb geovector2pbgeovector( const GeoVector& gv );

meteo::TypeGeoPointPb gptype2pbgptype( meteo::typeGeoCoord type );
meteo::typeGeoCoord pbgptype2gptype( meteo::TypeGeoPointPb type );

QRect rectByOrient( const QRect& rect, Position orient );

static const QString kMAPCOLORSETTINGS_IDENT("MAP_BORDER_BRUSH");
static const QString kMAPISLAND31_IDENT("31110001");
static const QString kMAP_INCUTBORDER_SETTINGS_IDENT("MAP_INCUTBORDER_STYLE");
static const QString kMAP_RAMKAPEN_SETTINGS_IDENT("MAP_RAMKA_PEN");

static const QColor kMAP_COAST_COLOR(56,123,248);
//static const QColor kMAP_ISLAND_COLOR(250,255,200);
static const QColor kMAP_ISLAND_COLOR(255,255,255);
static const QColor kMAP_SEA_COLOR(210,255,255);
static const QColor kMAP_HYDRO_COLOR(0,7,205);
static const QColor kMAP_DELIM_COLOR(255,54,195);
static const QColor kMAP_GRID_COLOR(127,127,127,255);
static const QColor kMAP_STATION_COLOR(47,200,255);
static const QColor kMAP_STATION_HYDRO_COLOR(255,0,0);
static const QColor kMAP_CITY_COLOR(47,200,255);
static const QColor kMAP_ISO_COLOR(0,0,0);

static const QColor kMAP_SEA_COLOR_SXF = QColor("#7bb3e0");

//static const QString kMAP_INCUTBORDER_STYLE("QLabel {border: 1px solid rgb(56,123,248);background-color: white;}");
static const QPen kMAP_RAMKAPEN_DEFAULT( QColor(56,123,248) );

QMap< QString, Property > mapSettings();
QMap< QString, Property > gridSettings();
   
int paintBit( QPainter* painter, QImage* img, const QPolygon& poly, int pnt_indx, QPointF* pnt );
bool angleOnPolygon( const QRect r, const QPolygon& poly, int pnt_indx, QPointF* pnt, float* resangle );

/*! 
 * \brief создание дефолтного файла с параметрами генерализации и расрашивания карты
 * \param filename - имя файла с настройкми
 */
void createDefaultColorSettings( const QString& filename );

void datadesc2weatherlayer( const field::DataDesc& datadesc, map::proto::WeatherLayer* layer );
void sigwx2weatherlayer( const surf::SigwxDesc& sigwx, map::proto::WeatherLayer* layer );

QString layernameFromInfo( const surf::SigwxDesc& info );
QString layernameFromInfo( const field::DataDesc& info );
QString layernameFromInfo( const map::proto::WeatherLayer& info );
QString replacePatternByDate( const QString& pattern, const QDateTime& dt );

}

#endif
