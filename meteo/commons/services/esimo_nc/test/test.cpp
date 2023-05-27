#include <meteo/commons/services/esimo_nc/convert_nc.h>

#include <commons/obanal/tfield.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>


#include <qcoreapplication.h>
#include <qdir.h>

#include <unistd.h>
#include <stdlib.h>

//метео: или по имени папки, или задав тип данных
// ./test -n RU_FERHRI_04 -f ./esimo/FEHRI/RU_FERHRI_04/RU_FERHRI_04_1.nc
// ./test -t 1 -f ./esimo/FEHRI/RU_FERHRI_04/RU_FERHRI_04_1.nc

//сетка: или по имени папки, или задав тип и сетку
// ./test -n RU_FERHRI_10 -f ./esimo/FEHRI/RU_FERHRI_10/RU_FERHRI_10_1.nc
// ./test -g -f ./esimo/FEHRI/RU_FERHRI_10/RU_FERHRI_10_1.nc

//для всех nc в папке (-t, или -g, или -n)
// ./test -t 1 -d ./esimo/FEHRI/RU_FERHRI_10



void test_read_esimodir(const QString& path, int type, bool grid, NetType net, const QString& esimoId)
{
  bool ok = false;
  
  meteo::surf::DataType datatype = meteo::surf::DataType(type);
  meteo::EsimoType esimonc = meteo::kEsMeteo;
  NetType nettype = net;
  
  meteo::ConvertNc nc;
  if (!esimoId.isEmpty()) {
    ok = nc.esimoid2datatype(esimoId, &datatype, &esimonc, &nettype);
    if (!ok) {
      error_log << QObject::tr("Ошибка преобразования id в тип данных");
      return;
    }
  } else if (grid) {
    esimonc = meteo::kEsGrid;
  }

  QDir esimodir(path);
  ok = nc.assimilateDir(esimodir, datatype, esimonc, nettype);
  
  var(ok);
}


void test_read_esimo(const QString& file, int type, bool grid, NetType net, const QString& esimoId)
{
  bool ok = false;
  
  meteo::surf::DataType datatype = meteo::surf::DataType(type);
  meteo::EsimoType esimonc = meteo::kEsMeteo;
  NetType nettype = net;
 
  meteo::ConvertNc nc;
  if (!esimoId.isEmpty()) {
    ok = nc.esimoid2datatype(esimoId, &datatype, &esimonc, &nettype);
    if (!ok) {
      error_log << QObject::tr("Ошибка преобразования id в тип данных");
      return;
    }
  } else if (grid) {
    esimonc = meteo::kEsGrid;
  }

  debug_log << "datatype" << datatype << "nc" << esimonc << "net" << nettype;

  switch (esimonc) {
  case meteo::kEsMeteo:
    //метеодата
    ok = nc.assimilateFile(file, datatype);
    break;
  case meteo::kEsGrid:
    //поля
    ok = nc.assimilateGridFile(file, nettype);
    break;
  default:
    break;
  }
  
  var(ok);
}

//----

void help(const QString& progName)
{
  info_log << QObject::tr("\nИспользование:") << progName << "-f <file>\n"
    "-f <file>\t Файл со данными\n"
    "-t <datatype>\t Тип данных (для метеоданных)\n"
    "-g\t Данные в узлах сетки\n"
    "-s <step>\t Шаг сетки в градусах (должен соответствовать типам сетки NetType). Если не указан, будет определен по координатам в nc файле"
    "-i\t objectSystemId из xml есимо (название папки). Пример: 'RU_FERHRI_04'"
    "-d <dirname>\t Папка с папками есимо";
}

int main (int argc, char* argv[])
{
  TAPPLICATION_NAME("meteo");
  //  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

  //  ::meteo::gGlobalObj(new ::meteo::SborGlobal);
  QCoreApplication app(argc, argv);

  QString file;
  int opt;

  bool grid = false; //метеодата или сетка
  int datatype = -1;  //тип данных
  QString esimoId;   //objectSystemId из xml есимо (название папки)
  QString dirname;
  NetType net = NO_TYPE;
  float step = 0;

  while ((opt = getopt(argc, argv, "f:t:gn:d:s:h")) != -1) {
    switch (opt) {
    case 'f':
      file = optarg;
      break;
    case 't':
      datatype = atoi(optarg);
      break;
    case 'g':
      grid = true;
      break;
    case 'n':
      esimoId = optarg;
      break;
    case 'd':
      dirname = optarg;
      break;
    case 's':
      step = QString(optarg).toFloat();
      net = obanal::netTypeFromStep(step * 1000);      
      break;
    case 'h':
      help(argv[0]);
      exit(0);
      break;
    default: /* '?' */
      error_log<<"Option"<<opt<<"not realized";
      exit(-1);
    }
  }

  if (file.isEmpty() && dirname.isEmpty()) {
    error_log << "Необходимо указать файл или директорию\n";
    help(argv[0]);
    exit(-1);
  }

  //TODO check opt

  debug_log << QObject::tr("Сетка:") << obanal::netTypetoStr(net);
  
  ::meteo::gSettings(::meteo::global::Settings::instance());
  meteo::global::Settings::instance()->load();


  if (!dirname.isEmpty()) {
    test_read_esimodir(dirname, datatype, grid, net, esimoId);
  } else {
    //test_convert(file);
    test_read_esimo(file, datatype, grid, net, esimoId);
  }
  

  return 0;// app.exec();
}
