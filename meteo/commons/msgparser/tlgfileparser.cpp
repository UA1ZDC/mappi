#include "tlgfileparser.h"

#include <qdir.h>
#include <qstringlist.h>
#include <qdatastream.h>
#include <qbuffer.h>

#include <time.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/grib/parser/tgribformat.h>

const int TMsgParser::CHUNKSIZE = 64;
const int TMsgParser::TWOCHUNKSIZE = 2*64;

int64_t TMsgParser::readFirstBuf( QByteArray* buf, QIODevice* source, int64_t size )
{
  buf->fill( '\000', size + 1);
  return source->read( buf->data(), size );
}

int64_t TMsgParser::readNextBuf( QByteArray* buf, QIODevice* source, int64_t size )
{
  char* first = buf->data();
  char* second = first + size;
  ::memcpy( first, second, size );
  return source->read( second, size );
}

//bool createMsgFile( QFile* file, const QString& tlgfilename )
//{
//  QDir dir( tlgfilename + ".d" );
//  if ( false == dir.exists() && false == dir.mkpath( dir.absolutePath() ) ) {
//    error_log << QObject::tr("Ошибка. Невозможно создать директорию =") << dir.absolutePath();
//    return false;
//  }
//  QStringList list = dir.entryList( QDir::Files | QDir::NoDotAndDotDot );
//  file->setFileName( dir.absolutePath() + "/" + QString::number( list.size() ) );
//  return true;
//}

TMsgParser::TMsgParser( const QString& filename, bool unk )
  : unk_(unk)
{
  loadMsgFormat();
  setTelegramFile(filename);
}

TMsgParser::TMsgParser()
  : unk_(false)
{
  loadMsgFormat();
}

TMsgParser::~TMsgParser()
{
}

bool TMsgParser::setTelegramFile( const QString& filename )
{
  resetTelegramFile();
  tlgfile_.setFileName(filename);
  if ( false == tlgfile_.exists() ) {
    error_log << QObject::tr("Ошибка. Файл не существует =") << filename;
    return false;
  }
  if ( false == tlgfile_.open( QIODevice::ReadOnly ) ) {
    error_log << QObject::tr("Ошибка. Не могу открыть файл =") << filename
      << "\n\t*" << tlgfile_.errorString();
    return false;
  }
  if ( false == unk_ ) {
    unknowndatafile_.setFileName("/dev/null");
  }
  else {
    unknowndatafile_.setFileName(filename + ".UNK" );
  }
  if ( false == unknowndatafile_.open( QIODevice::WriteOnly ) ) {
    warning_log << QObject::tr("Не могу открыть файл для записи неопознаных данных =") << unknowndatafile_.fileName()
      << "\n\t*" << unknowndatafile_.errorString();
  }
  unknowndatastream_.setDevice(&unknowndatafile_);

  return true;
}

void TMsgParser::resetTelegramFile()
{
  if ( true == tlgfile_.isOpen() ) {
    tlgfile_.close();
  }
  if ( true == unknowndatafile_.isOpen() ) {
    unknowndatafile_.flush();
    unknowndatafile_.close();
    if ( 0 == unknowndatafile_.size() && "/dev/null" != unknowndatafile_.fileName() ) {
      unknowndatafile_.remove();
    }
  }
}

