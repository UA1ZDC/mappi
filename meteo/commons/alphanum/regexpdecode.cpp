#include "regexpdecode.h"
#include "metadata.h"
#include "tmeteoelement.h"
#include "xmllinedecode.h"

#include <cross-commons/debug/tlog.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <qhash.h>
#include <qdom.h>
#include <qlist.h>

#include <commons/mathtools/mnmath.h>

using namespace meteo;
using namespace anc;

RegExpDecode::RegExpDecode(): 
  _stationIdx(-1),
  _diffUtc(0),
  _replaceDt(false)
{
}

RegExpDecode::~RegExpDecode()
{
  clear();
}

void RegExpDecode::clear()
{
  // _childs.clear();
  // _inchilds.clear();
  delete _lineDecoder;
  _lineDecoder = nullptr;
  
  delete _data;
  _data = nullptr;
  // _data.clear();
  _code.clear();


  _endWith = QString();
  _category = -1;
}

//! инициализация при получении новой сводки для раскодирования
void RegExpDecode::init()
{
  clear();
  _data = new TMeteoData;
  _lineDecoder = new XmlLineDecoder;
}


bool RegExpDecode::decode(int code, const QByteArray& report)
{
  init();

  if (!MetaData::instance()->isCodeLoaded(code)) {
#ifdef PARSE_DEBUG_LOG
    debug_log << code << " not loaded";
#endif
    return false;
  }
  //var(code);
  const QDomDocument &dec = MetaData::instance()->domDecoder(code);
  _code = dec.documentElement().attribute("type");
  _category = dec.documentElement().attribute("category", "-1").toInt();
  _endWith = dec.documentElement().attribute("end");

  //  debug_log << "decode " << code << _code;
  MetaData::instance()->setMeteoData(_data);

  if (! decode(report, dec)) {
#ifdef PARSE_LOG
    error_log << QObject::tr("Ошибка раскодирования сводки '%1'. Заголовок:'%2'").arg(_code).arg(_caption);
#endif
    _dt = QDateTime();
    _stationName = QString();
    _stationIdx = -1;
    return false;
  }

  if (_data->count() == 0) {
    info_log << QObject::tr("Пустая сводка '%1'. Заголовок:'%2'").arg(_code).arg(_caption);
    return true;
  }

  _data->set(MetaData::instance()->dataElements()->getDescriptor("category"),
             QString::number(_category),
             _category,
             control::RIGHT);
  _data->set(MetaData::instance()->dataElements()->getDescriptor("level_type"),
             "1",
             1,
             control::RIGHT);

  convertMeteoParams();

  /*bool ok = */  fillIdentification(); //TODO

  _dt = QDateTime();
  _stationName = QString();
  _stationIdx = -1;
  _replaceDt = false;

  return true;
}

//! Заполнение данных идентификации (координаты, дата/время)
/*! 
  \return
*/
bool  RegExpDecode::fillIdentification()
{
  if (! _stationName.isEmpty()) {
    _data->set(MetaData::instance()->dataElements()->getDescriptor("station"),
               _stationName,
               BAD_METEO_ELEMENT_VAL,
               control::RIGHT);
  }


  if ( _stationIdx > 0 ) {
    _data->setStation(_stationIdx);
  } else {
    int stindex =  _data->getValue(TMeteoDescriptor::instance()->descriptor("station_index"), -1, false);
    if (-1 != stindex) {
      _data->setStation(stindex);
    } else {
      int fullIndex = TMeteoDescriptor::instance()->station(*_data, -1);
      if (fullIndex > 0) {
        _data->set(TMeteoDescriptor::instance()->descriptor("station_index"),
                   TMeteoParam(QString::number(fullIndex), fullIndex, control::RIGHT));
      }
    }
  }

  int buoy = TMeteoDescriptor::instance()->buoy(*_data, -1);
  if (buoy != -1) {
    _data->set(TMeteoDescriptor::instance()->descriptor("buoy"),
               TMeteoParam(QString::number(buoy), buoy, control::RIGHT));
  }

  int mountainLev = TMeteoDescriptor::instance()->mountainLevel(*_data);
  if (BAD_METEO_ELEMENT_VAL != mountainLev) {
    _data->set(TMeteoDescriptor::instance()->descriptor("ims"),
               TMeteoParam("1", 1, control::RIGHT));
  }
  
  if (! setDataMonthYear()) {
    return false;
  }

  convertCoords();

  return true;
}

