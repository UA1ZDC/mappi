#include <meteo/commons/bufr/tbufr.h>

//#include <sql/nspgbase/ns_pgbase.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/complexmeteo.h>
#include <commons/meteo_data/tmeteodescr.h>


#include <meteo/commons/global/global.h>
//#include <meteo/novost/settings/tmeteosettings.h>

#include <qcoreapplication.h>
#include <qtextcodec.h>

#include <unistd.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qdebug.h>


//CHANGE_OPERATOR chop;

void testDecodeBufr(const QString& file)
{
  TBufr bufr;
  bufr.setCodec(QTextCodec::codecForName("Windows-1251"));
  int res =  bufr.decode(file);
  var(res);

  
  // debug_log;
  // debug_log;
  // return;
 
  //это только для последнего в файле:
  QList<TMeteoData> data;
  
  meteo::bufr::TransfType bt;
  bufr.fill(&data, &bt);
  var(data.count());
  // bufr.removeBad(data);
  // var(data.count());

  //return;

  printvar(data.size());

  if (data.size() == 0) return;

   QDateTime dt = QDateTime::currentDateTime();
  
  for (int idx=0; idx < data.size(); idx++) {
    printvar(idx);
    TMeteoData one = data.at(idx);
    //one.setDateTime(dt);
    debug_log << "printdata";
    one.printData();
    QDateTime dt = TMeteoDescriptor::instance()->dateTime(data.at(idx));
    var(dt);
    ComplexMeteo cm;
    cm.fill(one);
    debug_log << "complex";
    cm.printData();
    
    // QHashIterator<int, TMeteoParam> it(one.data());
    // while (it.hasNext()) {
    //   it.next();
    //   if (it.value().quality() < TControl::MISTAKEN) {
    // 	qDebug() << it.key() << ": " << it.value();
    //   }
    // }
    debug_log;
  }

}

struct MeteoInterpret {
  QString name;
  int idx;
  QString value;
  QString description;
};
  

// void readMsg(int id, NS_PGBase* db, QList<MeteoInterpret>* msg)
// {
//   Q_UNUSED(id);
//   Q_UNUSED(db);
//   Q_UNUSED(msg);

  // TAlphaNumDb nDb(db);

  // var(id);

  // TMeteoData meteo;
  // bool ok = nDb.read(&meteo, id);
  // var(ok);
  
  // // debug_log<<"Print:";
  // // debug_log << "dt=" << TMeteoDescriptor::instance()->dateTime(meteo);
  // // printvar(meteo.count());
  // // meteo.printData();

  // NS_PGBase* ptkppDb = meteo::global::dbTelegram();

  // const QHash<descr_t, QHash<int, TMeteoParam> >& data = meteo.data();
  // QHashIterator<descr_t, QHash<int, TMeteoParam> > it(data);
  // while (it.hasNext()) {
  //   it.next();

  //   descr_t descr = it.key();
  //   QHashIterator<int,TMeteoParam> itval(it.value());
  //   while (itval.hasNext()) {
  //     itval.next();
      
  //     TMeteoParam param = itval.value();
  //     if (param.isInvalid()) continue;

  //     MeteoInterpret mi;
  //     mi.name = TMeteoDescriptor::instance()->name(descr);
  //     mi.idx = itval.key();

  //     if (TMeteoDescriptor::instance()->property(descr).units == "NO") {
  // 	QString query = QString("SELECT get_bufr_code('%1', %2)").
  // 	  arg(QString::number(descr % 100000).rightJustified(6, '0')).arg(param.code());

  // 	if (!ptkppDb->ExecQuery(query)) {
  // 	  error_log << QObject::tr("Ошибка получения определения значения") << db->ErrorMessage();
  // 	  continue;
  // 	}
  // 	mi.value = ptkppDb->GetValue(0,0);
  // 	if (!mi.value.isEmpty()) {
  // 	  mi.description = TMeteoDescriptor::instance()->property(descr).description;
  // 	} else {
  // 	  mi.value = QString::number(param.value());
  // 	  mi.description = TMeteoDescriptor::instance()->property(descr).description;
  // 	}
  //     } else {
  // 	mi.value = QString::number(param.value());
  // 	mi.description = TMeteoDescriptor::instance()->property(descr).description;
  //     }
      
  //     msg->append(mi);
  //   }
  // }

  //}


// void meteoDataInterpretation(int idPtkpp, QList<QList<MeteoInterpret> >* miList)
// {
//   trc;
//   //  meteo::settings::TMeteoSettings::instance()->load();
//   //NS_PGBase* db = meteo::global::dbMeteo();
//   NS_PGBase* db = new NS_PGBase("astrase","db_meteo","postgres","");

//   QString query = QString("SELECT service_id FROM codes_service WHERE ptkpp_id='%1'").arg(idPtkpp);
//   if (!db->ExecQuery(query)) {
//     error_log << QObject::tr("Ошибка получения id") << db->ErrorMessage();
//     return;
//   }
  
//   bool ok;
//   QList<uint64_t> id;
//   var(db->RecordCount());
//   for (int i = 0; i < db->RecordCount(); i++) {
//     uint64_t ci = db->GetValue(i,0).toULongLong(&ok);  
//     if (ok) {
//       id.append(ci);
//     }
//   }

//   for (int i = 0; i < id.count(); i++) {
//     QList<MeteoInterpret> mi;
//     readMsg(id.at(i), db, &mi);
//     miList->append(mi);
//   }

  
//   delete db;
// }

// void testMsgText(uint64_t id)
// {
//   QList<QList<MeteoInterpret> > miList;

//   meteoDataInterpretation(id, &miList);

//    for (int i = 0; i < miList.count(); i++) {
//     debug_log << QObject::tr("Сводка %1(%2)").arg(i).arg(miList.count());

//     QList<MeteoInterpret> mi = miList.at(i);
//     for (int num = 0; num < mi.count(); num++) {
//       debug_log << mi.at(num).name  << "\t" << mi.at(num).idx << "\t" 
// 		<< mi.at(num).value << "\t" << mi.at(num).description;
//     }
//   }

// }

void testLoadTables()
{
  BufrMetaData bmd;
  
  bmd.load(2, 255, 255, 10, 255);
  bmd.load(4, 216, 0, 22, 0);
}

int main ( int argc, char *argv[] ) 
{
  TAPPLICATION_NAME( "meteo" );
  //QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());

  int flag = 0, opt;
  //  uint id = 0;

  while ((opt = getopt(argc, argv, "12:h")) != -1) {
    switch (opt) {
    case '1':
      flag = 1;
      break;
    case '2':
      flag = 2;
      //id = atoi(optarg);
      break;
    case 'h':
      info_log << "Usage:" << argv[0] << "-1|-2 <id>\n-1 <fileName> - parse bufr\n-2 <id> - info about text telegram\n";
      // info_log<<"Usage:"<<argv[0]<<"-1|-3 <fileName>\n-1 - save grib in db\n-3 <id> - parse grib from db with id=<id>";
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
      error_log<<"Need point file name. \""<<argv[0]<<"-h\" for help";
      exit(-1);
    } else {
      fileName = argv[optind];
    }
  }

  QCoreApplication app(argc, argv);
  //NS_PGBase* db = new NS_PGBase("127.0.0.1","meteo","postgres","");


  switch (flag) {
  case 1:
    testDecodeBufr(fileName);
    break;
  case 2:
    //testMsgText(id);
    break;
  default: {}
  }
  
  //  delete db;

  return 0;
}