bool TMsgParser::parseNextMessageInFile()
{
  if ( false == tlgfile_.isOpen() ) {
    error_log << QObject::tr("Ошибка. Не открыт файл с телеграммами =") << tlgfile_.fileName();
    return false;
  }
  if ( true == tlgfile_.atEnd() ) {
    info_log << QObject::tr("Файл обработан =") << tlgfile_.fileName();
    return false;
  }
  bool ok = false;
  msgformat::TMsgFormat::MsgRule rule = parseStartLine(&ok);
  if ( false == ok ) {
    error_log << QObject::tr("Ошибка поиска начальной строки в файле =") << tlgfile_.fileName()
      << QObject::tr("Позиция в файле =") << tlgfile_.pos();
    return false;
  }
  int lastpos = tlgfile_.pos();
  if ( false == parseHeader( rule ) ) {
    error_log << QObject::tr("Ошибка поиска заголовка в файле =") << tlgfile_.fileName()
      << QObject::tr("Позиция в файле =") << tlgfile_.pos();
    if ( true == tlgfile_.atEnd() ) {
      return false;
    }
    else {
      tlgfile_.seek(lastpos);
      return parseNextMessageInFile();
    }
  }
  lastpos = tlgfile_.pos();
  if ( false == parseMessage( rule ) ) {
    error_log << QObject::tr("Ошибка поиска сообщения в файле =") << tlgfile_.fileName()
      << QObject::tr("Позиция в файле =") << tlgfile_.pos();
    if ( true == tlgfile_.atEnd() ) {
    }
    else {
      tlgfile_.seek(lastpos);
      return parseNextMessageInFile();
    }
    return false;
  }
  return true;
}

msgformat::TMsgFormat::MsgRule TMsgParser::parseStartLine( bool* ok )
{
  *ok = false;
  QByteArray tlgbuf;
  int64_t readbytes = readFirstBuf( &tlgbuf, &tlgfile_, TWOCHUNKSIZE );
  while ( 0 < readbytes ) {
    int index = -1;
    QMapIterator< QString, msgformat::TMsgFormat::MsgRule > it(msgformat_);
//    for ( int i = 0, sz = msgformat_.size(); i < sz; ++i ) {
    while ( true == it.hasNext() ) {
      it.next();
      const msgformat::TMsgFormat::MsgRule& rule = it.value();
      const QRegExp& re = rule.bits["startline"];
      QString str(tlgbuf);
      if ( -1 != ( index = re.indexIn(str) ) ) {
        QString cap = re.cap();
        index = tlgbuf.indexOf(cap);
        unknowndatastream_.writeRawData( tlgbuf.data(), index );
        tlgfile_.seek( tlgfile_.pos() - readbytes + index + cap.length() );
        QString filename = QFileInfo(tlgfile_).fileName();
        if ( 0 < msgbuf_.size() ) {
          msgbuf_.clear();
        }
        msgbuf_.append( cap.toUtf8() );
        *ok = true;
        return rule;
      }
    }
    //начальная строка не найдена в tlgbuf. считать из файла следующую порцию данных
    readbytes = readNextPortionForSearchStartLine( &tlgbuf, readbytes );
  }
  return msgformat_[0];
}

int64_t TMsgParser::readNextPortionForSearchStartLine( QByteArray* arr, int64_t readbytes )
{
  int index = arr->indexOf('\000');
  if ( -1 != index && TWOCHUNKSIZE != index ) {
    unknowndatastream_.writeRawData( arr->data(), index );
    tlgfile_.seek( tlgfile_.pos() - readbytes + index + 1 );
    readbytes = readFirstBuf( arr, &tlgfile_, TWOCHUNKSIZE );
    if ( -1 == readbytes ) {
      return readbytes;
    }
  }
  else {
    unknowndatastream_.writeRawData( arr->data(), CHUNKSIZE );
    readbytes = readNextBuf( arr, &tlgfile_, CHUNKSIZE );
    if ( 0 < readbytes ) {
      readbytes += CHUNKSIZE;
    }
    else {
      return readbytes;
    }
  }
  return readbytes;
}

bool TMsgParser::parseHeader( const msgformat::TMsgFormat::MsgRule& rule )
{
  QByteArray tlgbuf;
  int64_t readbytes = readFirstBuf( &tlgbuf, &tlgfile_, TWOCHUNKSIZE );
  if ( -1 == readbytes ) {
    return false;
  }
  const QRegExp& re = rule.bits["header"];
  QString str(tlgbuf);
  if ( 0 != re.indexIn(str) ) {
    unknowndatastream_.writeRawData( msgbuf_.data(), msgbuf_.size() );
    return false;
  }
  QString cap = re.cap();
//  if ( -1 != rule.dayhourcap ) {
//    dayhourmincap_ = re.cap( rule.dayhourcap );
//  }
  tlgfile_.seek( tlgfile_.pos() - readbytes + cap.length() );
  msgbuf_.append( cap.toUtf8() );
  return true;
}

