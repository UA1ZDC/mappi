#include "satelliteimage.h"

#include <qelapsedtimer.h>
#include <qpainter.h>
#include <QtMath>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/document.h>

#include <mappi/settings/mappisettings.h>
#include <mappi/ui/satlayer/satlayer.h>

#include <unordered_set>

namespace meteo {
  namespace map {

    SatelliteImage::SatelliteImage(meteo::map::Layer *layer) :
        Object(layer) {
    }

    SatelliteImage::SatelliteImage(meteo::Projection *projection) :
        Object(projection) {
    }

    SatelliteImage::SatelliteImage(meteo::map::Object *parent) :
        Object(parent) {
    }

    SatelliteImage::~SatelliteImage() {
    }

    meteo::map::Object *SatelliteImage::copy(meteo::map::Layer *l) const {
      if (0 == l) {
        error_log << QObject::tr("Нулевой указатель на слой");
        return 0;
      }
      SatelliteImage *si = new SatelliteImage(l);
      foreach(Object * o, objects_)
      {
        o->copy(si);
      }
      return si;
    }

    meteo::map::Object *SatelliteImage::copy(meteo::map::Object *o) const {
      if (0 == o) {
        error_log << QObject::tr("Нулевой указатель на объект");
        return 0;
      }
      SatelliteImage *si = new SatelliteImage(o);
      foreach(Object * obj, objects_)
      {
        obj->copy(si);
      }
      return si;
    }

    meteo::map::Object *SatelliteImage::copy(meteo::Projection *proj) const {
      if (0 == proj) {
        error_log << QObject::tr("Нулевой указатель на проекцию");
        return 0;
      }
      SatelliteImage *si = new SatelliteImage(proj);
      foreach(Object * obj, objects_)
      {
        obj->copy(si);
      }
      return si;
    }

    QList <meteo::GeoVector> SatelliteImage::skeletInRect(const QRect &rect, const QTransform &transform) const {
      Q_UNUSED(rect);
      Q_UNUSED(transform);
      QList <meteo::GeoVector> list;
      return list;
    }

    bool SatelliteImage::render(QPainter *painter, const QRect &docRect, const QTransform &transform) {
      Q_UNUSED(docRect);
      Q_UNUSED(transform);

      if (false == visible()) {
        return false;
      }

      QElapsedTimer t;
      t.start();

      if (cache_.isNull()) {
        buildCache();
        debug_log << "build cache:" << t.restart() << "msec.";
      }

      QList <QRect> list = boundingRect(transform);
      QRect boundRect;
      if (list.isEmpty()) {
        error_log << "Не удалось получить ограничивающий прямоугольник";
        return false;
      }

      boundRect = list.at(0);
      QRect cacheRect = cache_.rect();
      cacheRect.moveTopLeft(QPoint(0, 0));

      QPolygon source = QPolygon(cacheRect);
      QPolygon target = QPolygon(boundRect);

      QTransform matrix;
      QTransform::quadToQuad(source, target, matrix);

      QRect visBoundRect = docRect.intersected(boundRect);

      QTransform matrixInv = matrix.inverted();
      QRect visCacheRect = matrixInv.mapRect(visBoundRect);

      if (!visCacheRect.isValid()) {
        return false;
      }

      QImage img2 = cache_.copy(visCacheRect);
      QImage img(img2.transformed(matrix));

      uchar *pixels = img.bits();
      for (int i = 0, isz = 4 * (img.width() * img.height()); i < isz; i += 4) {
        //прозрачность
        if (0 != pixels[i + 3]) {
          pixels[i + 3] = transparency_;
        }
        //контраст
        int redContrast = ((pixels[i] - 127) * (contrast_) / 100) + 127;
        pixels[i] = qBound(0, redContrast, 255);
        int greenContrast = ((pixels[i + 1] - 127) * (contrast_) / 100) + 127;
        pixels[i + 1] = qBound(0, greenContrast, 255);
        int blueContrast = ((pixels[i + 2] - 127) * (contrast_) / 100) + 127;
        pixels[i + 2] = qBound(0, blueContrast, 255);
        //яркость
        int redBright = pixels[i] + ((brightness_) * 255 / 100);
        pixels[i] = qBound(0, redBright, 255);
        int greenBright = pixels[i + 1] + ((brightness_) * 255 / 100);
        pixels[i + 1] = qBound(0, greenBright, 255);
        int blueBright = pixels[i + 2] + ((brightness_) * 255 / 100);
        pixels[i + 2] = qBound(0, blueBright, 255);
        //маскирование
        if (blackCoeff_ >= 0 || whiteCoeff_ <= 255) {
          int gray = qGray(*(pixels + i), *(pixels + i + 1), *(pixels + i + 2));
          if (gray < blackCoeff_ || gray > whiteCoeff_) {
            pixels[i + 3] = 0;
          }
        }
      }

      painter->drawImage(visBoundRect.topLeft(), img);
      return true;
    }

