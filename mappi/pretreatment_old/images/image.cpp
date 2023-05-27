#include "image.h"
#include "geomcorr.h"
#include "filters.h"
#include "colors.h"

#include <mappi/proto/satellite.pb.h>
#include <cross-commons/debug/tlog.h>

// #include <tiffio.h>
// #include <geotiffio.h>
// #include <xtiffio.h>
// #include <geokeys.h>

#include <QDir>

#include <gdal/gdal_priv.h>
#include <gdal/gdal_alg.h>
#include <gdal/ogr_spatialref.h>
#include <gdal/ogr_geometry.h>
#include <gdal/ogr_featurestyle.h>
#include <gdal/gdalwarper.h>



// #define NODATA 42113
// static const TIFFFieldInfo xtiffFieldInfo[] =
//   {
//    {NODATA, -1, -1, TIFF_ASCII, FIELD_CUSTOM, 1, 0, const_cast<char*>("GDAL_NODATA")}
//   };   

// #define N(a) (sizeof(a) / sizeof (a[0]))




using namespace mappi;

Image::Image(const QVector<uchar>& data, const meteo::global::PoHeader& pohead, SaveNotify* notify):
  _imData(data),
  _pohead(pohead),
  _notify(notify),
  _rows(pohead.lines),
  _cols(pohead.samples)
{
  // _palette.resize(256);
  // for (uint i=0; i< 256; i++) {
  //   _palette[i] = QRgb((i<<16)+ (i<<8) + i);
  // }
  
}


Image::~Image()
{
}


bool Image::save(const QString& baseName, const mappi::conf::ImageTransform& conf, const po::GeomCorrection& geom,
                 QImage::Format format)
{
  bool ok = true;

  applyFilters(conf);

  ok |= save(conf, baseName, format);

  if (conf.geocoding()) {
    ok |= saveGeo(geom, baseName);
  }

  return ok;
}

void Image::applyFilters(const mappi::conf::ImageTransform& conf)
{
  if (_imData.isEmpty() || _cols == 0 || _rows == 0) {
    error_log << QObject::tr("Данные не инициализированны");
    return;
  }
  
  for (auto filter : conf.filter()) {
    info_log << QObject::tr("Фильтрация. Тип %1").arg(filter);
    switch (filter) {
      case conf::kInvert:
        colors::invert(_imData.data(), _imData.size());
      break;
      case conf::kStretchHist:
        colors::stretchHist(_imData.data(), _imData.size());
      break;
      case conf::kEqualization:
        colors::equalization(_imData.data(), _imData.size(), _cols, _rows);
      break;
      case conf::kFillEmpty:
        //TODO
      break;
      case conf::kMedian3:
        meteo::median3(_rows, _cols, &_imData);
      break;
      case conf::kMedian5:
        meteo::median5(_rows, _cols, &_imData);
      break;
      case conf::kNagaoMacuyamaModify:
        meteo::nagaoMacuyamaModify(_rows, _cols, &_imData);
      break;
      default: {}
    }
  }
}

bool Image::save(const mappi::conf::ImageTransform& conf, const QString& name, QImage::Format format)
{
  if (name.isEmpty() || _imData.size() == 0) {
    return false;
  }
  
  QDir dir;
  dir.mkpath(name.left(name.lastIndexOf("/")));

  QImage imqt;
  switch (format) {
    case QImage::Format_Grayscale8:
      imqt = QImage(_imData.data(), _cols, _rows, _cols, format);
    break;
    case QImage::Format_Indexed8:
      imqt = QImage(_imData.data(), _cols, _rows, _cols, format);
      if(false == palette_.isEmpty()){
        imqt.setColorTable(palette_);
      }
    break;
    default:
      imqt = QImage(_imData.data(), _cols, _rows, _cols*4, format);
      imqt = imqt.convertToFormat(QImage::Format_Indexed8);
      int siz = imqt.sizeInBytes();
      _imData.resize(siz);
      const uchar * bit = imqt.bits();
      for(int i =0 ; i< siz; ++i){
        _imData[i] = bit[i];
      }
    break;
  }

  if (conf.has_width()) {
    imqt = imqt.scaled(conf.width(), imqt.height(), Qt::IgnoreAspectRatio);
  }
  if (conf.has_height()) {
    imqt = imqt.scaled(conf.height(), imqt.height(), Qt::IgnoreAspectRatio);
  }

  QString filename = name + ".png";
  bool ok = imqt.save(filename, "PNG");

  if (nullptr != _notify) {
    _notify->imageNotify(_pohead, "SAT", "png" , filename);
  }
  
  return ok;
}


