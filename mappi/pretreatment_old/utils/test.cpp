#include <qcoreapplication.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <mappi/global/streamheader.h>
#include <mappi/pretreatment/formats/channel.h>
#include <mappi/pretreatment/images/filters.h>
#include <mappi/pretreatment/formats/stream_utility.h>

#include <unistd.h>

#include <qfile.h>
#include <qdatastream.h>
#include <qimage.h>
#include <qdebug.h>

#include <gdal/gdal_priv.h>
#include <gdal/gdal_alg.h>
#include <gdal/ogr_spatialref.h>
#include <gdal/ogr_geometry.h>
#include <gdal/ogr_featurestyle.h>


//возвращает двоичное представление числа (для отладки)
QString db(uint64_t value)
{
  QString value_binary("\0");
  
  if ((value)!=1 && value != 0) { value_binary=db(value/2);}
  char a=value%2+48;
  return value_binary+=a;
}

void testBitToImg(const QString& fileName, int w, int skip)
{
  trc;
  QFile file(fileName);
  file.open(QIODevice::ReadOnly);
  file.seek(qint64(1)*180*1024*1024 /*+ 2418*192*8*/);
  QByteArray datasrc = file.read(100*1024*1024);//file.readAll(); 
  file.close();

  int off = 0;
  
  int cntParts = 10; //10

  //388 мсу
  //  int skip = 652; или 650 кмсс //пропустить бит в начале
  var(skip);
   
  for (int kk = 0; kk < 1 /*cntParts*/; kk++) {
    QByteArray data = datasrc.mid(off, datasrc.size() / cntParts);
    off += datasrc.size() / cntParts;
    
    int width = w * 8;
    uint64_t height = data.size() / w;
    
    debug_log << data.size() << data.size()*8 << width << height;
    
    uchar* res = new uchar[uint64_t(data.size()*8) - skip/8];
    uint64_t cnt = 0;
    
    for (int64_t idx = 0; idx < data.size(); idx ++) {
      for (int64_t bits = 0; bits < 8; bits++) {
	if (cnt >= (uint64_t)data.size()*8) {
	  error_log << QObject::tr("Ошибка размера") << cnt << data.size()*8 << idx << bits;
	  return;
	}

	if (skip > 0) {
	  skip--;
	  continue;
	}
	
	uchar bit = ((uchar)(data.at(idx)) >> (7-bits)) & 0x1;
	// if (bits == 1 || bits == 3 || bits == 5 || bits == 7) {
	 //	  bit = (~bit) & 0x1;
	// } else {
	// //   bit = (~bit) & 0x1;
	// }
	
	if (bit == 1) {
	  res[cnt] = 1;
	} else {
	  res[cnt] = 0;
	}
	// if (idx > data.size() - 5) {
	// 	debug_log << idx <<  db((uchar)data.at(idx)) << bit << cnt;
	// }
	
	cnt++;
      }
      // debug_log << idx << idx /8 << ll << data.size() / 8;
    }
    
    // QFile rfile("tmp.dat");
    // rfile.open(QIODevice::WriteOnly);
    // rfile.write(res, data.size()*8);
    // rfile.close();

    int cnt1 = 0;
    for (int idx = 0; idx < 100; idx++) {
      QString str[8];
      bool ok;
      for (int byte = 0; byte < 8*8; byte++) {
	str[byte/8] += QString::number((res[cnt1 + byte])&0x1);	
      }
      qDebug() << str[0] << str[1] << str[2] << str[3] << str[4] << str[5] << str[6] << str[7]
	       << str[0].toULong(&ok, 2) << str[1].toULong(&ok, 2) << str[2].toULong(&ok, 2) << str[3].toULong(&ok, 2) 
	       << str[4].toULong(&ok, 2) << str[5].toULong(&ok, 2) << str[6].toULong(&ok, 2) << str[7].toULong(&ok, 2)
	       << "\t" << hex << str[0].toULong(&ok, 2) << str[1].toULong(&ok, 2) << str[2].toULong(&ok, 2) << str[3].toULong(&ok, 2);
      cnt1 += 192*8;
    }
    
    QImage imqt(res, width, height, width, QImage::Format_Indexed8 /*QImage::Format_Grayscale8*/);
    imqt.setColorCount(2);
    QVector<QRgb> palette(2);
    palette[0] = QRgb(0x0);
    palette[1] = QRgb(0x00ff00);
    
    imqt.setColorTable(palette);
    imqt.save(fileName + QString::number(kk) + ".bit.bmp", "BMP");
    debug_log << "write" << fileName + QString::number(kk) +  ".bit.bmp";
    
    delete[] res;
    
  }
}