    int SatelliteImage::minimumScreenDistance(const QPoint &pos, QPoint *cross) const {
      Q_UNUSED(pos);
      Q_UNUSED(cross);
      int dist = 10000000;
      return dist;
    }

    QList <QRect> SatelliteImage::boundingRect(const QTransform &transform) const {
      Q_UNUSED(transform);

      meteo::map::Document *doc = document();

      QPoint min = doc->coord2screen(topLeft_);
      QPoint max = doc->coord2screen(bottomRight_);

      return {QRect(min, max)};
    }

    double SatelliteImage::coordsValue(const meteo::GeoPoint &gp, bool *ok) const {
      Q_UNUSED(gp);

      if (0 != ok) { *ok = false; }

      return -9999;
    }

    bool SatelliteImage::loadImg(const QString &filename) {
      QElapsedTimer t;
      t.start();
      fileName_ = filename;
      if (!fileName_.endsWith(".to") && !fileName_.endsWith(".png") && !fileName_.endsWith(".tiff")) return false;
      if (fileName_.endsWith(".tiff")){
        cacheFileName_ = fileName_;
        cacheFileName_.replace(".tiff", "_cached.tiff");
        return reprojectImg();
      }

      if (fileName_.endsWith(".to")) fileName_.replace(".to", ".png");
      cacheFileName_ = fileName_;
      cacheFileName_.replace(".png", "_cached.tiff");

      if (!orig_.load(fileName_)) {
        error_log << QObject::tr("Не удалось загрузить изображение %1").arg(fileName_);
        return false;
      }
      orig_ = orig_.convertToFormat(QImage::Format_ARGB32);
      orig_ = orig_.scaledToWidth(projection()->samples(), Qt::SmoothTransformation);
      if (!projection()->buildGridF2X(orig_.width(), orig_.height())) {
        debug_log << QObject::tr("Не удалось построить сетку.");
        return false;
      }
      debug_log << QObject::tr("Построена сетка размером %1х%2").arg(projection()->samples()).arg(projection()->lines());
      return true;
    }

    bool SatelliteImage::load(const QString &filename) {
      return loadImg(filename);
    }

