#include "tmsgrecv.h"

#include <stdlib.h>

#include <qdatetime.h>
#include <qfileinfo.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
// #include <decode_data.h>
#include "tlgfileparser.h"
// #include <tgribdecode.h>
// #include <sql/nspgbase/ns_pgbase.h>
//#include <tlg.h>


const QString HOMEPATH = QString(QString( QDir::homePath() ) + "/").replace( "//", "/" );

const QString TMsgRecv::ERROR_EXT("ERROR");
const QString TMsgRecv::DONE_EXT("DONE");
const QString TMsgRecv::UNK_EXT("UNK");
const QStringList TMsgRecv::PROC_EXT_LIST = QStringList()
  << ERROR_EXT
  << DONE_EXT;

TMsgRecv::TMsgRecv( QObject*  p)
  : QObject(p),
  error_(kNoError),
  timerid_(0),
    //  decodata_(0),
  meteodatasize_(0),
  mark_(false),
  once_(false)
{
//  if ( false == TMsgFormat::instance()->loadRules() ) {
//    error_ = ERR_LOAD_MSGFMTS;
//    error_log << QObject::tr("Ошибка загрузки форматов сообщений.");
//    return;
//  }
//   if ( false == metadata_.loadXmlCodeforms() ) {
//     error_ = ERR_LOAD_CODEFORMS;
//     error_log << QObject::tr("Ошибка загрузки кодовых форм.");
//     return;
//   }
//   if ( false == metadata_.loadXmlDecoders() ) {
//     error_ = ERR_LOAD_DECODERS;
//     error_log << QObject::tr("Ошибка загрузки декодеров.");
//     return;
//   }
//   if ( NO_ERRORS != metadata_.loadStations() ) {
// //    error_ = FILE_STATS_ERROR;
//     error_log << QObject::tr("Ошибка загрузки станций.");
// //    return;
//   }
//   decodata_ = new TDecodeData(metadata_, datahash_);
//  mc_.setControlType(Pravdopodobie); //meteocontrol.h in editing
//  mc_.setMeteoDataType(FM12);        //meteocontrol.h in editing
}

TMsgRecv::~TMsgRecv()
{
  //  delete decodata_; decodata_ = 0;
}

void TMsgRecv::start()
{
//  QObject::startTimer( TSettings::checkDirInterval()*1000 ); TODO сделать класс настроек
  timerid_ = QObject::startTimer( 10*1000 );
  if ( 0 == timerid_ ) {
    error_log << QObject::tr("Ошибка. Не удалось запустить таймер");
  }
  slotTimeout();
}

void TMsgRecv::stop()
{
  if ( 0 != timerid_ ) {
    QObject::killTimer(timerid_);
    timerid_ = 0;
  }
}

void TMsgRecv::addDirectory( const QString& n )
{
  QString localn(n);
  if ( 0 == localn.indexOf("~/") ) {
    localn.replace( 0, 2, HOMEPATH );
  }

  debug_log<<localn;

  QDir dir(localn);
  if ( false == msgdirs_.contains(dir) ) {
    msgdirs_.append(dir);
  }
}

void TMsgRecv::timerEvent( QTimerEvent* /*event*/ )
{
  slotTimeout();
}

void TMsgRecv::processDirs()
{
//  QList<QDir> dirs = TSettings::msgDirectories(); TODO реализовать функцию
//  if ( 0 == dirs.size() ) {
//    QList<QDir>::iterator it = dirs.begin();
//    QList<QDir>::iterator end = dirs.end();
//    for ( ; it != end; ++it ) {
//      QString n(*it);
//      if ( 0 == n.indexOf("~/") ) {
//        n.replace( 0, 2, HOMEPATH );
//      }
//      if ( false == msgdirs_.contains(n) ) {
//        msgdirs_.append(n);
//      }
//    }
//  }
  if ( 0 == msgdirs_.size() ) {
    error_log << QObject::tr("Ошибка. Не указана ни одна директория с сообщениями.");
    return;
  }
  QList<QDir>::iterator it = msgdirs_.begin();
  QList<QDir>::iterator end = msgdirs_.end();
  for ( ; it != end; ++it ) {
    processDir(*it);
  }
}

void TMsgRecv::processDir( const QDir& dir )
{
  QDateTime dt = QDateTime::currentDateTime();
//  int secs = TSettings::checkFileInterval(); TODO реализовать
  int secs = 10;
  QFileInfoList files;
  if ( 0 != exts_.size() ) {
    QStringList list;
    for ( int i = 0, sz = exts_.size(); i < sz; ++i ) {
      list << ("*." + exts_[i]);
    }
    files = dir.entryInfoList( list, QDir::Files | QDir::NoDotAndDotDot );
  }
  else {
    files = dir.entryInfoList( QDir::Files | QDir::NoDotAndDotDot );
  }
  for ( int i = 0, sz = files.size(); i < sz; ++i ) {
    const QFileInfo& fi = files[i];
    if ( true == PROC_EXT_LIST.contains( fi.suffix() ) ) {
      continue;
    }
    QDateTime mdt = fi.lastModified();
    if ( secs > mdt.secsTo(dt) ) {
      continue;
    }
    processFile(fi);
  }
}

