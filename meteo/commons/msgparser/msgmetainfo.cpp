#include "msgmetainfo.h"

#include <cross-commons/debug/tlog.h>

#include <cross-commons/debug/tmap.h>
#include <commons/textproto/pbtools.h>

#include <meteo/commons/proto/msgparser.pb.h>


namespace meteo {

MsgMetaInfo::MsgMetaInfo(const QString& parseTemplate, const QString& filePath)
{
  clear();

  templ = parseTemplate;
  fileName = filePath;

  QString checkTempl = parseTemplate;
  QMapIterator<QString,int> checkIt(kLens);
  while ( checkIt.hasNext() ) {
    checkIt.next();
    checkTempl.replace(checkIt.key(), QString().fill('*', checkIt.value()));
  }

  for ( int i=0,isz=checkTempl.size(); i<isz; ++i ) {

    if ( '*' == checkTempl[i] ) { continue; }

    if ( i >= filePath.size() || checkTempl[i] != filePath[i] ) {
      debug_log << QObject::tr("Строка не соответствует шаблону:");
      debug_log << var(templ);
      debug_log << var(checkTempl);
      debug_log << var(fileName);
      parseError_ = true;
      return;
    }
  }

  // определяем позицию каждого элемента в строке-шаблоне
  QMap<int,QString> templMap; // позиция, элемент
  foreach ( const QString& key, kLens.keys() ) {
    int p = templ.indexOf(key);
    while ( -1 != p ) {
      templMap[p] = key;
      p = templ.indexOf(key, p + key.size());
    }
  }

  // определяем позицию, начиная с которой будут считываться значения элементов
  int delta = 0;
  QMap< int, QPair<QString,int> > sections; // позиция, элемент, количество символов
  foreach ( int pos, templMap.keys() ) {
    sections[pos - delta] = qMakePair(templMap[pos], kLens[templMap[pos]]);
    delta += templMap[pos].size() - kLens[templMap[pos]];
  }

  QMap<QString,QString> map;
  foreach ( int pos, sections.keys() ) {
    map[sections[pos].first] = fileName.mid(pos, sections[pos].second);
  }


  QMapIterator<QString,QString> it(map);
  while ( it.hasNext() ) {
    it.next();

    bool ok = true;
    if ( "{YYYY}" == it.key() ) {
      int v = it.value().toInt(&ok);
      year = ok ? v : -1;
    }
    else if ( "{YY}" == it.key() ) {
      int v = (QDate::currentDate().year() / 100) * 100 + it.value().toInt(&ok);
      year = ok ? v : -1;
    }
    else if ( "{MM}" == it.key() ) {
      int v = it.value().toInt(&ok);
      month = ok ? v : -1;
    }
    else if ( "{DD}" == it.key() || "{J}" == it.key() ) {
      int v = it.value().toInt(&ok);
      day = ok ? v : -1;
    }
    else if ( "{hh}" == it.key() ) {
      int v = it.value().toInt(&ok);
      hh = ok ? v : -1;
    }
    else if ( "{mm}" == it.key() ) {
      int v = it.value().toInt(&ok);
      mm = ok ? v : -1;
    }
    else if ( "{T1}" == it.key() ) {
      t1 = it.value();
    }
    else if ( "{T2}" == it.key() ) {
      t2 = it.value();
    }
    else if ( "{A1}" == it.key() ) {
      a1 = it.value();
    }
    else if ( "{A2}" == it.key() ) {
      a2 = it.value();
    }
    else if ( "{CCCC}" == it.key() ) {
      cccc = it.value();
    }
    else if ( "{ii}" == it.key() ) {
      ii = it.value();
    }
    else if ( "{YYGGgg}" == it.key() ) {
      yygggg = it.value();
    }

    if ( false == ok ) { parseError_ = true; }
  }

  if ( true == parseError_ ) {
    debug_log << QObject::tr("Ошибка при извлечении метаданных.")
              << var(templ)
              << var(checkTempl)
              << var(fileName);
  }

  if ( map.keys().contains("{J}") ) {
    QDate tmp = QDate(year, 1, 1);
    for ( int i=0; i<=366; ++i ) {
      if ( tmp.addDays(i).dayOfYear() == day ) {
        month = tmp.addDays(i).month();
        day = tmp.addDays(i).day();
        break;
      }
    }
  }

  if ( yygggg.isEmpty() ) {
    QDateTime utc = QDateTime::currentDateTimeUtc();
    int d = ( -1 == day ? utc.date().day() : day );
    int h = ( -1 == hh ? utc.time().hour() : hh );
    int m = ( -1 == mm ? utc.time().minute() : mm );
    yygggg = QString("%1%2%3").arg(d, 2, 10, QChar('0')).arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0'));
  }
}

QDateTime MsgMetaInfo::calcConvertedDt(const QDateTime& cur) const
{
  int y = ( -1 == year ? cur.date().year() : year );
  int m = ( -1 == month ? cur.date().month() : month );

  int d = cur.date().day();
  if ( !yygggg.isEmpty() && yygggg.size() == 6 ) {
    d = yygggg.left(2).toInt();
  }
  else if ( -1 != day ) {
    d = day;
  }

  int HH = cur.time().hour();
  if ( !yygggg.isEmpty() && yygggg.size() == 6 ) {
    HH = yygggg.mid(2,2).toInt();
  }
  else if ( -1 != hh ) {
    HH = hh;
  }

  int MM = -1;
  if ( !yygggg.isEmpty() && yygggg.size() == 6 ) {
    MM = yygggg.right(2).toInt();
  }
  else if ( -1 != mm ) {
    MM = mm;
  }

  QDateTime dt = QDateTime(QDate(y,m,d), QTime(HH,MM));
  if ( !dt.isValid() ) {
    if ( 1 == m ) {
      m = 12;
      y -= 1;
    }
    else {
      m -= 1;
    }
    dt = QDateTime(QDate(y,m,d), QTime(HH,MM));
  }

  if ( !dt.isValid() ) {
    debug_log << "can't parse dt:\n" << *this;
    return QDateTime();
  }

  dt.setTimeSpec( Qt::UTC );

//  // при небольшом опрежении дату формирования не пересчитываем
//  if ( cur.addSecs(60*30).secsTo(dt) <= 0 ) {
//    dt.setTimeSpec(Qt::UTC);
//    return dt;
//  }

  if ( dt.date() > cur.date() ) {
    // список TT, для которых время формирования равняется времени начала действия прогноза (т.е. телеграммы из будущего)
//    QStringList list = QStringList() << "FT" << "FC" << "FA" << "WS";

//    QString tt = t1 + t2;
//    if ( list.contains(tt, Qt::CaseInsensitive) ) {
//      dt.setTimeSpec(Qt::UTC);
//      return dt;
//    }
//    if ( d >= 25 && cur.date().day() <= 5) {
      if ( 1 == m ) {
        m = 12;
        y -= 1;
      }
      else {
        m -= 1;
      }
//    }
    dt = QDateTime(QDate(y,m,d), QTime(HH,MM));
    if ( !dt.isValid() ) {
      debug_log << "can't parse dt:\n" << *this;
      return QDateTime();
    }
  }

//  dt.setTimeSpec(Qt::UTC);
  return dt;
}

MsgMetaInfo::MsgMetaInfo(const tlg::MessageNew& msg)
{
  clear();
  update(msg);

  if ( msg.metainfo().has_converted_dt() ) {
    QDateTime dt = QDateTime::fromString(pbtools::toQString(msg.metainfo().converted_dt()), Qt::ISODate);
    if ( dt.isValid() ) {
      year = dt.date().year();
      month = dt.date().month();
    }
  }
}

void MsgMetaInfo::update(const ::meteo::tlg::MessageNew& msg)
{
  if ( msg.header().has_t1() ) { t1 = pbtools::toQString(msg.header().t1()); }
  if ( msg.header().has_t2() ) { t2 = pbtools::toQString(msg.header().t2()); }
  if ( msg.header().has_a1() ) { a1 = pbtools::toQString(msg.header().a1()); }
  if ( msg.header().has_a2() ) { a2 = pbtools::toQString(msg.header().a2()); }
  if ( msg.header().has_ii() ) { ii = QString::number(msg.header().ii()); }
  if ( msg.header().has_cccc() )   { cccc = pbtools::toQString(msg.header().cccc()); }
  if ( msg.header().has_yygggg() ) {
    yygggg = pbtools::toQString(msg.header().yygggg());
    if ( yygggg.size() == 6 ) {
      day = yygggg.left(2).toInt();
      hh = yygggg.mid(2,2).toInt();
      mm = yygggg.right(2).toInt();
    }
  }
}

tlg::Header MsgMetaInfo::getHeader() const
{
  tlg::Header header;
  if ( !t1.isEmpty()) {
    header.set_t1(t1.toStdString());
  }
  if ( !t2.isEmpty()) {
    header.set_t2(t2.toStdString());
  }
  if ( !a1.isEmpty()) {
    header.set_a1(a1.toStdString());
  }
  if ( !a2.isEmpty()) {
    header.set_a2(a2.toStdString());
  }
  if ( !ii.isEmpty()) {
    header.set_ii(ii.toInt());
  }
  if ( !cccc.isEmpty()) {
    header.set_cccc(cccc.toStdString());
  }
  if ( !yygggg.isEmpty()) {
    header.set_yygggg(yygggg.toStdString());
  }
  return header;
}

void MsgMetaInfo::clear()
{
  templ.clear();
  fileName.clear();
  year  = -1;
  month = -1;
  day   = -1;
  hh = -1;
  mm = -1;
}

}

TLog&operator<<(TLog& log, const meteo::MsgMetaInfo& info)
{
  log << "T1:" << info.t1;
  log << "T2:" << info.t2;
  log << "A1:" << info.a1;
  log << "A2:" << info.a2;
  log << "ii:" << info.ii;
  log << "cccc:" << info.cccc;
  log << "yygggg:" << info.yygggg;
  log << "year:" << info.year;
  log << "month:" << info.month;
  log << "day:" << info.day;
  log << "hours:" << info.hh;
  log << "mins:" << info.mm;
  log << "template:" << info.templ;
  log << "file:" << info.fileName;

  return log;
}

