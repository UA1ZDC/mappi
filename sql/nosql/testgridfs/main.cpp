#include <qstringlist.h>
#include <qcoreapplication.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/targ.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/app/paths.h>

#include <sql/nosql/gridfs.h>

#define HELP(opt, text)  (QStringList() << opt << QObject::tr(text))

const QStringList kHelpOpt = QStringList() << "h" << "help";
const QStringList kListOpt = QStringList() << "l" << "list";
const QStringList kRunOpt  = QStringList() << "r" << "run";

const HelpFormatter kHelp = HelpFormatter()
  << HELP(kHelpOpt, "Эта справка.")
  << HELP(kListOpt, "Список тестов.")
  << HELP(kRunOpt, "Выполнить заданный тест.")
     ;



void fileList();
void fileCount();
void removeFile();
void fileInfo();
void putFile();
void getFile();



int main(int argc, char** argv)
{
  QCoreApplication* app = new QCoreApplication(argc,argv);

  TArg args(argc,argv);

  if ( args.contains(kHelpOpt) ) {
    kHelp.print();
    return EXIT_SUCCESS;
  }

  QStringList list;
  list << "list";
  list << "count";
  list << "remove";
  list << "info";
  list << "put";
  list << "get";

  if ( args.contains(kListOpt) ) {
    none_log << list.join("\n");
    return EXIT_SUCCESS;
  }

  for ( const QString& r : args.values(kRunOpt) ) {
    var(r);
    if ( "list" == r ) {
      fileList();
    }
    else if ( "count" == r ) {
      fileCount();
    }
    else if ( "remove" == r ) {
      removeFile();
    }
    else if ( "info" == r ) {
      fileInfo();
    }
    else if ( "put" == r ) {
      putFile();
    }
    else if ( "get" == r ) {
      getFile();
    }
  }

//  auto gridfs = nosql::NoSqlManager::instance().getGridFs();
//  if ( !gridfs->put("/home/maxim/my/projects/tools/gridfs/file2.txt", nullptr, "test") ) {
//    error_log << gridfs->lastError();
//    return 0;
//  }


//  QByteArray ba;
//  if ( !gridfs->get("TEST-FILE.TXT", &ba, "test") ) {
//    error_log << gridfs->lastError();
//    return;
//  }
//  info_log << QObject::tr("ФАЙЛ %1:").arg("TEST-FILE.TXT");
//  none_log << ba;

//  nosql::GridFile file;
//  if ( file.open(gridfs, "filename.txt", nosql::GridFile::ReadOnly) ) {
//    return;
//  }
//  file.seek(1000);
//  QByteArray raw = file.read(128);

//  nosql::GridFile fw;
//  if ( fw.open(gridfs, "filename.txt", nosql::GridFile::WriteOnly) ) {
//    return;
//  }

//  QFile f("/home/maxim/filename.txt");
//  fw.write(&f);

  return 0;
}

// вывод списка файлов
void fileList()
{
  nosql::GridFs gfs;
  gfs.connect("localhost", 27017);
  gfs.use("testdb", "fs");

  nosql::GridFileList list = gfs.find("{}");
//  if ( !list.isValid() ) {
//    error_log << gfs.lastError();
//    return;
//  }
  info_log << QObject::tr("СПИСОК ФАЙЛОВ:");
  while ( list.next() ) {
    nosql::GridFile file = list.file();
    none_log << "id:" << file.id() << "name:" << file.fileName();
  }
}

// вывод количества файлов
void fileCount()
{
  nosql::GridFs gfs;
  gfs.connect("localhost", 27017);
  gfs.use("testdb", "fs");

  nosql::GridFileList list = gfs.find("{}");
//  if ( !list.isValid() ) {
//    error_log << gfs.lastError();
//    return;
//  }
  int n = 0;
  while ( list.next() ) {
    ++n;
  }
  info_log << QObject::tr("Количество файлов: %1").arg(n);
}

// удаление файла по имени
void removeFile()
{
  nosql::GridFs gfs;
  gfs.connect("localhost", 27017);
  gfs.use("testdb", "fs");

  nosql::GridFileList list = gfs.find("{}");
//  if ( !list.isValid() ) {
//    error_log << gfs.lastError();
//    return;
//  }
  int n = 0;
  while ( list.next() ) {
    info_log << QObject::tr("Удаление файла %1").arg(list.file().fileName()) << (list.file().remove() ? "true" : "false");
  }
}

// информация о файле
void fileInfo()
{
  nosql::GridFs gfs;
  gfs.connect("localhost", 27017);
  gfs.use("testdb", "fs");

  nosql::GridFile file = gfs.findOne("{}");
  if ( !file.isValid() ) {
    error_log << gfs.lastError();
    return;
  }

  if ( !file.hasFile() ) {
    error_log << QObject::tr("Нет файлов");
    return;
  }

  info_log << QObject::tr("ID: %1").arg(file.id());
  info_log << QObject::tr("MD5: %1").arg(file.md5());
  info_log << QObject::tr("Имя: %1").arg(file.fileName());
  info_log << QObject::tr("Дата: %1").arg(file.uploadDate().toString("yyyy-MM-dd hh:mm"));
  info_log << QObject::tr("Размер: %1 байт").arg(file.size());

  nosql::Document metadata = file.metadata();
  info_log << QObject::tr("Метаданные: \n") << metadata.jsonString();
}

// добавление файла
void putFile()
{
  qsrand(QDateTime::currentMSecsSinceEpoch());
  int r = qrand() % 99999;

  nosql::GridFs gfs;
  gfs.connect("localhost", 27017);
  gfs.use("testdb", "fs");

  QString fileName = QString("file_%1.txt").arg(r, 5, 10, QChar('0'));
  QString text = QString("Hello world #%1!").arg(r);

  nosql::GridFile file;
  if ( !gfs.put(fileName, text.toUtf8(), &file) ) {
    error_log << gfs.lastError();
    return;
  }
  file.setMetadata("{ \"info\": \"some info...\" }");

  info_log << "write:" << file.write("data-1", true) << file.lastError();
  info_log << "write:" << file.write("HELLO WORLD!!!", true) << file.lastError();

  info_log << QObject::tr("Добавлен файл: %1").arg(fileName);
}

void getFile()
{
  nosql::GridFs gfs;
  gfs.connect("localhost", 27017);
  gfs.use("testdb", "fs");

  nosql::GridFile file =gfs.findOne("{}");
  trc << file.readAll(nullptr);
}
