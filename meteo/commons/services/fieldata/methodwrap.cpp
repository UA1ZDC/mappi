#include "methodwrap.h"
#include "fieldservice.h"

#include <qmath.h>
#include <qelapsedtimer.h>
#include <qtemporarydir.h>
#include <quuid.h>


#include <commons/meteo_data/meteo_data.h>
#include <commons/obanal/tisolinedata.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/global/gradientparams.h>
#include <meteo/commons/zond/zond.h>
#include <meteo/commons/zond/zondfunc.h>
#include <commons/funcs/tcolorgrad.h>
#include <meteo/commons/global/gradientparams.h>

#include <gdal/gdal_priv.h>
#include <gdal/gdal_alg.h>
#include <gdal/cpl_vsi.h>
#include <gdal/ogr_spatialref.h>
#include <gdal/ogr_geometry.h>
#include <gdal/ogr_featurestyle.h>
#include <gdal/ogrsf_frmts.h>
#include <gdal/gdalwarper.h>

namespace meteo {
namespace field {

//static const QString kFieldPrefix      = "obanal";

MethodWrap::MethodWrap(TFieldDataService* service) : QObject()
{
  service_ = service;
  auto opt = Global::instance()->mongodbConfObanal();
  const_cast<QString&>(kObanaldb) = opt.name();
  auto conf = meteo::Global::instance()->mongodbConfMeteo();
  dbname_ = conf.name();
}

MethodWrap::~MethodWrap()
{

}

void MethodWrap::GetFieldData(const DataRequest *req, DataReply *resp)
{
  QString error;
  QElapsedTimer ttt; ttt.start();
  if ( ( nullptr == req ) || ( nullptr == resp ) ) {
    error = QObject::tr("Неверные входные данные");
    debug_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    return;
  }

  DataDesc* dataDesc = resp->mutable_descr();
  QString file_id;
  if ( false == loadFieldInfo( req, dataDesc, &file_id, &error ) ) {
    resp->clear_descr();
    //NOTE: при вызове в TFieldCalc при ОА может быть штатной ситуацией (ещё не все данные доступны)
    //
    debug_log << QObject::tr("Не удалось получить описание поля. Ошибка = %1").arg(error);
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    return;
  }

  QByteArray barr;
  if( false == getFieldFromFile( file_id, &barr ) ) {
    debug_log << msglog::kFileReadFailed.arg(file_id).arg(-1);
    resp->set_comment(msglog::kFileReadFailed.arg(file_id).arg(-1).toStdString());
    resp->set_result(false);
    return;
  }
  resp->set_fielddata( barr.data(), barr.size() );
  resp->set_comment(msglog::kServerAnswerOK.toStdString());
  resp->set_result(true);
  int cur = ttt.elapsed();
  if( 1000 < cur ) {
    debug_log << msglog::kServiceRequestTime.arg("GetFieldData").arg(cur);
  }
}

void MethodWrap::GetGeoTiff(const DataRequest *req, DataReply *resp)
{
  QString error;
  QElapsedTimer ttt; ttt.start();
  if ( ( nullptr == req ) || ( nullptr == resp ) ) {
    error = QObject::tr("Неверные входные данные");
    debug_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    return;
  }

  DataDesc* dataDesc = resp->mutable_descr();
  QString file_id;
  if ( false == loadFieldInfo( req, dataDesc, &file_id, &error ) ) {
    resp->clear_descr();
    //NOTE: при вызове в TFieldCalc при ОА может быть штатной ситуацией (ещё не все данные доступны)
    //
    debug_log << QObject::tr("Не удалось получить описание поля. Ошибка = %1").arg(error);
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    return;
  }

  QByteArray barr;
  if( false == getFieldFromFile( file_id, &barr ) ) {
    debug_log << msglog::kFileReadFailed.arg(file_id).arg(-1);
    resp->set_comment(msglog::kFileReadFailed.arg(file_id).arg(-1).toStdString());
    resp->set_result(false);
    return;
  }
  ::obanal::TField* field = new ::obanal::TField();
  if(false == field->fromBuffer(&barr)) {
    debug_log << msglog::kFileReadFailed.arg(file_id).arg(-1);
    resp->set_comment(msglog::kFileReadFailed.arg(file_id).arg(-1).toStdString());
    resp->set_result(false);
    delete field;
    return;
  }
  field->smootchField(10.0);
  QString uniqueName = QUuid::createUuid().toString().remove("{").remove("}");
  QString tiffname(QDir::tempPath()+"/"+uniqueName+".tif");

  bool rettiff = createGeotiff(field, tiffname, layernameFromInfo(*dataDesc));
  delete field; field = nullptr;
  if(false == rettiff ){
    resp->set_result(false);
    resp->set_comment(msglog::kServiceAnsverFailed.toStdString());
  } else {
    if( true == fromFileToByteAndRm(tiffname, &barr)){
      resp->set_fielddata( barr.data(), barr.size() );
    }
    resp->set_comment(msglog::kServerAnswerOK.toStdString());
    resp->set_result(true);
  }

  int cur = ttt.elapsed();
  if( 1000 < cur ) {
    debug_log << msglog::kServiceRequestTime.arg("GetFieldData").arg(cur);
  }
}


void MethodWrap::GetIsoLinesShp( const DataRequest* req, meteo::IsoLinesShp* resp )
{
  DataDesc desc;
  QString filename;
  QString error;
  if ( false == loadFieldInfo( req, &desc, &filename, &error ) ) {
    error_log << QObject::tr("Не удалось получить описание поля. Ошибка = %1").arg(error);
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    return;
  }
  //  debug_log << "FIELD DESCR =" << desc.Utf8DebugString();
  QByteArray arr;
  obanal::TField* field = new obanal::TField;

  if ( true == getFieldFromFile( filename, &arr ) ) {
    if ( false == field->fromBuffer(&arr) ) {
      debug_log << QObject::tr("Ошибка извлечения поля из массива байтов");
      delete field;
      resp->set_comment( QObject::tr("Ошибка извлечения поля из массива байтов").toStdString() );
      resp->set_result(false);
      return;
    }
  }
  //field->smootchField(10.0);

  auto kcenters = global::kMeteoCenters();
  QString layername = layernameFromInfo(desc);

  auto header = resp->mutable_header();
  header->set_dt( field->getDate().toString(Qt::ISODate).toStdString() );
  header->set_unit( field->unit().toStdString() );
  header->set_name( field->name().toStdString() );
  header->set_level( field->getLevel() );
  header->set_level_type( field->getLevelType() );
  header->set_meteodescr( field->getDescr() );
  header->set_hour( field->getHour() );
  header->set_model( field->getModel() );
  header->set_center( field->getCenter() );
  header->set_center_name( kcenters[field->getCenter()].first.toStdString() );
  header->set_layer_name( layername.toStdString() );
  header->set_field_id( desc.id() );

  float step_lat;
  float step_lon;
  if ( false == field->stepFiDeg( &step_lat ) || false == field->stepLaDeg( &step_lon )  ) {
    error = QString::fromUtf8("Шаг сетки неизвестен для поля = %1")
            .arg( QString::fromStdString( header->Utf8DebugString() ) );
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    return;
  }
  header->set_step_lat( step_lat );
  header->set_step_lon( step_lon );
  header->mutable_begin()->CopyFrom( geopoint2pbgeopoint( field->begin() ) );
  header->mutable_end()->CopyFrom( geopoint2pbgeopoint( field->end() ) );

  map::GradientParams params( meteo::global::kIsoParamPath() );
  const map::proto::FieldColor& clr = params.protoParams( field->getDescr() );
  map::proto::LevelColor lvlclr = map::GradientParams::levelProto( field->getLevel(), field->getLevelType(), clr );
  auto protoparams = params.protoParams( field->getDescr() );
  TColorGrad colorgrad = map::GradientParams::isoParams( field->getLevel(), field->getLevelType(), protoparams );
  //float min = int(field->min_field_value()/step)*step;
  //float max = field->max_field_value();
  QString uniqueName = QUuid::createUuid().toString().remove("{").remove("}");
  QString tiffname(QDir::tempPath()+"/"+uniqueName+".tif");
  QString shp_dir_name(QDir::tempPath()+"/"+uniqueName+"/");
  QString shp_file_name(uniqueName);

  bool rettiff = createGeotiff(field, tiffname, layernameFromInfo(desc));
  delete field; field = nullptr;
  if(false == rettiff ){
    resp->set_result(false);
    resp->set_comment(msglog::kServiceAnsverFailed.toStdString());
    return;
  }

  GDALAllRegister();
  CPLPushErrorHandler(CPLQuietErrorHandler);

  GDALDataset* tdataset = (GDALDataset*)GDALOpen(tiffname.toStdString().data(), GA_ReadOnly);

  //*****************************
  GDALDriver * shpDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
  if (nullptr == shpDriver )
  {
    debug_log << "shpDriver is null";
    resp->set_result(false);
    resp->set_comment(msglog::kServiceAnsverFailed.toStdString());
    GDALDestroyDriverManager();
    OGRCleanupAll();
    return ;
  }


  GDALDataset* contourDataset = shpDriver->Create(shp_dir_name.toStdString().c_str(), 0, 0, 0, GDT_Unknown, nullptr);
  if (contourDataset == nullptr){
    debug_log << "contourDataset is null";
    resp->set_result(false);
    resp->set_comment(msglog::kServiceAnsverFailed.toStdString());
    GDALDestroyDriverManager();
    OGRCleanupAll();
    return ;
  }


  OGRSpatialReference* spatialReference = new OGRSpatialReference(tdataset->GetProjectionRef());

  if (spatialReference == nullptr){
    debug_log << "spatialReference is null";
    resp->set_result(false);
    resp->set_comment(msglog::kServiceAnsverFailed.toStdString());
    GDALClose(contourDataset);
    GDALDestroyDriverManager();
    OGRCleanupAll();
    return ;
  }
  OGRLayer* layer =  contourDataset->CreateLayer(shp_file_name.toStdString().c_str(),
                                                 spatialReference,
                                                 wkbLineString,
                                                 nullptr);
  delete spatialReference;
  if (layer == nullptr){
    debug_log << "layer is null";
    resp->set_result(false);
    resp->set_comment(msglog::kServiceAnsverFailed.toStdString());
    GDALClose(contourDataset);
    GDALDestroyDriverManager();
    OGRCleanupAll();
    return ;
  }
  OGRFieldDefn* idDefn = new OGRFieldDefn( "ID", OFTInteger );
  idDefn->SetWidth(8);
  layer->CreateField(idDefn, false);
  idDefn->SetName("value");
  layer->CreateField(idDefn, false);
  delete idDefn;
  // generate contours

  double _baseLevel = 0;
  double _regularInterval = lvlclr.step_iso();
  CPLErr err = GDALContourGenerate(tdataset->GetRasterBand(1),
                                   _regularInterval,
                                   _baseLevel,
                                   0,//_customIntervals.length(),
                                   nullptr,//_customIntervals.toStdVector().data(),
                                   true, /* ignore nodata in dataset */
                                   -9999.,    /* nodata value */
                                   layer,
                                   layer->FindFieldIndex("ID", true), /* make unique Feature IDs */
                                   layer->FindFieldIndex("value", true), /* elevation index of the dataset, or -1 if not given */
                                   nullptr,
                                   nullptr);
  GDALClose(contourDataset);
  GDALDestroyDriverManager();
  OGRCleanupAll();

  if (err != CE_None)
  {
    debug_log << "err != CE_None";
    resp->set_result(false);
    resp->set_comment(msglog::kServiceAnsverFailed.toStdString());
    return;
  }
  QByteArray barr;
  if( true == fromFileToByteAndRm(shp_dir_name + shp_file_name+".dbf", &barr)){
    resp->set_dbf_data(barr.data(), barr.size() );
  }
  if( true == fromFileToByteAndRm(shp_dir_name + shp_file_name+".prj", &barr)){
    resp->set_prj_data(barr.data(), barr.size() );
  }
  if( true == fromFileToByteAndRm(shp_dir_name + shp_file_name+".shp", &barr)){
    resp->set_shp_data(barr.data(), barr.size() );
  }
  if( true == fromFileToByteAndRm(shp_dir_name + shp_file_name+".shx", &barr)){
    resp->set_shx_data(barr.data(), barr.size() );
  }


  resp->set_comment(msglog::kServerAnswerOK.toStdString());
  resp->set_result(true);
}

bool MethodWrap::fromFileToByteAndRm(const QString & file_name, QByteArray * barr)
{
  QFile fl(file_name);
  if( true == fl.open(QIODevice::ReadOnly)){
    *barr = fl.readAll();
    fl.remove();
    return true;
  }
return false;
}

bool MethodWrap::createGeotiff(::obanal::TField* field, const QString& tiffname, const QString& layername)
{
  int nrows = field->kolFi()-1;
  int ncols = field->kolLa()-1;
  if(nrows <1 || ncols <1 ){
    debug_log <<"nrows <1 ";
    return false;
  }

  GDALAllRegister();
  CPLPushErrorHandler(CPLQuietErrorHandler);
  GDALDataset *geotiffDataset = nullptr;
  GDALDriver *driverGeotiff = nullptr;

  driverGeotiff = GetGDALDriverManager()->GetDriverByName("GTiff");
  if(nullptr == driverGeotiff){
    return false;
  }

  char **papszOptions = nullptr;
  papszOptions = CSLSetNameValue( papszOptions, "TILED", "YES" );
  papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "DEFLATE" );
//  papszOptions = CSLSetNameValue( papszOptions, "BLOCKXSIZE", "256");
//  papszOptions = CSLSetNameValue( papszOptions, "BLOCKYSIZE", "256");
//  papszOptions = CSLSetNameValue( papszOptions, "GDAL_TIFF_OVR_BLOCKSIZE", "256");   
  geotiffDataset = driverGeotiff->Create((tiffname + ".WGS84").toStdString().c_str(),ncols,nrows,1,GDT_Float32, papszOptions );