bool Image::saveGeo(const po::GeomCorrection& geom, const QString& name)
{
  trc;
  if ( name.isEmpty() || _imData.size() == 0) {
    return false;
  }
  GDALAllRegister();
  CPLPushErrorHandler(CPLQuietErrorHandler);
  //CPLSetConfigOption( "GDAL_TIFF_INTERNAL_MASK", "YES" );
  
  GDALDriver *driver = GetGDALDriverManager()->GetDriverByName("GTiff");
  if (nullptr == driver) {
    error_log << CPLGetLastErrorMsg();
    return false;
  }

  char **papszOptions = nullptr;
  papszOptions = CSLSetNameValue( papszOptions, "TILED", "YES" );
  papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "DEFLATE" );
  papszOptions = CSLSetNameValue( papszOptions, "BLOCKXSIZE", "256");
  papszOptions = CSLSetNameValue( papszOptions, "BLOCKYSIZE", "256");
  papszOptions = CSLSetNameValue( papszOptions, "GDAL_TIFF_OVR_BLOCKSIZE", "256");


  GDALDataset* gds = createWGS84(driver, papszOptions, geom, name + "_wgs84.tiff");
  if (nullptr != gds) {
    createEPSG3857(driver, papszOptions, gds, name + "_epsg3857.tiff");
    gds->Release();
  }
  

  GDALDestroyDriverManager();
  
  return true;
  
}


GDALDataset* Image::createWGS84(GDALDriver *driver, char **papszOptions, const po::GeomCorrection& geom, const QString& name)
{
  if (nullptr == driver) {
    error_log << CPLGetLastErrorMsg();
    return nullptr;
  }
  
  GDALDataset* gds = driver->Create(name.toStdString().c_str(), geom.cols(), geom.rows(), 1, GDT_Byte, papszOptions);
  if (nullptr == gds) {
    error_log << "create err" << CPLGetLastErrorMsg();
    return gds;
  }

  QVector<uchar> resImg = geom.transformImage(_imData);

  double transform[6];
  transform[0] = MnMath::rad2deg(geom.leftUp().lon);
  transform[1] = MnMath::rad2deg(geom.step());
  transform[2] = 0.;
  transform[3] = MnMath::rad2deg(geom.leftUp().lat);
  transform[4] = 0.;
  transform[5] = -MnMath::rad2deg(geom.step());
  gds->SetGeoTransform(transform);
  
  OGRSpatialReference oSRS;
  char *pszWKT = NULL;
  oSRS.SetWellKnownGeogCS( "WGS84" ); // = EPSG:4326
  oSRS.exportToWkt( &pszWKT );
  gds->SetProjection(pszWKT);
  CPLFree( pszWKT );
  
  int nBandIn = 1;
  CPLErr er = gds->GetRasterBand(nBandIn)->RasterIO(GF_Write,0, 0, geom.cols(), geom.rows(), (void*)resImg.data(),
                                                    geom.cols(), geom.rows(), GDT_Byte, 0, 0);
  if(0 != er) error_log << "RasterIO" << "return" << er << CPLGetLastErrorMsg();
  
  gds->GetRasterBand(nBandIn)->SetNoDataValue(0);
  
  // er = geotiffDataset->CreateMaskBand(GMF_NODATA |GMF_PER_DATASET);
  // if(0 != er) debug_log << "BuildMask" << "return" << er << CPLGetLastErrorMsg();
  //      geotiffDataset->GetRasterBand(1)->SetColorInterpretation(GCI_AlphaBand);
  
  int ovCnt = resImg.size() / (50*1024*1024);
  int* ovScales = new int(ovCnt);
  for (int ovidx = 0; ovidx < ovCnt; ovidx++) {
    ovScales[ovidx] = (ovidx + 1)*2;
  }
  
  er = gds->BuildOverviews( "NEAREST", ovCnt, ovScales, 0, nullptr,
                            GDALDummyProgress, nullptr );
  
  delete ovScales;
  
  if(0 != er) {
    error_log << "BuildOverviews" << "return" << er << CPLGetLastErrorMsg();
  }

  if (nullptr != _notify) {
    _notify->imageNotify(_pohead, "WGS84", "tiff", name);
  }
  
  return gds;
}

