#include "tlgmsg.h"

#include <qtextcodec.h>
#include <qcoreapplication.h>
#include <qdir.h>
#include <qfile.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/grib/parser/tgribformat.h>

namespace meteo
{

const int TlgMsg::CHUNKSIZE = 64;
const int TlgMsg::TWOCHUNKSIZE = 2*64;
const int TlgMsg::TLGMAXSIZE = 1024*1024*50;
const int TlgMsg::TWOTLGMAXSIZE = 2*1024*1024*50;

const QString kForbiddenBufr("BUFRPK");

TlgMsg::TlgMsg()
  : state_( UNKNOWN_TLGSTATE ),
  type_( UNKTLG ),
  msgtype_( "unknown" ),
  arrpos_(0),
  toolittlearray_(false)
{
  loadMsgFormat();
}

TlgMsg::TlgMsg( const QString& rulename )
  : state_( UNKNOWN_TLGSTATE ),
  type_( UNKTLG ),
  msgtype_( "unknown" ),
  arrpos_(0),
  rulename_(rulename),
  toolittlearray_(false)
{
  loadMsgFormat();
}


void TlgMsg::setStuckedSplit( bool fl )
{
  splitstuckedmessages_ = fl;
}

void TlgMsg::reset()
{
  state_ = UNKNOWN_TLGSTATE;
  type_ = UNKTLG;
  arrpos_ = 0;
}

QByteArray TlgMsg::tlg() const
{
  if ( true == msgrule_.onlybody ) {
    return message_;
  }
  return startline_ + header_ + message_ + end_;
}

void TlgMsg::saveUnparsed( const QByteArray& arr )
{
  return;
  QDir dir( QDir::tempPath() );
  QString tmpname = dir.filePath(
      "unparsedstream."
      + QCoreApplication::applicationName()
      + '.'
      + QString::number( QCoreApplication::applicationPid() )
      + ".arr" );
  QFile tmpfile(tmpname);
  if ( true == tmpfile.open(QIODevice::WriteOnly | QIODevice::Append) ) {
    tmpfile.write( arr );
    tmpfile.flush();
    tmpfile.close();
  }
}

TlgMsg::ParseState TlgMsg::parseArray( QByteArray* arr )
{
  toolittlearray_ = false;
//  debug_log << "PARSE ARRAY SIZE = " << arr->size();
  switch (state_) {
    case UNKNOWN_TLGSTATE:
    case SEARCH_START_LINE:
    case MESSAGE_PARSED:
      if ( false == parseStartLine(arr) ) {
        break;
      }
      if ( SEARCH_HEADER != state_ ) {
        break;
      }
//      debug_log << "start search";
    case SEARCH_HEADER:
      if ( false == parseHeader(arr) ) {
        break;
      }
      if ( TYPE_DETECT != state_ ) {
        break;
      }
//      debug_log << "header founded";
    case TYPE_DETECT:
//      if ( 0 != header_.size() ) {
//        if ( false == typeDetectByT1() ) {
//          error_log << QObject::tr("Неизвестная ошибка");
//        }
//      }
//      else {
        if ( false == typeDetect(arr) ) {
          return state_;
//          error_log << QObject::tr("Неизвестная ошибка 2");
        }
//      }
      if ( SEARCH_END == state_ ) {
//        debug_log << "type detected";
        if ( false == parseEnd(arr) ) {
          break;
        }
        return state_;
      }
      if ( SEARCH_BINARYSIZE != state_ ) {
        break;
      }
    case SEARCH_BINARYSIZE:
      if ( false == parseBinSize(arr) ) {
        break;
      }
      if ( SEARCH_BINARYPART != state_ ) {
        break;
      }
    case SEARCH_BINARYPART:
      if ( false == parseBinPart(arr) ) {
        break;
      }
      if ( SEARCH_BINARYCHUNK != state_ || true == stuckedSplit() ) {
        break;
      }
    case SEARCH_BINARYCHUNK:
      parseBinChunk(arr);
      if ( MESSAGE_PARSED == state_ ) {
        return state_;
      }
      if ( SEARCH_END!= state_ ) {
        break;
      }
    case SEARCH_END:
    default:
      if ( false == parseEnd(arr) ) {
        break;
      }
      state_ = MESSAGE_PARSED;
      return state_;
  }
  return state_;
}

void TlgMsg::loadMsgFormat()
{
//  if ( false == TMsgFormat::instance()->loadRules() ) {
//    error_log << QObject::tr("Ошибка. Не удалось загрузить форматы сообщений");
//    return;
//  }
  msgformat_ = TMsgFormat::instance()->rules();
  binformat_ = TMsgFormat::instance()->binrules();
}

bool TlgMsg::parseStartLine( QByteArray* arr )
{
  state_ = SEARCH_START_LINE;
  startline_.clear();
  header_.clear();
  message_.clear();
  end_.clear();
  binsize_ = 0;
  if ( 6 > arr->size() ) {
    toolittlearray_ = true;
    return false;
  }
  arrpos_ = 0;
  int64_t indx = -1;
  bool arrvisited = false;

  while ( false == arrvisited ) {
    if ( false == rulename_.isEmpty() ) {
      if ( false == msgformat_.contains( rulename_ ) ) {
       critical_log << QObject::tr("Не найдено правило выделения телеграмм с именем %1")
          .arg(rulename_);
        return false;
      }
      msgrule_ = msgformat_[rulename_];
      if ( false == msgrule_.bits.contains("startline") ) {
//        info_log << QObject::tr("В формате %1 нет правила для поиска начальной строки.")
//          .arg(rulename_);
        state_ = SEARCH_HEADER;
        return true;
      }
      if ( true == parseStartLine( msgrule_, arr ) ) {
        return true;
      }

    }
    QMapIterator< QString, msgformat::TMsgFormat::MsgRule > it(msgformat_);
    while ( true == it.hasNext() ) {
      it.next();
      const msgformat::TMsgFormat::MsgRule& rule = it.value();
      if ( true == rule.exclusive ) {
        continue;
      }
      if ( true == parseStartLine( rule, arr ) ) {
        return true;
      }
    }
    indx = arr->indexOf('\0');
    if ( -1 != indx ) {
      TlgMsg::saveUnparsed( arr->left(indx+1) );
      *arr = arr->mid(indx+1);
    }
    else {
      arrvisited = true;
    }
  }
  if ( TWOTLGMAXSIZE < arr->length() ) {
    TlgMsg::saveUnparsed( arr->left(TLGMAXSIZE) );
    *arr = arr->mid( TLGMAXSIZE );
  }
  state_ = SEARCH_START_LINE;
  return false;
}

bool TlgMsg::parseStartLine( const msgformat::TMsgFormat::MsgRule& rule, QByteArray* arr )
{
  startline_.clear();
  header_.clear();
  message_.clear();
  end_.clear();
  binsize_ = 0;
  int64_t indx = -1;
  const QRegExp& re = rule.bits["startline"];
  QString str(*arr);
  if ( -1 != ( indx = re.indexIn(str) ) ) {
    QString cap = re.cap();
    indx = arr->indexOf(cap);
    startline_ = cap.toUtf8();
    msgrule_ = rule;
    state_ = SEARCH_HEADER;
    arrpos_ = indx + startline_.length();
    startrule_ = rule.startline;
    return true;
  }
  return false;
}

bool TlgMsg::parseHeader( QByteArray* arr )
{
//  if ( 12 > arr->size() - arrpos_ ) {
  if ( 6 > arr->size() - arrpos_ ) {
    toolittlearray_ = true;
    return false;
  }
  header_.clear();
  message_.clear();
  end_.clear();
  binsize_ = 0;
  if ( 0 == msgrule_.headers.size() ) {
//    info_log << QObject::tr("В формате %1 нет правила для поиска начального заголовка.")
//      .arg(rulename_);
    state_ = TYPE_DETECT;
    return true;
  }
  for ( int i = 0, sz = msgrule_.headers.size(); i < sz; ++i ) {
    const QRegExp& re = msgrule_.headers[i].exp;
    QString str( arr->data() + arrpos_ );
    int indx = -1;
    if ( 0 != ( indx = re.indexIn(str) ) ) {
//      error_log << QObject::tr( "Начальный заголовок не найден" );
//      state_ = UNKNOWN_TLGSTATE;
      continue;
    }
    header_ = re.cap().toUtf8();
    arrpos_ += indx + header_.length();
    state_ = TYPE_DETECT;
    headerule_ = msgrule_.headers[i];
    return true;
  }
  error_log << QObject::tr( "Начальный заголовок не найден" ) << *arr;
  state_ = UNKNOWN_TLGSTATE;
  return false;
}

bool TlgMsg::typeDetectByT1()
{
  binsize_ = 0;
  message_.clear();
  //TODO extract header groups
  QByteArray grib("HOY");
  QByteArray bufr("IJ");
  QByteArray fax("PQ");
  char T1 = header_[0];
  if ( -1 != grib.indexOf( T1 ) ) {
    type_ = BINTLG;
    state_ = SEARCH_BINARYSIZE;
    binrule_ = binformat_["grib"];
    msgtype_ = binrule_.name;
  }
  else if ( -1 != bufr.indexOf( T1 ) ) {
    type_ = BINTLG;
    state_ = SEARCH_BINARYSIZE;
    binrule_ = binformat_["bufr"];
    msgtype_ = binrule_.name;
  }
  else if ( -1 != fax.indexOf( T1 ) ) {
    type_ = BINTLG;
    state_ = SEARCH_BINARYSIZE;
    binrule_ = binformat_["fax"];
    msgtype_ = binrule_.name;
  }
  else {
    state_ = SEARCH_END;
    type_ = TEXTLG;
    msgtype_ = "alphanum";
  }
  return true;
}

bool TlgMsg::typeDetect( QByteArray* arr )
{
  binsize_ = 0;
//  if ( 32 > arr->size() - arrpos_ ) {
  if ( 4 > arr->size() - arrpos_ ) {
    toolittlearray_ = true;
    return false;
  }
  message_.clear();
  QMapIterator< QString, msgformat::TMsgFormat::BinRule > it(binformat_);
  while ( true == it.hasNext() ) {
    it.next();
    const msgformat::TMsgFormat::BinRule& br = it.value();
    if ( arrpos_ == arr->indexOf( br.begin, arrpos_ ) ) {
//      debug_log << "BR.BEGIN SEARCH =" << br.begin;
      if ( "BUFR" == br.begin && arrpos_ == arr->indexOf(kForbiddenBufr, arrpos_) ) {
        state_ = UNKNOWN_TLGSTATE;
        return false;
      }
//      debug_log << "BR.BEGIN Ok =" << br.begin;
      type_ = BINTLG;
      state_ = SEARCH_BINARYSIZE;
      binrule_ = br;
      msgtype_ = br.name;
      return true;
    }
  }
  state_ = SEARCH_END;
  type_ = TEXTLG;
  msgtype_ = "alphanum";
  if ( false == msgrule_.msgtype.isEmpty() ) {
    msgtype_ = msgrule_.msgtype;
  }
  else {
    msgtype_ = "alphanum";
  }
  QByteArray clialphanum("y");
  char T1 = header_[0];
  if ( -1 != clialphanum.indexOf( T1 ) ) {
    type_ = TEXTLG;
    msgtype_ = "clialphanum";
  }
  return true;
}

bool TlgMsg::parseBinSize( QByteArray* arr )
{
  binsize_ = 0;
//  debug_log << "ZASHEL V SZ";
//  if ( CHUNKSIZE > arr->size() - arrpos_ ) {
  if ( 4 > arr->size() - arrpos_ ) {
    toolittlearray_ = true;
//    debug_log << "MALO DANNYH";
    return false;
  }
  if ( arrpos_ != arr->indexOf( binrule_.begin, arrpos_ ) ) {
    if ( -1 == arr->indexOf( "NIL", arrpos_ ) ) {
//      arr->remove( 1, 1 );
      state_ = UNKNOWN_TLGSTATE;
//      error_log << QObject::tr("Структура данных бинарного сообщения не соответствует правилам.");
    }
    else {
//      debug_log << "UPS";
      state_ = SEARCH_END;
    }
//    debug_log << "ERR SZ";
    return false;
  }
  int edition = 0;
  if ( -1 != binrule_.editionbyte ) {
    if ( 1 > arr->size() - arrpos_ - binrule_.editionbyte ) {
      toolittlearray_ = true;
//      debug_log << "MALO DANNYH";
      return false;
    }
    grib::char2dec( arr->data() + arrpos_ + binrule_.editionbyte, 1, &edition, true );
  }
  if ( false == binrule_.sizerules.contains(edition) ) {
//    error_log << QObject::tr("Ошибка. Нет правил поиска размера сообщения типа =")
//      << msgrule_.name << QObject::tr("номер редакции =") << edition;
    state_ = UNKNOWN_TLGSTATE;
//    arr->remove( 1, 1 );
    return false;
  }
  msgformat::TMsgFormat::SizeRule sr = binrule_.sizerules[edition];
  binsize_ = 0;
  if ( 0 == sr.first || 1 >= sr.second ) {
    binsize_ = -1;
  }
  else if ( msgformat::TMsgFormat::BinRule::LE_ORDER == sr.byteorder ) {
    if ( sr.second > arr->size() - arrpos_ - sr.first ) {
      toolittlearray_ = true;
//      debug_log << "MALO DANNYH";
      return false;
    }
    ::memcpy( &binsize_, arr->data() + arrpos_ + sr.first, sr.second );
  }
  else {
    if ( sr.second > arr->size() - arrpos_ - sr.first ) {
      toolittlearray_ = true;
//      debug_log << "MALO DANNYH";
      return false;
    }
    grib::char2dec( arr->data() + arrpos_ + sr.first, sr.second, &binsize_, true );
  }
  binsize_ += sr.addbytes;
//  debug_log << "BINSIZE =" << binsize_;
  if ( 1024*1024*100 < binsize_ ) {
    state_ = UNKNOWN_TLGSTATE;
//    error_log << QObject::tr("Размер бинарного сообщения превышает максимально допустимый");
//    arr->remove( 1, 1 );
    return false;
  }
  state_ = SEARCH_BINARYPART;
  return true;
}

bool TlgMsg::parseBinPart( QByteArray* arr )
{
  if ( arr->size() < binsize_ + arrpos_ ) {
    toolittlearray_ = true;
    return false;
  }
  if ( 0 >= binsize_ && false == binrule_.end.isEmpty() ) {
    int pos = arr->indexOf( binrule_.end, arrpos_ );
    if ( -1 == pos ) {
      toolittlearray_ = true;
      return false;
    }
    binsize_ = pos - arrpos_ + binrule_.end.size();
  }
  if ( true == splitstuckedmessages_ ) {
    message_.clear();
  }
  message_.append( arr->mid( arrpos_, binsize_ ) );
  if ( false == binrule_.end.isEmpty() ) {
    if ( message_.right( binrule_.end.length() ) != binrule_.end  ) {
//      error_log << QObject::tr( "Ошибка. Бинарная часть сообщения не соответсвует правилам.");
      arr->remove( 1, 1 );
      state_ = UNKNOWN_TLGSTATE;
      return false;
    }
  }
  arrpos_ += binsize_;
  while ( arrpos_ < arr->length() && '\000' ==  (*arr)[arrpos_] ) {
    message_.append((*arr)[arrpos_]);
    ++arrpos_;
  }
  state_ = SEARCH_BINARYCHUNK;
  return true;
}

bool TlgMsg::parseBinChunk( QByteArray* arr )
{
//  debug_log << "START CHUNK PARSE:";
  QByteArray endarr = ( false == msgrule_.endless ) ? msgformat::TMsgFormat::MsgRule::endline : QByteArray() ;
  if ( arr->length() < endarr.length() + arrpos_ ) {
    toolittlearray_ = true;
//    debug_log
//      << "Error. can't read end of bin msg arrpos = " << arrpos_
//      << "arr->length() =" << arr->length()
//      << "end = " << msgformat::TMsgFormat::MsgRule::endline.length();
    return false;
  }
  int64_t endindx = arrpos_;
  if ( 0 != endarr.size() ) {
    endindx = arr->indexOf( endarr, arrpos_ );
  }
  if ( -1 != endindx && arrpos_ + 5 > endindx ) {
    end_ = endarr;
    state_ = MESSAGE_PARSED;
    if ( endindx != arrpos_ ) {
//      warning_log << QObject::tr("Лишние символы на конце бинарного сообщения") << arr->mid( arrpos_, endindx - arrpos_ ).toHex();
      message_.append( arr->mid( arrpos_, endindx - arrpos_ ) );
    }
//    debug_log << "ERR. MES PARSED";
    *arr = arr->mid( endindx + end_.length() );
    return false;
  }
  else if ( arr->length() >= binrule_.begin.length() + arrpos_ ) {
    if ( arrpos_ == arr->indexOf( binrule_.begin, arrpos_ ) ) {
      state_ = SEARCH_BINARYSIZE;
      if ( true == splitstuckedmessages_ ) {
        end_ = msgrule_.bits["end"].pattern().toUtf8();
        end_.replace("\\r","\r");
        end_.replace("\\n","\n");
      }
//      debug_log << "POISK SIZE";
      return true;
    }
    else {
//      error_log << QObject::tr("Структура бинарного сообщения не соостветствует правилам");
//      debug_log << arrpos_ << arr->indexOf( msgformat::TMsgFormat::MsgRule::endline, arrpos_ );
//      debug_log << binrule_.name;
      arr->remove( 1, 1 );
      state_ = UNKNOWN_TLGSTATE;
      return false;
    }
  }
  else {
//    debug_log << "TTT";
    return false;
  }
//  debug_log << "ERR";
  state_ = SEARCH_END;
  return false;
}

bool TlgMsg::parseEnd( QByteArray* arr )
{
  QByteArray end = ( false == msgrule_.endless ) ? msgformat::TMsgFormat::MsgRule::endline : QByteArray() ;
  if ( true == msgrule_.bits.contains("end") ) {
    end = msgrule_.bits["end"].pattern().toUtf8();
    end.replace("\\r","\r"); //FIXME переделать MsgRule.bits
    end.replace("\\n","\n"); //FIXME переделать MsgRule.bits
  }
  if ( 0 == end.size() ) {
  }
  int64_t indx = arrpos_;
  if ( 0 != end.size() ) {
    indx = arr->indexOf( end, arrpos_ );
    if ( -1 == indx ) {
      return false;
    }
  }
  if ( indx != arrpos_ ) {
    message_.append( arr->mid( arrpos_, indx  - arrpos_ ) );
  }
  end_ = end;
  *arr = arr->mid( indx + end_.length() );
  state_ = MESSAGE_PARSED;
  return true;
}

}
