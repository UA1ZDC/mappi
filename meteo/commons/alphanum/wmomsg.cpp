#include "wmomsg.h"
#include "wmodecoder.h"

#include "metadata.h"

#include <cross-commons/debug/tlog.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/meteo_data/meteo_data.h>

#include <qdatetime.h>
#include <qregexp.h>
#include <QRegularExpressionMatch>

using namespace meteo;
using namespace anc;

WmoMsg::WmoMsg()
{
  _codec = QTextCodec::codecForLocale();
}

WmoMsg::~WmoMsg()
{
  delete _wmoDecoder;   
  _wmoDecoder = nullptr;
    
}

//! Раскодирование телеграммы в международном формате 
int WmoMsg::parse(const QMap<QString, QString>& type, const QByteArray& ba, const QDateTime& dt)
{ 
  _idPtkpp = type.value("id");

  _dt = dt;

  _msgList.clear();
  _msgIdx = 0;
  bool ok = false;
  // if (ba.contains(_codec->fromUnicode("ПРОГНОЗ")) ||ba.contains("FORECAST")) { //FIXME так плохо, д.б. тип телеграммы
  //   ok = parseIonfo(ba, &_msgList);
  // } else
  if (type.contains("ttaaii") && type.value("ttaaii") == "obmn91") {
    int msgtype = MetaData::instance()->forms().value("GMC", -1);
    ok = parseWithoutGroup(msgtype, ba, &_msgList);
  } else {
    ok = tlg(ba, &_msgList);
  }
  if (!ok) {
    return 0;
  }

  return _msgList.count();
}

QString WmoMsg::current()
{
  int cur = _msgIdx - 1; 
  
  if (cur < 0 || cur >= _msgList.count()) {
    return QString();
  }

  return _codec->toUnicode(_msgList.at(cur).second);
}

bool WmoMsg::hasNext()
{
  return _msgIdx < _msgList.count();
}

bool WmoMsg::decodeNext()
{
  if (_msgIdx >= _msgList.count()) {
    return false;
  }

  bool ok = false;
  switch (MetaData::instance()->format(_msgList.at(_msgIdx).first)) {
  case kWmoFormat: case kIonfoTlg:
    ok = decodeWmo(_msgList.at(_msgIdx).first, _msgList.at(_msgIdx).second);
    break;
  default: {}
  }

  ++_msgIdx;
  
  return ok;
}

QString WmoMsg::code()
{ 
  return _wmoDecoder->code(); 
}

int WmoMsg::category()
{
  return _wmoDecoder->category();
}

//! Раскодирование сводки в Буквенно-цифровом коде ВМО
bool WmoMsg::decodeWmo(int type, const QByteArray& ba)
{
  _data = nullptr;

  if (0 == _wmoDecoder) {
    _wmoDecoder = new WmoDecoder(QTextCodec::codecForName("UTF-8"));
  }

  _wmoDecoder->setCaption(_caption + " " + _idPtkpp);
  bool repldt = true;
  if ( type == MetaData::instance()->forms().value("GMC") ||
       type == MetaData::instance()->forms().value("FM64") ||
       type == MetaData::instance()->forms().value("FM63") ||
       type == MetaData::instance()->forms().value("FM18") ) {
    repldt = false;
  }
//  if (_dt.isValid() && type != MetaData::instance()->forms().value("GMC")) {
  if (_dt.isValid()  ) {
    _wmoDecoder->setDateTime(_dt, repldt );
  }

  //var(MetaData::instance()->forms().key(type));
  bool ok = _wmoDecoder->decode(type, ba);
  if (ok) {
    _data = _wmoDecoder->data();
  }

  return ok;
}

QList<TMeteoData*> WmoMsg::split()
{
  return _data->takeChilds(TMeteoDescriptor::instance()->descriptor("category"));
}