//! преобразование метеопараметров и добавление к метеоданным
void RegExpDecode::convertMeteoParams()
{ 
  if (_data->hasParam(TMeteoDescriptor::instance()->descriptor("U"))) {
    return;
  }

  const TMeteoParam& T = _data->getParam(TMeteoDescriptor::instance()->descriptor("T"));
  const TMeteoParam& Td = _data->getParam(TMeteoDescriptor::instance()->descriptor("Td"));
  if ( T.isInvalid() || Td.isInvalid() || T.value() < Td.value()) {
    return;
  }

  float Uval = MnMath::Td2U(T.value(), Td.value());

  TMeteoParam U("", Uval, T.quality());
  U.setQuality(Td.quality(), Td.controlFlag(), false); //чтоб был худший показатель

  _data->add(TMeteoDescriptor::instance()->descriptor("U"), U);
}


//---

/**
 * Декодирует сообщение в буквенно-цифровом коде (КН-01,КН-04) с использованием декодера на основе файла-словаря с
 * регулярными выражениями regexp в формате XML

 * @param node Текущий узел DOM - модели файла-словаря
 * @param msg   Сообщение
 * @return true - если всё норм
 */
bool RegExpDecode::decodeNode(const QDomNode& node, const QString &msgSrc)
{
  QRegExp rexp;
  QString msg = msgSrc;

#ifdef PARSE_DEBUG_LOG
  debug_log << _code << node.toElement().tagName() << node.hasChildNodes();// << "str=" << msg;
#endif

  if(!node.hasChildNodes() || "" == msg.trimmed()) { return true;}

  QList<QDomNode> nodeDecodersList;  //список дочерних узлов с декодерами
  QDomNode child = node.firstChild();
  bool addChild = false;
  int categ = -1;
  if (child.isElement() && child.toElement().tagName() == "item") {
    addChild = true;
    if (child.toElement().hasAttribute("category")) {
      categ = child.toElement().attribute("category").toInt();
    }
    child = child.firstChild();
  }
  
  //список дочерних узлов с декодерами (с выбором нужного варианта при необходимости)
  getDecodersList(child, &nodeDecodersList);
  

  //получение (из дочернего узла с индексом 0) регулярного(ых) выражения(ий) для текущей строки msg
  if(!getRexpFromFirstNode(nodeDecodersList.at(0), rexp)) {
#ifdef PARSE_DEBUG_LOG
    debug_log << QObject::tr("Ошибка получения регулярного выражения");
#endif
    return false;
  }

#ifdef PARSE_DEBUG_LOG
  //debug_log << "str= " << msg;
  debug_log << rexp.pattern();
#endif
  
  if(-1 == rexp.indexIn(msg)) {
#ifdef PARSE_LOG
    error_log << QObject::tr("Несоответствие регулярному выражению") << msg;
#endif
#ifdef PARSE_DEBUG_LOG
    // QString indicatorValueCode = _data->getCode(MetaData::instance()->dataElements()->getDescriptor("iR"));
    // debug_log << "Node name: " << node.nodeName() <<   node.toElement().attribute(NUMBER_ATTRIB) << "iR = " << indicatorValueCode;
#endif
    
    return false;
  }

  return decodeNode(nodeDecodersList, rexp, msg, addChild, categ);
}

