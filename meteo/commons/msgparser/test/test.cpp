#include <stdlib.h>

#include <qfile.h>
#include <qdir.h>
#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qapplication.h>
#include <qobject.h>
#include <qfileinfo.h>
#include <qmap.h>

#include <meteo/commons/msgparser/tlgparser.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/app/tsigslot.h>

#include <meteo/commons/proto/msgparser.pb.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/msgparser/tlgfileparser.h>
#include <meteo/commons/msgparser/tmsgformat.h>
//#include <tlg.h>
//#include <meta_data.h>

#include <mn_errdefs.h>

class BrokenSignal : public sigslot::has_slots<>
{
  public:
    void slotBrokenTlg() {
//      debug_log << "Broken tlg";
    }
};

const QString MSG_TEST_READ_CATALOG = "msgtestread/";

QMap< QString, msgformat::TMsgFormat::MsgRule> msgformat_;
QMap< QString, msgformat::TMsgFormat::BinRule> binformat_;

int testParser( const QString& path )
{
  const int chunk = 64*16;
  QFile file(path);
  file.open( QIODevice::ReadOnly );
  debug_log << "filename =" << file.fileName();
  QDir result("./result/"+file.fileName() );
  QDir restartlines( "./startlines/"+file.fileName() );
  if ( true == result.exists() ) {
    QStringList rmflist = result.entryList( QDir::Files | QDir::NoDotAndDotDot );
    for ( int j = 0, fsz = rmflist.size(); j < fsz; ++j  ) {
      result.remove(rmflist[j]);
    }
    result.rmpath( result.absolutePath() );
  }
  if ( false == result.exists() ) {
    result.mkpath("./");
  }
  if ( true == restartlines.exists() ) {
    QStringList rmflist = restartlines.entryList( QDir::Files | QDir::NoDotAndDotDot );
    for ( int j = 0, fsz = rmflist.size(); j < fsz; ++j  ) {
      restartlines.remove(rmflist[j]);
    }
    restartlines.rmpath( restartlines.absolutePath() );
  }
  if ( false == restartlines.exists() ) {
    restartlines.mkpath("./");
  }
  QByteArray filearr;
  meteo::TlgParser parser(filearr);
  BrokenSignal brok;
  parser.brokenTlg().connect( &brok, &BrokenSignal::slotBrokenTlg );
  while ( true ) {
    QByteArray arr(chunk,'\0');
    if ( false == file.atEnd() ) {
      int64_t fsz = file.read( arr.data(), chunk );
      parser.appendData( arr, fsz );
//      filearr.append( arr.left(fsz) );
    }

    meteo::tlg::MessageNew newmsg;
    while ( true == parser.parseNextMessage(&newmsg) ) {
      QFile f( result.filePath( QString::number( result.entryList( QDir::Files | QDir::NoDotAndDotDot ).size() + 1 ) + ".tlg" ) );
      f.open(QIODevice::WriteOnly);
      f.write( parser.tlg() );
      f.flush();
      QFile fs( restartlines.filePath( QString::number( restartlines.entryList( QDir::Files | QDir::NoDotAndDotDot ).size() + 1 ) + ".tlg" ) );
      fs.open(QIODevice::WriteOnly);
      fs.write( parser.startline() );
      fs.flush();
      TProtoText::toFile( newmsg, result.filePath( QString::number( result.entryList( QDir::Files | QDir::NoDotAndDotDot ).size() + 1 ) + ".tlg.proto" ) );
    }
    if ( true == file.atEnd() ) {
      break;
    }
  }
  QList<QString> countnames = parser.counternames();
  for ( int i = 0, sz = countnames.size(); i < sz; ++i ) {
    debug_log << "file =" << file.fileName() << countnames[i] << " found =" << parser.count(countnames[i]);
  }
  return EXIT_SUCCESS;
}

int testStartLine( const QString& path )
{
  QFile file(path);
  file.open( QIODevice::ReadOnly );
  QByteArray arr = file.readAll();
  QString filestr(arr.replace('\0',""));

  QStringList ptrns;
  QMapIterator<QString,msgformat::TMsgFormat::MsgRule> it(msgformat_);
  while ( true == it.hasNext() ) {
    it.next();
//  for ( int i = 0, sz = msgformat_.size(); i < sz; ++i ) {
    const msgformat::TMsgFormat::MsgRule& rule = it.value();
    const QRegExp& rex = rule.bits["startline"];
    QString ptrn = "(" + rex.pattern() + ")";
    ptrns << ptrn;
  }
  QRegExp fullrex("(" + ptrns.join("|") + ")");
  int pos = fullrex.indexIn( filestr );
  int tlgcnt = 0;
  while ( -1 != pos ) {
    ++tlgcnt;
    pos = fullrex.indexIn( filestr, pos + 1 );
  }
  debug_log << "startline count =" << tlgcnt;
  return EXIT_SUCCESS;
}

int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("meteo");
  QApplication app( argc, argv );
  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  if ( false == TMsgFormat::instance()->loadRules() ) {
    error_log << QObject::tr("Ошибка. Не удалось загрузить форматы сообщений");
    return EXIT_FAILURE;
  }
  msgformat_ = TMsgFormat::instance()->rules();
  binformat_ = TMsgFormat::instance()->binrules();
  QString path("./msgtestread");
  if ( 1 < argc ) {
    path  = argv[1];
  }

  QStringList files;
  QFileInfo fi(path);
  if ( true == fi.isDir() ) {
    QDir dir( fi.absoluteFilePath() );
    files = dir.entryList( QDir::Files | QDir::NoDotAndDotDot );
    for ( int i = 0, sz = files.size(); i < sz; ++i ) {
      files[i] = dir.absoluteFilePath(files[i]);
    }
  }
  else if ( true == fi.isFile() ) {
    files = QStringList( fi.absoluteFilePath() );
  }
  else {
    error_log << QObject::tr("Неправильно указан путь к файлам с телеграммами");
    return EXIT_FAILURE;
  }
  for ( int i = 0, sz = files.size(); i < sz; ++i ) {
    int res = testParser( files[i] );
 //   if ( EXIT_SUCCESS == res ) {
 //     res = testStartLine( files[i] );
 //   }
 //   else {
 //     debug_log << QObject::tr("Ошибка парсера");
 //     return res;
 //   }
    if ( EXIT_SUCCESS != res ) {
      debug_log << QObject::tr("Ошибка поиска начальных строк");
      return res;
    }
  }
  return EXIT_SUCCESS;
}