bool TMsgParser::parseMessage( const msgformat::TMsgFormat::MsgRule& rule )
{
  QByteArray tlgbuf;
  int64_t readbytes = readFirstBuf( &tlgbuf, &tlgfile_, CHUNKSIZE );
  if ( -1 == readbytes ) {
    unknowndatastream_.writeRawData( msgbuf_.data(), msgbuf_.size() );
    return false;
  }
  bool binfound = false;
  type_ = "text";
  msgformat::TMsgFormat::BinRule binrule;
  QList<msgformat::TMsgFormat::BinRule> binrules = TMsgFormat::instance()->binrules().values();
  for ( int i = 0, sz = binrules.size(); i < sz; ++i ) {
    const msgformat::TMsgFormat::BinRule& br = binrules[i];
    int index = tlgbuf.indexOf( br.begin );
    if ( 0 != index ) {
      continue;
    }
    binfound = true;
    binrule = br;
    type_ = br.name;
    while ( 0 == index && 0 < readbytes ) {
      int edition = 0;
      if ( -1 != br.editionbyte ) {
        grib::char2dec( tlgbuf.data() + br.editionbyte, 1, &edition, true );
      }
      if ( false == br.sizerules.contains(edition) ) {
        error_log << QObject::tr("Ошибка. Нет правил поиска размера сообщения типа =")
          << rule.name << QObject::tr("номер редакции =") << edition;
        unknowndatastream_.writeRawData( msgbuf_.data(), msgbuf_.size() );
        unknowndatastream_.writeRawData( tlgbuf.data(), readbytes );
        return false;
      }
      msgformat::TMsgFormat::SizeRule sr = br.sizerules[edition];
      int64_t size = 0;
      if ( msgformat::TMsgFormat::BinRule::LE_ORDER == sr.byteorder ) {
        ::memcpy( &size, tlgbuf.data() + sr.first, sr.second );
      }
      else {
        grib::char2dec( tlgbuf.data() + sr.first, sr.second, &size, true );
      }
      size += sr.addbytes;
      tlgfile_.seek( tlgfile_.pos() - readbytes );
      if ( false == processMessage( &tlgfile_, br, size ) ) {
        unknowndatastream_.writeRawData( msgbuf_.data(), msgbuf_.size() );
        return false;
      }
      readbytes = readFirstBuf( &tlgbuf, &tlgfile_, CHUNKSIZE );
      index = tlgbuf.indexOf( br.begin );
    }
    if ( 0 < readbytes ) {
      tlgfile_.seek( tlgfile_.pos() - readbytes );
    }
    break;
  }
  if ( false == binfound ) {
    tlgfile_.seek(  tlgfile_.pos() - readbytes );
  }
  bool res = processMessage( &tlgfile_, rule.bits["endline"].pattern() );
  if ( false == res ) {
    unknowndatastream_.writeRawData( msgbuf_.data(), msgbuf_.size() );
  }
//  else if ( true == handler_->binmsg() ) {
//    saveBinfile(binrule);
//  }
  return res;
}

bool TMsgParser::processMessage( QIODevice* source, const msgformat::TMsgFormat::BinRule& rule, int64_t size )
{
  QByteArray arr( TMsgParser::CHUNKSIZE, '\000' );
  while ( 0 < size ) {
    int64_t readsize = ( size > TMsgParser::CHUNKSIZE ) ? TMsgParser::CHUNKSIZE : size ;
    size -= readsize;
    int br = source->read( arr.data(), readsize );
    if ( -1 == br ) {
      error_log << QObject::tr("Ошибка при попытке прочитать данные");
      error_log << QObject::tr("Описание ошибки =") << source->errorString();
      return false;
    }
    if ( readsize != br ) {
      warning_log << QObject::tr("Внимание. Количество прочитанных байт не совпадает с требуемым.");
      size += readsize - br;
    }
    msgbuf_.append( arr.data(), br );
    if ( true == source->atEnd() ) {
      error_log << QObject::tr("Ошибка. Неожиданный конец файла.");
      return false;
    }
  }
  if ( false == rule.end.isEmpty() ) {
    source->seek( source->pos() - rule.end.length() );
    arr = source->read( rule.end.length() );
    if ( arr != rule.end ) {
      error_log << QObject::tr( "Ошибка. Бинарная часть сообщения не соответсвует правилам.");
      return false;
    }
  }
  char c('\000');
  while ( true == source->getChar(&c) && '\000' == c ) {
    msgbuf_.append(c);
  }
  if ( false == source->atEnd() ) {
    source->seek( source->pos() - 1 );
  }
  return true;
}