void Image::createEPSG3857(GDALDriver* driver, char **papszOptions, GDALDataset*gds, const QString& name)
{
  GDALWarpOptions *psWO = GDALCreateWarpOptions();
  if (nullptr == psWO) {
    return;
  }
  
  psWO->pfnTransformer = GDALGenImgProjTransform;
  psWO->pTransformerArg = GDALCreateGenImgProjTransformer( gds, nullptr, nullptr, nullptr, FALSE, 0, 0);
  if (psWO->pfnTransformer == nullptr) {
    GDALDestroyWarpOptions(psWO);
    error_log << "transformer null";
    return;
  }

  char *pszDstWKT = nullptr;
  OGRSpatialReference oSRS;
  oSRS.importFromEPSG(3857);
  oSRS.exportToWkt(&pszDstWKT);

  CPLErr er = GDALCreateAndReprojectImage(gds, nullptr,
                                          name.toStdString().c_str(),
                                          pszDstWKT,
                                          driver,
                                          papszOptions,
                                          GRA_NearestNeighbour, 0, 0, nullptr, nullptr, psWO);

  if(0 != er) error_log << "reproj" << "return" << er << CPLGetLastErrorMsg();

  CPLFree(pszDstWKT);
  GDALDestroyGenImgProjTransformer(psWO->pTransformerArg);
  GDALDestroyWarpOptions(psWO);

  //на лету сделать dataset не полчается (см. функцию ниже). Открываем созданный файл, чтобы добавить overview

  GDALDataset *dst = (GDALDataset *) GDALOpen(name.toStdString().c_str(), GA_Update);
  if (nullptr == dst) {
    error_log << "create err" << CPLGetLastErrorMsg();
    return;
  }

  int ovCnt = dst->GetRasterXSize()*dst->GetRasterYSize() / (50*1024*1024); //считает, что 1 байт на цвет

  int* ovScales = new int(ovCnt);
  for (int ovidx = 0; ovidx < ovCnt; ovidx++) {
    ovScales[ovidx] = (ovidx + 1)*2;
  }
  
  er = dst->BuildOverviews( "NEAREST", ovCnt, ovScales, 0, nullptr,
                            GDALDummyProgress, nullptr );
  
  delete ovScales;
  
  if(0 != er) error_log << "BuildOverviews" << "return" << er << CPLGetLastErrorMsg();
  
  GDALClose(dst);

  if (nullptr != _notify) {
    _notify->imageNotify(_pohead, "EPSG:3857" , "tiff", name);
  }
}


// создать на лету с помощью следующего кода не получается, изображение остается в той же проекции
// GDALDataset* Image::createEPSG3857(GDALDriver* driver, char **papszOptions, GDALDataset*gds, const QString& name)
// {
//   trc;
//   char *pszSrcWKT = NULL, *pszDstWKT = NULL;    
//   OGRSpatialReference oSRS;
//   oSRS.SetWellKnownGeogCS("WGS84");
//   oSRS.exportToWkt(&pszSrcWKT);

