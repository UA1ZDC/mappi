#include <commons/obanal/func_obanal.h>
#include <commons/obanal/obanal_struct.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <qapplication.h>
#include <qtextcodec.h>
#include <qdebug.h>

#define PREDEL_PROREJ 0.5 

void checkField(const TFieldData& field)
{
  bool flag = false;
  int cntFlag = 0;
  int cntNotNull = 0;
  int cntGoodNull = 0;
  int cntBad = 0;
  int cntGood = 0;
  for (int i=0; i < field.data.size(); i++) {
    if (field.mask.at(i) != false && field.data.at(i) != 0) {
      debug_log << i << field.mask.at(i) << field.data.at(i);
      flag = true;
      cntGood++;
    }
    if (field.mask.at(i) != false) cntFlag++;
    if (field.data.at(i) != 0) cntNotNull++;
    if (field.mask.at(i) != false && field.data.at(i) == 0) cntGoodNull++;
    if (field.mask.at(i) == false && field.data.at(i) != 0) cntBad++;
  }
  if (!flag) {
    debug_log << "all null";
  }
  var(field.data.size());
  var(cntFlag);
  var(cntNotNull);
  var(cntGoodNull);
  var(cntBad); 
  var(cntGood); 
}


void fillGeoData_reg( GeoData* data)
{
  int x = 5, y = 3;
  float la[3][5] = { {10, 20, 20, 30, 30},
		     {10, 20, 20, 30, 30.5},
		     {10, 20, 20, 30, 30.6} };
  float fi[3][5] = { {30, 30, 30, 30, 30.02},
		     {20, 20, 20, 20, 20},
		     {10, 10, 10, 10, 10} };

  for (int j = 0; j < y; j++) {
    for (int i = 0; i < x; i++) {
      if (i == 1  && j == 0 ) {
	data->append(GeoData(MnMath::deg2rad(fi[j][i]), MnMath::deg2rad(la[j][i]), 8, TRUE_DATA));
      } else {
	data->append(GeoData(MnMath::deg2rad(fi[j][i]), MnMath::deg2rad(la[j][i]), 10, TRUE_DATA));
      }
    }
  }

  // for (int i=0; i < data->count(); i++) {
  //   debug_log << i << data->at(i).mask << data->at(i).point.la() << data->at(i).point.fi() 
  // 	      << MnMath::rad2deg(data->at(i).point.la()) << MnMath::rad2deg(data->at(i).point.fi()) << data->at(i).data;
  // }
}

void fillGeoData_haos( GeoData* data)
{
  int x = 5, y = 3;

  float la[3][5] = { {30, 20, 10, 20, 20},
		     {20, 10, 30, 20, 30},
		     {30.5, 10, 30.6, 30, 20} };

  float fi[3][5] = { {30.02, 30, 30, 20, 20},
		     {10, 20, 30, 10, 20},
		     {20, 10, 10, 10, 30} };


  for (int j = 0; j < y; j++) {
    for (int i = 0; i < x; i++) {
      data->append(GeoData(MnMath::deg2rad(fi[j][i]), MnMath::deg2rad(la[j][i]), 10, TRUE_DATA));
    }
  }

  // for (int i=0; i < data->count(); i++) {
  //   debug_log << i << data->at(i).mask << data->at(i).point.la() << data->at(i).point.fi() 
  // 	      << MnMath::rad2deg(data->at(i).point.la()) << MnMath::rad2deg(data->at(i).point.fi());
  // }
}

void fillGeoData_reg1( GeoData* data)
{
  int x = 5, y = 3;
  float la[3][5] = { {210, 270, 270, 30, 30},
		     {210, 270, 270, 30, 30.5},
		     {350, 340, 340, 30, 30.6} };
  float fi[3][5] = { {30, 30, 30, 30, 30.02},
		     {20, -20, -20, 20, 20},
		     {10, -10, -10, 10, 10} };

  for (int j = 0; j < y; j++) {
    for (int i = 0; i < x; i++) {
      data->append(GeoData(MnMath::deg2rad(fi[j][i]), MnMath::deg2rad(la[j][i]), 10, TRUE_DATA));
    }
  }

  // for (int i=0; i < data->count(); i++) {
  //   debug_log << i << data->at(i).mask << data->at(i).point.la() << data->at(i).point.fi() 
  // 	      << MnMath::rad2deg(data->at(i).point.la()) << MnMath::rad2deg(data->at(i).point.fi());
  // }
}