  if(nullptr == geotiffDataset){
    VSIUnlink( (tiffname + ".WGS84").toStdString().c_str() );
    GDALDestroyDriverManager();
    return false;
  }

  double transform[6];
  //transform[0] = -180.;
  transform[0] = field->getLa(0)*RAD2DEG;
  float step;
  field->stepLaDeg(&step);
  transform[1] =  step; //1
  transform[2] = 0.;
  //transform[3] = 90.;
  transform[3] = field->getFi(field->kolFi()-1)*RAD2DEG;
  transform[4] = 0.;
  field->stepFiDeg(&step);
  transform[5] = -1.*step;//-1;
  geotiffDataset->SetGeoTransform(transform);

  OGRSpatialReference oSRS;
  char *pszWKT = NULL;
  oSRS.SetWellKnownGeogCS( "WGS84" );
  oSRS.exportToWkt( &pszWKT );

  geotiffDataset->SetProjection(pszWKT);
  CPLFree( pszWKT );
  float *rowBuff = (float*) CPLMalloc(sizeof(float)*ncols);

  int nBandIn = 1;

  for(int row=0; row<nrows; row++) {
    for(int col=0; col<ncols; col++) {
      //int data = (int)255* (field->getData(row,col)-minval) / (maxval -minval);
      float data = field->getData(row,col);
      if(field->getMask(row,col)){
        rowBuff[col] =  data;
      }else {
        rowBuff[col] =  -9999.;
        debug_log << "found -9999 value at [" << row << "," << col << "]";
      }
    }
    CPLErr er = geotiffDataset->GetRasterBand(nBandIn)->RasterIO(
        GF_Write, //eRWFlag – Either GF_Read to read a region of data, or GF_Write to write a region of data.
        0, //nXOff – The pixel offset to the top left corner of the region of the band to be accessed. This would be zero to start from the left side.
        nrows-row-1, //nYOff – The line offset to the top left corner of the region of the band to be accessed. This would be zero to start from the top.
        ncols, //nXSize – The width of the region of the band to be accessed in pixels.
        1, //nYSize – The height of the region of the band to be accessed in lines.
        rowBuff, //pData – [inout] The buffer into which the data should be read, or from which it should be written. This buffer must contain at least nBufXSize * nBufYSize words of type eBufType. It is organized in left to right, top to bottom pixel order. Spacing is controlled by the nPixelSpace, and nLineSpace parameters.
        ncols, //nBufXSize – the width of the buffer image into which the desired region is to be read, or from which it is to be written.
        1, //nBufYSize – the height of the buffer image into which the desired region is to be read, or from which it is to be written.
        GDT_Float32, //eBufType – the type of the pixel values in the pData data buffer. The pixel values will automatically be translated to/from the GDALRasterBand data type as needed.
        0, //nPixelSpace – The byte offset from the start of one pixel value in pData to the start of the next pixel value within a scanline. If defaulted (0) the size of the datatype eBufType is used.
        0 //nLineSpace – The byte offset from the start of one scanline in pData to the start of the next. If defaulted (0) the size of the datatype eBufType * nBufXSize is used.
    );

    if(0 != er) {
      debug_log << "geotiffDataset->GetRasterBand(1)->RasterIO" << "return" << er;
      VSIUnlink( (tiffname + ".WGS84").toStdString().c_str() );
      GDALClose(geotiffDataset) ;
      GDALDestroyDriverManager();
      return false;
    }
  }

  if ( false == createEPSG3857( driverGeotiff, papszOptions, geotiffDataset, tiffname, layername ) ) {
    debug_log << QObject::tr("Ошибка преобразования в epsg 3857");
  }

  geotiffDataset->GetRasterBand(nBandIn)->SetNoDataValue(-9999.);
  geotiffDataset->GetRasterBand(nBandIn)->SetDescription(layername.toStdString().c_str()); // This sets the band name!
  geotiffDataset->GetRasterBand(nBandIn)->ComputeStatistics(
    false, //bApproxOK – If TRUE statistics may be computed based on overviews or a subset of all tiles.
    nullptr, //pdfMin – Location into which to load image minimum (may be NULL).
    nullptr, //pdfMax – Location into which to load image maximum (may be NULL).-
    nullptr, //pdfMean – Location into which to load image mean (may be NULL).
    nullptr, //pdfStdDev – Location into which to load image standard deviation (may be NULL).
    nullptr, //pfnProgress – a function to call to report progress, or NULL. 
    nullptr  //pProgressData – application data to pass to the progress function.
  );
//  CPLSetConfigOption("COMPRESS_OVERVIEW", "DEFLATE");
  int   anOverviewList[3] = { 2, 4, 8 };
  geotiffDataset->BuildOverviews( "NEAREST", 3, anOverviewList, 0, nullptr, nullptr, nullptr );

  GDALClose(geotiffDataset) ;
  CPLFree( rowBuff );
  GDALDestroyDriverManager();
  return true;
}

bool MethodWrap::createEPSG3857(GDALDriver* driver, char **papszOptions, GDALDataset*gds, const QString& tiffname, const QString& layername)
{
  char *pszDstWKT = nullptr;    
  OGRSpatialReference oSRS;
  oSRS.importFromEPSG(3857);      
  oSRS.exportToWkt(&pszDstWKT);
    
  CPLErr er = GDALCreateAndReprojectImage(
    gds, //hSrcDS – the source image file.
    nullptr, //pszSrcWKT – the source projection. If NULL the source projection is read from from hSrcDS.
	tiffname.toStdString().c_str(), //hDstDS – the destination image file.
	pszDstWKT, //pszDstWKT – the destination projection. If NULL the destination projection will be read from hDstDS.
	driver, //Format specific driver.
	papszOptions, //list of driver specific control parameters.
	GRA_NearestNeighbour, //eResampleAlg – the type of resampling to use.
	0, //dfWarpMemoryLimit – the amount of memory (in bytes) that the warp API is allowed to use for caching.
	0, //dfMaxError – maximum error measured in input pixels that is allowed in approximating the transformation (0.0 for exact calculations).
	nullptr, //pfnProgress – a GDALProgressFunc() compatible callback function for reporting progress or NULL.
	nullptr, //pProgressArg – argument to be passed to pfnProgress. May be NULL.
	nullptr //psOptions – warp options, normally NULL.
  );
   
  if(0 != er) debug_log << "reproj" << "return" << er << CPLGetLastErrorMsg();
  CPLFree(pszDstWKT);

  //на лету сделать dataset не полчается (см. функцию ниже). Открываем созданный файл, чтобы добавить overview

  int nBandIn = 1;
  GDALDataset *dst = (GDALDataset *) GDALOpen(tiffname.toStdString().c_str(), GA_Update); 
  if (nullptr == dst) {
    debug_log << "create err" << CPLGetLastErrorMsg();
    return false;
  }

  dst->GetRasterBand(nBandIn)->SetNoDataValue(-9999.);
  dst->GetRasterBand(nBandIn)->SetDescription(layername.toStdString().c_str()); // This sets the band name!
  dst->GetRasterBand(nBandIn)->ComputeStatistics(
    false, //bApproxOK – If TRUE statistics may be computed based on overviews or a subset of all tiles.
    nullptr, //pdfMin – Location into which to load image minimum (may be NULL).
    nullptr, //pdfMax – Location into which to load image maximum (may be NULL).-
    nullptr, //pdfMean – Location into which to load image mean (may be NULL).
    nullptr, //pdfStdDev – Location into which to load image standard deviation (may be NULL).
    nullptr, //pfnProgress – a function to call to report progress, or NULL. 
    nullptr  //pProgressData – application data to pass to the progress function.
  );
  int   anOverviewList[3] = { 2, 4, 8 };
  dst->BuildOverviews( "NEAREST", 3, anOverviewList, 0, nullptr, nullptr, nullptr );
  
  GDALClose(dst);
  return true;
}

bool MethodWrap::loadFieldInfo( const DataRequest* req, DataDesc* desc, QString* file_id, QString* errorstr )
{
  QString& error = *errorstr;
  QElapsedTimer ttt; ttt.start();
  if ( nullptr == req ) {
    error = msglog::kServiceRequestFailed;
    debug_log << error;
    return false;
  }

  QList<int> descrs;
  if ( 0 != req->meteo_descr_size() ) {
    for ( auto d : req->meteo_descr() ) {
      descrs.append(d);
    }
  }
  else if ( 0 != req->descrname_size() ) {
    auto md = TMeteoDescriptor::instance();
    for ( auto dname : req->descrname() ) {
      auto d = md->descriptor( QString::fromStdString(dname) );
      descrs.append(d);
    }
  }

  if( ( 1 != descrs.size() )
      || ( 0 == req->hour_size() )
      || ( 0 == req->level_size() )
      || ( false == req->has_date_start() ) ) {
    error = msglog::kServiceRequestFailed;
    debug_log << error << req->Utf8DebugString();
    return false;
  }
  std::unique_ptr<Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db ) {
    error = QString::fromUtf8("Нет подключения к БД obanaldb");
    debug_log << error;
    return false;
  }

  auto query = db->queryptrByName("load_field");
  if ( nullptr == query ) {
    debug_log << error;
    return false;
  }
  if (true == req->has_date_start()) {
    if (true == req->has_interval()) {
      QDateTime dt = QDateTime::fromString(QString::fromStdString(req->date_start()), Qt::ISODate);
      QDateTime dts = dt.addSecs(req->interval() * 3600);
      QDateTime dte = dt.addSecs(req->interval() * -3600);
      query->arg("start_dt",dts);
      query->arg("end_dt",dte);
    }
    else if ( true == req->has_date_end() ) {
      query->argDt("start_dt",req->date_start());
      query->argDt("end_dt",req->date_end());
    }
    else {
      query->argDt("start_dt",req->date_start());
      query->argDt("end_dt",req->date_start());
    }
  }

  query->arg("descr", descrs );
  query->arg("level_type",req->type_level());
  query->arg("level",req->level());
  query->arg("hour",req->hour());
  query->arg("center",req->center());

  if (true == req->has_model()) {
    query->arg("model",req->model());
  }
  if (true == req->has_net_type()) {
    query->arg("net_type",req->net_type());
  }
  QString err;
  bool queryRes = query->execInit(&err);
  if (false == queryRes) {
    error = err;
    debug_log << error;
    //debug_log << "QUERY = " << query->query();
    return false;
  }

  if ( false == query->next() ) {
    error = QString::fromUtf8("Нет записей по запросу = \n %1").arg( QString::fromStdString( req->Utf8DebugString() ) );
    return false;
  }
  const DbiEntry& document = query->entry();
  QDateTime dt = document.valueDt("dt");
  QDateTime dt1 = document.valueDt("forecast_start");
  QDateTime dt2 = document.valueDt("forecast_end");
  int descr = document.valueInt32("descr");
  int hour = document.valueInt32("hour");
  int level_type = document.valueInt32("level_type");
  int level = document.valueInt32("level");
  int model = document.valueInt32("model");
  int center = document.valueInt32("center");
  int net_type = document.valueInt32("net_type");
  int count_points = document.valueInt32("count_points");
  desc->set_date(dt.toString(Qt::ISODate).toStdString());
  desc->set_dt1(dt1.toString(Qt::ISODate).toStdString());
  desc->set_dt2(dt2.toString(Qt::ISODate).toStdString());
  desc->set_meteodescr(descr);
  desc->set_hour(hour);
  desc->set_level_type(level_type);
  desc->set_level(level);
  desc->set_model(model);
  desc->set_center(center);
  desc->set_net_type(net_type);
  desc->set_count_point(count_points);
  if ( true == document.hasField("dates") ){
    bool ok;
    Array dates_arr = document.valueArray("dates",&ok);
    if(true == ok){
      while ( true == dates_arr.next() ) {
        QDateTime date = dates_arr.valueDt(&ok);
        if(false == ok){ continue; }
        desc->add_dates(date.toString(Qt::ISODate).toStdString());
      }
    }
  }
  QString id = document.valueOid("_id");
  desc->set_id( id.toStdString() );
  *file_id = document.valueOid("fs_id");
  return true;
}

