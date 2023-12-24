#ifndef MAPPI_UI_SATELLITEIMAGE_SATELLITEIMAGE_H
#define MAPPI_UI_SATELLITEIMAGE_SATELLITEIMAGE_H

#include <meteo/commons/proto/map_document.pb.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/object.h>

#include <mappi/proto/satelliteimage.pb.h>
#include <mappi/proto/thematic.pb.h>
#include <mappi/ui/pos/posgrid.h>

#include <gdal/gdal_priv.h>
#include <gdal/cpl_conv.h>
#include <gdal/cpl_string.h>
#include <gdal/gdal_alg.h>
#include <gdal/ogr_spatialref.h>
#include <gdal/ogr_geometry.h>
#include <gdal/ogr_featurestyle.h>
#include <gdal/gdalwarper.h>

struct GDALDatasetDeleter {
  void operator()(GDALDataset *dataset) const {
    if (dataset != nullptr) {
      GDALClose(dataset);
    }
  }
};

struct GDALWKTDeleter {
  void operator()(char *wkt) const {
    if (wkt != nullptr) {
      CPLFree(wkt);
    }
  }
};

struct GDALWarpOptionsDeleter {
  void operator()(GDALWarpOptions *warpOptions) const {
    if (warpOptions != nullptr) {
      GDALDestroyWarpOptions(warpOptions);
    }
  }
};

struct OCTCoordinateTransformationDeleter {
  void operator()(OGRCoordinateTransformation *poCT) const {
    if (poCT != nullptr) {
      OCTDestroyCoordinateTransformation(poCT);
    }
  }
};


typedef std::unique_ptr <GDALDataset, GDALDatasetDeleter> GDALDatasetPtr;
typedef std::unique_ptr <char, GDALWKTDeleter> GDALWKTPtr;
typedef std::unique_ptr <GDALWarpOptions, GDALWarpOptionsDeleter> GDALWarpOptionsPtr;
typedef std::unique_ptr <OGRCoordinateTransformation, OCTCoordinateTransformationDeleter> OGRCoordinateTransformationPtr;

namespace meteo {
namespace map {

class SatelliteImage: public meteo::map::Object
{
public:
  SatelliteImage(meteo::map::Layer* layer);
  SatelliteImage(meteo::Projection* projection);
  SatelliteImage(Object* parent);
  ~SatelliteImage();

  enum {
    kType = UserType + 10,
  };
  virtual int type() const { return kType; }

  Object* copy(meteo::map::Layer* l) const;
  Object* copy( Object* o ) const;
  Object* copy( meteo::Projection* grid ) const;

  QList<meteo::GeoVector> skeletInRect( const QRect& rect, const QTransform& transform ) const;

  bool render( QPainter* painter, const QRect& target, const QTransform& transform );

  int minimumScreenDistance( const QPoint& pos, QPoint* cross = 0 ) const;
  QList<QRect> boundingRect( const QTransform& transform ) const;

  bool hasValue() const { return false; }
  float value() const { return 0.0; }
  void setValue( float value, const QString& format = "4' '.2'0'", const QString& unit = QString() )
  { Q_UNUSED(value); Q_UNUSED(format); Q_UNUSED(unit); }

  virtual double coordsValue(const meteo::GeoPoint& gp, bool* ok = 0) const;

  virtual bool load(const QString& filename);
  bool loadImg(const QString& filename);

  bool georeferenceImg();
  bool reprojectImg();
  bool loadCache();
  bool setCacheCorners(GDALDataset *dataset, const OGRSpatialReference *poSource, const OGRSpatialReference *poTarget);
  virtual void buildCache();
  virtual void resetCache();

  ::mappi::conf::ThemType thematicType() const { return protoData_.type(); }

  QImage* img() {return &orig_;}

  void setProtoData(const ::mappi::proto::SatelliteImage& data);
  const ::mappi::proto::SatelliteImage& protoData() const {return protoData_;}

  void setColorToHide(int black, int white);
  void setBrightness(int brightness);
  void setContrast(int contrast);
  void setTransparency(int transparency);
  void setLocked(bool locked);
  bool isLocked() const { return locked_; }

  int brightness() const { return brightness_;}
  int contrast() const { return contrast_; }
  int blackMask() const { return blackCoeff_; }
  int whiteMask() const { return whiteCoeff_; }
  int transparency() const { return transparency_; }


  PosGrid* projection() const;

protected:
  QRect calcBoundingRect(map::Document* doc) const;

protected:
  QImage cache_;
  QImage orig_;
  QString fileName_;
  QString cacheFileName_;
  QRectF cacheCornersWGS84_;
  ::mappi::proto::SatelliteImage protoData_;

private:
  int blackCoeff_ = -9999;
  int whiteCoeff_ =  9999;
  int brightness_ = 0;
  int contrast_ = 100;
  int transparency_ = 255;

  GeoPoint topLeft_;
  GeoPoint bottomRight_;


  bool locked_ = false;
};

} // map
} // meteo

#endif // MAPPI_UI_SATELLITEIMAGE_SATELLITEIMAGE_H