//   oSRS.importFromEPSG(3857);      
//   oSRS.exportToWkt( &pszDstWKT );

//   GDALDataset* dst = driver->CreateCopy(name.toStdString().c_str(), gds, FALSE, papszOptions, nullptr, nullptr);
//   //    driver->Create(name.toStdString().c_str(), gds->GetRasterXSize(), gds->GetRasterYSize(), 1, GDT_Byte, papszOptions);
//   if (nullptr == dst) {
//     debug_log << "create err" << CPLGetLastErrorMsg();
//     return dst;
//   }

//   dst->SetProjection(pszDstWKT);


//   GDALWarpOptions *psWO = GDALCreateWarpOptions();

//   psWO->pfnTransformer = GDALGenImgProjTransform;
//   psWO->pTransformerArg = GDALCreateGenImgProjTransformer( gds, NULL, NULL, NULL, FALSE, 0, 0);
//   //psWO->pTransformerArg =  GDALCreateReprojectionTransformer(pszSrcWKT, pszDstWKT);



//   //   psWO->eResampleAlg = GRA_NearestNeighbour;

//   if (psWO->pfnTransformer == NULL) {
//     debug_log << "transformer null";
//     //   //todo return, memory clean
//   }

//   debug_log << "reproj";
//   CPLErr er = GDALReprojectImage(gds, pszSrcWKT,
// 				 dst, 
// 				 pszDstWKT,
// 				 GRA_NearestNeighbour, 0, 0, NULL, NULL, psWO);

//   if(0 != er) debug_log << "reproj" << "return" << er << CPLGetLastErrorMsg();


//   debug_log << "destroy";
//   GDALDestroyGenImgProjTransformer(psWO->pTransformerArg); //todo
//   GDALDestroyWarpOptions(psWO);

//   GDALReleaseDataset(dst);

//   return nullptr;
// }

// пока пусть код живет
// тестовая функция с проверкой возможности сохранения в разные проекции, проверка пары методов, сохранение vrt-файла
// для метода GDALGCPTransform плохое качество преобразования, не хватает точек, но быстро
// для метода GDALTPSTransform качество почти как при нашем преобразовании, но появляются волны на узких длинных участках,
//по времени более полутора минут при макс. количестве точек равном 10922
// template<class T> bool Channel<T>::saveGeoTifGdal(const QVector<Coords::GeoCoord>& points, float step,
// 						      QVector<uchar>& imData, const QString& baseName) const
// {
//   trc;
//   CPLErr er;

//   QString chanName = _prop.name;
//   if (chanName.isEmpty()) {
// 	chanName = QString::number(_prop.num).rightJustified(2, '0');
//   }

//   QString tifName =  baseName + chanName + ".tiff";
//   var(imData.size());


//   GDALAllRegister();
//   CPLPushErrorHandler(CPLQuietErrorHandler);
//   GDALDataset *gds = nullptr;
//   GDALDriver *driver = nullptr;

//   //CPLSetConfigOption( "GDAL_TIFF_INTERNAL_MASK", "YES" );

//   driver = GetGDALDriverManager()->GetDriverByName("GTiff");
//   if (nullptr == driver) {
// 	debug_log << CPLGetLastErrorMsg();
// 	return false;
//   }


//   char **papszOptions = nullptr;
//   papszOptions = CSLSetNameValue( papszOptions, "TILED", "YES" );
//   papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "DEFLATE" );
//   papszOptions = CSLSetNameValue( papszOptions, "BLOCKXSIZE", "256");
//   papszOptions = CSLSetNameValue( papszOptions, "BLOCKYSIZE", "256");
//   papszOptions = CSLSetNameValue( papszOptions, "GDAL_TIFF_OVR_BLOCKSIZE", "256");


