#ifndef MAPPI_PRETREATMENT_IMAGES_IMAGE_H
#define MAPPI_PRETREATMENT_IMAGES_IMAGE_H

#include <QVector>
#include <QImage>

#include <mappi/pretreatment/savenotify/savenotify.h>
#include <mappi/global/streamheader.h>
#include <commons/funcs/tcolorgrad.h>

class GDALDataset;
class GDALDriver;

namespace mappi {

  namespace conf {
    class ImageTransform;
  }

  namespace po {
    class GeomCorrection;
  }

  class Image {
  public:
    Image(const QVector<uchar>& data, const meteo::global::PoHeader& pohead, SaveNotify* saveNotify);
    ~Image();

    //сохранить в соответствии с настройками
    bool save(const QString& baseName, const mappi::conf::ImageTransform& conf,
              const po::GeomCorrection& geom, QImage::Format format = QImage::Format_Indexed8);
    
    void applyFilters(const mappi::conf::ImageTransform& conf);

    //в png
    bool save(const mappi::conf::ImageTransform& conf, const QString& name, QImage::Format format = QImage::Format_Indexed8);
    //в geotiff
    bool saveGeo(const po::GeomCorrection& geom, const QString& name);
    void setPalete(const QVector<QRgb> &palette){ palette_ = palette; } ;
  private:

    GDALDataset* createWGS84(GDALDriver *driver, char **papszOptions, const po::GeomCorrection& geom, const QString& name);
    void createEPSG3857(GDALDriver *driver, char **papszOptions, GDALDataset*gds, const QString& name);
    void convertDataTo8bitIndexed();
    
  private:
    QVector<uchar> _imData;
    const meteo::global::PoHeader& _pohead;
    SaveNotify* _notify;
    
    uint _rows;
    uint _cols;
    //TColorGradList palette_;
    QVector<QRgb> palette_;
  };
  
}


#endif
