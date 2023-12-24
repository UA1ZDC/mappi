#include "import.h"

#include <mappi/proto/satellite.pb.h>
#include <mappi/proto/thematic.pb.h>

#include <mappi/global/streamheader.h>
#include <cross-commons/debug/tlog.h>
#include <mappi/pretreatment/formats/satformat.h>
#include <mappi/pretreatment/savenotify/savenotify.h>

#include <gdal/gdal_priv.h>
#include <gdal/gdal_alg.h>
#include <gdal/ogr_spatialref.h>
#include <gdal/ogr_geometry.h>
#include <gdal/ogr_featurestyle.h>
#include <gdal/gdalwarper.h>

namespace mappi::Import {
  GDALImporter::GDALImporter(){
    GDALAllRegister();
    _store = QSharedPointer<mappi::to::DataStore>(new mappi::to::DataServiceStore());
    _notify = QSharedPointer<mappi::SaveNotify>(mappi::CreateNotify::createServiceNotify(mappi::CreateNotify::StubServiceNotify));
    _dtStart = QDateTime::currentDateTimeUtc();
    _dtStop = _dtStart.addSecs(120);
  }

  GDALImporter::~GDALImporter(){
    closeDataset();
  }


  void GDALImporter::closeDataset(){
    if (_ds == nullptr) return;
    GDALClose(_ds);
    _ds = nullptr;
  }

  void GDALImporter::clearImage() {
    _loadPath = "";
    _them_name = "Импорт";
    _image = QImage();
    _rasterBandsCount = 0;
    for (int i = 0; i <= ColorChannel::ALPHA; i++) _channels[i] = 0;
    closeDataset();
  }

  bool GDALImporter::load(const QString &loadPath) {
    clearImage();

    _loadPath = loadPath;
    _them_name = "Импорт: " + _loadPath.toStdString();
    _ds = static_cast<GDALDataset*>(GDALOpen(_loadPath.toLocal8Bit(), GA_ReadOnly));
    if (_ds == nullptr) {
      error_log << QObject::tr("Ошибка открытия файла: %1").arg(_loadPath);
      return false;
    }

    int xsize = _ds->GetRasterXSize();
    int ysize = _ds->GetRasterYSize();
    _image = QImage(xsize, ysize, QImage::Format_RGBA8888);
    _image.fill(QColor(0, 0, 0, 255));

    _rasterBandsCount = _ds->GetRasterCount();
    if(_rasterBandsCount == 0){
      error_log << QObject::tr("В файле нет растровых каналов");
      closeDataset();
      return false;
    }

    if(_rasterBandsCount == 1) {
      setBand(ColorChannel::RED, 1);
      setBand(ColorChannel::GREEN, 1);
      setBand(ColorChannel::BLUE, 1);
      _channels[ColorChannel::ALPHA] = 0;
      return true;
    }

    for (unsigned int i = 1; i <= _rasterBandsCount; ++i) {
      GDALRasterBand *band = _ds->GetRasterBand(i);
      switch (band->GetColorInterpretation()) {
        case GCI_RedBand:
          setBand(ColorChannel::RED, i);
          debug_log << QObject::tr("Заполнен красный канал");
          break;
        case GCI_GreenBand:
          setBand(ColorChannel::GREEN, i);
          debug_log << QObject::tr("Заполнен зеленый канал");
          break;
        case GCI_BlueBand:
          setBand(ColorChannel::BLUE, i);
          debug_log << QObject::tr("Заполнен синий канал");
          break;
        case GCI_AlphaBand:
          setBand(ColorChannel::ALPHA, i);
          debug_log << QObject::tr("Заполнен канал прозрачности");
          break;
        default:
          warning_log << "Необработанный канал " << i << " Тип: " << band->GetColorInterpretation();
      }
    }
    for(int c = 0; c < ColorChannel::ALPHA; c++) {
      if(_channels[c] == 0) setBand(static_cast<ColorChannel>(c), 1);
    }
    return true;
  }

  bool GDALImporter::save(const QString &filePath){
    if (!saveImage(filePath)) {
      error_log << QObject::tr("Ошибка сохранения файла: %1").arg(filePath);
      return false;
    }
    if (!createSession(filePath)) {
      error_log << QObject::tr("Ошибка создания сессии: %1").arg(filePath);
      return false;
    }
    if (!saveData(filePath)) {
      error_log << QObject::tr("Ошибка записи в БД: %1").arg(filePath);
      return false;
    }
    return true;
  }