void testProrej()
{
  GeoData data_reg, data_haos, data_reg1;

  debug_log << "\nregular";
  fillGeoData_reg(&data_reg);
  MnObanal::prorej(&data_reg, PREDEL_PROREJ);

  for (int i=0; i < data_reg.count() -1; i++) {
    for (int j=i+1; j < data_reg.count(); j++) {
      if ( // (i != j) &&
	  (fabs(data_reg.at(i).point.la() - data_reg.at(j).point.la()) < 0.000001) && 
	  (fabs(data_reg.at(i).point.fi() - data_reg.at(j).point.fi()) < 0.000001)) {
	debug_log << "equal"<< i << j << data_reg.at(i).point.la() << data_reg.at(j).point.la() << data_reg.at(i).point.fi() << data_reg.at(j).point.fi() 
		  << data_reg.at(i).mask << data_reg.at(j).mask;
      } else //грубая оценка
      if ( // (i != j) &&
	  (MnMath::rad2deg(fabs(data_reg.at(i).point.la() - data_reg.at(j).point.la())) <= PREDEL_PROREJ) && 
	  (MnMath::rad2deg(fabs(data_reg.at(i).point.fi() - data_reg.at(j).point.fi())) <= PREDEL_PROREJ)) {
	debug_log << "near"<< i << j << data_reg.at(i).point.la() << data_reg.at(j).point.la() << data_reg.at(i).point.fi() << data_reg.at(j).point.fi() 
		  << data_reg.at(i).mask << data_reg.at(j).mask;
      }
    }
  }

  for (int i=0; i < data_reg.count(); i++) {
    debug_log << i << data_reg.at(i).point.la() << data_reg.at(i).point.fi() << data_reg.at(i).data << data_reg.at(i).mask;
  }

  PropertyChooseData prop;
  QDateTime dt = QDateTime::currentDateTime();
  prop.sdate = dt.date().toString("yyyy-MM-dd");
  prop.stime = dt.time().toString("hh:mm:ss");
  prop.scenter = 250;
  prop.slevel = 1;
  prop.svaluelevel = 0;
  prop.shour = 0;
  prop.smodel = MSISE1;
  prop.stypenet = STEP_100x100;

  TFieldData field_reg(prop);
  field_reg.fromHaos(data_reg, T_PARAM, false);
  checkField(field_reg);


  //---

  debug_log << "\nhaos";
  data_haos.clear();
  fillGeoData_haos(&data_haos);

  MnObanal::prorej(&data_haos, PREDEL_PROREJ);
  for (int i=0; i < data_haos.count(); i++) {
    debug_log << i << data_haos.at(i).point.la() << data_haos.at(i).point.fi() << data_reg.at(i).data << data_haos.at(i).mask;
  }

  TFieldData field_haos(prop);
  field_haos.fromHaos(data_haos, T_PARAM, false);
  //  checkField(field_haos);

  //---

  debug_log << "\nregular 1";
  data_reg1.clear();
  fillGeoData_reg1(&data_reg1);

  MnObanal::prorej(&data_reg1, PREDEL_PROREJ);
  for (int i=0; i < data_reg1.count(); i++) {
    debug_log << i << data_reg1.at(i).point.la() << data_reg1.at(i).point.fi() << data_reg.at(i).data << data_reg1.at(i).mask;
  }

  TFieldData field_reg1(prop);
  field_reg1.fromHaos(data_reg1, T_PARAM, false);
  //  checkField(field_reg1);
}

int main( int argc, char** argv )
{
  TAPPLICATION_NAME( "meteo" );

  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale());

  int flag = 0, opt;

  while ((opt = getopt(argc, argv, "1h")) != -1) {
    switch (opt) {
    case '1':
      flag = 1;
      break;
    // case '3':
    //   flag = 3;
    //   id = atoi(optarg);
    //   break;
    case 'h':
      info_log<<"Usage:"<<argv[0]<<"-1\n-1 test prorej\n";
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

  // QString fileName;
  // if (flag == 1) {
  //   if (optind >= argc) {
  //     error_log<<"Need point file name. \""<<argv[0]<<"-h\" for help";
  //     exit(-1);
  //   } else {
  //     fileName = argv[optind];
  //   }
  // }

  QApplication app(argc, argv, false);

  switch (flag) {
  case 1:
    testProrej();
    break;
  default: {}
  }
  

  return 0;
}