//! 
/*! 
  
  \param nodeDecodersList Список дочерних узлов с декодерами
  \param rexp Регулярное выражение, соответствующее nodeDecodersList
  \param msg Сообщение
  
  \return false в случае ошибки
*/
bool RegExpDecode::decodeNode(const QList<QDomNode>& nodeDecodersList, const QRegExp& rexp, const QString& msg, bool addChild, int categ)
{
  int pos = 0;
  bool end = false;
  QString cap;

  //цикл поиска во всей строке msg повторяющихся регулярных выражений regexp (например - главные и особые точки в телеграмме КН-04)
  while (-1 != (pos = rexp.indexIn(msg, pos))) {
    //исключение пустых строк и пустых подстрок-серий (когда pos указывает на конец строки,
    //а regexp - мягкий, например "\\d?") из обработки
    if ("" == rexp.cap()) break;
    _lineDecoder->startBlock();
    //    _inchilds.clear();

    QStringList strList = rexp.capturedTexts();
    cap = rexp.cap();

#ifdef PARSE_DEBUG_LOG
    //    debug_log << rexp.pattern();
    debug_log << "msg = " << msg.right(msg.size() - pos);
    debug_log << "capture = " << rexp.cap();
    debug_log << "capturedTexts = " << strList;
#endif

    //проверка (в текущем узле XML) соовтетствия количества подузлов количеству серий (captures)
    //в регулярном выражении (rexp) для данного узла
    if(nodeDecodersList.count() != strList.count()) {
      error_log << QObject::tr("Ошибка структуры файла для сводки") << _code;
      
      // var(nodeDecodersList.count());
      // var(strList.count());
      // for (int i = 0; i < nodeDecodersList.count(); i++) {
      //   debug_log << nodeDecodersList.at(i).toElement().attribute("number") << nodeDecodersList.at(i).toElement().tagName();
      // }
      return false;
    }

    //проверка, если не захвачены никакие переменные
    bool isEmpty = true;
    for (int idx = 1; idx < strList.count(); ++idx) {
      if (!strList.at(idx).isEmpty()) {
        isEmpty = false;
        break;
      }
    }
    if (isEmpty) {
      end = true;
      break;
    }

    if (addChild) {
#ifdef PARSE_DEBUG_LOG
      debug_log << "addChild";
#endif
      _data = &_data->addChild();

      if (-1 != categ) {
        _data->set(MetaData::instance()->dataElements()->getDescriptor("category"),
                   QString::number(categ),
                   categ,
                   control::RIGHT);
      }
    }

    //рекурсивный вызов функции для каждого из дочерних узлов и подстрок (captures)
    for (int i = 1; i < nodeDecodersList.count(); i++) {
      //условие выполнения рекурсии
      if (nodeDecodersList.at(i).hasChildNodes() &&
          !nodeDecodersList.at(i).toElement().hasAttribute("descrName")) {

#ifdef PARSE_DEBUG_LOG
				debug_log << nodeDecodersList.at(i).toElement().tagName()
									<< "number="
		  << nodeDecodersList.at(i).toElement().attribute("number");
#endif

				bool ok = decodeNode(nodeDecodersList.at(i), strList.at(i));
				if (!ok) return ok;
			} else {
				if (!(strList.at(i).isEmpty() && !nodeDecodersList.at(i).toElement().hasAttribute("default"))) {
					bool ok = _lineDecoder->setMeteoDataValue(_code, nodeDecodersList.at(i), strList.at(i), _data);
					if (!ok) return ok;
				}
			}
		}
		pos += rexp.matchedLength();
		if (pos >= msg.trimmed().size()) {
			end = true;
		}

		if (addChild && _data->hasParent()) {
			_data = _data->parent();
		}
	}

	if (end == false) {
#ifdef PARSE_DEBUG_LOG
		error_log << QObject::tr("Несоответствие регулярному выражению.\n msg='%1'\n cap='%2'").arg(msg).arg(cap);
#endif
    return false;
  }



  return true;
}

//! Сохранение раскодированной метеовеличины
/*! 
  \param node Узел описывающий метеовеличину
  \param str  Метеовеличина
*/
// bool RegExpDecode::setMeteoDataValue(const QDomNode& node, const QString& astr)
// {
//   if (!node.isElement()) {
//     error_log<<QObject::tr("Ошибка структуры xml %1").arg(_code);
//     return false;
//   }

//   QDomElement elem = node.toElement();

//   //получение значения дескриптора descriptor из атрибутов i-го узла 
//   QString descr = elem.attribute("descrName");
// #ifdef PARSE_DEBUG_LOG
//   debug_log<<"descr="<<descr;
// #endif

//   if (descr.isEmpty()) {
//     error_log<<QObject::tr("Ошибка xml %1. Неверный дескриптор=%2 (number=%3, text=%4)").
//       arg(_code).arg(elem.attribute("descrName")).arg(elem.attribute("number")).
//       arg(elem.attribute("text"));
//     return false;
//   }