    bool SatelliteImage::georeferenceImg() {
      QElapsedTimer t;
      t.start();

      PosGrid *grid = projection();
      if (nullptr == grid) {
        debug_log << QObject::tr("Отсутствует проекция");
        return false;
      }

      GDALAllRegister();
      CPLSetConfigOption("GDAL_NUM_THREADS", "ALL_CPUS");
      GDALDriver *pMemDriver = (GDALDriver *) GDALGetDriverByName("MEM");
      GDALDriver *pGTiffDriver = (GDALDriver *) GDALGetDriverByName("GTiff");
      if (pMemDriver == nullptr) {
        error_log << QObject::tr("Не удалось загрузить драйвер GDAL: %1").arg(CPLGetLastErrorMsg());
        return false;
      }

      GDALDatasetPtr pDataset(pMemDriver->Create("", orig_.width(), orig_.height(), 4, GDT_Byte, NULL));
      if (!pDataset) {
        error_log << QObject::tr("Не удалось загрузить изображение: %1").arg(CPLGetLastErrorMsg());
        return false;
      }
      for (int i = 0; i < 4; i++) {
        GByte *bandData = new GByte[orig_.width() * orig_.height()];
        for (int y = 0; y < orig_.height(); y++) {
          for (int x = 0; x < orig_.width(); x++) {
            QColor pixelColor = QColor::fromRgba(orig_.pixel(x, y));
            switch (i) {
              case 0:
                bandData[y * orig_.width() + x] = pixelColor.red();
                break;
              case 1:
                bandData[y * orig_.width() + x] = pixelColor.green();
                break;
              case 2:
                bandData[y * orig_.width() + x] = pixelColor.blue();
                break;
              case 3:
                bandData[y * orig_.width() + x] = pixelColor.alpha();
                break;
            }
          }
        }
        GDALRasterBand *poBand = pDataset->GetRasterBand(i + 1);
        poBand->RasterIO(GF_Write, 0, 0, orig_.width(), orig_.height(), bandData, orig_.width(), orig_.height(), GDT_Byte, 0, 0, NULL);
        delete[] bandData;
      }
      debug_log << QObject::tr("Открыто изображение: %1x%2").arg(pDataset->GetRasterXSize()).arg(pDataset->GetRasterYSize());

      OGRSpatialReference oSourceSRS;
      oSourceSRS.SetWellKnownGeogCS("WGS84");
      char *rawSourceWKT = nullptr;
      oSourceSRS.exportToWkt(&rawSourceWKT);
      GDALWKTPtr pszWGS84WKT(rawSourceWKT);

      OGRSpatialReference oTargetSRS;
      oTargetSRS.importFromEPSG(3395);
      char *rawTargetWKT = nullptr;
      oTargetSRS.exportToWkt(&rawTargetWKT);
      GDALWKTPtr pszEPSG3395WKT(rawTargetWKT);

      const int GCPs4Width = 8;
      const int GCPs4Height = 16;
      const int maxGCPLength = (GCPs4Width+1) * (GCPs4Height+1);
      int realGCPLength = 0;
      QVector <GDAL_GCP> gcps(maxGCPLength);

      const float half_width = grid->samples() / 2.0;
      const float half_height = grid->lines() / 2.0;
      GDAL_GCP gcp;
      GDALInitGCPs(1, &gcp);
      gcp.dfGCPZ = 0.0;
      GeoPoint geoCoord;
      for (int index_x = 0; index_x <= GCPs4Width; index_x++){
        for (int index_y = 0; index_y <= GCPs4Height; index_y++){
          QPointF screenCoord(grid->samples() * index_x / GCPs4Width - half_width, grid->lines() * index_y  / GCPs4Height -  half_height);
          if (!grid->X2F_one(screenCoord, &geoCoord)) {
            debug_log << QObject::tr("Нет координат для (%2 %3)").arg(screenCoord.x() + half_width).arg(screenCoord.y() + half_height);
            continue;
          }
          gcp.dfGCPPixel = screenCoord.x() + half_width;
          gcp.dfGCPLine = screenCoord.y() + half_height;
          gcp.dfGCPX = MnMath::rad2deg(geoCoord.lon());
          gcp.dfGCPY = MnMath::rad2deg(geoCoord.lat());
          gcps[realGCPLength] = gcp;
          realGCPLength++;

          debug_log << QObject::tr("Point %1: (%2 %3) (%4 %5)").arg(realGCPLength)
              .arg(gcp.dfGCPPixel).arg(gcp.dfGCPLine)
              .arg(gcp.dfGCPX).arg(gcp.dfGCPY);
        }
      }
      gcps.resize(realGCPLength);
      GDALSetGCPs(pDataset.get(), gcps.size(), gcps.data(), "");
      if (realGCPLength < 20) {
        error_log << QObject::tr("Недостаточно контрольных точек для привязки: %1").arg(realGCPLength);
        return false;
      }

      debug_log << QObject::tr("Получено %1 GCP").arg(gcps.size());

      GDALWarpOptionsPtr psWarpOptions(GDALCreateWarpOptions());
      psWarpOptions->nBandCount = 0;
      psWarpOptions->eResampleAlg = GRA_Cubic; // или GRA_Lanczos
      psWarpOptions->papszWarpOptions = CSLSetNameValue(psWarpOptions->papszWarpOptions, "NUM_THREADS", "ALL_CPUS");
      GDALDatasetPtr pGeocodedDataset(static_cast<GDALDataset *>(GDALAutoCreateWarpedVRT(pDataset.get(), NULL, pszWGS84WKT.get(), psWarpOptions->eResampleAlg, 1.0, psWarpOptions.get())));
      if (!pGeocodedDataset) {
        error_log << QObject::tr("Не удалось привязать изображение");
        return false;
      }
      debug_log << QObject::tr("Привязано изображение: %1x%2").arg(pGeocodedDataset->GetRasterXSize()).arg(
          pGeocodedDataset->GetRasterYSize());

      GDALDatasetPtr pWarpedDataset( static_cast<GDALDataset *>(GDALAutoCreateWarpedVRT(pGeocodedDataset.get(), nullptr, pszEPSG3395WKT.get(), psWarpOptions->eResampleAlg, 1.0, psWarpOptions.get())));
      if (!pWarpedDataset) {
        error_log << QObject::tr("Не удалось перепроецировать изображение");
        return false;
      }
      if(pWarpedDataset->GetRasterCount() >= 3){
        pWarpedDataset->GetRasterBand(1)->SetColorInterpretation(GCI_RedBand);
        pWarpedDataset->GetRasterBand(2)->SetColorInterpretation(GCI_GreenBand);
        pWarpedDataset->GetRasterBand(3)->SetColorInterpretation(GCI_BlueBand);
      }
      if(pWarpedDataset->GetRasterCount() == 4) {
        pWarpedDataset->GetRasterBand(4)->SetColorInterpretation(GCI_AlphaBand);
      }
      pWarpedDataset->SetMetadataItem("COMPRESS", "DEFLATE");
      pWarpedDataset->SetMetadataItem("PREDICTOR", "2");
      debug_log << QObject::tr("Перепроецировано изображение: %1x%2").arg(pWarpedDataset->GetRasterXSize()).arg(pWarpedDataset->GetRasterYSize());

      GDALDatasetPtr pOutputDataset(pGTiffDriver->CreateCopy(cacheFileName_.toStdString().c_str(), pWarpedDataset.get(), FALSE, nullptr, nullptr, nullptr));
      int xsize = pOutputDataset->GetRasterXSize();
      int ysize = pOutputDataset->GetRasterYSize();
      cache_ = QImage(xsize, ysize, QImage::Format_RGBA8888);
      cache_.fill(QColor(0, 0, 0, 255));

      CPLErr ioError = CE_None;
      for (int i = 1; i <= pOutputDataset->GetRasterCount(); ++i) {
        CPLErr bandErr = pOutputDataset->GetRasterBand(i)->RasterIO(GF_Read, 0, 0, xsize, ysize, cache_.bits() + i - 1, xsize, ysize, GDT_Byte, 4, 0);
        if (bandErr != CE_None) ioError = bandErr;
      }
      if (ioError != CE_None) {
        error_log << QObject::tr("Не удалось загрузить изображение в QImage: %1").arg(CPLGetLastErrorMsg());
        return false;
      }

      debug_log << "завершено за" << t.elapsed()/1000.0 << "секунд";
      return setCacheCorners(pOutputDataset.get(), &oTargetSRS, &oSourceSRS);
    }