  bool GDALImporter::saveImage(const QString &filePath){
    if(filePath.endsWith(".png")) {
      QImage temp_image;
      switch (_format) {
        case GRAYSCALE:
          temp_image = _image.convertToFormat(QImage::Format_Grayscale8);
          break;
        case RGB:
          temp_image = _image.convertToFormat(QImage::Format_RGB888);
          break;
        default:
          temp_image = _image.convertToFormat(QImage::Format_RGBA8888);
      }
      return temp_image.save(filePath, "PNG");
    }
    if(filePath.endsWith(".tiff")) {
      if(_loadPath.endsWith(".tiff")) return QFile::copy(_loadPath, filePath);

      GDALDataset *importDataset = (GDALDataset *)GDALOpen(_loadPath.toStdString().c_str(), GA_ReadOnly);
      if (importDataset == nullptr) return false;
      GDALDriver *gtiffDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
      if (gtiffDriver == nullptr) return false;
      GDALDataset *gtiffDataset = gtiffDriver->CreateCopy(filePath.toStdString().c_str(), importDataset, FALSE, nullptr, nullptr, nullptr);
      if (gtiffDataset == nullptr) return false;

      gtiffDataset->SetProjection(importDataset->GetProjectionRef());
      gtiffDataset->FlushCache();
      GDALClose(importDataset);
      GDALClose(gtiffDataset);
      return true;
    }
    return false;
  }

  bool GDALImporter::createSession(const QString &filePath){
    if(_notify.isNull()) return false;

    meteo::global::StreamHeader strhead;
    Satellite sat;
    QString weatherFile = mappi::po::singleton::SatFormat::instance()->getWeatherFilePath();
    sat.readTLE(_satellite, weatherFile);
    auto receptionConf = mappi::po::singleton::SatFormat::instance()->getWeatherFilePath();
    QString siteName = mappi::po::singleton::SatFormat::instance()->getSiteName();
    meteo::GeoPoint siteCoord = mappi::po::singleton::SatFormat::instance()->getSiteCoord();

    strhead.site = siteName;
    strhead.siteCoord = siteCoord;
    strhead.satellite = _satellite;
    //strhead.direction = ctx->session.data().direction;
    strhead.tle = sat.getTLEParams();
    strhead.start = _dtStart;
    strhead.stop = _dtStop;
    if(!_notify->rawNotify(strhead, _loadPath)) return false;

    meteo::global::PoHeader pohead;
    pohead.start = _dtStart;
    pohead.stop = _dtStop;
    pohead.satellite = _satellite;
    pohead.name = getName();
    pohead.instr = _instr;
    pohead.channel = _channel;
    return _notify->dataNotify(pohead, _loadPath);
  }

  bool GDALImporter::saveData(const QString &filePath) {
    meteo::global::PoHeader pohead;
    pohead.start = _dtStart;
    pohead.stop = _dtStop;
    pohead.satellite = _satellite;
    pohead.name = getName();
    pohead.instr = _instr;
    pohead.channel = _channel;
    if(_store.isNull()) return false;
    return _store->save(pohead, mappi::conf::ThemType::kImport, _them_name, filePath, "png");
  }

  bool GDALImporter::setBand(ColorChannel color, unsigned int idx) {
    if(idx < 1){
      if(color == ColorChannel::ALPHA){
        _channels[ColorChannel::ALPHA] = 0;
        QImage alphaChannel(_image.size(), QImage::Format_Grayscale8);
        alphaChannel.fill(255);
        _image.setAlphaChannel(alphaChannel);
        return true;
      }
      return false;
    }

    if(idx > _rasterBandsCount) return false;
    GDALRasterBand *band = _ds->GetRasterBand(idx);
    int xsize = _ds->GetRasterXSize();
    int ysize = _ds->GetRasterYSize();

    CPLErr bandError = band->RasterIO(GF_Read, 0, 0, xsize, ysize, _image.bits() + color, xsize, ysize, GDT_Byte, 4, 0);
    if (bandError == CE_Failure || bandError == CE_Fatal){
      error_log << QObject::tr("Невозможно получить данные из канала");
      return false;
    }
    _channels[color] = idx;
    return true;
  }
}