void MethodWrap::GetFieldsData(const DataRequest *req, ManyDataReply *res)
{
  QElapsedTimer ttt; ttt.start();

  DataDescResponse descrresp;

  if( false == getAvailableForecastData(req, &descrresp) ) {
    //res->set_comment(msglog::kServiceAnswerFailed.toStdString());
    res->set_result(false);
    return;
  }
  for( DataDesc descr : descrresp.descr() ) {
    DataReply* dr = res->add_data();
    DataDesc* mdescr = dr->mutable_descr();
    mdescr->CopyFrom(descr);
    QString file_id = QString::fromStdString(descr.file_id());
    QByteArray barr;

    if( true == getFieldFromFile(file_id, &barr)) {
      dr->set_fielddata(barr.data(), barr.size());
    }
    else {
      debug_log << msglog::kFileReadFailed.arg(file_id).arg(-1);
    }
  }
  res->set_comment(msglog::kServerAnswerOK.toStdString());
  res->set_result(true);
  int cur = ttt.elapsed();
  if(1000 < cur) {
    debug_log << msglog::kServiceRequestTime.arg("GetFieldData").arg(cur);
  }
}

void MethodWrap::GetFieldsMData(const DataRequest *req, surf::DataReply *resp)
{
  QElapsedTimer ttt; ttt.start();

  if ( ( 1 < req->center_size() )
       || ( 1 != req->level_size() ) ) {
    resp->set_comment(msglog::kServiceRequestFailedErr.toStdString());
    resp->set_result(false);
    return;
  }

  ManyDataReply fresp;
  if(false == loadFieldsForPuanson(req, &fresp)) {
    resp->set_comment(msglog::kServiceRequestFailedErr.toStdString());
    resp->set_result(false);
    return;
  }

  QMap<int, ::obanal::TField*> fieldlist;
  for(const DataReply& dr: fresp.data()) {
    if(fieldlist.contains(dr.descr().meteodescr())) {
      continue;
    }
    ::obanal::TField* fd = new ::obanal::TField();
    QByteArray barr(dr.fielddata().data(), dr.fielddata().size());
    if(false == fd->fromBuffer(&barr)) {
      delete fd;
      continue;
    }
    fieldlist.insert(dr.descr().meteodescr(), fd);
  }

  QList<int> fdescr = fieldlist.uniqueKeys();
  int koldescr = fdescr.count();
  if(1 > koldescr) {
    resp->set_comment(msglog::kServiceRequestFailedErr.toStdString());
    resp->set_result(false);
    return;
  }
  QString code = "";

  ::obanal::TField* field0 = fieldlist.value(fdescr[0]);
  int kol_fi = field0->kolFi();
  int kol_la = field0->kolLa();

  float oldfi = -9999;
  float oldla = -9999;
  for(int i = 0; i < kol_fi; ++i) {
    float fi = field0->netFi(i) * RAD2DEG;
    if(std::fabs(fi - oldfi) < 2.0) {
      continue;
    }

    for(int j = 0; j< kol_la; ++j) {
      float la = field0->netLa(j) * RAD2DEG;
      if(std::fabs(la - oldla) < 2.0) {
        continue;
      }

      TMeteoData md;
      md.setCoord(fi, la);

      for ( auto descr : fdescr ) {
        ::obanal::TField *fd = fieldlist.value(descr);
        if( nullptr == fd ) {
          continue;
        }
        if( true == fd->getMask(i, j) ) {
          float val = fd->getData(i, j);
          if(10009 == descr) {
            val *= 10.0;
          }
          TMeteoParam param(code, val, control::NO_CONTROL);
          md.add(descr, param);
        }
      }
      QByteArray buf;
      md >> buf;
      resp->add_meteodata(buf.data(), buf.size());
      oldla = la;
    }
    oldfi = fi;
  }

  for(::obanal::TField* field : fieldlist) {
    delete field;
  }
  fieldlist.clear();

  resp->set_comment(msglog::kServerAnswerOK.toStdString());
  resp->set_result(true);
  int cur = ttt.elapsed();
  if(1000 < cur) {
    debug_log << msglog::kServiceRequestTime.arg("GetFieldsMData").arg(cur);
  }
}

void MethodWrap::GetFieldsDataWithoutForecast(const DataRequest *req, ManyDataReply *resp)
{
  QElapsedTimer ttt; ttt.start();
  if(true == req->has_date_start()) {
    if(true == loadFieldsForPuanson(req, resp)) {
      resp->clear_error();
      resp->set_result(true);
      int cur = ttt.elapsed();
      if(1000 < cur) {
        debug_log << msglog::kServiceRequestTime.arg("GetFieldsDataWithoutForecast").arg(cur);
      }
      return;
    }
  }
  resp->set_comment(msglog::kServiceRequestFailedErr.toStdString());
  resp->set_result(false);
}
void MethodWrap::GetFieldDataPoID(const SimpleDataRequest *req, DataReply *resp){
  GetFieldDataPoID(req, resp, 0);
}

void MethodWrap::GetFieldDataPoID(const SimpleDataRequest *req, DataReply *resp, int num_f)
{
  QElapsedTimer ttt; ttt.start();
  QString error;
  if(num_f+1 > req->id_size()) {
    error = msglog::kServiceRequestFailedErr;
    debug_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    int cur = ttt.elapsed();
    if(1000 < cur) {
      debug_log << msglog::kServiceRequestTime.arg("GetFieldDataPoID").arg(cur);
    }
    return;
  }
  QString id = QString::fromStdString(req->id(num_f));
  GetFieldPoID(id, resp);
  int cur = ttt.elapsed();
  if(1000 < cur) {
    debug_log << msglog::kServiceRequestTime.arg("GetFieldDataPoID").arg(cur);
  }
}

void MethodWrap::GetFieldDataForMeteosummary(const DataRequest *req, ValueDataReply *resp)
{
  QString error;
  if(0 == req->coords_size()) {
    resp->set_comment(msglog::kServiceRequestFailedErr.toStdString());
    resp->set_result(false);
    return;
  }

  if(true == req->has_field_id()) {
    error = QObject::tr("Попытка запросить значение по id поля");
    debug_log << error;
    return;
  }
  GridFs gridfs;
  if ( false == gridfs.connect(  global::mongodbConfObanal() ) ) {
    resp->set_comment(gridfs.lastError().toStdString());
    resp->set_result(false);
    info_log << gridfs.lastError();
    return;
  }

  gridfs.use(global::mongodbConfObanal().name(), kFieldPrefix);

  DataDescResponse response;
  if( false == getAvailableDataForMeteosummary(req, &response) ) {
    resp->set_comment(msglog::kServiceRequestFailedErr.toStdString());
    resp->set_result(false);
    return;
  }
  for(const DataDesc& descr : response.descr()) {
    GridFile file = gridfs.findOneById(QString::fromStdString(descr.file_id()));
    if(false == getOnePointData(file, req, resp)) {
      continue;
    }
    else {
      auto data = resp->mutable_data(resp->data_size()-1);
      if ( nullptr != data ) {
        data->mutable_fdesc()->set_hour(descr.hour());
        data->mutable_fdesc()->set_center(descr.center());
      }
    }
    //    if ( ( true == req->has_need_field_descr() )
    //       && ( true == req->need_field_descr() ) ) {
    //      auto data = resp->add_data();
    //      data->mutable_fdesc()->CopyFrom(descr);
    //      for(int i =0; i < resp->data_size(); ++i) {
    //        debug_log << i;
    //        resp->mutable_data(i)->mutable_fdesc()->CopyFrom(descr);
    //      }
    //    }
  }

  resp->set_comment(msglog::kServerAnswerOK.arg("GetFieldDataForMeteosummary").toStdString());
  resp->set_result(true);
}

void MethodWrap::GetAvailableCenters(const DataRequest *req, CentersResponse *resp)
{
  QString error;
  if( false == req->has_date_start() ) {
    error = msglog::kServiceRequestFailedErr;
    debug_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    return;
  }
  if ( (nullptr == req)
       || (nullptr == resp)) {
    error = QObject::tr("Неверные аргументы для функции getAvailableCenters");
    debug_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    return;
  }

  std::unique_ptr<Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) {
    resp->set_result(false);
    return;
  }

  auto query = db->queryptrByName("get_available_centers");
  if(nullptr == query) {
    resp->set_result(false);
    return;
  }

  if (true == req->has_date_start()) {
    if (true == req->has_interval()) {
      QDateTime dt = QDateTime::fromString(QString::fromStdString(req->date_start()), Qt::ISODate);
      QDateTime dts = dt.addSecs(req->interval() * 3600);
      QDateTime dte = dt.addSecs(req->interval() * -3600);
      query->arg("start_dt",dts);
      query->arg("end_dt",dte);
    }
    else if ( true == req->has_date_end() ) {
      query->argDt("start_dt",req->date_start());
      query->argDt("end_dt",req->date_end());
    }
    else {
      query->argDt("start_dt",req->date_start());
      query->argDt("end_dt",req->date_start());
    }
  }
  query->arg("descr",req->meteo_descr());
  query->arg("level_type",req->type_level());
  query->arg("level",req->level());
  query->arg("hour",req->hour());
  query->arg("center",req->center());

  if (true == req->has_model()) {
    query->arg("model",req->model());
  }

  bool queryRes = query->execInit(&error);
  if (false == queryRes) {
    //debug_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    return;
  }
  while (true == query->next()) {
    const DbiEntry& document = query->entry();
    CenterInfo *ci = resp->add_info();
    ci->set_name(document.valueString("name").toStdString());
    ci->set_short_name(document.valueString("short_name").toStdString());
    ci->set_number(document.valueInt32("center"));
    ci->set_priority(document.valueInt32("priority"));//было _id.priority
  }
  resp->set_comment(msglog::kServerAnswerOK.toStdString());
  resp->set_result(true);
}

void MethodWrap::GetAvailableCentersForecast(const DataRequest *req, CentersResponse *resp)
{
  QString error;
  if(false == req->has_date_start()) {
    error = msglog::kServiceRequestFailedErr;
    debug_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    return;
  }

  if ( (nullptr == req)
       || (nullptr == resp)) {
    error = QObject::tr("Неверные аргументы для функции getAvailableCenters");
    debug_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    return;
  }

  std::unique_ptr<Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) {
    resp->set_result(false);
    return;
  }

  auto query = db->queryptrByName("get_available_centers_forecast");
  if(nullptr == query) {
    resp->set_result(false);
    return;
  }
  if (true == req->has_date_start()) {
    if (true == req->has_interval()) {
      QDateTime dt = QDateTime::fromString(QString::fromStdString(req->date_start()), Qt::ISODate);
      QDateTime dts = dt.addSecs(req->interval() * 3600);
      QDateTime dte = dt.addSecs(req->interval() * -3600);
      query->arg("start_fdt",dts);
      query->arg("end_fdt",dte);
    }
    else if ( true == req->has_date_end() ) {
      query->argDt("start_fdt",req->date_start());
      query->argDt("end_fdt",req->date_end());
    }
    else {
      query->argDt("start_fdt",req->date_start());
      query->argDt("end_fdt",req->date_start());
    }
  }
  query->arg("descr",req->meteo_descr());
  query->arg("level_type",req->type_level());
  query->arg("level",req->level());
  query->arg("hour",req->hour());
  query->arg("center",req->center());

  if (true == req->has_model()) {
    query->arg("model",req->model());
  }

  QString err;
  bool queryRes = query->execInit(&err);
  if (false == queryRes) {
    error = err;
    //debug_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    return;
  }
  if (true == query->next()) {
    const DbiEntry& document = query->entry();
    CenterInfo *ci = resp->add_info();
    if(true == document.hasField("name")) {
      QString name = document.valueString("name");
      ci->set_name(name.toStdString());
    }
    if(true == document.hasField("short_name")) {
      QString short_name = document.valueString("short_name");
      ci->set_short_name(short_name.toStdString());
    }
    int center = document.valueInt32("center");
    int priority = document.valueInt32("priority");
    ci->set_number(center);
    ci->set_priority(priority);
  }
  resp->set_comment(msglog::kServerAnswerOK.toStdString());
  resp->set_result(true);
}