//   if (MetaData::instance()->dataElements()->contains(descr)) {
//     meteo::ElementConvertType ctype = MetaData::instance()->dataElements()->convertType(descr);

//     if (ctype == meteo::COMPLEX_CONVERT) {
//       TMeteoData* cur = currentMeteoData(elem);
//       if (0 == cur || !MetaData::instance()->dataElements()->parseComplex(descr, astr, cur)) {
// #ifdef PARSE_LOG
// 	error_log << QObject::tr("Ошибка раскодирования дескриптора '%1'").arg(descr);
// #endif
//       }
//     } else {
//       control::QualityControl quality = control::NO_CONTROL;
//       float val = BAD_METEO_ELEMENT_VAL;
//       QString str = astr;

//       if (ctype == meteo::TRANSLIT_CONVERT) {
// 	str = global::translitFuncKoi7(astr);
// 	val = MetaData::instance()->bufrcode(str);
//       } else {

// 	if (MetaData::instance()->dataElements()->outName(descr) != "text") {
// 	  str = str.trimmed();
// 	}
// 	if (str.isEmpty() && elem.hasAttribute("default")) {
// 	  str = elem.attribute("default");
// 	}
// 	//численная интерпретация строкового значения в коде (давление PPPP, температура SnTTT...)  
// 	val = MetaData::instance()->dataElements()->getValue(descr, str, &quality); 

// 	if (quality == control::MISTAKEN) {
// 	  //warning_log << QObject::tr("Ошибка семантического контроля. Сводка '%1'").arg(_caption); TODO обратно раскомментить
// 	}	
//       }
// #ifdef PARSE_DEBUG_LOG
//       debug_log<<" DESCR=" << MetaData::instance()->dataElements()->getDescriptor(descr) << descr << str << val << quality;
// #endif
//       //--
//       if (elem.hasAttribute("num")) {
// 	bool ok;
// 	int num = elem.attribute("num").toInt(&ok);
// 	if (!ok) {
// 	  error_log<<QObject::tr("Ошибка xml %1. Неверный номер для связи значений=%2 (number=%3, text=%4)").
// 	    arg(_code).arg(num).arg(elem.attribute("number")).arg(elem.attribute("text"));
// 	  return false;
// 	}
// 	TMeteoData* child;
// 	if (_childs.contains(num)) {
// 	  child = _childs.value(num);
// 	} else {
// 	  child = &_data->addChild();
// 	  _childs.insert(num, child);
// 	}
// 	if (0 != child) {
// 	  child->add(MetaData::instance()->dataElements()->getDescriptor(descr), str, val, quality);
// 	}
//       } else if (elem.hasAttribute("innum")) {
// 	bool ok;
// 	int innum = elem.attribute("innum").toInt(&ok);
// 	if (!ok) {
// 	  error_log<<QObject::tr("Ошибка xml %1. Неверный номер для связи внутренних значений=%2 (number=%3, text=%4)").
// 	    arg(_code).arg(innum).arg(elem.attribute("number")).arg(elem.attribute("text"));
// 	  return false;
// 	}
// 	TMeteoData* child;
// 	if (_inchilds.contains(innum)) {
// 	  child = _inchilds.value(innum);
// 	} else {
// 	  child = &_data->addChild();
// 	  _inchilds.insert(innum, child);
// 	}
// 	if (0 != child) {
// 	  child->add(MetaData::instance()->dataElements()->getDescriptor(descr), str, val, quality);
// 	}
//       } else {
// 	_data->add(MetaData::instance()->dataElements()->getDescriptor(descr), str, val, quality);
//       }
//     }

//   } else {
//     error_log << QObject::tr("Нет описания локального дескриптора") << descr;
//   }
//   return true;
// }