    bool SatelliteImage::reprojectImg() {
      QElapsedTimer t;
      t.start();
      if (QFile::exists(cacheFileName_)) return loadCache();

      GDALAllRegister();
      CPLSetConfigOption("GDAL_NUM_THREADS", "ALL_CPUS");
      GDALDriver *pGTiffDriver = (GDALDriver *) GDALGetDriverByName("GTiff");
      if (pGTiffDriver == nullptr) {
        error_log << QObject::tr("Не удалось загрузить драйвер GDAL: %1").arg(CPLGetLastErrorMsg());
        return false;
      }

      GDALDatasetPtr pGeocodedDataset(static_cast<GDALDataset *>(GDALOpen(fileName_.toStdString().c_str(), GA_ReadOnly)));
      if (!pGeocodedDataset) {
        error_log << QObject::tr("Не удалось открыть файл: %1").arg(CPLGetLastErrorMsg());
        return false;
      }

      const char *rawSourceWKT = pGeocodedDataset->GetProjectionRef();
      OGRSpatialReference oSourceSRS;
      oSourceSRS.importFromWkt(&rawSourceWKT);
      GDALWKTPtr pszWGS84WKT(strdup(rawSourceWKT));
      if (oSourceSRS.IsEmpty()){
        error_log << QObject::tr("Файл не имеет проекции");
        return false;
      }

      if (oSourceSRS.GetEPSGGeogCS() == 3395) {
        cacheFileName_ = fileName_;
        return loadCache();
      }

      OGRSpatialReference oTargetSRS;
      oTargetSRS.importFromEPSG(3395);
      char *rawTargetWKT = nullptr;
      oTargetSRS.exportToWkt(&rawTargetWKT);
      GDALWKTPtr pszEPSG3395WKT(rawTargetWKT);

      GDALWarpOptionsPtr psWarpOptions(GDALCreateWarpOptions());
      psWarpOptions->nBandCount = 0;
      psWarpOptions->eResampleAlg = GRA_Cubic; // или GRA_Lanczos
      psWarpOptions->papszWarpOptions = CSLSetNameValue(psWarpOptions->papszWarpOptions, "NUM_THREADS", "ALL_CPUS");
      GDALDatasetPtr pWarpedDataset( static_cast<GDALDataset *>(GDALAutoCreateWarpedVRT(pGeocodedDataset.get(), nullptr, pszEPSG3395WKT.get(), psWarpOptions->eResampleAlg, 1.0, psWarpOptions.get())));
      if (!pWarpedDataset) {
        error_log << QObject::tr("Не удалось перепроецировать изображение");
        return false;
      }
      if(pWarpedDataset->GetRasterCount() >= 3){
        pWarpedDataset->GetRasterBand(1)->SetColorInterpretation(GCI_RedBand);
        pWarpedDataset->GetRasterBand(2)->SetColorInterpretation(GCI_GreenBand);
        pWarpedDataset->GetRasterBand(3)->SetColorInterpretation(GCI_BlueBand);
      }
      if(pWarpedDataset->GetRasterCount() == 4) {
        pWarpedDataset->GetRasterBand(4)->SetColorInterpretation(GCI_AlphaBand);
      }
      pWarpedDataset->SetMetadataItem("COMPRESS", "DEFLATE");
      pWarpedDataset->SetMetadataItem("PREDICTOR", "2");
      debug_log << QObject::tr("Перепроецировано изображение: %1x%2").arg(pWarpedDataset->GetRasterXSize()).arg(pWarpedDataset->GetRasterYSize());

      GDALDatasetPtr pOutputDataset(pGTiffDriver->CreateCopy(cacheFileName_.toStdString().c_str(), pWarpedDataset.get(), FALSE, nullptr, nullptr, nullptr));
      int xsize = pOutputDataset->GetRasterXSize();
      int ysize = pOutputDataset->GetRasterYSize();
      cache_ = QImage(xsize, ysize, QImage::Format_RGBA8888);
      cache_.fill(QColor(0, 0, 0, 255));

      CPLErr ioError = CE_None;
      for (int i = 1; i <= pOutputDataset->GetRasterCount(); ++i) {
        CPLErr bandErr = pOutputDataset->GetRasterBand(i)->RasterIO(GF_Read, 0, 0, xsize, ysize, cache_.bits() + i - 1, xsize, ysize, GDT_Byte, 4, 0);
        if (bandErr != CE_None) ioError = bandErr;
      }
      if (ioError != CE_None) {
        error_log << QObject::tr("Не удалось загрузить изображение в QImage: %1").arg(CPLGetLastErrorMsg());
        return false;
      }

      debug_log << "завершено за" << t.elapsed()/1000.0 << "секунд";
      return setCacheCorners(pOutputDataset.get(), &oTargetSRS, &oSourceSRS);
    }