void MethodWrap::GetAvailableCentersAD(const DataRequest *req, CentersResponse *resp)
{
  if(false == req->has_date_start()&&
     false == req->has_forecast_start()) {
    resp->set_comment(msglog::kServiceRequestFailedErr.toStdString());
    resp->set_result(false);
    debug_log << msglog::kServiceRequestFailedErr;
    return;
  }
  DataRequest request = *req;
  /*!
* {
*   100 -  Изобарическая поверхность
*   7   - Тропопауза
*   6   -Уровень максимального ветра
* }
*/
  request.clear_type_level();
  for(const int type_level : { 100, 7, 6 }) {
    request.add_type_level(type_level);
  }

  /*!
*   10009 - Геопотенциальная высота
*   11003 - Ветер U составляющая
*   11004 - Ветер V составляющая
*   12101 - Температура
*   12103 - Температура точки росы
*   12108 - Дефицит точки росы на уровнях
*/
  for(const int descr : { 10009, 11003, 11004, 12101, 12103, 12108 }) {
    request.add_meteo_descr(descr);
  }

  for(const int level : { 1000, 925, 850, 700, 500, 400, 300, 250, 200, 150, 100, 70, 50, 30, 20, 10, 5 /*, 0*/ }) {
    request.add_level(level);
  }
  QMap<int, QPair<int,QString> > centers;
  ManyFieldsForDatesResponse response;
  if ( false == getFieldsForAD(&request, &response) ) {
    resp->set_comment(response.comment());
    resp->set_result(false);
    return;
  }
  for (int i = 0, dsz = response.date_size(); i < dsz ; ++i) {
    ManyFieldsForDate date = response.date(i);
    for (int j = 0, zsz = date.zonds_size(); j < zsz ; ++j) {
      if  ( (1 < date.zonds(j).levels_size())
            && (10 < date.zonds(j).count()) ) {
        int center = date.zonds(j).id().center();
        if ( false == centers.contains( center ) ) {
          QString centerStr;
          if ( true == date.zonds(j).id().has_short_name()) {
            centerStr = QString::fromStdString( date.zonds(j).id().short_name() );
          }
          else if ( true == date.zonds(j).id().has_name() ) {
            centerStr = QString::fromStdString( date.zonds(j).id().name() );
          }
          else {
            centerStr = QString::number( center );
          }
          int priority = date.zonds(j).id().priority();
          centers.insert(center, QPair<int, QString> (priority, centerStr) );
        }
      }
    }
  }
  QMap<int, QPair< int, QString > >::Iterator it;
  for (it = centers.begin(); it != centers.end(); ++it) {
    CenterInfo* info = resp->add_info();
    info->set_number(it.key());
    QPair<int, QString> pair = it.value();
    info->set_priority( pair.first );
    info->set_name( pair.second.toStdString() );
  }
  resp->set_comment(msglog::kServerAnswerOK.arg("GetAvailableCentersAD").toStdString());
  resp->set_result(true);
  return;
}

void MethodWrap::GetForecastValues(const DataRequest *req, ValueDataReply *resp)
{
  QString error;
  if(0 == req->coords_size()) {
    resp->set_comment(msglog::kServiceRequestFailedErr.toStdString());
    resp->set_result(false);
    return;
  }
  DataDescResponse response;

  if ( true == req->has_field_id() ) {
    DataReply fresp;
    QString fid = QString::fromStdString(req->field_id());
    if ( false == GetFieldDescrPoID(fid, &fresp) ) {
      error = msglog::kDataServiceNoData;
      info_log << error;
      resp->set_comment(error.toStdString());
      resp->set_result(false);
      return;
    }
    response.add_descr()->CopyFrom(fresp.descr());
  }
  else {
    if(false == getAvailableForecastData(req, &response)) {
      resp->set_error(msglog::kServiceRequestFailedErr.toStdString());
      resp->set_result(false);
      return;
    }
  }


  GridFs gridfs;
  if ( false == gridfs.connect( global::mongodbConfObanal() ) ) {
    resp->set_error(gridfs.lastError().toStdString());
    resp->set_result(false);
    debug_log << gridfs.lastError();
    return;
  }
  gridfs.use(global::mongodbConfObanal().name(), kFieldPrefix);

  for(const DataDesc& descr : response.descr()) {

    GridFile file = gridfs.findOneById(QString::fromStdString(descr.file_id()));
    ValueDataReply resp_vs;
    if(false == getOnePointData(file, req, &resp_vs)) {
      continue;
    }
    if( ( true == req->has_need_field_descr() )
        && (true == req->need_field_descr() ) ) {
      for(int i =0; i < resp_vs.data_size(); ++i) {
        resp_vs.mutable_data(i)->mutable_fdesc()->CopyFrom(descr);
      }
    }
    resp->MergeFrom(resp_vs);
  }
  resp->set_comment(msglog::kServerAnswerOK.arg("GetForecastValues").toStdString());
  resp->set_result(true);
}

void MethodWrap::GetAvailableData(const DataRequest *req, DataDescResponse *resp)
{
  QElapsedTimer ttt; ttt.start();

  //error_log<<req->Utf8DebugString();

  if(false == req->has_date_start()&& false == req->has_forecast_start()) {
    debug_log << msglog::kServiceRequestFailedErr;
    resp->set_comment(msglog::kServiceRequestFailedErr.toStdString());
    resp->set_result(false);
    return;
  }
  if (true == getAvailableForecastData(req,resp)) {
    resp->set_comment(msglog::kServerAnswerOK.arg("getAvailableData").toStdString());
    resp->set_result(true);
    int cur = ttt.elapsed();
    if(1000 < cur) {
      debug_log << msglog::kServiceRequestTime.arg("getAvailableData").arg(cur);
    }
  }
  else {
    debug_log << msglog::kServerAnswerError.arg("getAvailableData");
    resp->set_comment(msglog::kServerAnswerError.arg("getAvailableData").toStdString());
    resp->set_result(false);
    int cur = ttt.elapsed();
    if(1000 < cur) {
      debug_log << msglog::kServiceRequestTime.arg("getAvailableData").arg(cur);
    }
  }
}

void MethodWrap::GetAvailableHours( const DataRequest* req, HourResponse* resp )
{
  std::unique_ptr<Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) {
    QString error = QObject::tr("Нет подключения к БД");
    resp->set_comment( error.toStdString() );
    error_log << error;
    resp->set_result(false);
    return;
  }
  QString err;
  auto query = db->queryptrByName("get_available_hours");
  if ( nullptr == query ) {
    QString error = QObject::tr("Не найден запрос get_available_hours");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    return;
  }
  if ( false == req->has_date_start() ) {
    QString error = QObject::tr("Не указана дата для поиска доступных часов");
    error_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    return;
  }
  query->argDt( "dt", req->date_start() );
  if ( 0 != req->meteo_descr_size() ) {
    query->arg("descr",req->meteo_descr());
  }
  if ( 0 != req->type_level_size() ) {
    query->arg("level_type",req->type_level());
  }
  if ( 0 != req->level_size() ) {
    query->arg("level",req->level());
  }
  if ( 0 != req->center_size() ) {
    query->arg("center",req->center());
  }

  if (true == req->has_model()) {
    query->arg("model",req->model());
  }
  if ( true == req->has_min_level_count() ) {
    query->arg("min_level_count",req->min_level_count());
  }

  bool queryRes = query->execInit(&err);
  if (false == queryRes) {
    error_log << err;
    resp->set_comment(err.toStdString());
    resp->set_result(false);
    return;
  }
  QMap< int, QMap< int, QString > > center_hours;
  while ( true == query->next() ) {
    const DbiEntry& document = query->entry();
    QDateTime dt = document.valueDt("dt");
    int center = document.valueInt32("center");
    center_hours[center].insert( document.valueInt32("hour"), dt.toString(Qt::ISODate) );
    //    HourInfo* info = resp->add_info();
    //    info->set_datetime( dt.toString(Qt::ISODate).toStdString() );
    //    info->set_hour( document.valueInt32("hour") );
  }

  CentersResponse center_resp;
  GetAvailableCenters( req, &center_resp );
  for ( auto it = center_hours.begin(), end = center_hours.end(); it != end; ++it ) {
    int cnumber =  it.key();
    CenterInfo* info = resp->add_info();
    info->set_number(cnumber);
    for ( auto c : center_resp.info() ) {
      if ( c.number() == cnumber ) {
        info->MergeFrom(c);
      }
    }
    QMap< int, QString > hours = it.value();
    for ( auto hit = hours.begin(), hend = hours.end(); hit != hend; ++hit ) {
      HourInfo* proto_hour = info->add_hour();
      proto_hour->set_hour( hit.key() );
      proto_hour->set_datetime( hit.value().toStdString() );
    }
  }

  resp->set_result(true);
}

int sortZond(const DataRequest *req,const FieldsResponse &zond,
             GridFs *gridfs,
             QMap<QString, QMap<int, zond::Zond>> *dtZondsMap )
{
  int procFields = 0; // подсчёт кол. обработанных полей
  for (int i = 0, lsz = zond.levels_size(); i < lsz; ++i) {
    Level levelP = zond.levels(i);
    for (int j = 0, dsz = levelP.descrs_size(); j < dsz; ++j) {
      Descr descr = levelP.descrs(j);
      GridFile file = gridfs->findOneById(QString::fromStdString(descr.fs_id()));
      ::obanal::TFieldMongo fd;
      if ( false == fd.fromGridFile(file) ) {
        debug_log << QObject::tr("Не удалось обработать файл %1").arg(file.fileName());
        continue;
      }

      int hour = fd.getHour();
      QDateTime dtAnalyse = fd.getDate();
      QDateTime dt  = dtAnalyse.addSecs(hour);
      QString strDt = dt.toString(Qt::ISODate).replace("Z","");//FIXME replace("Z","")
      int level = levelP.level();
      int level_type = descr.level_type();
      int cur_descr = descr.descr();
      /* if(meteodescr::kIsobarLevel != level_type) {
          level = 10000 * level_type;
        }*/
      for(int cc = 0; cc < req->coords_size(); ++cc) {
        meteo::GeoPoint point(req->coords(cc).fi(), req->coords(cc).la());
        bool ok;
        float value = fd.pointValue(point, &ok);
        if(false == ok) {
          continue;
        }
        zond::Zond &znd = (*dtZondsMap)[strDt].operator[](cc);
        znd.addCenterName( QString::fromStdString( zond.id().short_name() ) );
        znd.setDateTime(dt);
        znd.setDateTimeAnalyse(dtAnalyse);
        //special case =) для зонда считаем, что уровень 2 и 10 метров - это у земли
        if(meteodescr::kHeightLevel == level_type &&
           (10 == level || 2 == level ) )
        {
          level_type = meteodescr::kSurfaceLevel;
          level = 0;
        }
        switch(cur_descr) {
          case 10009:
            value *=10.;
          default:
            znd.setValue(level_type,level,cur_descr,value,control::NO_CONTROL);
          break;
          case 20010:
            znd.cloudP()->set(zond::N,value,control::NO_CONTROL);
          break;
          case 20011:
            znd.cloudP()->set(zond::Nh,value,control::NO_CONTROL);
          break;
          case 20012:
            znd.cloudP()->setC(value,control::NO_CONTROL);
          break;
          case 20013:
            znd.cloudP()->set(zond::h,value,control::NO_CONTROL);
          break;
        }
      }
      ++procFields;
    }
  }
  return procFields;
}