// TMeteoData* RegExpDecode::currentMeteoData(QDomElement elem)
// {
//   if (elem.hasAttribute("num")) {
//     bool ok;
//     int num = elem.attribute("num").toInt(&ok);
//     if (!ok) {
//       error_log<<QObject::tr("Ошибка xml %1. Неверный номер для связи значений=%2 (number=%3, text=%4)").
// 	arg(_code).arg(num).arg(elem.attribute("number")).arg(elem.attribute("text"));
//       return 0;
//     }
//     TMeteoData* child;
//     if (_childs.contains(num)) {
//       child = _childs.value(num);
//     } else {
//       child = &_data->addChild();
//       _childs.insert(num, child);
//     }
//     if (0 != child) {
//       return child;
//     }
//   } else if (elem.hasAttribute("innum")) {
//     bool ok;
//     int innum = elem.attribute("innum").toInt(&ok);
//     if (!ok) {
//       error_log<<QObject::tr("Ошибка xml %1. Неверный номер для связи внутренних значений=%2 (number=%3, text=%4)").
// 	arg(_code).arg(innum).arg(elem.attribute("number")).arg(elem.attribute("text"));
//       return 0;
//     }
//     TMeteoData* child;
//     if (_inchilds.contains(innum)) {
//       child = _inchilds.value(innum);
//     } else {
//       child = &_data->addChild();
//       _inchilds.insert(innum, child);
//     }
//     if (0 != child) {
//       return child;
//     }
//   }

//   return _data;
// }

//получение (из дочернего узла с индексом 0) регулярного(ых) выражения(ий) для текущей строки
bool RegExpDecode::getRexpFromFirstNode(const QDomNode& node, QRegExp& rexp)
{
  bool ok = true;

#ifdef PARSE_DEBUG_LOG
  trc;
#endif
  if(!node.isElement()) {
    return false;
  }
  //получение дескриптора для индикатора используемого варианта регулярного выражения
  QString indicatorDescr = node.toElement().attribute("i_descrName");

#ifdef PARSE_DEBUG_LOG
  var(indicatorDescr);
#endif
  
  if(!indicatorDescr.isEmpty()) {
    //выбор регулярного выражения из имеющихся вариантов
    ok = setRexpVariant(indicatorDescr, node.childNodes(), rexp);
    //var(ok);
  } else {
    //получение (из дочернего узла с индексом 0) регулярного выражения для текущей строки
    rexp.setPattern(node.toElement().text());
  }

  //debug_log << "regexp(0) = " << nodeDecodersList.at(0).toElement().text(); //TEST
  return ok;
}


//! Получение списка возможных вариантов регулярного выражения и выбор подходящего в соответствии с значением индикатора
/*! 
  
  \param indicatorDescr Название дескриптора-индикатора варианта регулярного выражения
  \param rexpNodesList  Список узлов с вариантами регулярных выражений
  \param rexp
  
  \return false - в случае ошибки
*/
bool RegExpDecode::setRexpVariant(const QString& indicatorDescr, const QDomNodeList& rexpNodesList, QRegExp& rexp)
{
  QString pattern, indicatorValuesPattern;
  QRegExp indicatorValuesRexp;

  // _data->printData();
  
  const TMeteoParam& indicator = _data->getParam(MetaData::instance()->dataElements()->getDescriptor(indicatorDescr));
  bool okXml = true;

  if(indicator.isInvalid()) {
    debug_log << QObject::tr("Ошибочное значение указателя %1: '%2' (качество='%3')").
                 arg(indicatorDescr).arg(indicator.code()).arg(indicator.quality());
    return false;
  }

  for(int idx = 0; idx < rexpNodesList.count(); idx++) {
    //TODO может быть имеет смысл сделать выбор "дефолтного" варианта регулярного выражения
#ifdef PARSE_DEBUG_LOG
    printvar(idx);
#endif
    
    if("" != (indicatorValuesPattern = rexpNodesList.at(idx).toElement().attribute("i_values"))) {
      //выбор регулярного выражения для раскодирования по соответствию значения индикатора регулярному выражению

      indicatorValuesRexp.setPattern(indicatorValuesPattern);
      
      if(-1 != indicatorValuesRexp.indexIn(indicator.code())) {
        //debug_log << "section 333 : " << indicatorDescr << indicatorValueCode << indicatorValuesRexp.pattern();
        rexp.setPattern(rexpNodesList.at(idx).toElement().text());
        break;
      }
      
    } else {
      okXml = false;
      break;
    }
    
#ifdef PARSE_DEBUG_LOG
    printvar(indicator.code());
#endif
    
  }
  
  if (!okXml) {
    error_log << QObject::tr("Ошибка структуры xml %1. Не указаны значения для выбора варианта регулярного выражения").arg(_code);
    return false;
  }

  return true;
}