void testByteToImg(const QString& fileName, int w/*, int skip*/)
{
  trc;
  QFile file(fileName);
  file.open(QIODevice::ReadOnly);
  // file.seek(qint64(1)*180*1024*1024 /*+ 2418*192*8*/);
  QByteArray datasrc = file.read(500*1024*1024); 
  file.close();
  
  //data.resize(data.size()/10);
  int off = 0;

  int cntParts = 50;
   
  for (int kk = 0; kk < cntParts; kk++) {
    QByteArray data = datasrc.mid(off, datasrc.size() / cntParts);
    off += datasrc.size() / cntParts;
    
    
    int width = w;
    uint height = data.size() / w;

    debug_log << data.size() << width << height;
    
    uchar* res = new uchar[data.size()];
    uint cnt = 0;
  
    for (int64_t idx = 0; idx < data.size(); idx ++) {
      if (cnt >= (uint)data.size()) {
	error_log << QObject::tr("Ошибка размера") << cnt << data.size() << idx;
	return;
      }
      
      res[cnt] = (uchar)data.at(idx);
      cnt++;
    }

  
    QImage imqt(res, width, height, width, QImage::Format_Indexed8 /*QImage::Format_Grayscale8*/);
    imqt.setColorCount(256);
    
    QVector<QRgb> palette(256);
    for (uint i=0; i< 256; i++) {
      palette[i] = QRgb((i<<16)+ (i<<8) + i);
    }
    
    imqt.setColorTable(palette);
    imqt.save(fileName + "." + QString::number(kk) + ".byte.bmp", "BMP");
    debug_log << "write" << fileName + "." + QString::number(kk) + ".byte.bmp";

    delete[] res;
  }

 
  
}


void ciftoImg(const QString& fileName, int cifWidth, int skip)
{
  trc;
  QFile file(fileName);
  file.open(QIODevice::ReadOnly);
  file.seek(qint64(1)*180*1024*1024 + 2418*192*8);
  QByteArray cif = file.read(100*1024*1024); //readAll(); 
  file.close();

  //  cif.resize(cif.size()/10);

  var(skip);
  
  uint offset = skip;
  int wordSize = 10;
  int remain;
  QByteArray raw;

  bool ok = meteo::cif2raw(cif, offset, &raw, &remain, wordSize, false);
  var(ok);
  debug_log << "cifSize" << (uint64_t)cif.size() << "rawSize" << (uint64_t)raw.size();

  cif.clear();
  
  file.setFileName(fileName + ".raw");
  file.open(QIODevice::WriteOnly);
  file.write(raw); 
  file.close();
  debug_log << "write" << fileName + ".raw";

  uint64_t rawWidth = (cifWidth*8 /*- offset*/)/wordSize * 2;

  //raw = raw.mid(1744*2, raw.size() - 1744*4);
    
  // testBitToImg(fileName + ".raw", rawWidth);
  int imgWidth = (rawWidth)/2;
  var(rawWidth);
  var(imgWidth);
  var(raw.size() / imgWidth / 2);

  debug_log << "set channel";
  mappi::po::Channel<short> ch;
  ch.setup(raw.size() / imgWidth / 2, imgWidth, 0);
  ch.setLimits(0, 0x3ff);
  for (int idx = 0, num = 0; idx + 1 < raw.size(); idx += 2, num++) {
    ch.set(num, ((uchar)raw.at(idx) << 8) + (uchar)raw.at(idx+1));
  }

  raw.clear();
  
  ch.saveImage(fileName);
  
}