//   gds = driver->Create(tifName.toStdString().c_str(), _prop.cols, _prop.rows, 1, GDT_Byte, papszOptions);
//   if (nullptr == gds) {
// 	debug_log << "create err" << CPLGetLastErrorMsg();
// 	return false;
//   }


//   Q_UNUSED(step);
//   // double transform[6];
//   // transform[0] = MnMath::rad2deg(points[0].lon);
//   // transform[1] = MnMath::rad2deg(step);
//   // transform[2] = 0.;
//   // transform[3] = MnMath::rad2deg(points[0].lat);
//   // transform[4] = 0.;
//   // transform[5] = -MnMath::rad2deg(step);
//   // CPLErr er = gds->SetGeoTransform(transform);
//   // if(0 != er) debug_log << "transf" << er << CPLGetLastErrorMsg();



//   // Set up the warping.
//   char *pszSrcWKT = NULL, *pszDstWKT = NULL;
//   OGRSpatialReference oSRS;
//   oSRS.SetWellKnownGeogCS("WGS84");
//   oSRS.exportToWkt(&pszSrcWKT);
//   // oSRS.SetWellKnownGeogCS("WGS84");
//   oSRS.importFromEPSG(3857);
//   oSRS.exportToWkt( &pszDstWKT );


//   int nBandIn = 1;
//   er = gds->GetRasterBand(nBandIn)->RasterIO(GF_Write,0, 0, _prop.cols, _prop.rows, (void*)imData.data(),
// 							_prop.cols, _prop.rows, GDT_Byte, 0, 0);
//   if(0 != er) debug_log << "RasterIO" << er << CPLGetLastErrorMsg();

//   gds->GetRasterBand(nBandIn)->SetNoDataValue(0);


//   GDALWarpOptions *psWO = GDALCreateWarpOptions();
//   psWO->eResampleAlg = GRA_NearestNeighbour;


//   int gcpCnt = 10922; //максимально возможное для хранения GDAL 3.2.2
//   GDAL_GCP *gcp = (GDAL_GCP*)CPLCalloc(gcpCnt, sizeof(GDAL_GCP));
//   int gidx = 0;


//   for (uint row = 0; row < _prop.rows; row++) {
// 	for (uint col = 0; col < _prop.cols; col++) {
// 	  if ( ( row%100 == 0 || row == _prop.rows - 1) &&
// 	       (col % 20 == 0 || col == _prop.cols - 1) )
// 	    {
// 	      std::string snum = std::to_string(gidx);
// 	    gcp[gidx].pszId = new char[snum.size() + 1];
// 	    snum.copy(gcp[gidx].pszId, snum.size());
// 	    gcp[gidx].pszId[snum.size()] = '\0';
// 	    gcp[gidx].pszInfo = nullptr;
// 	    gcp[gidx].dfGCPPixel = col;
// 	    gcp[gidx].dfGCPLine = row;
// 	    gcp[gidx].dfGCPX = MnMath::rad2deg(points.at(row*_prop.cols + col).lon);
// 	    gcp[gidx].dfGCPY = MnMath::rad2deg(points.at(row*_prop.cols + col).lat);
// 	    gcp[gidx].dfGCPZ = 0;
// 	    gidx++;
// 	    if (gidx == gcpCnt) break;
// 	  }
// 	}
// 	if (gidx == gcpCnt) break;
//   }

//   var(gcpCnt);
//   var(gidx);
//   var(_prop.rows * _prop.cols);
//   gcpCnt = gidx;

//   gds->SetProjection(pszSrcWKT);
//   er = gds->SetGCPs(gcpCnt, gcp, pszSrcWKT);
//   if(0 != er) debug_log << "setgcp" << er << CPLGetLastErrorMsg();