//! Разбиение телеграммы на отдельные сводки
bool WmoMsg::tlg(const QByteArray& abuf, QList<QPair<int, QByteArray> >* msgList)
{
  QString string = _codec->toUnicode(abuf);
  //debug_log << "string = " << string.simplified();

  //QRegExp expr("=+\\s*\\s*(?:\x3)*\r*(?:\n|$)");
  static const QRegularExpression expr("=+(?:(?!\n|$).)*(?:\n|$)");
  QRegularExpressionMatch rmatch;

  int end = 0;  //положение конца сводки
  int prev = 0; //положение начала сводки
  QString wstr; //сводка
  QString firstline; //первая строка сводки
  int type = -1; //номер кодовой формы
  bool eol = false;
  _caption = "";

  while (!eol) {
    end = string.indexOf(expr, end + rmatch.capturedLength(), &rmatch);
    if (end == -1 || end == string.size() - 1) {
      eol = true;
      if (string.endsWith('=')) {
	end = string.size() - 1;
      } else {
	end = string.size();
      }
    }

    wstr = string.mid(prev, end - prev);

    int groupIdx = findGroup(&wstr, &firstline, &type);
    // var(groupIdx); 
    // var(type);
    // var(firstline);
    // var(wstr);

    if (-1 == groupIdx) {
      prev = end + rmatch.capturedLength();
      //debug_log << "unk" << wstr;
      continue;
    } else {
      wstr = wstr.remove(0, groupIdx);
      wstr.remove(QChar(0x3));
      wstr = wstr.simplified();

      if (wstr.count(' ') < 2 || (wstr.count(' ') <= 4 && wstr.contains("NIL")) ) {
	//debug_log << "little" << firstline <<  wstr;
	//в сводке нет групп кроме заголовка (даты, времени, индекса станции)
	prev = end + rmatch.capturedLength();
	continue;
      }
    }
    // var(wstr);
    if (kIonfoTlg == MetaData::instance()->format(type)) {
      parseGeophysics(type, wstr, msgList);
    } else {
      msgList->append(QPair<int, QByteArray> (type, wstr.toUtf8()));
    }
    
    prev = end + rmatch.capturedLength();
  }

  return true;
}

void WmoMsg::parseGeophysics(int type, const QString& wstr, QList<QPair<int, QByteArray> >* msgList)
{
  if (type == MetaData::instance()->forms().value("IONFO")) {
    parseIonfo(wstr.toUtf8(), msgList);
  } else if (type == MetaData::instance()->forms().value("GF")) {
    parseGeoph(wstr, msgList);
  } else if (type == MetaData::instance()->forms().value("HY")) {
    parseHydro(wstr, msgList);
  }
}

bool WmoMsg::parseHydro(const QString& astr, QList<QPair<int, QByteArray> >* msgList)
{
  QString str = astr;
  str = str.remove(QChar(0x3));
  str = str.simplified();

  //QString rxstr = "((?:HHXX))\\s(?:(?!HHXX|HYFOR)\\S+(?:\\s+|$))+";
  const QString rxstr = "((?:HHXX))\\s(?:[\\d/]+(?:\\s+|\\s*NIL\\s*|$))+";
  
  const static QRegExp rexp(rxstr);
  int pos = 0;

  //var(str);
  while (-1 != (pos = rexp.indexIn(str, pos))) {
    QStringList strList = rexp.capturedTexts();
    if (strList.size() < 2) {
      pos += rexp.matchedLength();
      continue;
    }
    
    // debug_log << "capturedTexts = " << strList;
    // debug_log << strList.at(1) << strList.at(0);

    int type = MetaData::instance()->subgroups().key(strList.at(1));
    
    msgList->append(QPair<int, QByteArray> (type, strList.at(0).trimmed().toUtf8()));
    
    pos += rexp.matchedLength();
  }

  //var(*msgList);
  return true;
}

bool WmoMsg::parseGeoph(const QString& astr, QList<QPair<int, QByteArray> >* msgList)
{
  QString str = astr;
  str = str.remove(QChar(0x3));
  str = str.simplified();

  const QString rxstr = "((?:AFRED|APFRD|APMOS|UMAGF|МАГКС|magks|SATIN|TENCM|TENMN|URANJ|OPPCM|UABSE|USIDS|USSPS|USSPT|USSPI|USSPY|UFLAE|UGEOA|UGEOE|UGEOI|UGEOR|IONKA|ionka))\\s(?:(?![A-Za-z]{5})\\S+(?:\\s+|$))+";
  
  static const QRegExp rexp(rxstr);
  int pos = 0;

  //var(str);
  while (-1 != (pos = rexp.indexIn(str, pos))) {
    QStringList strList = rexp.capturedTexts();
    if (strList.size() < 2) {
      pos += rexp.matchedLength();
      continue;
    }
    
    //debug_log << "capturedTexts = " << strList;
    //debug_log << strList.at(1) << strList.at(0);

    int type = MetaData::instance()->subgroups().key(strList.at(1));
    
    msgList->append(QPair<int, QByteArray> (type, strList.at(0).trimmed().toUtf8()));
    
    pos += rexp.matchedLength();
  }
  
  return true;
}