void MethodWrap::GetADZond( const DataRequest *areq, surf::TZondValueReply *resp )
{
  ManyFieldsForDatesResponse response;
  QElapsedTimer ttt;
  ttt.start();
  DataRequest req = *areq;

  if(req.meteo_descr_size() < 1 ){
    for(const int descr : { 7004, 20010, 20011, 20012, 20013,10004,10051, 10009, 11003, 11004, 12101, 12103, 12108 }) {
      req.add_meteo_descr(descr);
    }
  }

  if ( false == getFieldsForAD(&req, &response) ) {
    debug_log << msglog::kServerAnswerFailed.arg("getFieldsForAD").toStdString();
    resp->set_comment(msglog::kServerAnswerFailed.arg("getFieldsForAD").toStdString());
    resp->set_result(false);
    return;
  }
  // debug_log<<req.DebugString();
  QMap<int, zond::Zond> stzond; //номер точки - зонд

  for(int i = 0, isz = req.coords_size(); i < isz; ++i) {
    zond::Zond zond;
    float height = (req.coords(i).has_height() ? req.coords(i).height():0);
    zond.setCoord(GeoPoint(req.coords(i).fi(), req.coords(i).la(),height));
    if( true == req.coords(i).has_index() ) {
      zond.setStIndex(QString::fromStdString(req.coords(i).index()));
    }
    stzond.insert(i, zond);
  }
  GridFs gridfs;
  if ( false == gridfs.connect( meteo::global::mongodbConfObanal() ) ) {
    response.set_comment(gridfs.lastError().toStdString());
    response.set_result(false);
    error_log << gridfs.lastError();
    return;
  }

  gridfs.use(global::mongodbConfObanal().name(), kFieldPrefix);

  QMap<QString, QMap<int, zond::Zond>> dtZondsMap;


  //FieldsResponse zond;
  int max_levels = 1;
  int num_best = -1;
  QMap<QString, FieldsResponse> frespMap;
  for ( int i = 0, dsz = response.date_size(); i < dsz; ++i ) {
    ManyFieldsForDate date = response.date(i);
    const QString &forecast_start = QString::fromStdString( date.forecast_start() );
    for ( int j = 0, zsz = date.zonds_size(); j < zsz; ++j ) {
      if ( max_levels < date.zonds(j).levels_size() ) {
        if(true == req.only_best() ){
          max_levels = date.zonds(j).levels_size();
          num_best = j;
        } else {
          frespMap.insertMulti(forecast_start,date.zonds(j));
        }
      }
    }

    if(true == req.only_best() && -1!=num_best ){
      frespMap.insertMulti(forecast_start, date.zonds(num_best));
    }
    // создаем набор(по количеству точек) зондов для каждой даты
    if ( false == dtZondsMap.contains(forecast_start) ) {
      dtZondsMap[forecast_start] = stzond;
    }
    if(true == req.only_last() ){
      break;
    }
  }

  QList<QString> dates = dtZondsMap.uniqueKeys();
  for(int i = 0; i< dates.size(); ++i  ){
    foreach (const FieldsResponse &zond, frespMap.values(dates.at(i))) {
      sortZond(&req, zond, &gridfs, &dtZondsMap);
    }
  }
  foreach(const QString& strDt, dtZondsMap.keys()) {
    foreach( zond::Zond z, dtZondsMap[strDt].values()) {
      meteo::surf::OneTZondValueOnStation* onezond = resp->add_data();
      z.restoreUrList();
      z.preobr();
      onezond->mutable_point()->set_fi(z.coord().fi());
      onezond->mutable_point()->set_la(z.coord().la());
      onezond->mutable_point()->set_height(z.coord().alt());
      onezond->mutable_point()->set_id(z.stIndex().toStdString());
      onezond->set_date(z.dateTime().toString(Qt::ISODate).toStdString());
      onezond->set_hour(z.dateTimeAnalyse().secsTo(z.dateTime()));
      QStringList centers = z.centersName();
      onezond->set_center( centers.join("").toStdString() );

      // если нужен прото-ответ
      if ( req.has_as_proto() && req.as_proto() ) {
        QList<meteo::surf::MeteoDataProto> meteodata_proto = z.toMeteoDataProto();
        for (int i = 0; i < meteodata_proto.size(); i++) {
          meteo::surf::MeteoDataProto *mdata = onezond->add_meteodata_proto();
          mdata->CopyFrom(meteodata_proto.at(i));
        }
        calculateZondLayers( &z, onezond );
        //        QList <float> layers ;
        //        // болтанка
        //        if ( z.oprGranBoltan(&layers) ){
        //          fillZondLayersFromList(layers, onezond->mutable_boltanka(), &z );
        //        }
        //        layers.clear();
        //        // облачность
        //        if ( z.oprGranOblak(&layers) ){
        //          fillZondLayersFromList(layers, onezond->mutable_cloudlayers(), &z );
        //        }
        //        layers.clear();
        //        // обледенение
        //        if ( z.oprGranObled(&layers) ){
        //          fillZondLayersFromList(layers, onezond->mutable_obledenenie(), &z );
        //        }
        //        layers.clear();
        //        // слои конденсационный следов
        //        if ( z.oprGranTrace(&layers) ){
        //          fillZondLayersFromList(layers, onezond->mutable_kondensate_trace(), &z );
        //        }
        //        layers.clear();
        //
        //
        //        //
        //        // Ищем границы КНС
        //        if ( z.oprGranKNS(&layers) ){
        //          if ( layers.size()==2 ){
        //            meteo::surf::ZondLayer* zondlayer = onezond->mutable_kns();
        //            zondlayer->set_bottom( layers.at(0)   );
        //            zondlayer->set_top( layers.at(1) );
        //          }
        //        }
        //
        //        // забираем тропопаузу
        //        QList<zond::Uroven> ur_tropo;
        //        if( true == z.getTropo(&ur_tropo) && ur_tropo.size()>0 ) {
        //          for (int idx = 0; idx < ur_tropo.size(); idx++) {
        //            meteo::surf::ZondLayer *zondlayer = onezond->add_tropo();
        //            zondlayer->set_bottom( ur_tropo.at(idx).value(zond::UR_H) );
        //            zondlayer->set_top( ur_tropo.at(idx).value(zond::UR_H) );
        //            zondlayer->set_t( ur_tropo.at(idx).value(zond::UR_T) );
        //          }
        //        }
        //
        //        // слои инверсии и изотремии
        //        QVector<zond::InvProp> vectorlayers;
        //        if ( z.getSloiInver(&vectorlayers) ){
        //          for (int i =0; i< vectorlayers.size(); ++i ){
        //            meteo::surf::ZondLayer *zondlayer;
        //            if ( vectorlayers.at(i).invType == zond::InvProp::InvType::INVERSIA ) {
        //              zondlayer = onezond->add_inverse();
        //            }else{
        //              zondlayer = onezond->add_isoterm();
        //            }
        //            zondlayer->set_bottom( vectorlayers.at(i).h_lo   );
        //            zondlayer->set_top(    vectorlayers.at(i).h_hi   );
        //          }
        //        }
        //
        //        // энергия неустойчивости
        //        int energy = 0;
        //        energy = z.oprZnakEnergy( 850.0 );
        //        onezond->set_energy( energy );
        //
        //        // значение индекса
        //        float index_value;
        //
        //        // индексы неустойчивости
        //        if ( zond::kiIndex(z, &index_value) ) {
        //          // ссылка на протоструктуру
        //          meteo::surf::MeteoParamProto* cur_index = onezond->add_indexes();
        //          cur_index->set_value(index_value);
        //          cur_index->set_descrname("K_index");
        //        }
        //        if ( zond::verticalTotalsIndex(z, &index_value) ) {
        //          // ссылка на протоструктуру
        //          meteo::surf::MeteoParamProto* cur_index = onezond->add_indexes();
        //          cur_index->set_value(index_value);
        //          cur_index->set_descrname("VT_index");
        //        }
        //        if ( zond::crossTotalsIndex(z, &index_value) ) {
        //          // ссылка на протоструктуру
        //          meteo::surf::MeteoParamProto* cur_index = onezond->add_indexes();
        //          cur_index->set_value(index_value);
        //          cur_index->set_descrname("CT_index");
        //        }
        //        if ( zond::totalTotalsIndex(z, &index_value) ) {
        //          // ссылка на протоструктуру
        //          meteo::surf::MeteoParamProto* cur_index = onezond->add_indexes();
        //          cur_index->set_value(index_value);
        //          cur_index->set_descrname("T_index");
        //        }
        //        if ( zond::sweatIndex(z, &index_value) ) {
        //          // ссылка на протоструктуру
        //          meteo::surf::MeteoParamProto* cur_index = onezond->add_indexes();
        //          cur_index->set_value(index_value);
        //          cur_index->set_descrname("SWEAT_index");
        //        }

        
      }
      else {
        QByteArray buf;
        z >> buf;
        onezond->set_meteodata(buf.data(),buf.size());
      }

      //TODO модель, срок прогноза и проч.
    }
  }

  if ( ttt.elapsed() > 2000 ) {
    debug_log << QString::fromUtf8("Время обработки запроса GetADZond: %1 мсек")
                 .arg(ttt.elapsed());
  }

  resp->set_comment(msglog::kServerAnswerOK.arg("GetADZond").toStdString());
  resp->set_result(true);
  return;
}


///**
// * @brief функция, которая будет заполнять протосообщение из QList
// * 
// * @param layers 
// * @param res 
// */
//void MethodWrap::fillZondLayersFromList(QList<float> &layers, 
//                          google::protobuf::RepeatedPtrField<meteo::surf::ZondLayer> *res,
//                          zond::Zond* znd)
//{
//  // если слоев четное количество
//  if ( layers.size()%2 == 0 ){
//    for (int i =0; i< layers.size(); i+=2 ){
//      meteo::surf::ZondLayer *zondlayer = res->Add();
//      zondlayer->set_bottom( znd->oprHpoP(layers.at(i))   );
//      zondlayer->set_top( znd->oprHpoP(layers.at(i+1)) );
//    }
//  }
//  return;
//}

void MethodWrap::GetLastDate(const DataRequest *req, SimpleDataReply *resp)
{
  QElapsedTimer ttt; ttt.start();
  //  if( ( false == req->has_date_start() )
  //    || ( 0 == req->center_size() ) ) {
  //    resp->set_error(msglog::kServiceRequestFailedErr.toStdString());
  //    resp->set_result(false);
  //    return;
  //  }
  std::unique_ptr<Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) {
    resp->set_result(false);
    return;
  }
  QString err;
  auto query = db->queryptrByName("get_last_date");
  if(nullptr == query) {
    resp->set_result(false);
    return;
  }
  if (true == req->has_date_start()) {
    if (true == req->has_interval()) {
      QDateTime dt = QDateTime::fromString(QString::fromStdString(req->date_start()), Qt::ISODate);
      QDateTime dts = dt.addSecs(req->interval() * 3600);
      QDateTime dte = dt.addSecs(req->interval() * -3600);
      query->arg("start_fdt",dts);
      query->arg("end_fdt",dte);
    }
    else if ( true == req->has_date_end() ) {
      query->argDt("start_fdt",req->date_start());
      query->argDt("end_fdt",req->date_end());
    }
    else {
      query->argDt("start_fdt",req->date_start());
      query->argDt("end_fdt",req->date_start());
    }
  }
  if ( 0 != req->meteo_descr_size() ) {
    query->arg("descr",req->meteo_descr());
  }
  if ( 0 != req->type_level_size() ) {
    query->arg("level_type",req->type_level());
  }
  if ( 0 != req->level_size() ) {
    query->arg("level",req->level());
  }
  if ( 0 != req->hour_size() ) {
    query->arg("hour",req->hour());
  }
  if ( 0 != req->center_size() ) {
    query->arg("center",req->center());
  }

  if (true == req->has_model()) {
    query->arg("model",req->model());
  }

  bool queryRes = query->execInit(&err);
  if (false == queryRes) {
    // debug_log << err;
    resp->set_comment(err.toStdString());
    resp->set_result(false);
    return;
  }
  if (true == query->next()) {
    const DbiEntry& document = query->entry();
    QDateTime dt = document.valueDt("forecast_start");;
    resp->set_id( dt.toString(Qt::ISODate).toStdString());
    int cur = ttt.elapsed();
    if(1000 < cur) { debug_log << msglog::kServiceRequestTime.arg("GetLastDate").arg(cur); }
    resp->set_comment(msglog::kServerAnswerOK.arg("GetLastDate").toStdString());
    resp->set_result(true);
    return;
  }
  else {
    QString error;
    error = QObject::tr("В ответе нет документа");
    debug_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    return;
  }
}

void MethodWrap::GetFieldDates(const DataRequest *req, DateReply *resp)
{
  QElapsedTimer ttt; ttt.start();
  QString error;
  if((false == req->has_date_start())
     || (false == req->has_date_end())) {
    resp->set_comment(msglog::kServerAnswerError.arg("GetFieldDates").toStdString());
    resp->set_result(false);
    return;
  }
  resp->clear_date();

  std::unique_ptr<Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) {
    resp->set_result(false);
    return;
  }
  auto query = db->queryptrByName("get_field_dates");
  if(nullptr == query) {
    resp->set_result(false);
    return;
  }
  query->argDt("start_fdt",req->date_start());
  query->argDt("end_fdt",req->date_end());


  if(0 != req->center_size()) {
    query->arg("center", req->center() );
  }

  QString err;
  bool queryRes = query->execInit(&err);
  if (false == queryRes) {
    //debug_log << err;
    resp->set_comment(err.toStdString());
    resp->set_result(false);
    return;
  }
  for (int i = 0; true == query->next();++i ) {
    const DbiEntry& doc = query->entry();
    resp->add_date();
    resp->set_date(i,doc.valueDt("forecast_start").toString(Qt::ISODate).toStdString());
  }
  resp->set_comment(msglog::kServerAnswerOK.arg("GetFieldDates").toStdString());
  resp->set_result(true);
  int cur = ttt.elapsed();
  if(1000 < cur) {
    debug_log << msglog::kServiceRequestTime.arg("GetFieldDates").arg(cur);
  }
  return;
}