    bool SatelliteImage::loadCache() {
      QElapsedTimer t;
      t.start();

      GDALAllRegister();
      CPLSetConfigOption("GDAL_NUM_THREADS", "ALL_CPUS");
      GDALDriver *pGTiffDriver = (GDALDriver *) GDALGetDriverByName("GTiff");
      if (pGTiffDriver == nullptr) {
        error_log << QObject::tr("Не удалось загрузить драйвер GDAL: %1").arg(CPLGetLastErrorMsg());
        return false;
      }

      OGRSpatialReference oSourceSRS;
      oSourceSRS.SetWellKnownGeogCS("WGS84");
      char *rawSourceWKT = nullptr;
      oSourceSRS.exportToWkt(&rawSourceWKT);
      GDALWKTPtr pszWGS84WKT(rawSourceWKT);

      OGRSpatialReference oTargetSRS;
      oTargetSRS.importFromEPSG(3395);
      char *rawTargetWKT = nullptr;
      oTargetSRS.exportToWkt(&rawTargetWKT);
      GDALWKTPtr pszEPSG3395WKT(rawTargetWKT);

      GDALDatasetPtr pOutputDataset(static_cast<GDALDataset *>(GDALOpen(cacheFileName_.toStdString().c_str(), GA_ReadOnly)));
      int xsize = pOutputDataset->GetRasterXSize();
      int ysize = pOutputDataset->GetRasterYSize();
      cache_ = QImage(xsize, ysize, QImage::Format_RGBA8888);
      cache_.fill(QColor(0, 0, 0, 255));

      CPLErr ioError = CE_None;
      if(pOutputDataset->GetRasterCount() == 1){
        for (int i = 1; i <= 3; ++i) {
          CPLErr bandErr = pOutputDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, xsize, ysize, cache_.bits() + i - 1, xsize, ysize, GDT_Byte, 4, 0);
          if (bandErr != CE_None) ioError = bandErr;
        }
      }else{
        for (int i = 1; i <= pOutputDataset->GetRasterCount() && i <= 4; ++i) {
          CPLErr bandErr = pOutputDataset->GetRasterBand(i)->RasterIO(GF_Read, 0, 0, xsize, ysize, cache_.bits() + i - 1, xsize, ysize, GDT_Byte, 4, 0);
          if (bandErr != CE_None) ioError = bandErr;
        }
      }
      if (ioError != CE_None) {
        error_log << QObject::tr("Не удалось загрузить изображение в QImage: %1").arg(CPLGetLastErrorMsg());
        return false;
      }