void filter(const QString& /*fileName*/)
{
  error_log << "commeted func";
//   QVector<short> shdata;
//   meteo::global::PoHeader header;
  
//   QFile file(fileName);
//   file.open(QIODevice::ReadOnly);
//   QDataStream ds(&file);
//   ds >> header;
//   ds >> shdata;
//   file.close();
 
//   uint width = 3248;
//   uint height = 3400;
//   //  int size = width*height;
  
//   mappi::po::Channel<short> ch;
//   ch.setup(height, width, 0);
//   ch.setLimits(0, 0x3ff);

//   QVector<short> res(shdata.size());
//   meteo::median3<short>(height, width, shdata, &res);

  
//   ch.setData(res, height, width); 
//   ch.saveImage(fileName + "filtr");
}



    bool testsaveGeoTifGdal()
    {
      trc;
      CPLErr er;
      Q_UNUSED(er);
      
      GDALAllRegister();
      CPLPushErrorHandler(CPLQuietErrorHandler);
      GDALDataset *gds = nullptr;
      GDALDriver *driver = nullptr;

      //CPLSetConfigOption( "GDAL_TIFF_INTERNAL_MASK", "YES" );
      
      //driver = GetGDALDriverManager()->GetDriverByName("COG");
      driver = GetGDALDriverManager()->GetDriverByName("GTiff");
      if (nullptr == driver) {
	debug_log << CPLGetLastErrorMsg();
	return false;
      }
      
           
      GDALDataset *poSrcDS =
	(GDALDataset *) GDALOpen( "/home/ma/satdata/pretreatment/20180302_0705/201803020705_201803020717_NOAA18_1_1.tiff", GA_ReadOnly );

      if (nullptr == poSrcDS) {
	debug_log << "create err" << CPLGetLastErrorMsg();
	return false;
      }

      
      char **papszOptions = NULL;
      papszOptions = CSLSetNameValue( papszOptions, "TILED", "YES" );
      papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "JPEG" );
      papszOptions = CSLSetNameValue( papszOptions, "BLOCKXSIZE", "512");
      papszOptions = CSLSetNameValue( papszOptions, "BLOCKYSIZE", "512");
      papszOptions = CSLSetNameValue( papszOptions, "GDAL_TIFF_OVR_BLOCKSIZE", "512");
      

      gds = driver->CreateCopy("/home/ma/satdata/pretreatment/20180302_0705/test.tiff",
			       poSrcDS, false, papszOptions, NULL, NULL);
      if (nullptr == gds) {
	debug_log << "create err" << CPLGetLastErrorMsg();
	return false;
      }
  

      debug_log << gds->GetRasterXSize();
      // int nBandIn = 1;
      
      // CPLErr er = gds->GetRasterBand(nBandIn)->RasterIO(GF_Write,0, 0, poSrcDS->GetRasterXSize(),
      // 							geoRows, (void*)resImg.data(),
      // 							geoCols, geoRows, GDT_Byte, 0, 0);
      // if(0 != er) debug_log << "RasterIO" << "return" << er;
      
      // geotiffDataset->GetRasterBand(nBandIn)->SetNoDataValue(0);
      
      // er = geotiffDataset->CreateMaskBand(GMF_NODATA |GMF_PER_DATASET);
      //if(0 != er) debug_log << "BuildOverviews" << "return" << er << CPLGetLastErrorMsg();
      
      //geotiffDataset->CreateMaskBand(GMF_NODATA | GMF_PER_DATASET);
      //      geotiffDataset->GetRasterBand(1)->SetColorInterpretation(GCI_AlphaBand);      
      
      int anOverviewList[2] = { 2, 4 };
      
      // er = gds->GetRasterBand(nBandIn)->BuildOverviews( "NEAREST", 2, anOverviewList, 
      // 							GDALDummyProgress, nullptr );
      // if(0 != er) debug_log << "BuildOverviews" << "return" << er << CPLGetLastErrorMsg();
      er = gds->BuildOverviews( "NEAREST", 2, anOverviewList, 0, nullptr,
			   GDALDummyProgress, nullptr );
      if(0 != er) debug_log << "BuildOverviews" << "return" << er << CPLGetLastErrorMsg();

      
      GDALClose(gds) ;
      GDALDestroyDriverManager();
    
      return true;

    }


// void testTiff(const QString& fileName)
// {
//   QVector<short> shdata;
//   meteo::global::PoHeader header;
  
//   QFile file(fileName);
//   file.open(QIODevice::ReadOnly);
//   QDataStream ds(&file);
//   ds >> header;
//   ds >> shdata;
//   file.close();
 
//   uint width = 3248;
//   uint height = 3400;
//   //  int size = width*height;
  
//   mappi::po::Channel<short> ch;
//   ch.setup(height, width, 0);
//   ch.setLimits(0, 0x3ff);

//   ch.setData(res, height, width); 
//   ch.saveImage(fileName + "filtr");
  
// }



int main(int argc, char* argv[])
{
  TAPPLICATION_NAME("test");


  testsaveGeoTifGdal();
  return 0;

  
  int flag = 0, opt;
  int width;
  int skip = 0;
  
  while ((opt = getopt(argc, argv, "1:2:3:4s:h")) != -1) {
    switch (opt) {
    case '1':
      flag = 1;
      sscanf(optarg,"%d", &width);
      break;
    case '2':
      flag = 2;
      sscanf(optarg,"%d", &width);
      break;
    case '3':
      flag = 3;
      sscanf(optarg,"%d", &width);
      break;
    case 's':
      sscanf(optarg,"%d", &skip);
      break;
    case '4':
      flag = 4;
      break;
    case 'h':
      info_log<<"Usage:"<<argv[0]<<"-1|-2|-3|-4 [-s <skip_bits>] <fileName>\n"
	"-1 <width> - data to bin img\n"
	"-2 <width> - data to byte img\n"
	"-3 <width> - from cif to img\n"
	"-s <skip_bits>\n"
	"-4 - filter"
	"fileName - raw data\n";
      exit(0);
      break;
    default: /* '?' */
      error_log<<"Option"<<opt<<"not realized";
      exit(-1);
    }
  }

  if (flag == 0) {
    error_log<<"Need select test with option. \""<<argv[0]<<"-h\" for help";
    exit(-1);
  }

  QString fileName;
  if (flag == 1 || flag == 2 || flag == 3 || flag == 4) {
    if (optind >= argc) {
      error_log << "Need point file name. \"" << argv[0] << "-h\" for help";
      exit(-1);
    } else {
      fileName = argv[optind];
    }
  }

  QCoreApplication app(argc, argv);

  switch (flag) {
  case 1:
    testBitToImg(fileName, width, skip);
    break;

  case 2:
    testByteToImg(fileName, width/*, skip*/);
    break;

  case 3:
    ciftoImg(fileName, width, skip);
    break;

  case 4:
    filter(fileName);
    break;
    
    
  default: {}
  }

  return 0;
}