void MethodWrap::GetNearDate(const DataRequest *req, DateReply *resp)
{
  QElapsedTimer ttt; ttt.start();
  if(false == req->has_date_start()) {
    resp->set_comment(msglog::kServerAnswerError.arg("GetNearDate").toStdString());
    resp->set_result(false);
    return;
  }
  DataRequest request;
  request.CopyFrom(*req);
  QDateTime dt = QDateTime::fromString(QString::fromStdString(request.date_start()), "yyyy-MM-ddThh:mm:ss");
  if (false == request.has_interval()) {
    QDateTime dts = dt.addSecs(-12 * 3600);
    QDateTime dte = dt.addSecs(12 * 3600);
    request.set_date_start(dts.toString(Qt::ISODate).toStdString());
    request.set_date_end(dte.toString(Qt::ISODate).toStdString());
  }
  else {
    QDateTime dts = dt.addSecs(-request.interval() * 3600);
    QDateTime dte = dt.addSecs(request.interval() * 3600);
    request.set_date_start(dts.toString(Qt::ISODate).toStdString());
    request.set_date_end(dte.toString(Qt::ISODate).toStdString());
  }

  GetFieldDates(&request,resp);
  if ( false == resp->result()) {
    return;
  }
  if (0 == resp->date_size()) {
    resp->set_comment(msglog::kServerAnswerError.arg("GetNearDate").toStdString());
    resp->set_result(false);
    return;
  }
  QDateTime date = QDateTime::fromString(QString::fromStdString(resp->date(0)), "yyyy-MM-ddThh:mm:ss");
  qint64 diff;
  if (req->only_last())  {
    diff = dt.secsTo(date);
  }
  else if (req->only_best()) {
    if (dt.secsTo(date) < 0) {
      diff = dt.secsTo(date);
    }
    else {
      diff = -9999999;
    }
  }
  else {
    diff = qFabs(dt.secsTo(date));
  }
  int number = 0;
  for (int i=0, sz=resp->date_size(); i<sz; ++i) {
    date = QDateTime::fromString(QString::fromStdString(resp->date(i)), "yyyy-MM-ddThh:mm:ss");
    if (req->only_last())  {
      if ( dt.secsTo(date) >= 0 && dt.secsTo(date) <= diff ) {
        diff = dt.secsTo(date);
        number = i;
      }
    }
    else if (req->only_best()) {
      if ( dt.secsTo(date) <= 0 && dt.secsTo(date) >= diff ) {
        diff = dt.secsTo(date);
        number = i;
      }
    }
    else {
      if (qFabs(dt.secsTo(date)) <= diff) {
        diff = qFabs(dt.secsTo(date));
        number = i;
      }
    }
  }
  date = QDateTime::fromString(QString::fromStdString(resp->date(number)), "yyyy-MM-ddThh:mm:ss");
  resp->clear_date();
  std::string near = date.toString(Qt::ISODate).toStdString();
  resp->add_date(near);
  resp->set_comment(msglog::kServerAnswerOK.arg("GetNearDate").toStdString());
  resp->set_result(true);
  int cur = ttt.elapsed();
  if(1000 < cur) {
    debug_log << msglog::kServiceRequestTime.arg("GetNearDate").arg(cur);
  }
  return;
}

void MethodWrap::GetFieldsForDate(const DataRequest *req, ManyFieldsForDatesResponse *resp)
{
  QString error;
  if ( ( false == req->has_date_start() )
       || ( false == req->has_date_end() ) ) {
    resp->set_comment(msglog::kServerAnswerError.arg("GetFieldsForDate").toStdString());
    resp->set_result(false);
    return;
  }

  std::unique_ptr<Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) {
    resp->set_result(false);
    return;
  }

  auto query = db->queryptrByName("aggregate_get_fields_for_date");
  if(nullptr == query) {
    resp->set_result(false);
    return;
  }
  query->argDt("start_fdt",req->date_start());
  query->argDt("end_fdt",req->date_end());

  if ( 0 != req->meteo_descr_size() ) {
    QList<int> descrlist;
    for (int i = 0; i < req->meteo_descr_size(); ++i){
      descrlist << req->meteo_descr(i);
    }
    query->arg("descr",descrlist);
  }

  QString err;
  bool queryRes = query->execInit(&err);
  if (false == queryRes) {
    //debug_log << err;
    resp->set_comment(err.toStdString());
    resp->set_result(false);
    return;
  }
  for (int i = 0; true == query->next();++i ) {
    const DbiEntry& doc = query->entry();
    //Document id = doc.valueDocument("_id");
    Array zonds = doc.valueArray("zonds");
    field::ManyFieldsForDate* date = resp->add_date();
    date->set_forecast_start(doc.valueDt("forecast_start").toString(Qt::ISODate).toStdString());
    while (zonds.next()) {
      Document zond;
      if (true == zonds.valueDocument(&zond)) {
        field::FieldsResponse* zondsResp = date->add_zonds();
        field::FieldId* fieldId = zondsResp->mutable_id();
        fieldId->set_center( zond.valueInt32("center") );
        fieldId->set_name(zond.valueString("name").toStdString());
        fieldId->set_short_name(zond.valueString("short_name").toStdString());
        fieldId->set_priority(zond.valueInt32("priority"));
        fieldId->set_model( zond.valueInt32("model") );
        fieldId->set_hour( zond.valueInt32("hour") );
        zondsResp->set_count(zond.valueInt32("count"));
        Array levels = zond.valueArray("levels");
        while (levels.next()) {
          field::Level* fieldLevel = zondsResp->add_levels();
          Document level;
          if ( true == levels.valueDocument(&level) ) {
            fieldLevel->set_level( level.valueInt32("level") );
            Array descrs = level.valueArray("descrs");
            while (descrs.next()) {
              Document descrDoc;
              if (true == descrs.valueDocument(&descrDoc)&& !descrDoc.isEmpty()) {
                field::Descr* descr = fieldLevel->add_descrs();
                descr->set_descr(descrDoc.valueInt32("descr"));
                descr->set_fileid(descrDoc.valueOid("id").toStdString());
                descr->set_fs_id(descrDoc.valueOid("fs_id").toStdString());
                descr->set_level_type(descrDoc.valueInt32("level_type"));
              }
            }
          }
        }
      }
    }
  }
  resp->set_comment(msglog::kServerAnswerOK.arg("GetFieldsForDate").toStdString());
  resp->set_result(true);
  return;
}

void MethodWrap::GetIsoLines( const DataRequest* req, meteo::IsoLines* resp )
{
  DataDesc desc;
  QString filename;
  QString error;
  if ( false == loadFieldInfo( req, &desc, &filename, &error ) ) {
    error_log << QObject::tr("Не удалось получить описание поля. Ошибка = %1").arg(error);
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    return;
  }
  //  debug_log << "FIELD DESCR =" << desc.Utf8DebugString();
  QByteArray arr;
  if ( false== getFieldFromFile( filename, &arr ) ) {
    error_log << QObject::trUtf8("Ошибка получения файла с полем метеорологической величины");
    return;
  }
  obanal::TField* field = new obanal::TField;
  if ( false == field->fromBuffer(&arr) ) {
    debug_log << QObject::tr("Ошибка извлечения поля из массива байтов");
    delete field;
  }
  field->smootchField(10.0);

  auto kcenters = global::kMeteoCenters();
  QString layername = layernameFromInfo(desc);

  auto header = resp->mutable_header();
  header->set_dt( field->getDate().toString(Qt::ISODate).toStdString() );
  header->set_unit( field->unit().toStdString() );
  header->set_name( field->name().toStdString() );
  header->set_level( field->getLevel() );
  header->set_level_type( field->getLevelType() );
  header->set_meteodescr( field->getDescr() );
  header->set_hour( field->getHour() );
  header->set_model( field->getModel() );
  header->set_center( field->getCenter() );
  header->set_center_name( kcenters[field->getCenter()].first.toStdString() );
  header->set_layer_name( layername.toStdString() );
  header->set_field_id( desc.id() );

  float step_lat;
  float step_lon;
  if ( false == field->stepFiDeg( &step_lat ) || false == field->stepLaDeg( &step_lon )  ) {
    error = QString::fromUtf8("Шаг сетки неизвестен для поля = %1")
            .arg( QString::fromStdString( header->Utf8DebugString() ) );
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    delete field;
    return;
  }
  header->set_step_lat( step_lat );
  header->set_step_lon( step_lon );
  header->mutable_begin()->CopyFrom( geopoint2pbgeopoint( field->begin() ) );
  header->mutable_end()->CopyFrom( geopoint2pbgeopoint( field->end() ) );

  map::GradientParams params( meteo::global::kIsoParamPath() );
  const map::proto::FieldColor& clr = params.protoParams( field->getDescr() );
  map::proto::LevelColor lvlclr = map::GradientParams::levelProto( field->getLevel(), field->getLevelType(), clr );
  float step = lvlclr.step_iso();
  auto protoparams = params.protoParams( field->getDescr() );
  TColorGrad colorgrad = map::GradientParams::isoParams( field->getLevel(), field->getLevelType(), protoparams );
  float min = int(field->min_field_value()/step)*step;
  float max = field->max_field_value();

  QVector<meteo::GeoVector> isolines;
  TIsoLineData iso_iso(field);
  auto descrname = TMeteoDescriptor::instance()->name( field->getDescr() ).toStdString();
  for ( float i = min; i <= max; i+=step ) {
    isolines.clear();
    iso_iso.makeOne( i, &isolines );
    bool ok = false;
    QColor clr = colorgrad.color( i, &ok );
    if ( false == ok ) {
      clr = Qt::black;
    }
    Pen pen = protoparams.pen();
    pen.set_color( clr.rgba() );
    Brush brush;
    brush.set_color( clr.rgba() );
    for ( auto isovector : isolines ) {
      if ( 0 == isovector.size() ) {
        continue;
      }
      auto iso = resp->add_isoline();
      iso->mutable_pen()->CopyFrom(pen);
      iso->set_name(descrname);
      iso->set_datetime( field->getDate().toString(Qt::ISODate).toStdString() );
      iso->set_hour( field->getHour() );
      iso->set_value(i);
      iso->mutable_skelet()->CopyFrom( meteo::geovector2pbgeovector(isovector) );
      iso->mutable_skelet()->mutable_props()->mutable_font()->CopyFrom( protoparams.font() );
      iso->mutable_skelet()->mutable_props()->mutable_brush()->CopyFrom(brush);
      iso->mutable_skelet()->mutable_props()->mutable_pen()->CopyFrom(pen);
    }
  }
  if ( true == req->need_extremums() ) {
    QVector<fieldExtremum> extremums = field->calcExtremum();
    for ( int i =0, j= extremums.size(); i < j ;++i ) {
      auto value = resp->add_extremums();
      value->set_tip(extremums.at(i).tip);
      value->set_value(extremums.at(i).znach);
      value->mutable_pbcoord()->CopyFrom(geopoint2pbgeopoint(extremums.at(i).koord ) );
    }
  }
  resp->set_result(true);
  delete field;
}

void MethodWrap::GetFieldProto( const DataRequest* req, meteo::field::FieldReply* resp )
{
  QString error;
  QElapsedTimer ttt; ttt.start();
  if ( ( nullptr == req ) || ( nullptr == resp ) ) {
    error = QObject::tr("Неверные входные данные");
    debug_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    return;
  }

  DataDesc desc;
  QString filename;
  if ( false == loadFieldInfo( req, &desc, &filename, &error ) ) {
    error_log << QObject::tr("Не удалось получить описание поля. Ошибка = %1").arg(error);
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    return;
  }

  QByteArray arr;
  if( false == getFieldFromFile( filename, &arr ) ) {
    debug_log << msglog::kFileReadFailed.arg(filename).arg(-1);
    resp->set_comment(msglog::kFileReadFailed.arg(filename).arg(-1).toStdString());
    resp->set_result(false);
    return;
  }
  obanal::TField field;
  if ( false == field.fromBuffer(&arr) ) {
    error = QString::fromUtf8("Не удалось загрузить поле из буфера");
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    return;
  }

  auto kcenters = global::kMeteoCenters();

  QString layername = layernameFromInfo(desc);

  auto protofield = resp->add_field();
  auto header = protofield->mutable_header();
  header->set_dt( field.getDate().toString(Qt::ISODate).toStdString() );
  header->set_unit( field.unit().toStdString() );
  header->set_name( field.name().toStdString() );
  header->set_level( field.getLevel() );
  header->set_level_type( field.getLevelType() );
  header->set_meteodescr( field.getDescr() );
  header->set_hour( field.getHour() );
  header->set_model( field.getModel() );
  header->set_center( field.getCenter() );
  header->set_center_name( kcenters[field.getCenter()].first.toStdString() );
  header->set_layer_name( layername.toStdString() );

  float step_lat;
  float step_lon;
  if ( false == field.stepFiDeg( &step_lat ) || false == field.stepLaDeg( &step_lon )  ) {
    error = QString::fromUtf8("Шаг сетки не известен для поля = %1")
            .arg( QString::fromStdString( desc.Utf8DebugString() ) );
    error_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    return;
  }
  //  step_lat = round(step_lat*1000)/1000.0;
  //  step_lon = round(step_lon*1000)/1000.0;
  header->set_step_lat( step_lat );
  header->set_step_lon( step_lon );
  header->mutable_begin()->CopyFrom( geopoint2pbgeopoint( field.begin() ) );
  header->mutable_end()->CopyFrom( geopoint2pbgeopoint( field.end() ) );
  map::GradientParams params( meteo::global::kIsoParamPath() );
  const map::proto::FieldColor& clr = params.protoParams( field.getDescr() );
  protofield->mutable_color()->CopyFrom(clr);
  for ( int i = 0, sz = field.kolData(); i < sz; ++i ) {
    if ( true == field.getMask(i) ) {
      auto value = protofield->add_value();
      value->set_value( field.getData(i) );
      value->set_index(i);
    }
  }
  if ( true == req->need_extremums() ) {
    QVector<fieldExtremum> extremums = field.calcExtremum();
    for(int i =0,j= extremums.size();i<j;++i ) {
      auto value = protofield->add_extremums();
      value->set_tip(extremums.at(i).tip);
      value->set_value(extremums.at(i).znach);
      value->mutable_pbcoord()->CopyFrom(geopoint2pbgeopoint(extremums.at(i).koord ) );
    }
  }
  resp->set_result(true);
}