      debug_log << "завершено за" << t.elapsed()/1000.0 << "секунд";
      return setCacheCorners(pOutputDataset.get(), &oTargetSRS, &oSourceSRS);
    }

    bool SatelliteImage::setCacheCorners(GDALDataset *dataset, const OGRSpatialReference *poSource, const OGRSpatialReference *poTarget){
      if(dataset == nullptr) return false;

      double adfGeoTransform[6];
      int xsize = dataset->GetRasterXSize();
      int ysize = dataset->GetRasterYSize();

      if (dataset->GetGeoTransform(adfGeoTransform) != CE_None) {
        error_log << QObject::tr("Невозможно получить координаты углов спутникового снимка");
        return false;
      }
      double minLon = adfGeoTransform[0];
      double maxLon = adfGeoTransform[0] + adfGeoTransform[1] * xsize;
      double maxLat = adfGeoTransform[3];
      double minLat = adfGeoTransform[3] + adfGeoTransform[5] * ysize;

      OGRCoordinateTransformationPtr poCT(OGRCreateCoordinateTransformation(poSource, poTarget));
      if (!poCT->Transform(1, &minLon, &maxLat) || !poCT->Transform(1, &maxLon, &minLat)) {
        error_log << QObject::tr("Невозможно перепроецировать координаты углов спутникового снимка");
        return false;
      }

      double leftX = std::min(maxLat, minLat);
      double width = std::max(maxLat, minLat) - std::min(maxLat, minLat);
      double topY  = std::min(maxLon, minLon);
      double height = std::max(maxLon, minLon) - std::min(maxLon, minLon);

      cacheCornersWGS84_ = QRectF(leftX, topY, width, height);
      debug_log << cacheCornersWGS84_.left() << cacheCornersWGS84_.right() << cacheCornersWGS84_.top() << cacheCornersWGS84_.bottom();
      return true;
    }