//   // psWO->pfnTransformer = GDALGCPTransform;
//   // psWO->pTransformerArg = GDALCreateGCPTransformer(gcpCnt, gcp, 3, 0);
//    psWO->pfnTransformer = GDALTPSTransform;
//    psWO->pTransformerArg = GDALCreateTPSTransformer(gcpCnt, gcp, 0);


//   if (psWO->pfnTransformer == NULL) {
// 	debug_log << "transformer null";
// 	//todo return, memory clean
//   }

//   er = GDALCreateAndReprojectImage(gds, pszSrcWKT, "/home/maria/test_3857.tiff", pszDstWKT, driver, papszOptions, GRA_NearestNeighbour,
// 				       0, 0, nullptr, nullptr, psWO);
//   if(0 != er) debug_log << "reproj" << er << CPLGetLastErrorMsg();



//   // GDALDatasetH hDstDS = driver->CreateCopy("/home/maria/test.tiff",
//   // 					       gds, false, papszOptions, NULL, NULL);
//   // er = GDALReprojectImage(gds, pszSrcWKT, hDstDS, pszDstWKT, GRA_NearestNeighbour, 0, 0, NULL, NULL, psWO);


//   //----
//   // psWO->nBandCount = nBandIn;
//   // psWO->panSrcBands = (int *) CPLMalloc(sizeof(int) * psWO->nBandCount);
//   // psWO->panDstBands = (int *) CPLMalloc(sizeof(int) * psWO->nBandCount);
//   // for(int i = 0; i < psWO->nBandCount; i++) {
//   // 	psWO->panSrcBands[i] = i+1;
//   // 	psWO->panDstBands[i] = i+1;
//   // }
//   // psWO->hSrcDS = gds; //так надо проверять, чтоб всех полей хватило в psWO
//   // psWO->hDstDS = NULL;
//   // GDALDatasetH hDstDS = GDALCreateWarpedVRT(gds, _prop.cols, _prop.rows, transform, psWO);
//   //----
//   // так работает
//   // GDALDatasetH hDstDS = GDALAutoCreateWarpedVRT(gds, pszSrcWKT, pszDstWKT, GRA_NearestNeighbour, 0, psWO);

//   // if (hDstDS == nullptr) {
//   // 	debug_log << "null hDstDS" << CPLGetLastErrorMsg();
//   // }

//   // GDALSetDescription(hDstDS, "/home/maria/test.vrt");

//   //----


//   // if(0 != er) debug_log << "reproj" << er << CPLGetLastErrorMsg();


//   CPLFree(gcp);
//   GDALClose(gds);
//   //GDALClose(hDstDS);
//   //GDALReleaseDataset(gds);
//   //GDALReleaseDataset(hDstDS);
//   //GDALDestroyGenImgProjTransformer(psWO->pTransformerArg); //todo
//   GDALDestroyWarpOptions(psWO);
//   GDALDestroyDriverManager();

//   return true;

// }


//-------------------

//сохранение tif и geotiff с помощью самих либ

//    /*!
//   * \brief Установка tiff-тегов
//   * \param tifFile TIFF-файл для установки тегов
//   */
//  template<class T> void Channel<T>::setTiffTags(TIFF* tifFile, int cols, int rows) const
//  {
//    trc;
//    //uint spp = 1;
//    uint spp = 2;
//    uint bps = 8;
//    //uint bps = 16;

//    TIFFSetField(tifFile, TIFFTAG_IMAGEWIDTH,   cols);
//    TIFFSetField(tifFile, TIFFTAG_IMAGELENGTH,  rows);
//    TIFFSetField(tifFile, TIFFTAG_BITSPERSAMPLE, bps);
//    TIFFSetField(tifFile, TIFFTAG_SAMPLESPERPIXEL, spp);
//    TIFFSetField(tifFile, TIFFTAG_ROWSPERSTRIP, rows);