//! список дочерних узлов с декодерами (с выбором нужного варианта при необходимости)
void RegExpDecode::getDecodersList(const QDomNode& node, QList<QDomNode>* nodeDecodersList)
{
#ifdef PARSE_DEBUG_LOG
  trc;
  debug_log << node.toElement().tagName() << node.hasChildNodes();
#endif

  QDomNode child = node;
  while (! child.isNull()) {
    if (child.isElement()) {
      break;
    }
    child = child.nextSibling();
  }

  if (child.isNull() || !child.isElement()) {
    return;
  }
  
  QString variantDescr = child.toElement().attribute("var_descrName");
  if (!variantDescr.isEmpty()) {
    child = getVariantDecodersList(child.childNodes());
  }
  
  while (! child.isNull()) {
    if (child.isElement()) {
      nodeDecodersList->append(child);
    }
    child = child.nextSibling();
  }
}

//! Поиск варианта раскодирования в соотв-ии с номером страны. nodes - родительские узлы разных вариантов раскодирования
QDomNode RegExpDecode::getVariantDecodersList(const QDomNodeList& nodes)
{
#ifdef PARSE_DEBUG_LOG
  trc;
#endif
  bool okXml = true;
  QString station = TMeteoDescriptor::instance()->stationIdentificator(*_data);
  QString country = MetaData::instance()->currentCountry(_category, station);
  QDomNode varNode;
  
  for(int idx = 0; idx < nodes.count(); idx++) {
    if (nodes.at(idx).isNull() || !nodes.at(idx).isElement()) { continue; }

#ifdef PARSE_DEBUG_LOG
    debug_log <<  nodes.at(idx).toElement().tagName() << nodes.at(idx).hasChildNodes();// << "str=" << msg;
#endif
    
    
    QString nodeCountry;
    if("" != (nodeCountry = nodes.at(idx).toElement().attribute("var_values"))) {
      if (nodeCountry == "default" || nodeCountry == country) {
        varNode =  nodes.at(idx);
        break;
      }
      
    } else {
      okXml = false;
    }
  }
  
  if (!okXml) {
    error_log << QObject::tr("Ошибка структуры xml %1. Не указаны значения для выбора варианта раскодирования").arg(_code);
  }

  return varNode.firstChild();
}


//TODO в meteo_data ?
QDateTime RegExpDecode::dateTime()
{ 
  return TMeteoDescriptor::instance()->dateTime(*_data);
}


