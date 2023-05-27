#include "common.h"

#include <qstring.h>
#include <qregexp.h>
#include <qcryptographichash.h>

#include <cross-commons/debug/tlog.h>
#include <commons/textproto/pbtools.h>

#include "tmsgformat.h"
#include "tlgparser.h"

namespace meteo {
namespace tlg {

static const std::string kNoPriotity("CNcn");
static const std::string kLowPriority("ADEFGHJOPQYLSadefghjopqyls");
static const std::string kNormalPriority("IKTUViktuv");
static const std::string kHighPriority("Ww");

Priority calcWMOPriority( const Header& header )
{
  if ( false == header.has_t1() ) {
    return kNoUrgent;
  }

  if ( true == kNoPriotity.find(header.t1()) ) {
    return kNoUrgent;
  }
  else if ( true == kLowPriority.find(header.t1()) ) {
    return kLow;
  }
  else if ( true == kNormalPriority.find(header.t1()) ) {
    return kNormal;
  }
  else if ( true == kHighPriority.find(header.t1()) ) {
    return kHigh;
  }
  else if ( "B" == header.t1() ) {
    std::string a1a2 = header.a1()+header.a2();
    if ( "BB" == a1a2 ) {
      return kHigh;
    }
    else if ( "RR" == a1a2 || "RQ" == a1a2 || "DA" == a1a2 ) {
      return kNormal;
    }
    else {
      return kNoUrgent;
    }
  }
  return kNoUrgent;
}

Priority priority( const MessageNew& msg )
{
  Priority p;
  switch ( msg.format() ) {
    case kWMO:
      p = calcWMOPriority( msg.header() );
      break;
    case kGMS:
      if ( 0 == msg.startline().gms_size() ) {
        p = kNoUrgent;
      }
      else {
        p = msg.startline().gms(0).priority();
      }
      break;
    case kOther:
      p = kNoUrgent;
      break;
  }
  return p;
}

MsgType detectGMSType( const MessageNew& msg )
{
  MsgType t = kMessage;
  QString t1t2 = pbtools::toQString( msg.header().t1() + msg.header().t2() );
  QRegExp adminexp(QObject::tr("[AaАа]{2}"));
  QRegExp confirmexp(QObject::tr("[BbБб]{1}[AaАа]{1}"));
  QRegExp servicexp(QObject::tr("[BbБб]{2}"));
  QRegExp convertexp(QObject::tr("[EeЕе]{2}"));
  QRegExp gmsmsgexp(QObject::tr("[MmМм]{2}"));
  QRegExp requestexp(QObject::tr("[RrРр]{2}"));
  if ( -1 != adminexp.indexIn(t1t2) ) {
    t = kAdmin;
  }
  else if ( -1 != confirmexp.indexIn(t1t2) ) {
    t = kConfirm;
  }
  else if ( -1 != servicexp.indexIn(t1t2) ) {
    t = kService;
  }
  else if ( -1 != convertexp.indexIn(t1t2) ) {
    t = kData;
  }
  else if ( -1 != gmsmsgexp.indexIn(t1t2) ) {
    t = kMessageGMS;
  }
  else if ( -1 != requestexp.indexIn(t1t2) ) {
    t = kRequest;
  }

  return t;
}

MsgType detectWMOType( const MessageNew& msg )
{
  MsgType t = kMessage;
  if ( kAddress == msg.addrmode() ) {
    QString a1a2 = pbtools::toQString( msg.header().a1() + msg.header().a2() );
    if ( "AA" == a1a2 ) {
      t = kAdmin;
    }
    else if ( "BB" == a1a2 ) {
      t = kService;
    }
    else if ( "RR" == a1a2 ) {
      t = kRequest;
    }
    else if ( "RQ" == a1a2 ) {
      t = kDbRequest;
    }
    else if ( "DA" == a1a2 ) {
      t = kData;
    }
  }
  return t;
}

MsgType msgtype( const MessageNew& msg )
{
  MsgType t;
  switch ( msg.format() ) {
    case kWMO:
      t = detectWMOType(msg);
      break;
    case kGMS:
      t = detectGMSType(msg);
      break;
    case kOther:
      t = kMessage;
      break;
  }
  return t;
}

QByteArray raw2tlg( const MessageNew& msg )
{
  QByteArray arr;
  arr += QByteArray( msg.startline().data().data(), msg.startline().data().size() );
  arr += QByteArray( msg.header().data().data(), msg.header().data().size() );
  arr += QByteArray( msg.msg().data(), msg.msg().size() );
  arr += QByteArray( msg.end().data(), msg.end().size() );
  return arr;
}

MessageNew tlg2proto( const QByteArray& arr )
{
  MessageNew proto;
  if ( 0 == arr.size() ) {
    return proto;
  }
//  if ( false == TMsgFormat::instance()->loadRules() ) {
//    error_log << QObject::tr("Ошибка. Не удалось загрузить форматы сообщений");
//    return proto;
//  }
  TlgParser parser(arr);
  parser.parseNextMessage( &proto );
  return proto;
}

QStringList targets(const MessageNew& msg)
{
  QStringList targets;

  if ( tlg::kCircular == msg.addrmode() ) {
    targets << "*";
  }
  else if ( tlg::kAddress == msg.addrmode() && tlg::kWMO == msg.format() ) {
    assert_log( msg.header().has_cccc() );

    targets << QString::fromUtf8(msg.header().cccc().c_str());
  }
  else { // address hms and multiaddres hms
    if ( msg.header().gms().has_dddd() ) {
      targets << QString::fromUtf8(msg.header().gms().dddd().c_str());
    }

    for ( int i=0,isz=msg.startline().gms_size(); i<isz; ++i ) {
      const tlg::StartGMS& gms = msg.startline().gms(i);

      QString trg;
      if ( gms.has_k1k2() ) { trg += QString::number(gms.k1k2()).rightJustified(2, '0'); }
      if ( gms.has_k3k4() ) { trg += QString::number(gms.k3k4()).rightJustified(2, '0'); }
      if ( gms.has_k5k6() ) { trg += QString::number(gms.k5k6()).rightJustified(2, '0'); }

      targets << trg;
    }
  }

  assert_log( 0 == targets.removeAll(QString()) );

  return targets;
}

QString creator(const MessageNew& msg)
{
  if ( tlg::kCircular == msg.addrmode() ) {
    assert_log( msg.header().has_cccc() );
    return QString::fromUtf8(msg.header().cccc().c_str());
  }
  else if ( tlg::kAddress == msg.addrmode() && tlg::kWMO == msg.format() ) {
    QByteArray ba = QByteArray::fromRawData(msg.msg().data(), msg.msg().size());

    if ( ba.size() < 4 ) { return QString(); }

    return ba.left(4);
  }
  else if ( tlg::kAddress == msg.addrmode() && tlg::kGMS == msg.format() ) {
    assert_log( msg.header().has_cccc() );
    return QString::fromUtf8(msg.header().cccc().c_str());
  }
  else {
    not_impl;
  }

  return QString();
}

QByteArray md5hash(const MessageNew& msg)
{
  QByteArray ba;
  ba += QByteArray( msg.header().data().data(), msg.header().data().size() );
  ba += QByteArray( msg.msg().data(), msg.msg().size() );

  return QCryptographicHash::hash(ba, QCryptographicHash::Md5);
}

QByteArray proto2tlg(const MessageNew& msg)
{
  if ( !msg.has_format() ) {
    debug_log << QObject::tr("Необходимо указать формат.");
    return QByteArray();
  }
  if ( kConvert == msg.addrmode() ) {
    debug_log << QObject::tr("Формирование сообщения типа 'конверт' не реализовано.");
    return QByteArray();
  }
  if ( kMultiAddres == msg.addrmode() ) {
    debug_log << QObject::tr("Формирование многоадресного сообщения не реализовано.");
    return QByteArray();
  }

  if ( msg.format() == kWMO || msg.format() == kOther ) {
    if ( msg.addrmode() == kCircular ) {
      QByteArray raw;

      // start line
      raw += 001;
      raw += "\r\r\n";
      if ( msg.startline().has_number() ) {
        assert_log ( msg.startline().number() >= 0 && msg.startline().number() <= 999 ) << var(msg.Utf8DebugString());
        raw += QByteArray::number(qBound(0, msg.startline().number(), 999));
      }
      else {
        raw += "999";
      }
      raw += "\r\r\n";

      // header
      raw += ( msg.header().has_t1() ) ? pbtools::toQString(msg.header().t1()).leftJustified(1, 'X', true) : "X";
      raw += ( msg.header().has_t2() ) ? pbtools::toQString(msg.header().t2()).leftJustified(1, 'X', true) : "X";
      raw += ( msg.header().has_a1() ) ? pbtools::toQString(msg.header().a1()).leftJustified(1, 'X', true) : "X";
      raw += ( msg.header().has_a2() ) ? pbtools::toQString(msg.header().a2()).leftJustified(1, 'X', true) : "X";
      raw += QByteArray::number(qBound(0, msg.header().ii(), 99)).rightJustified(2, '0');
      raw += " ";
      raw += ( msg.header().has_cccc() ) ? pbtools::toQString(msg.header().cccc()).leftJustified(4, 'C', true) : "CCCC";
      raw += " ";
      QByteArray yygggg = QDateTime::currentDateTimeUtc().toString("ddhhmm").toUtf8();
      if ( msg.metainfo().has_converted_dt() ) {
        QDateTime dt = QDateTime::fromString(pbtools::toQString(msg.metainfo().converted_dt()), Qt::ISODate);
        dt.setTimeSpec(Qt::UTC);
        if ( dt.isValid() ) { yygggg = dt.toString("ddhhmm").toUtf8(); }
      }
      raw += ( msg.header().has_yygggg() ) ? pbtools::toQString(msg.header().yygggg()).left(6) : yygggg;
      raw += ( msg.header().has_bbb() ) ? " " + pbtools::toQString(msg.header().bbb()).left(3) : "";
      raw += "\r\r\n";

      // message
      raw += QByteArray::fromRawData(msg.msg().data(), msg.msg().size());

      // end line
      raw += "\r\r\n";
      raw += 003;

      return raw;
    }
  }
  else if ( msg.format() == kGMS ) {
  }

  not_impl;

  return QByteArray();
}

QByteArray proto2image(const MessageNew& msg)
{
  QByteArray arr = proto2tlg(msg);
  return tlg2image(arr);
}

QByteArray tlg2image(const QByteArray& arr)
{
  QStringList remove_list;
  QByteArray ba(arr);
  remove_list << "MAP_RASTR" << "SPUTNIK" << "F_IMAGE";
  foreach( const QString& rm, remove_list ){
    if( true == ba.startsWith(rm.toUtf8()) ){
      ba.remove(0, rm.length());
    }
  }
  return ba;
}

QString ahd(const MessageNew& msg)
{
  QString ahd;
  switch ( msg.addrmode() ) {
    case tlg::kCircular: {
      ahd += msg.header().has_t1() ? pbtools::toQString(msg.header().t1()) : QString(" ");
      ahd += msg.header().has_t2() ? pbtools::toQString(msg.header().t2()) : QString(" ");
      ahd += msg.header().has_a1() ? pbtools::toQString(msg.header().a1()) : QString(" ");
      ahd += msg.header().has_a2() ? pbtools::toQString(msg.header().a2()) : QString(" ");
      ahd += msg.header().has_ii() ? QString::number(msg.header().ii()).rightJustified(2, '0') : QString("  ");
      ahd += " ";
      ahd += msg.header().has_cccc() ? pbtools::toQString(msg.header().cccc()) : QString("    ");
      ahd += " ";
      ahd += msg.header().has_yygggg() ? pbtools::toQString(msg.header().yygggg()) : QString("      ");
      if ( msg.header().has_bbb() ) {
        ahd += " ";
        ahd += pbtools::toQString(msg.header().bbb()).remove(" ");
      }
    } break;
    case tlg::kAddress:
    case tlg::kMultiAddres: {
      if ( tlg::kGMS == msg.format() ) {
        ahd += msg.header().has_t1() ? pbtools::toQString(msg.header().t1()) : QString(" ");
        ahd += msg.header().has_t2() ? pbtools::toQString(msg.header().t2()) : QString(" ");
        ahd += msg.header().gms().has_dddd() ? pbtools::toQString(msg.header().gms().dddd()) : QString("    ");
        ahd += " ";
        ahd += msg.header().has_cccc() ? pbtools::toQString(msg.header().cccc()) : QString("    ");
        ahd += " ";
        ahd += msg.header().has_yygggg() ? pbtools::toQString(msg.header().yygggg()) : QString("      ");
      }
      else {
        ahd += msg.header().has_t1() ? pbtools::toQString(msg.header().t1()) : QString(" ");
        ahd += msg.header().has_t2() ? pbtools::toQString(msg.header().t2()) : QString(" ");
        ahd += msg.header().has_a1() ? pbtools::toQString(msg.header().a1()) : QString(" ");
        ahd += msg.header().has_a2() ? pbtools::toQString(msg.header().a2()) : QString(" ");
        ahd += msg.header().has_ii() ? QString::number(msg.header().ii()).rightJustified(2, '0') : QString("  ");
        ahd += " ";
        ahd += msg.header().has_cccc() ? pbtools::toQString(msg.header().cccc()) : QString("    ");
        ahd += " ";
        ahd += msg.header().has_yygggg() ? pbtools::toQString(msg.header().yygggg()) : QString("      ");
        ahd += " ";
        ahd += tlg::creator(msg).leftJustified(4, ' ');
      }
    } break;
    default: {
      debug_log << QObject::tr("Определение AHD для %1 не реализовано!").arg(tlg::AddressMode_Name(msg.addrmode()).c_str());
    }
  }

  return ahd;
}

} // tlg
} // meteo