//    TIFFSetField(tifFile, TIFFTAG_XRESOLUTION, 72.0);
//    TIFFSetField(tifFile, TIFFTAG_YRESOLUTION, 72.0);
//    TIFFSetField(tifFile, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

//    TIFFSetField(tifFile, TIFFTAG_COMPRESSION,  1);
//    TIFFSetField(tifFile, TIFFTAG_PHOTOMETRIC,  PHOTOMETRIC_MINISBLACK);
//    TIFFSetField(tifFile, TIFFTAG_FILLORDER,    FILLORDER_MSB2LSB);
//    TIFFSetField(tifFile, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

//    int16 out[] = { EXTRASAMPLE_UNASSALPHA };

//    TIFFSetField(tifFile, TIFFTAG_EXTRASAMPLES, 1, out);
//  }



// // ------------------- вариант с записью TLE, может что-то не сработать из-за изменений в тегах tiff
//  template<class T> bool Channel<T>::saveGeoTif(const QString& baseName, const meteo::global::PoHeader& header, bool isCalibrate)
//  {
//    if (_data.size() == 0 || _isEmpty) return false;

//    QByteArray ba;

//    if (isCalibrate) {
// 	for(int idx = 0; idx < _data.size(); idx++) {
// 	  short val = 0x3ff;
// 	  if (_data.at(idx) != -9999) {
// 	    val = (short)round(100.0 * (_data.at(idx) + _min)); //+2 знака после запятой
// 	  }
// 	  ba.append((uint8_t)(val >> 8));
// 	  ba.append((uint8_t)(val & 0xff));
// 	}
//    } else {
// 	for(int idx = 0; idx < _data.size(); idx++) {
// 	  T val = _data.at(idx);
// 	  ba.append((uint8_t)(val >> 8));
// 	  ba.append((uint8_t)(val & 0xff));
// 	}
//    }

//    return saveGeoTif(baseName, header, &ba);
//  }


//  template<class T> bool Channel<T>::saveGeoTif(const QString& baseName, const meteo::global::PoHeader& header, QByteArray* data)
//  {
//    if (_data.size() == 0 || _isEmpty) return false;

//    if (baseName.isNull() || data->isEmpty()) {
// 	return false;
//    }

//    QString tifName = baseName + QString::number(_prop.num).rightJustified(2, '0') + ".tif";

//    TIFF* tifFile = XTIFFOpen(tifName.toStdString().data(), "w");
//    if (!tifFile ) {
// 	return false;
//    }

//    GTIF* gtif = GTIFNew(tifFile);
//    if (gtif) {
// 	setGeoTiffTags(tifFile, gtif, header);
// 	GTIFWriteKeys(gtif);
// 	GTIFFree(gtif);
//    }

//    setTiffTags(tifFile);
//    TIFFWriteEncodedStrip(tifFile, 0, (void*)data->data(), data->size());

//    XTIFFClose(tifFile);
//    return true;
//  }

//  //! установка geotiff-тегов
//  template<class T> void Channel<T>::setGeoTiffTags(TIFF* tifFile, GTIF* gtif, const meteo::global::PoHeader& header)
//  {
//    Q_UNUSED(tifFile);

//    GTIFKeySet(gtif, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelTypeGeographic);

//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 1), TYPE_SHORT, 1, 1); //версия
//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 2), TYPE_DOUBLE, 1, header.siteCoord.lat());
//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 3), TYPE_DOUBLE, 1, header.siteCoord.lon());
//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 4), TYPE_DOUBLE, 1, header.siteCoord.alt());

//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 5), TYPE_ASCII, 0, header.start.toString(Qt::ISODate).toStdString().data());
//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 6), TYPE_ASCII, 0, header.stop.toString(Qt::ISODate).toStdString().data());
//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 7), TYPE_ASCII, 0, header.satellite.toStdString().data());