void TMsgRecv::processFile( const QFileInfo& fi )
{
  if ( true == isTreated(fi) ) {
    return;
  }
  QDateTime dt = QDateTime::fromString( fi.baseName(), "ddMMyyyy_hhmmss" );
  if ( false == dt.isValid() ) {
    dt = QDateTime::currentDateTime();
  }
  TMsgParser parser( fi.absoluteFilePath(), unk_ );
  parser.setUnk( unk_ );
  // int datasizeinfile = 0;
  while ( true == parser.parseNextMessageInFile() ) {
    //    if ( true == handler.binmsg() ) {
//      processBinMessage( handler.type(), parser.binfilename() );
//    }
//    else {
      QString dhmcap = parser.dayhourmincap();
      int day = dhmcap.mid( 0, 2 ).toInt();
      int hour = dhmcap.mid( 2, 2 ).toInt();
      int min = dhmcap.mid( 4, 2 ).toInt();
      QDate date = dt.date();
      if ( day < 1 || day > date.daysInMonth() ) {
        error_log << QObject::tr("Не удалось определить день месяца =") << day;
        day = date.day();
      }
      date.setDate( date.year(), date.month(), day );
      QTime time = dt.time();
      time.setHMS( hour, min, 0 );
      dt = QDateTime( date, time );
      QString msgfile = processMessage( parser.msgbuf(), parser.type(), dt );
      debug_log<<msgfile;
      emit messageParsed( msgfile, parser.msgbuf(), parser.type(), dt );  


      //processTextMessage( parser.msgbuf(), dt, &datasizeinfile );
//    }
  }
//  meteodatasize_ += datasizeinfile;
//  info_log << QObject::tr("Количество TMeteoData в файле =") << datasizeinfile;
//  info_log << QObject::tr("Всего количество TMeteoData =") << meteodatasize_;

  if ( true == mark_ ) {
    markAsTreated( fi, DONE_EXT );
  }
}

QString TMsgRecv::processMessage( const QByteArray& buf, const QString& type, const QDateTime& dt )
{
  QString dirpath( MnCommon::varPath() + "/telegrams/" + dt.toString("dd-MM-yyyy") + "/" + dt.toString("hh-mm-ss") + "/" + type );
  QDir dir(dirpath);
  if ( false == dir.exists() ) {
    dir.mkpath( dir.absolutePath() );
  }
  QStringList list = dir.entryList( QDir::Files | QDir::NoDotAndDotDot );
  QFile file( dir.absoluteFilePath( QString::number(list.size() + 1) + ".msg" ) );
  file.open( QIODevice::WriteOnly );
  file.write( buf );
  file.flush();
  file.close();
  return file.fileName();
}

// void TMsgRecv::processTextMessage( const QByteArray& buf, const QDateTime& dt, int* datasizeintlg )
// {
//   QHash<int, QVector<tlgParse::TSvodka> > mass_svod; //здесь будут лежать отдельные сводки из сообщения buf
//   int rc = tlgParse::tlg( QByteArray(buf), &mass_svod, dt, metadata_.headers() ); //извлекаем из сообщения все сводки
//   if ( 0 > rc ) {
//     return;
//   }
//   rc = decodata_->decodeWithXmlDomDecoder(&mass_svod); //каждую сводку из массива раскодируем и получаем список TMeteoData
//   if ( 0 > rc ) {
//     return;
//   }

//   QHash<int, QHash<int, TMeteoData> >::iterator it = datahash_.begin();
//   while ( it != datahash_.end() ) {
// //    mc_.setAllMeteoData( it.value() ); //meteocontrol.h in editing
// //    mc_.control();                     //meteocontrol.h in editing
//     *datasizeintlg += it.value().size();
//     ++it;
//   }
//   decodata_->clearData();
// }

// void TMsgRecv::processBinMessage( const QString& type, const QString& filename )
// {
//   if (type == "grib") {
//     NS_PGBase* db = new NS_PGBase("127.0.0.1","meteo","postgres",""); //TODO

//     TDecodeGrib* gribDec = new TDecodeGrib(db);
//     int err = gribDec->decode(filename);
//     if (err != 0) {
//       error_log<<"Error in "<<filename;
//     }

//     delete db;
//     delete gribDec;
//   }
// }

void TMsgRecv::markAsTreated( const QFileInfo& fi, const QString& status )
{
  QFile file( treatedFileName( fi.absoluteFilePath(), status ) );
  if ( false == file.open( QIODevice::WriteOnly ) ) {
    error_log << QObject::tr("Ошибка. Не могу открыть файл для записи =") << file.fileName()
      << QObject::tr("\n\tОписание ошибки =") << file.errorString();
  }
  file.close();
}

bool TMsgRecv::isTreated( const QFileInfo& fi )
{
  QString n = fi.absoluteFilePath();
  for ( int i = 0, sz  = PROC_EXT_LIST.size(); i < sz; ++i ) {
    if ( true == QFileInfo( treatedFileName( n, PROC_EXT_LIST[i] ) ).exists() ) {
      return true;
    }
  }
  return false;
}

void TMsgRecv::slotTimeout()
{
  processDirs();
}
