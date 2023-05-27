#include <unistd.h>
#include <math.h>
#include <float.h>

#include <qcoreapplication.h>
#include <qtextcodec.h>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <meteo/commons/grib/decoder/tgribdecode.h>
#include <meteo/commons/grib/iface/tgribiface.h>
#include <meteo/commons/grib/parser/tgribformat.h>
#include <meteo/commons/grib/parser/tgridgrib.h>
#include <meteo/commons/grib/parser/tproductgrib.h>
#include <meteo/commons/grib/parser/trepresent.pb.h>
#include <meteo/commons/proto/tgribformat.pb.h>
#include <meteo/commons/proto/tgrid.pb.h>
#include <meteo/commons/proto/tproduct.pb.h>
#include <meteo/commons/proto/surface.pb.h>


using namespace google::protobuf;

void testFill();
QString db(unsigned value);
void testSaveGrib(const QString& file);


int main( int argc, char** argv )
{
  TAPPLICATION_NAME( "meteo" );
  //  meteo::settings::TMeteoSettings::instance()->load();

  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale());

  int flag = 0, opt;

  while ((opt = getopt(argc, argv, "1h")) != -1) {
    switch (opt) {
      case '1':
        flag = 1;
        break;
      case 'h':
        info_log<<"Usage:"<<argv[0]<<"-1 <fileName> - parse grib\n";
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
  if (flag == 1) {
    if (optind >= argc) {
      error_log<<"Need point file name with GRIB data. \""<<argv[0]<<"-h\" for help";
      exit(-1);
    } else {
      fileName = argv[optind];
    }
  }

  QCoreApplication app(argc, argv);

  switch (flag) {
    case 1:
      testSaveGrib(fileName);
      break;
  default: {}
  }

  return 0;
}


QString db(unsigned value)
{
  QString value_binary("\0");

  if ((value)!=1 && value != 0) { value_binary=db(value/2);}
  char a=value%2+48;
  return value_binary+=a;
}

void  testFill()
{

  int bits = 9;
  int mask[9] = {0x0, 0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F, 0x7F, 0xFF};
  const int size = 5;
  uchar src[] = {105, 214, 105, 150, 121};

  uint32_t cnt = size*8 / bits;

  for (int i=0; i<size; i++) {
    var(db(src[i]));
  }
  var(cnt);


  uint8_t shiftBit = 0;//уже считанных из байта
  uint32_t shiftByte = 0;

  for (uint i=0; i< cnt; i++) {
    uint32_t val = 0;
    uint8_t bitsCnt = 0;

    while (bitsCnt < bits) {
      uint8_t byte = src[shiftByte];
      if (shiftBit != 0) {
        byte &= mask[8-shiftBit];
      }
      uint8_t curbits = 0; //кол-во бит, к-е будем считывать из этого байта
      if (bitsCnt + (8-shiftBit) > bits) {
        curbits = bits-bitsCnt;
        byte >>= 8 - shiftBit  - curbits;
        debug_log<<8 - shiftBit - curbits;
        shiftBit += curbits;
      } else {
        curbits = 8 - shiftBit;
        shiftBit = 0;
        shiftByte++;
      }
      bitsCnt += curbits;

      val <<= curbits;
      val += byte;
      var(bitsCnt);
      var(shiftBit);
      var(shiftByte);
      var(byte);
      var(val);
    }
    printf("val=%d\n", val);
  }
}

void testSaveGrib(const QString& file)
{
  // debug_log<<"Read from file and save (only last) in db";
  TDecodeGrib dec;

  QFile f(file);
  f.open(QIODevice::ReadOnly);
  QByteArray baa = f.readAll();
  f.close();
  int res = dec.decode(baa);

  //int res = dec.decode(file);

  var(res);
  // return;

  const grib::TGribData* grib = dec.grib();
  if (!grib) {
    error_log << QObject::tr("Ошибка. Данные не сформированы");
    return;
  }

  if (grib->has_product1()) {
    debug_log << "bitmap" << grib->has_bitmap() << "number" << grib->product1().number();
  } else if (grib->has_product2()) {
    debug_log << "bitmap"   << grib->has_bitmap() << "discipline" << grib->discipline()
  	      << "category" << grib->product2().category()
  	      << "number"   << grib->product2().number();
  } else {
    debug_log << "bitmap" << grib->has_bitmap() << "no product";
  }
  //return;
  //--

   debug_log<<QObject::tr("Общие данные");
   //var(QString::fromStdString(grib->DebugString()));
  var(grib->center());
  var(grib->subcenter());

  const grib::TGridDefinition* grid = dec.gridDefinition();
  if (grid->definition != 0) {
    debug_log<<QObject::tr("Описание сетки:");
    var(grid->type);
    var(QString::fromStdString(grid->definition->DebugString()));
    TGrid::LatLon* latlon = static_cast<TGrid::LatLon*>(grid->definition);
    debug_log << "la" << latlon->la1() << latlon->la2()
        << "lo" << latlon->lo1() <<  latlon->lo2();
  }

  //  return;

  const grib::TProductDefinition* prod = dec.productDefinition();
  if (prod->definition != 0) {
    debug_log<<QObject::tr("Описание продукта:");
    var(prod->type);
    var(QString::fromStdString(prod->definition->DebugString()));
  } else {
    var(grib->product1().DebugString());
  }
  debug_log << "---------------";

  TGribIface* iface = new TGribIface(grib);
  QVector<float> data;
  bool ok = iface->fillData(&data);
  var(ok);
  if (data.isEmpty()) {
    error_log<<"No data";
    return;
  }

  float min = FLT_MAX;
  float max = -FLT_MAX;
  for (int64_t i=0; i< data.size(); i++) {
    //debug_log << "val=" << data[i] << "min=" << min << "max=" << max;
    if (data[i] > max /*&& data[i] < 99999999*/) max = data[i];
    if (data[i] < min) min = data[i];
  }
  debug_log << QObject::tr("Мин. и макс. значения в данных:");
  debug_log << "min=" << min << "max=" << max << "size" << data.size();
  return;
  {
    meteo::surf::GribData gdata;
    iface->fillData(gdata.mutable_value());
    double min = DBL_MAX;
    double max = -DBL_MAX;
    for (int64_t vi = 0; vi < gdata.value_size(); vi++) {
      debug_log << min << max << gdata.value(vi);
      if (gdata.value(vi) > max) max = gdata.value(vi);
      if (gdata.value(vi) < min) min = gdata.value(vi);
    }
    debug_log << "min=" << min << "max=" << max;
  }

  delete iface;

  return;

  // QFile fd(file);
  // fd.open(QIODevice::ReadOnly);
  // QByteArray ba = fd.readAll();
  // fd.close();

  // err = dec.decode(ba);
  // var(err);

  // const grib::TGribData* gribba = dec.grib();
  // if (!gribba) {
  //   error_log << QObject::tr("Ошибка. Данные не сформированы");
  //   return;
  // }


  // debug_log<<QObject::tr("Общие данные");
  // var(QString::fromStdString(gribba->DebugString()));

  // grid = dec.gridDefinition();
  // if (grid->definition != 0) {
  //   debug_log<<QObject::tr("Описание сетки:");
  //   var(grid->type);
  //   var(QString::fromStdString(grid->definition->DebugString()));
  // }

  // prod = dec.productDefinition();
  // if (prod->definition != 0) {
  //   debug_log<<QObject::tr("Описание продукта:");
  //   var(prod->type);
  //   var(QString::fromStdString(prod->definition->DebugString()));
  // }
}


