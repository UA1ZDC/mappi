#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <qcoreapplication.h>
#include <qtextcodec.h>
#include <qfile.h>
#include <qregexp.h>
#include <qdebug.h>

#include <unistd.h>

void convertTableB(const QString& fileName, const QString& dest)
{
  QFile file(fileName);
  file.open(QIODevice::ReadOnly);
  QTextStream ts(&file);

  QString all = ts.readAll();

  file.close();

  QFile fileDest(dest);
  fileDest.open(QIODevice::WriteOnly);
  QTextStream ds(&fileDest);

  int start = all.indexOf("Class 00");
  int end = all.lastIndexOf("#");

  all = all.mid(start, end - start);
  all.replace(QRegExp("Class \\d\\d\\s+\\n"), "");
  all.replace(QRegExp("-+\\s+\n"), "");
  all.replace(QRegExp("#\\s*\\w+\\:\\s*\\n"), "");  

  QStringList lines = all.split('#');
  for (int i = 0; i < lines.count(); i++) {
    lines[i] = lines.at(i).trimmed();
    QStringList dlist = lines[i].split('\n');
    if (dlist.count() != 3) continue;
    dlist[0]= dlist.at(0).trimmed();
    dlist[1]= dlist.at(1).trimmed();
    dlist[2]= dlist.at(2).trimmed();
    dlist[0].truncate(63);
    dlist[1].truncate(24);

    if (dlist.at(0).isEmpty() ||
	dlist.at(1).isEmpty() ||
	dlist.at(2).isEmpty()) {
      continue;
    }
    
    ds << " " << dlist.at(0).leftJustified(71, ' ') << " " 
       << dlist.at(1).leftJustified(25, ' ') << " "
       << dlist.at(2) << "\n";
  }

  fileDest.close();
  //  debug_log << lines;
}

void convertTableD(const QString& fileName, const QString& dest)
{
  QFile file(fileName);
  file.open(QIODevice::ReadOnly);
  QTextStream ts(&file);

  QString all = ts.readAll();

  file.close();

  QFile fileDest(dest);
  fileDest.open(QIODevice::WriteOnly);
  QTextStream ds(&fileDest);


  int start = all.indexOf("Class 00");
  int end = all.lastIndexOf("#");

  all = all.mid(start, end - start);
  all.replace(QRegExp("Class \\d\\d\\s+\\n"), "");
  all.replace(QRegExp("-+\\s+\n"), "");
  all.replace(QRegExp("#\\w+\\n# Note.+\\n#\\w+\\n"), "");
  all.replace(QRegExp("#\\s*\\w+\\:\\s*\\n"), "");

  QStringList lines = all.split('#');

  for (int i = 0; i < lines.count(); i++) {
    lines[i] = lines.at(i).trimmed();

    debug_log << i << lines[i];

    QStringList descrLines = lines[i].split('\n');
    QStringList dlist;

    QRegExp rx("\\s*(\\d\\d\\d\\d\\d\\d)");
    int pos = rx.indexIn(descrLines[0]);
    if (pos == -1) continue;
    
    QString dDesc = rx.cap(1);
    // var(dDesc);    
    // var(descrLines);
    for (int dl = 1; dl < descrLines.count(); dl++) {
      pos = rx.indexIn(descrLines[dl], 0);
      if (pos == -1) {
	// var(descrLines);
	// var(descrLines.at(dl));
	continue;
      }
      dlist << rx.cap(1);
    }
  
    var(dlist);
    
    ds << dDesc << " " << QString::number(dlist.count()).rightJustified(2, ' ')  << " " << dlist.at(0) << "\n";
    for (int kk = 1; kk < dlist.count(); kk++) {
      ds << "          " << dlist.at(kk) << "\n";
    }
  }



  fileDest.close();
  //  debug_log << lines;
}

int main ( int argc, char *argv[] ) 
{
  TAPPLICATION_NAME( "meteo" );
  //meteo::global::setLogHandler();
  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale());

  int opt;
  int flag = 0;

  while ((opt = getopt(argc, argv, "bdh")) != -1) {
    switch (opt) {
    case 'h':
      info_log << QObject::tr("Приведение таблиц B и D BUFR с сайта http://www.northern-lighthouse.com к нужному нам формату");
      info_log << QObject::tr("Использование:") << argv[0] << " -b|-d <source fileName> <dest filename>";
      exit(0);
      break;
    case 'b':
      flag = 1;
      break;
    case 'd':
      flag = 2;
      break;
    default: /* '?' */
      error_log<<"Option"<<opt<<"not realized";
      exit(-1);
    }
  }

  if (flag == 0) {
    info_log << QObject::tr("Использование:") << argv[0] << " -b|-d <source fileName> <dest filename>";
    exit(-1);
  }

  QString fileName;
  if (optind + 1>= argc) {
    error_log<<"Need point file name. \""<<argv[0]<<"-h\" for help";
    exit(-1);
  } 
  
  fileName = argv[optind];
  QString dest = argv[optind + 1];

  QCoreApplication app(argc, argv);
 
  if (flag == 1) {
    convertTableB(fileName, dest);
  } else if (flag == 2) {
    convertTableD(fileName, dest);
  }
  
  return 0;
}