//! Заполнение полной даты/времени формирования метеоданных
/*! 
  \return false - в случае ошибки
*/
bool RegExpDecode::setDataMonthYear() 
{
  int year, month;
  int day, hour, minute;
  control::QualityControl qual;

  QDateTime datetimeData;
  QDateTime datetimeSystem;
  if (_dt.isValid()) {
    datetimeSystem = _dt;
    datetimeData = _dt; //NOTE чтоб время было таким, как задали извне, а не из сводки
  } else {
    datetimeSystem = QDateTime::currentDateTimeUtc();
  }

  if ( !datetimeData.isValid() || !_replaceDt ) {

    if (!_data->getValue(MetaData::instance()->dataElements()->getDescriptor("MM"), &month, &qual)) {
      month = datetimeSystem.date().month();
    }
    
    if (!_data->getValue(MetaData::instance()->dataElements()->getDescriptor("JJJ"), &year, &qual)) {
      year = datetimeSystem.date().year();
    } else if (year / 10 == 0) {
      year += (datetimeSystem.date().year() / 10) * 10;
    } else if (year / 100 == 0) {
      year += (datetimeSystem.date().year() / 100) * 100;
    } else if (year / 1000 == 0) {
      year += (datetimeSystem.date().year() / 1000) * 1000;
    }
    
    if(!_data->getValue(MetaData::instance()->dataElements()->getDescriptor("YY"), &day, &qual)) {
      if (_dt.isValid()) {
        day = _dt.date().day();
      } else {
        day = datetimeSystem.date().day();
        //error_log << QObject::tr("Нет информации о времени образования данных");
        //return false;
      }
    }
    
    if( !_data->getValue(MetaData::instance()->dataElements()->getDescriptor("GG"), &hour, &qual)) {
      if (_dt.isValid()) {
        hour = _dt.time().hour();
      } else {
        hour = datetimeSystem.time().hour();
        // error_log << QObject::tr("Нет информации о времени образования данных");
        // return false;
      }
    }
    if(!_data->getValue(MetaData::instance()->dataElements()->getDescriptor("gg"), &minute, &qual)) {
      minute = 0;
      // if (_dt.isValid()) {
      //   minute = _dt.time().minute();
      // }
    }
    
    if (day > 50) day -= 50; //в некоторых сводках прибавляется 50 для указания единиц измерения ветра
    
    datetimeData.setTime(QTime(hour, minute, 0));
    datetimeData.setDate(QDate(year, month, day));
    
    //-10*60 = -10 мин, если время не точно идет, чтоб месяц не изменить
    if(datetimeData.addSecs(-_diffUtc*3600-10*60) > datetimeSystem || !datetimeData.isValid()) {
      datetimeData = datetimeData.addMonths(-1);
    }
    
    if (_diffUtc != 0) {
      _data->add(MetaData::instance()->dataElements()->getDescriptor("diff_utc"),
                 TMeteoParam(QString::number(_diffUtc),
                             _diffUtc,
                             control::RIGHT),
                 0);
    }
    
    if (!datetimeData.isValid()) {
      // error_log << QObject::tr("Некорректная дата h=%1 min=%2 y=%3 m=%4 d=%5").
      // 	arg(hour).arg(minute).arg(year).arg(month).arg(day);
      return false;
    }
    
  }
  
  _data->add(MetaData::instance()->dataElements()->getDescriptor("gg"),
             TMeteoParam(QString::number(datetimeData.time().minute()),
                         datetimeData.time().minute(),
                         control::RIGHT),
             0);

  _data->add(MetaData::instance()->dataElements()->getDescriptor("GG"),
             TMeteoParam(QString::number(datetimeData.time().hour()),
                         datetimeData.time().hour(),
                         control::RIGHT),
             0);

  _data->add(MetaData::instance()->dataElements()->getDescriptor("YY"),
             TMeteoParam(QString::number(datetimeData.date().day()),
                         datetimeData.date().day(),
                         control::RIGHT),
             0
             );
  _data->add(MetaData::instance()->dataElements()->getDescriptor("MM"),
             TMeteoParam(QString::number(datetimeData.date().month()),
                         datetimeData.date().month(),
                         control::RIGHT),
             0);
  _data->add(MetaData::instance()->dataElements()->getDescriptor("JJJ"),
             TMeteoParam( QString::number(datetimeData.date().year()),
                          datetimeData.date().year(),
                          control::RIGHT),
             0);

  return true;
}

//! преобразование координат, если был задан квадрант земного шара
void RegExpDecode::convertCoords()
{
  if (!_data->hasParam(MetaData::instance()->dataElements()->getDescriptor("Qc"), false) ||
      !_data->hasParam(MetaData::instance()->dataElements()->getDescriptor("LaLaLa"), false) ||
      !_data->hasParam(MetaData::instance()->dataElements()->getDescriptor("LoLoLoLo"), false)) {
    return;
  }
  
  control::QualityControl qual3;
  float Qc;
  TMeteoParam* fi = _data->paramPtr(MetaData::instance()->dataElements()->getDescriptor("LaLaLa"), false);
  TMeteoParam* la = _data->paramPtr(MetaData::instance()->dataElements()->getDescriptor("LoLoLoLo"), false);
  if (fi->quality() == control::MISTAKEN ||
      la->quality() == control::MISTAKEN) return;

  if(!_data->getValue(MetaData::instance()->dataElements()->getDescriptor("Qc"), &Qc, &qual3, false) ||
     qual3 == control::MISTAKEN) {
    fi->setQuality(control::MISTAKEN, control::TIME_CTRL, false);
    la->setQuality(control::MISTAKEN, control::TIME_CTRL, false);
    return;
  }
  
  switch( static_cast<int>(Qc) ) {
    case 3:
      fi->setValue(-fi->value());
    break;
    case 5:
      fi->setValue(-fi->value());
      la->setValue(-la->value());
    break;
    case 7:
      la->setValue(-la->value());
    break;
    default: {}
  }
}