    void SatelliteImage::buildCache() {
      QElapsedTimer t;
      t.start();

      PosGrid *grid = projection();
      if (nullptr == grid) {
        debug_log << QObject::tr("Отсутствует проекция");
        return;
      }

      if (QFile::exists(cacheFileName_)) {
        if(!loadCache()){
          debug_log << QObject::tr("Невозможно загрузить изображение из кеша!");
          QFile::remove(cacheFileName_);
          if(!georeferenceImg()) return;
        }
      } else {
        if(!georeferenceImg()){
          debug_log << QObject::tr("Невозможно привязать изображение к координатной сетке");
          return;
        }
      }

      map::proto::Document docParam = grid->document()->property();
      *docParam.mutable_doc_center() = meteo::geopoint2pbgeopoint(document()->center());
      *docParam.mutable_map_center() = meteo::geopoint2pbgeopoint(document()->projection()->getMapCenter());

      map::Document tmpdoc;
      if (!tmpdoc.init(docParam)) {
        return;
      }

      QRect screenBoundRect = calcBoundingRect(document());
      GeoVector gv = document()->screen2coord(QPolygon(screenBoundRect));

      topLeft_ = gv.value(0, GeoPoint());
      bottomRight_ = gv.value(2, GeoPoint());

      const QRect cacheBoundRect = calcBoundingRect(&tmpdoc);
      if (cacheBoundRect.width() > 6000 || cacheBoundRect.height() > 6000) {
        error_log << QObject::tr("Ошибка размера рамки");
        return;
      }
    }

    void SatelliteImage::resetCache() {
      cache_ = QImage();
    }

    void SatelliteImage::setProtoData(const ::mappi::proto::SatelliteImage &data) {
      protoData_.CopyFrom(data);
    }

    void SatelliteImage::setColorToHide(int black, int white) {
      if (locked_) return;
      blackCoeff_ = black;
      whiteCoeff_ = white;
    }

    void SatelliteImage::setBrightness(int brightness) {
      if (!locked_) brightness_ = brightness;
    }

    void SatelliteImage::setContrast(int contrast) {
      if (!locked_) contrast_ = contrast;
    }

    void SatelliteImage::setTransparency(int transparency) {
      if (!locked_) transparency_ = transparency;
    }

    void SatelliteImage::setLocked(bool locked) {
      locked_ = locked;
      document()->eventHandler()->notifyLayerChanges(layer_, LayerEvent::ObjectChanged);
    }

    PosGrid *SatelliteImage::projection() const {
      SatLayer *l = maplayer_cast<SatLayer *>(layer());

      if (nullptr == l) { return nullptr; }

      return l->projection();
    }

    QRect SatelliteImage::calcBoundingRect(Document* doc) const
    {
      PosGrid* grid = projection();
      if ( nullptr == grid ) {
        debug_log << QObject::tr("Отсутствует проекция");
        return QRect();
      }

      if(!cacheCornersWGS84_.isValid()){
        debug_log << QObject::tr("Не заданы координаты углов cache");
        debug_log << cacheCornersWGS84_;
        return QRect();
      }

      bool res = true;
      QPoint topLeftPoint = doc->coord2screen(GeoPoint::fromDegree(cacheCornersWGS84_.bottom(), cacheCornersWGS84_.left()), &res);
      QPoint rightBottomPoint = doc->coord2screen(GeoPoint::fromDegree(cacheCornersWGS84_.top(), cacheCornersWGS84_.right()), &res);
      if(!res){
        debug_log << QObject::tr("Невзможно перепроецировать углы");
        return QRect();
      }
      return QRect(topLeftPoint, rightBottomPoint);
    }

  } // map
} // meteo