//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 8), TYPE_SHORT, 1, header.channel);
//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 9), TYPE_DOUBLE, 1, (double)header.tle.epochYear);
//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 10), TYPE_DOUBLE, 1, header.tle.epochDay);

//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 11), TYPE_DOUBLE, 1, header.tle.bstar);
//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 12), TYPE_DOUBLE, 1, header.tle.inclination);
//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 13), TYPE_DOUBLE, 1, header.tle.raan);
//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 14), TYPE_DOUBLE, 1, header.tle.eccentr);
//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 15), TYPE_DOUBLE, 1, header.tle.perigee);
//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 16), TYPE_DOUBLE, 1, header.tle.meanAnomaly);
//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 17), TYPE_DOUBLE, 1, header.tle.meanMotion);
//    GTIFKeySet(gtif, geokey_t(KvUserDefined + 18), TYPE_DOUBLE, 1, (double)header.tle.revolNumber);

//    GTIFWriteKeys(gtif);
//  }



//  // ------------------- преобразованное изображение


//  /*! установка geotiff-тегов
//   * \param corners Массив из 4 точек ( начальный левый, начальный правый, конечный левый, конечный правый )
//   */
//  template<class T> void Channel<T>::setGeoTiffTags(TIFF* tifFile, GTIF* gtif, const Coords::GeoCoord& leftUp, float step) const
//  {
//    trc;

//    GTIFKeySet(gtif, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelTypeGeographic);
//    GTIFKeySet(gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1, 1); //RasterPixelIsArea
//    GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, 4326); //GCS_WGS_84

//    double pixsize[3] = {MnMath::rad2deg(step), MnMath::rad2deg(step), 0.0};
//    double tiepoint[6] = {0.0, 0.0, 0.0, MnMath::rad2deg(leftUp.lon),
// 			    MnMath::rad2deg(leftUp.lat),
// 			    0.0};

//    TIFFSetField(tifFile, 33922, /* ModelTiepointTag*//*, TYPE_DOUBLE,*/ 6, tiepoint);
//    TIFFSetField(tifFile, 33550, /*ModelPixelScale*//*, TYPE_DOUBLE,*/ 3, pixsize);


//    TIFFMergeFieldInfo(tifFile, xtiffFieldInfo, N(xtiffFieldInfo));
//    TIFFSetField(tifFile, NODATA, "0");

//    GTIFWriteKeys(gtif);
//  }



//  /*!
//   * \param corners Массив из 4 точек ( начальный левый, начальный правый, конечный левый, конечный правый )
//   */
//  template<class T> bool Channel<T>::saveGeoTif(const QVector<QMap<float, QPair<float, int64_t>>>& sortedCoords, int geoCols, int geoRows, float step,
// 						  const Coords::GeoCoord& leftUp, const QVector<uchar>& imData, const QString& baseName) const
//  {
//    trc;
//    QString chanName = _prop.name;
//    if (chanName.isEmpty()) {
// 	chanName = QString::number(_prop.num).rightJustified(2, '0');
//    }

//    QVector<uchar> resImg;
//    GeomCorrection::transformImage(sortedCoords, geoCols, geoRows, imData, &resImg);

//    QString tifName =  baseName + chanName + ".tiff";


//    var(imData.size());
//    var(resImg.size());

//    TIFF* tifFile = XTIFFOpen(tifName.toStdString().data(), "w");
//    if (!tifFile ) {
// 	return false;
//    }

//    GTIF* gtif = GTIFNew(tifFile);
//    if (nullptr != gtif) {
// 	setGeoTiffTags(tifFile, gtif, leftUp, step);
// 	GTIFWriteKeys(gtif);
// 	GTIFFree(gtif);
//    }


//    setTiffTags(tifFile, geoCols, geoRows);
//    TIFFWriteEncodedStrip(tifFile, 0, (void*)resImg.data(), resImg.size());

//    XTIFFClose(tifFile);
//    return true;
//    //      debug_log << "write" << name;
//  }