bool TMsgParser::processMessage( QIODevice* source, const QString& /*end*/ )
{
  QByteArray endtmp("\r\r\n\003");
  QByteArray tlgbuf;
  int64_t readbytes = readFirstBuf( &tlgbuf, source, TMsgParser::TWOCHUNKSIZE );
  int index = -1;
  if ( -1 != ( index = tlgbuf.indexOf(endtmp) ) ) {
    msgbuf_.append( tlgbuf.data(), index + endtmp.length() );
    source->seek( source->pos() - readbytes + index + endtmp.length() );
    return true;
  }
  else if (readbytes < TMsgParser::CHUNKSIZE ) {
    msgbuf_.append( tlgbuf.data(), readbytes );
  }
  else {
    msgbuf_.append( tlgbuf.data(), TMsgParser::CHUNKSIZE );
  }
  while ( -1 != readbytes ) {
    index = tlgbuf.indexOf(endtmp);
    if ( -1 != index ) {
      msgbuf_.append( tlgbuf.data() + TMsgParser::CHUNKSIZE, index + endtmp.length() - TMsgParser::CHUNKSIZE );
      source->seek( source->pos() - readbytes + index + endtmp.length() );
      return true;
    }
    readbytes = readNextBuf( &tlgbuf, source, TMsgParser::CHUNKSIZE );
    if ( -1 == readbytes ) {
      return false;
    }
    else {
      msgbuf_.append( tlgbuf.data(), TMsgParser::CHUNKSIZE );
      readbytes += TMsgParser::CHUNKSIZE;
    }
  }
  return false;

}

void TMsgParser::loadMsgFormat()
{
//  if ( false == TMsgFormat::instance()->loadRules() ) {
//    error_log << QObject::tr("Ошибка. Не удалось загрузить форматы сообщений");
//    return;
//  }
  msgformat_ = TMsgFormat::instance()->rules();
  binformat_ = TMsgFormat::instance()->binrules().values();
}
//
//bool TMsgParser::saveBinfile( const msgformat::TMsgFormat::BinRule& rule )
//{
//  if ( 0 == handler_->msgbuf().size() ) {
//    return true;
//  }
//  QDateTime dt = QDateTime::currentDateTime();
//  QString dtstr = dt.toString("yyyyMMdd_hhmmss");
//  QString path = rule.path + "/";
//  QFile binfile;
//  QDir dir(path);
//  if ( false == dir.exists() && false == dir.mkpath(path) ) {
//    warning_log << QObject::tr("Ошибка. Не удалось создать директорию =") << dir.absolutePath();
//    binfile.setFileName("/dev/null");
//  }
//  else {
//    int inc = 0;
//    QString name = path + dtstr + "." + QString::number(inc) + ".bin";
//    while ( true == binfile.exists( name ) ) {
//      name = path + dtstr + "." + QString::number(++inc) + ".bin";
//    }
//    binfile.setFileName(name);
//  }
//  if ( false == binfile.open( QIODevice::WriteOnly | QIODevice::Append ) ) {
//    error_log << QObject::tr("Ошибка при попытке открыть файл для записи =") << binfile.fileName();
//    return false;
//  }
//  binfilename_ = binfile.fileName();
//  binfile.write( handler_->msgbuf() );
//  binfile.flush();
//  binfile.close();
//  return true;
//}