bool MethodWrap::getFieldFromFile(const QString &file_id, QByteArray *barr)
{
  if ( nullptr == barr ) {
    return false;
  }

  GridFs gridfs;
  if ( false == gridfs.connect( meteo::global::mongodbConfObanal() ) ) {
    debug_log << gridfs.lastError();
    return false;
  }
  gridfs.use(dbname_, kFieldPrefix); //TODO

  auto gridfile = gridfs.findOneById(file_id);
  bool ok;
  *barr = gridfile.readAll(&ok);

  if (false == ok) {
    debug_log << QObject::tr("Ошибка при получении файла. Имя файла = %1 (%2)")
                 .arg(gridfile.fileName()).arg(file_id);
    return false;
  }

  return true;
}

bool MethodWrap::getAvailableForecastData(const DataRequest* req, DataDescResponse* resp)
{
  std::unique_ptr<Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) {
    resp->set_result(false);
    return false;
  }
  auto query = db->queryptrByName("aggregate_obanal_get_available_data");
  if(nullptr == query) {
    resp->set_result(false);
    return false;
  }
  if ( true == req->has_only_best() && true == req->only_best() ){
    query->arg("limit",1);
  }

  if (true == req->has_date_start() ) {
    query->argDt("start_dt",req->date_start());
  }
  if( true == req->has_date_end() ) {
    query->argDt("end_dt",req->date_end());
  }

  if (true == req->has_forecast_start() ) {
    query->argDt("start_fdt",req->forecast_start());
  }
  if( true == req->has_forecast_end() ) {
    query->argDt("end_fdt",req->forecast_end());
  }
  query->arg("level_type",req->type_level());
  query->arg("level",req->level());
  query->arg("hour",req->hour());
  query->arg("center",req->center());
  query->arg("only_last",req->only_last());
  if ( 0 < req->meteo_descr_size() ) {
    query->arg( "descr", req->meteo_descr() );
  }
  else if ( 0 < req->descrname_size() ) {
    auto tmd = TMeteoDescriptor::instance();
    QList<int> descrs;
    for ( auto descrname : req->descrname() ) {
      int descr = tmd->descriptor( descrname );
      descrs.append(descr);
    }
    query->arg( "descr", descrs );
  }

  if(true == req->has_model()){
    QList<int> models;
    models<<req->model();
    query->arg("model",models);
  }
  if(true == req->has_net_type()){
    QList<int> ntypes;
    ntypes<<req->net_type();
    query->arg("net_type",ntypes);
  }
  //debug_log<< query->query();
  //debug_log<<req->DebugString();
  QString err;


  bool queryRes = query->execInit(&err);
  if (false == queryRes) {
    //debug_log << err;
    resp->set_comment(err.toStdString());
    resp->set_result(false);
    return false;
  }
  int size = 0;

  //debug_log<< query->query();
  while ( true == query->next() ) {
    const DbiEntry& document = query->entry();
    ++size;
    DataDesc* descr           = resp->add_descr();
    const     QDateTime dt    = document.valueDt("dt");
    const     QDateTime dt1   = document.valueDt("forecast_start");
    const     QDateTime dt2   = document.valueDt("forecast_end");
    int       currentDescr    = document.valueInt32("descr");
    int       hour            = document.valueInt32("hour");
    int       model           = document.valueInt32("model");
    int       levelType       = document.valueInt32("level_type");
    int       level           = document.valueInt32("level");
    int       center          = document.valueInt32("center");
    int       netType         = document.valueInt32("net_type");
    int       countPoints     = document.valueInt32("count_points");
    QString   centerName      = document.valueString("centers.name");
    QString   levelTypesNames = document.valueString("level_types.full_name");
    QString   oid             = document.valueOid("_id");
    QString   file_id         = document.valueOid("fs_id");
    QString   centerShortName = document.valueString("centers.short_name");
    int priority = document.valueInt32("priority");
    auto descrname = TMeteoDescriptor::instance()->name( currentDescr ).toStdString();
    descr->set_descr_name(descrname);
    descr->set_date(dt.toString(Qt::ISODate).toStdString());
    descr->set_dt1(dt1.toString(Qt::ISODate).toStdString());
    descr->set_dt2(dt2.toString(Qt::ISODate).toStdString());
    descr->set_meteodescr(currentDescr);
    descr->set_hour(hour);
    descr->set_model(model);
    descr->set_level_type(levelType);
    descr->set_level(level);
    descr->set_center(center);
    descr->set_net_type(netType);
    descr->set_count_point(countPoints);
    descr->set_level_type_name(levelTypesNames.toStdString());
    descr->set_id(oid.toStdString());
    descr->set_file_id(file_id.toStdString());
    descr->set_priority(priority);
    if ( true == centerShortName.isEmpty() ) {
      descr->set_center_name(centerName.toStdString());
    }
    else {
      descr->set_center_name(centerShortName.toStdString());
    }
  }

  resp->set_comment(msglog::kServerAnswerOKArg.arg(size).toStdString());
  resp->set_result(true);
  //debug_log << resp->DebugString();
  return true;
}

bool MethodWrap::getAvailableDataForMeteosummary(const DataRequest *req, DataDescResponse *resp)
{
  auto errFunc = [resp](QString err)
  {
    debug_log << err;
    resp->set_comment(err.toStdString());
    resp->set_result(false);
    return false;
  };

  resp->set_result(false);

  if (( false == req->has_date_start() )
      || ( 0 == req->meteo_descr_size() )) {
    debug_log << req->Utf8DebugString();
    return errFunc(QObject::tr("Отсутствует часть обязательных параметров."));
  }
  std::unique_ptr<Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) {
    resp->set_result(false);
    return false;
  }

  auto query = db->queryptrByName("get_available_data_for_meteosummary");
  if(nullptr == query) {
    resp->set_result(false);
    return false;
  }
  query->argDt("fdt",req->date_start());

  QString err;
  bool queryRes = query->execInit(&err);
  if (false == queryRes) {
    //debug_log << err;
    resp->set_comment(err.toStdString());
    resp->set_result(false);
    return false;
  }
  int size = 0;
  while ( true == query->next() ) {
    const DbiEntry& document = query->entry();
    ++size;
    auto arr = document.valueArray("ids");
    while ( true == arr.next() ) {
      DataDesc* descr = resp->add_descr();
      descr->set_file_id(arr.valueOid().toStdString());
      descr->set_date(document.valueDt("forecast_start").toString(Qt::ISODate).toStdString());
    }
  }

  resp->set_comment(msglog::kServerAnswerOKArg.arg(size).toStdString());
  resp->set_result(true);
  return true;
}

bool MethodWrap::loadFieldsForPuanson(const DataRequest *req, ManyDataReply *resp)
{
  QString error;
  if ( ( nullptr == req )
       || ( nullptr == resp ) ) {
    error = QObject::tr("Неверные входные данные");
    debug_log << error;
    resp->set_comment( error.toStdString() );
    resp->set_result(false);
    return false;
  }

  std::unique_ptr<Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) {
    resp->set_result(false);
    return false;
  }

  auto query = db->queryptrByName("load_fields_for_puanson");
  if(nullptr == query) {
    resp->set_result(false);
    return false;
  }


  if ( true == req->has_date_start() ) {
    if ( true == req->has_interval() ) {
      QDateTime dt = QDateTime::fromString(QString::fromStdString(req->date_start()), Qt::ISODate);
      QDateTime dts = dt.addSecs(req->interval() * 3600);
      QDateTime dte = dt.addSecs(req->interval() * -3600);
      query->arg("start_dt",dts);
      query->arg("end_dt",dte);
    }
    else if ( true == req->has_date_end() ) {
      query->argDt("start_dt",req->date_start());
      query->argDt("end_dt",req->date_end());
    }
    else {
      query->argDt("start_dt",req->date_start());
      query->argDt("end_dt",req->date_start());
    }
  }

  query->arg("descr",req->meteo_descr());
  query->arg("level_type",req->type_level());
  query->arg("level",req->level());
  query->arg("hour",req->hour());
  query->arg("center",req->center());

  if (true == req->has_model()) {
    query->arg("model",QList<int>()<<req->model());
  }

  if ( true == req->has_net_type() ) {
    query->arg("net_type",QList<int>()<<req->net_type());
  }

  QString err;
  bool queryRes = query->execInit(&err);
  if (false == queryRes) {
    resp->set_comment(err.toStdString());
    resp->set_result(false);
    return false;
  }
  while ( true == query->next() ) {
    const DbiEntry& document = query->entry();
    DataReply* dr = resp->add_data();
    DataDesc* dateDescr = dr->mutable_descr();
    QDateTime dt = document.valueDt("forecast_start");
    int descr = document.valueInt32("descr");
    int hour = document.valueInt32("hour");
    int level_type = document.valueInt32("level_type");
    int level = document.valueInt32("level");
    int model = document.valueInt32("model");
    int center = document.valueInt32("center");
    int net_type = document.valueInt32("net_type");
    int count_points = document.valueInt32("count_points");
    QString name = document.valueString("centers.name");
    QString full_name = document.valueString("level_types.full_name");
    QString id = document.valueOid("_id");
    dateDescr->set_date(dt.toString(Qt::ISODate).toStdString());
    dateDescr->set_meteodescr(descr);
    dateDescr->set_hour(hour);
    dateDescr->set_model(model);
    dateDescr->set_center(center);
    dateDescr->set_net_type(net_type);
    dateDescr->set_count_point(count_points);
    dateDescr->set_level_type(level_type);
    dateDescr->set_level(level);
    dateDescr->set_center_name(name.toStdString());
    dateDescr->set_level_type_name(full_name.toStdString());
    dateDescr->set_id(id.toStdString());
    QByteArray barr;
    QString file_id = document.valueOid("fs_id");
    if ( true == getFieldFromFile(file_id, &barr) ) {
      dr->set_fielddata(barr.data(), barr.size());
    }
    else {
      debug_log << msglog::kFileReadFailed.arg(file_id).arg(-1);
      resp->set_comment(msglog::kFileReadFailed.arg(file_id).arg(-1).toStdString());
    }
  }
  resp->set_comment(msglog::kServerAnswerOK.arg("loadFieldsForPuanson").toStdString());
  resp->set_result(true);
  return true;
}

bool MethodWrap::GetFieldDescrPoID(const QString &oid, DataReply *resp)
{
  QString error;
  if (true == oid.isEmpty()) {
    error = QObject::tr("Ошибка. Индекс документа пуст");
    debug_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    return false;
  }

  std::unique_ptr<Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) {
    resp->set_result(false);
    return false;
  }

  auto query = db->queryptrByName("get_field_descr_po_id");
  if(nullptr == query) {
    resp->set_result(false);
    return false;
  }

  query->argOid("id",oid);
  QString err;
  bool queryRes = query->execInit(&err);
  if (false == queryRes) {
    resp->set_comment(err.toStdString());
    resp->set_result(false);
    return false;
  }

  if ( true == query->next() ) {
    const DbiEntry& document = query->entry();
    DataDesc *dataDescr = resp->mutable_descr();
    QDateTime dt = document.valueDt("dt");
    int descr = document.valueInt32("descr");
    int hour = document.valueInt32("hour");
    int model = document.valueInt32("model");
    int level_type = document.valueInt32("level_type");
    int level = document.valueInt32("level");
    int center = document.valueInt32("center");
    int net_type = document.valueInt32("net_type");
    int count_points = document.valueInt32("count_points");
    QString name = document.valueString("centers.name");
    QString full_name = document.valueString("level_types.full_name");
    QString file_id = document.valueOid("fs_id");
    dataDescr->set_date(dt.toString(Qt::ISODate).toStdString());
    dataDescr->set_meteodescr(descr);
    dataDescr->set_hour(hour);
    dataDescr->set_model(model);
    dataDescr->set_level_type(level_type);
    dataDescr->set_level(level);
    dataDescr->set_center(center);
    dataDescr->set_net_type(net_type);
    dataDescr->set_count_point(count_points);
    dataDescr->set_center_name(name.toStdString());
    dataDescr->set_level_type_name(full_name.toStdString());
    dataDescr->set_file_id(file_id.toStdString());

    resp->set_comment(msglog::kServerAnswerOKArg.arg(1).toStdString());
    resp->set_result(true);

    return true;
  }
  else {
    resp->set_comment(msglog::kServerAnswerError.arg("GetFieldDescrPoID").toStdString());
    resp->set_result(false);
    info_log << query->query();
    return false;
  }
}

bool MethodWrap::GetFieldPoID(const QString &oid, DataReply *resp)
{
  if( ( nullptr == resp)
      || ( false == GetFieldDescrPoID(oid,resp) )
      || ( false == resp->has_descr() )
      || ( false == resp->descr().has_file_id() ) ) {
    resp->set_comment(msglog::kServerAnswerError.arg("GetFieldPoID").toStdString());
    resp->set_result(false);
    return false;
  }

  QString error;
  QString fileId = QString::fromStdString( resp->descr().file_id() );
  QByteArray ba;
  if ( true == getFieldFromFile(fileId, &ba ) ) {
    resp->set_fielddata(ba.data(), ba.size());
  }
  else {
    error = msglog::kFileReadFailed.arg(fileId).arg(-1);
    debug_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    return false;
  }
  error = msglog::kServerAnswerOKArg.arg(1);
  resp->set_comment(error.toStdString());
  resp->set_result(true);
  return true;
}