bool WmoMsg::parseIonfo(const QByteArray& abuf, QList<QPair<int, QByteArray> >* msgList)
{
  QString string = _codec->toUnicode(abuf);
  static const QRegExp expr("(?:ПРОГНОЗ|FORECAST)\\s*\\d{5}");
  static const QRegExp group("(?!\\s*(?:IONFO|IONES|IONDP|IONFF|FORECAST|MAGPO|ИОНФО|ИОНЕС|ИОНДП|ИОНФФ|МАГПО|ПРОГНОЗ)\\s*)\\s[\\D]+(?:\\s|$)");

  int end = 0;  //положение конца сводки
  int prev = 0; //положение начала сводки
  QString wstr; //сводка
  QString firstline; //первая строка сводки
  bool eol = false;
  _caption = "";
  int type = MetaData::instance()->forms().value("IONFO");//номер кодовой формы
  
  prev = string.indexOf(expr, 0);
  end  = prev;

  while (!eol) {
    end = string.indexOf(expr, end + 1);
    //var(end);
    if (end == -1 || end == string.size() - 1) {
      eol = true;
      end = string.size();
    }

    wstr = string.mid(prev, end - prev);
    //var(wstr);

    wstr.remove(QChar(0x3));
    wstr = wstr.simplified();

    // if (eol) {
      int cend = wstr.indexOf(group, 0);
      wstr = wstr.mid(0, cend);
      //debug_log << prev << end << wstr;
      // }
   
    if (wstr.count(' ') < 4) {
      //debug_log << "little" << firstline <<  wstr;
      //в сводке нет групп кроме заголовка (даты, времени, индекса станции)
      prev = end + 1;
      continue;
    }
    
    //var(wstr);
    
    msgList->append(QPair<int, QByteArray> (type, wstr.toUtf8()));
    
    prev = end;
  }
  
  // var(msgList->count());

  return true;
}

//! сводки без идентификационных символов
bool WmoMsg::parseWithoutGroup(int type, const QByteArray& abuf, QList<QPair<int, QByteArray> >* msgList)
{
  QString string = _codec->toUnicode(abuf);
  //debug_log << "string = " << string.simplified();

  static const QRegExp expr("=+\\s*(?:\x10)?\r*(?:\n|$)");

  int end = 0;  //положение конца сводки
  int prev = 0; //положение начала сводки
  QString wstr; //сводка
  QString firstline; //первая строка сводки
  bool eol = false;
  _caption = "";

  while (!eol) {
    end = string.indexOf(expr, end + 1);
    if (end == -1 || end == string.size() - 1) {
      eol = true;
      if (string.endsWith('=')) {
	end = string.size() - 1;
      } else {
	end = string.size();
      }
    }

    wstr = string.mid(prev, end - prev);

    //var(wstr);

    wstr.remove(QChar(0x3));
    wstr = wstr.simplified();

    //var(wstr);
    
    if (wstr.count(' ') < 10) {
      //debug_log << "little" << firstline <<  wstr;
      //в сводке не достаточно групп
      prev = end + 1;
      continue;
    }  
    
    // var(wstr);
    
    msgList->append(QPair<int, QByteArray> (type, wstr.toUtf8()));
    
    prev = end + 1;
  }

  return true;
}

//! проверка сообщения на совпадение с форматом начала сводки
int WmoMsg::checkReportHeader(const QString& str, int type)
{
  int groupIdx = -1;
  QList<QRegExp> MiMiMjMj = MetaData::instance()->headers(type);
  
  for (auto rx : MiMiMjMj) {
    groupIdx = rx.indexIn(str);
    if (-1 != groupIdx) break;
  }
      
  return groupIdx;
}

