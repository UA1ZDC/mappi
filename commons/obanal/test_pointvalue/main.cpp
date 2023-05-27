#include <qfile.h>
#include <qelapsedtimer.h>
#include <qcoreapplication.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/targ.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/app/paths.h>

#include <commons/obanal/tfield.h>


#define HELP(opt, text)  (QStringList() << opt << QObject::tr(text))

const QStringList kHelpOpt  = QStringList() << "h" << "help";
const QStringList kPointOpt = QStringList() << "p" << "point-value";

const HelpFormatter kHelp = HelpFormatter()
  << HELP(kHelpOpt, "Эта справка")
  << HELP(kPointOpt, "Запуск теста для pointValue (по умолчанию).")
       ;

int main(int argc, char** argv)
{
  QCoreApplication app(argc,argv);

  TArg args(argc, argv);

  if ( args.contains(kHelpOpt) ) {
    kHelp.print();
    return 0;
  }

  const QString appPath = app.applicationDirPath();

  const QStringList inputFiles = { "12101_0_7_34_4_1_302400.obn" };

//  for ( const QString& n : inputFiles ) {
//    var(n) << "------------------------";

//    QFile file(appPath + "/" + n);
//    file.open(QFile::ReadOnly);

//    obanal::TField field;
//    QByteArray ba = file.readAll();
//    field.fromBuffer(&ba);

//    field.save(appPath + "/new_" + n);
//  }

  if ( args.contains(kPointOpt) ) {
    for ( const QString& n : inputFiles ) {
      obanal::TField field;


      if ( !field.fromFile(appPath + "/" + n) ) {
        return -1;
      }

//      QFile file(appPath + "/" + n);
//      file.open(QFile::ReadOnly);
//      QByteArray ba = file.readAll();
//      field.fromBuffer(&ba);

      // сохранение контрольных данных (1/2)
//      QFile outFile(appPath + "/expected_smootch10." + n);
//      if ( !outFile.open(QFile::WriteOnly | QFile::Truncate) ) {
//        error_log << QObject::tr("Не удалось открыть файл %1.").arg(outFile.fileName())
//                  << outFile.errorString();
//        return -1;
//      }

      //
      // TEST: pointValueF()
      //
      {
        QFile expFile(appPath + "/expected." + n);
        if ( !expFile.open(QFile::ReadOnly) ) {
          error_log << QObject::tr("Не удалось открыть файл %1.").arg(expFile.fileName())
                    << expFile.errorString();
          return -1;
        }

        bool ok = true;
        for ( double lat = 0.12; lat <= 360; lat += 2.25 ) {
          QString line;
          for ( double lon = 0.12; lon <= 180; lon += 1.75 ) {
            double v = field.pointValueF(meteo::GeoPoint(lat*meteo::DEG2RAD, lon*meteo::DEG2RAD));
            line += QString::number(v, 'f', 4).rightJustified(9);
          }
          line += '\n';

          QString expLine = expFile.readLine();

          if ( line != expLine ) {
            none_log << "lat:" << lat;
            none_log << QObject::tr("Expected: %1").arg(expLine);
            none_log << QObject::tr("Actual  : %1").arg(line);
            ok = false;
          }

          // сохранение контрольных данных (2/2)
  //        outFile.write(line.toUtf8() + "\n");
        }
        none_log << "[" << (ok ? "PASS" : "FAIL") << "]" << "pointValueF()" << n;
      }

      //
      // TEST: pointValueF() для smootch 10
      //
      {
        field.smootchField(10);

        QFile expFile(appPath + "/expected_smootch10." + n);
        if ( !expFile.open(QFile::ReadOnly) ) {
          error_log << QObject::tr("Не удалось открыть файл %1.").arg(expFile.fileName())
                    << expFile.errorString();
          return -1;
        }

        bool ok = true;
        for ( double lat = 0.12; lat <= 360; lat += 2.25 ) {
          QString line;
          for ( double lon = 0.12; lon <= 180; lon += 1.75 ) {
            double v = field.pointValueF(meteo::GeoPoint(lat*meteo::DEG2RAD, lon*meteo::DEG2RAD));
            line += QString::number(v, 'f', 4).rightJustified(9);
          }
          line += '\n';

          QString expLine = expFile.readLine();

          if ( line != expLine ) {
            none_log << "lat:" << lat;
            none_log << QObject::tr("Expected: %1").arg(expLine);
            none_log << QObject::tr("Actual  : %1").arg(line);
            ok = false;
          }
        }
        none_log << "[" << (ok ? "PASS" : "FAIL") << "]" << "smootch 10, pointValueF()" << n;
      }
    }
  }

  return 0;
}