bool MethodWrap::getOnePointData(const GridFile &file, const DataRequest *req,
                                 ValueDataReply *resp)
{
  if(0 == req->coords_size()) {
    return false;
  }
  QString error;
  obanal::TFieldMongo fd;
  if ( false == fd.fromGridFile(file) ) {
    error = QObject::tr("Не удалось обработать файл %1").arg(file.fileName());
    debug_log << error;
    resp->set_comment(error.toStdString());
    resp->set_result(false);
    return false;
  }
  for(const meteo::surf::Point& coord : req->coords()) {
    meteo::GeoPoint point(coord.fi(), coord.la());
    if (true == coord.has_height()) {
      point.setAlt(coord.height());
    }

    bool ok;
    float value = fd.pointValue(point, &ok);
    if(false == ok) {
      continue;
    }

    OnePointData *op = resp->add_data();
    op->set_value(value);
    DataDesc d;
    d.set_meteodescr(fd.getDescr());
    d.set_date(fd.getDate().toString(Qt::ISODate).toStdString());
    op->mutable_fdesc()->CopyFrom(d);

    float rval=0;
    int ii, jj;
    float step;
    fd.stepFi(&step);

    step = 1.0 / step;

    if(fd.getNumPoFila(point, &ii, &jj)) {
      if(fd.radKriv(ii, jj, &rval)) {
        op->set_rad_kriv(rval);
        op->set_dd_x(fd.dPx(ii, jj, step));
        op->set_dd_y(fd.dPy(ii, jj, step));
        op->set_dd2_xx(fd.dPxx(ii, jj, step));
        op->set_dd2_yy(fd.dPyy(ii, jj, step));
        op->set_dd2_xy(fd.dPxy(ii, jj, step, step));
      }
    }
    op->mutable_coord()->CopyFrom(coord);
  }
  return true;
}

bool MethodWrap::getFieldsIdForProfile(const DataRequest *req, DataDescResponse *resp)
{
  if( (false == req->has_date_start()
       && false == req->has_forecast_start() )
      || ( 0 == req->coords_size() ) ) {
    debug_log << msglog::kServiceRequestFailedErr;
    resp->set_comment(msglog::kServiceRequestFailedErr.toStdString());
    resp->set_result(false);
    return false;
  }
  DataRequest request = *req;
  /*!
* {
*   100 - Изобарическая поверхность
*   200 - Вся атмосфера
*   7   - Тропопауза
*   1   - Земная или водная поверхность
*   6   - Уровень максимального ветра
*   102 - Уровень моря
* }
*/
  request.clear_type_level();
  for(const int level_type : { 100, 200, 7, 1, 6, 102 }) {
    request.add_type_level(level_type);
  }

  /*!
* {
*   10009 - Геопотенциальная высота
*   10004 - Давление
*   10051 - Давление
*   12101 - Температура
*   12103 - Температура точки росы
*   12108 - Дефицит точки росы на уровнях
*   11003 - Ветер
*   11004 - Ветер
*   7004  - Давление
*   20010 - Количество всех наблюдающихся облаков CL или CM
*   20011 - Количество всех наблюдающихся облаков CL или CM
*   20012 - Род облаков
*   20013 - Высота основания самых низких видимых облаков над поверхностью земли(м)
* }
*/
  for(const int descr: { 10009, 10004, 10051, 12101, 12103, 12108, 11003, 11004, 7004, 20010, 20011, 20012, 20013}) {
    request.add_meteo_descr(descr);
  }

  for(const int level : { 1000, 925, 850, 700, 500, 400, 300, 250, 100, 70, 50, 30, 20, 10, 5, 0 }) {
    request.add_level(level);
  }
  resp->Clear();
  if ( true == getAvailableForecastData(&request, resp) ) {
    return true;
  }
  else {
    resp->set_comment(msglog::kServerAnswerError.arg("getFieldsIdForProfile").toStdString());
    resp->set_result(false);
    return false;
  }
}

bool MethodWrap::getTzonds(const DataRequest *req, surf::TZondValueReply *resp)
{
  DataDescResponse response;

  if ( false == getFieldsIdForProfile(req, &response) )
  {
    resp->set_comment(msglog::kServerAnswerError.arg("getTzonds").toStdString());
    resp->set_result(false);
    return false;
  }
  int field_count = response.descr_size();
  if(0 == field_count) {
    resp->set_comment(msglog::kServerAnswerError.arg("getTzonds").toStdString());
    resp->set_result(false);
    return false;
  }

  QMap<QString, QMap<int, zond::Zond>> dtZondsMap;
  QMap<int, zond::Zond> stzond; //номер точки - зонд

  for(int i = 0, isz = req->coords_size(); i < isz; ++i) {
    zond::Zond zond;
    zond.setCoord(GeoPoint(req->coords(i).fi(), req->coords(i).la()));
    if(req->coords(i).has_height()){
      zond.setCoord(GeoPoint(req->coords(i).fi(), req->coords(i).la(), req->coords(i).height()));
    }
    if(req->coords(i).has_index()){
      zond.setStIndex(QString::fromStdString(req->coords(i).index()));
    }
    stzond.insert(i, zond);
  }

  const QString kDbName = meteo::global::mongodbConfObanal().name();
  GridFs gridfs;

  if ( false == gridfs.connect( meteo::global::mongodbConfObanal()) ) {

    resp->set_comment(gridfs.lastError().toStdString());
    resp->set_result(false);
    return false;
  }
  gridfs.use(kDbName,kFieldPrefix);


  int model = response.descr(0).model();
  int hour  = response.descr(0).hour();
  for ( int i = 0; i < field_count; ++i) {
    if (( model!= response.descr(i).model() )
        ||( hour!= response.descr(i).hour() )){
      continue;
    }

    GridFile file = gridfs.findOneById(QString::fromStdString(response.descr(i).file_id()));
    ::obanal::TFieldMongo *fd = nullptr;
    fd = new ::obanal::TFieldMongo;
    if ( false == fd->fromGridFile(file) ) {
      QString error = QObject::tr("Не удалось обработать файл %1").arg(file.fileName());
      debug_log << error;
      resp->set_comment(error.toStdString());
      resp->set_result(false);
      delete fd;
      return false;
    }

    int hour = fd->getHour();

    QDateTime dtAnalyse = QDateTime::fromString( QString::fromUtf8( response.descr(i).date().c_str() ), Qt::ISODate );
    QDateTime dt = dtAnalyse;
    if(false == dt.isValid()) {
      debug_log <<msglog::kDateTimeError.arg(QString::fromUtf8(response.descr(i).date().c_str() ));
    }
    else {
      dt = dt.addSecs(hour);
    }
    QString strDt =  dt.toString(Qt::ISODate);
    // создаем зонд для каждой даты
    if(false == dtZondsMap.contains(strDt)) {
      dtZondsMap[strDt] = stzond;
    }

    int level = response.descr(i).level();
    int level_type = response.descr(i).level_type();
    int cur_descr = response.descr(i).meteodescr();
    if(meteodescr::kIsobarLevel != level_type) {
      level = 10000 * level_type;
    }

    for ( int cc = 0; cc < req->coords_size(); ++cc ) {
      meteo::GeoPoint point(req->coords(cc).fi(), req->coords(cc).la());
      bool ok;
      float value = fd->pointValue(point, &ok);
      if(false == ok) {
        continue;
      }
      zond::Zond &znd = dtZondsMap[strDt].operator[](cc);

      znd.setDateTime(dt);
      znd.setDateTimeAnalyse(dtAnalyse);

      znd.addCenterName(fd->getCenterName());
      switch(cur_descr) {
        case 10009:
          value *=10.;
        default:
          znd.setValue(level_type,level,cur_descr,value,control::NO_CONTROL);
        break;
        case 20010:
          znd.cloudP()->set(zond::N,value,control::NO_CONTROL);
        break;
        case 20011:
          znd.cloudP()->set(zond::Nh,value,control::NO_CONTROL);
        break;
        case 20012:
          znd.cloudP()->setC(value,control::NO_CONTROL);
        break;
        case 20013:
          znd.cloudP()->set(zond::h,value,control::NO_CONTROL);
        break;
      }
    }
    if(nullptr != fd) {
      delete fd;
      fd = nullptr;
    }
  }

  foreach(const QString& strDt, dtZondsMap.keys()) {
    foreach(zond::Zond z, dtZondsMap[strDt].values()) {
      meteo::surf::OneTZondValueOnStation* onezond = resp->add_data();
      z.restoreUrList();
      onezond->mutable_point()->set_fi(z.coord().fi());
      onezond->mutable_point()->set_la(z.coord().la());
      onezond->mutable_point()->set_height(z.coord().alt());
      onezond->mutable_point()->set_id(z.stIndex().toStdString());
      onezond->set_date(z.dateTime().toString(Qt::ISODate).toStdString());
      onezond->set_hour(z.dateTimeAnalyse().secsTo(z.dateTime()));
      QByteArray buf;
      z >>buf;
      onezond->set_meteodata(buf.data(),buf.size());
      //TODO модель, срок прогноза и проч.
    }
  }
  dtZondsMap.clear();
  resp->set_comment(msglog::kServerAnswerOK.arg("GetProfile").toStdString());
  resp->set_result(true);
  return true;
}

bool MethodWrap::getFieldsForAD(const DataRequest *req, ManyFieldsForDatesResponse* resp)
{
  QString error;
  std::unique_ptr<Dbi> db(meteo::global::dbObanal());
  if ( nullptr == db.get() ) {
    resp->set_result(false);
    return false;
  }

  auto query = db->queryptrByName("aggregate_get_fields_for_hour");
  if(nullptr == query) {
    resp->set_result(false);
    return false;
  }

  if (true == req->has_date_start()) {
    query->argDt("start_dt", req->date_start() );
  }
  if (true == req->has_date_end()) {
    query->argDt("end_dt",req->date_end() );
  }
  if (true == req->has_forecast_start()) {
    query->argDt("start_fdt", req->forecast_start() );
  }
  if (true == req->has_forecast_end()) {
    query->argDt("end_fdt",req->forecast_start() );
  }

  query->arg("hour",req->hour());
  query->arg("descr",req->meteo_descr());
  query->arg("level_type",req->type_level());
  query->arg("level",req->level());
  query->arg("center",req->center());

  QString err;
  bool queryRes = query->execInit(&err);
  if (false == queryRes) {
    resp->set_comment(err.toStdString());
    resp->set_result(false);
    return false;
  }

  while ( query->next() ) {
    const DbiEntry& doc = query->entry();
    //Document id = doc.valueDocument("_id");
    Array zonds = doc.valueArray("zonds");
    field::ManyFieldsForDate* date = resp->add_date();
    date->set_forecast_start(doc.valueDt("forecast_start").toString(Qt::ISODate).toStdString());
    while (zonds.next()) {
      Document zond;
      if (true == zonds.valueDocument(&zond)) {
        field::FieldsResponse* zondsResp = date->add_zonds();
        //Document idField = zond.valueDocument("id");
        //if( true == idField.isEmpty() ) continue;
        field::FieldId* fieldId = zondsResp->mutable_id();
        fieldId->set_center( zond.valueInt32("center") );
        fieldId->set_name( zond.valueString("name").toStdString() );
        fieldId->set_short_name(zond.valueString("short_name").toStdString());
        fieldId->set_priority(zond.valueInt32("priority"));
        fieldId->set_model( zond.valueInt32("model") );
        fieldId->set_hour( zond.valueInt32("hour") );
        zondsResp->set_count(zond.valueInt32("count"));
        Array levels = zond.valueArray("levels");
        while (levels.next()) {
          field::Level* fieldLevel = zondsResp->add_levels();
          Document level;
          if ( true == levels.valueDocument(&level) ) {
            fieldLevel->set_level( level.valueInt32("level") );
            Array descrs = level.valueArray("descrs");
            while (descrs.next()) {
              Document descrDoc;
              if (true == descrs.valueDocument(&descrDoc) && !descrDoc.isEmpty()) {
                field::Descr* descr = fieldLevel->add_descrs();
                //debug_log << descrDoc.jsonExtendedString();
                descr->set_descr(descrDoc.valueInt32("descr"));
                descr->set_fileid(descrDoc.valueOid("id").toStdString());
                descr->set_fs_id(descrDoc.valueOid("fs_id").toStdString());
                descr->set_level_type(descrDoc.valueInt32("level_type"));
              }
            }
          }
        }
      }
    }
  }
  resp->set_comment(msglog::kServerAnswerOK.arg("getFieldsForAD").toStdString());
  resp->set_result(true);
  return true;
}

}
}