//! Определение типа сводки (поиск начала сводки, добавление первой строки при необходимости)
int WmoMsg::findGroup(QString* wstr, QString* firstline, int* type)
{
  // int groupIdx = findWmoFormatGroup(*wstr, firstline, type);
  // if (groupIdx == -1) {
  //   groupIdx = findGroupInAllMsg(*wstr, firstline, type);
  // }

  int groupIdx = findGroupInAllMsg(*wstr, firstline, type);
  
  if (-1 == groupIdx && !firstline->isEmpty() && !wstr->endsWith("NIL")) {
    wstr->remove(QChar(0x3));    
    *wstr = wstr->simplified();
    if (wstr->count(' ') > 1) {       //проверка на пустое сообщение или мало групп (только индекс станции)
      wstr->prepend(*firstline + " ");
      groupIdx = checkReportHeader(*wstr, *type);
    }
  }

  return groupIdx;
}

//! поиск заголовка в формате WMO, за которым идет тип сводки
// int WmoMsg::findWmoFormatGroup(const QString& str, QString* firstline, int* type)
// {
//   bool ok = false;
//   var(str);
//   if (str.length() + firstline->length() < 20) {
//     //мало для заголовка
//     return -1;
//   }

//   int number = 0;
//   QList<int> keys = MetaData::instance()->headers().uniqueKeys();
//   QList<QByteArray> groups = MetaData::instance()->headers().values();

//   QRegExp rx("\\w{4}\\d{2}\\s\\w{4}\\s\\d{6}(?:\\s\\w{3})?\r{1,2}\n");

//   number = rx.indexIn(str);
//   var(number);
//   if (number == -1) return -1;

//   number += rx.matchedLength();
//   var(number);

//   for (int i = 0; i < 2; i++) {
//     int end = str.indexOf(' ', number);
//     var(end);
//     QByteArray w = str.mid(number, end-number).simplified().toUtf8();
//     var(w);
//     //var(*firstline);
//     if (groups.contains(w)) {
//       if (firstline->isEmpty()) {
// 	*firstline = str.mid(number, str.indexOf('\x0A', number) + 1 - number).simplified();//считываем первую строку с группой  MiMiMjMj
// 	if (firstline->length() < 10 || firstline->length() > 30) {
// 	  *firstline = "";
// 	}
//       }
//       //var(*firstline);
//       if (_caption.isEmpty()) {
// 	_caption = str.left(number).simplified();
// 	//var(_caption);
//       }
//       *type = MetaData::instance()->headers().key(w);
//       var(MetaData::instance()->format(*type));
//       ok = true;
//       break;
//     }

//     number = end + 1;
//   }

//   if (!ok) number = -1;

//   return number;
// }

//! поиск начала сводки по всему сообщению
int WmoMsg::findGroupInAllMsg(const QString& str, QString* firstline, int* type)
{
  if (str.length() < 10) {
    //мало для заголовка
    return -1;
  }

  int number = 0;

  QList<QRegExp> groups;//code groups TTAA, TTBB, ... e. a.
  QList<int>::iterator it;

  QList<int> keys;
  if (*type != -1) {
    //сначала проверяем с предыдущим типом, т.к. обычно в одной телеграмме один тип сводок
    keys << *type;
  }
  
  keys << MetaData::instance()->uniqHeaderKeys();

  for(it = keys.begin(); it != keys.end(); ++it) { //перебор по кодовым формам
    groups = MetaData::instance()->headers(*it);
    //debug_log << *it << "groups:"<< groups;
    for (auto MiMiMjMj : groups) { //перебор по разделам (A,B,C,D) каждой кодовой формы
      //var(groups.at(i));
      number = 0;
      if(-1 < (number = str.indexOf(MiMiMjMj, number))) {
	//var(number);

	if (firstline->isEmpty()) {
	  *firstline = str.mid(number, str.indexOf('\x0A', number) + 1 - number).simplified();//считываем первую строку с группой  MiMiMjMj
	  if (firstline->length() < 4 || firstline->length() > 30) {
	    *firstline = "";
	  }
	}
	//var(*firstline);
	if (_caption.isEmpty()) {
	  _caption = str.left(number).simplified();
	}
	*type = *it;
	//var(*type);
	break;
      }
    }
    if (number != -1) break;
  }

  return number;
}

