#include "xmllinedecode.h"
#include "metadata.h"
#include "tmeteoelement.h"

#include <cross-commons/debug/tlog.h>
#include <commons/meteo_data/tmeteodescr.h>


using namespace meteo;
using namespace anc;


XmlLineDecoder::XmlLineDecoder()
{ 
}

XmlLineDecoder::~XmlLineDecoder()
{
}


void XmlLineDecoder::startBlock()
{
  _inchilds.clear();
}

bool XmlLineDecoder::setMeteoDataValue(const QString& code, const QDomNode& node, const QString& astr, TMeteoData* data)
{
  if (!node.isElement()) {
    error_log<<QObject::tr("Ошибка структуры xml %1").arg(code);
    return false;
  }

  QDomElement elem = node.toElement();

  //получение значения дескриптора descriptor из атрибутов i-го узла 
  QString descr = elem.attribute("descrName");
#ifdef PARSE_DEBUG_LOG
  debug_log<<"descr="<<descr;
#endif

  if (descr.isEmpty()) {
    error_log<<QObject::tr("Ошибка xml %1. Неверный дескриптор=%2 (number=%3, text=%4)").
      arg(code).arg(elem.attribute("descrName")).arg(elem.attribute("number")).
      arg(elem.attribute("text"));
    return false;
  }

  if (MetaData::instance()->dataElements()->contains(descr)) {
    TMeteoData* cur = currentMeteoData(code, elem, data);
    
    meteo::ElementConvertType ctype = MetaData::instance()->dataElements()->convertType(descr);

#ifdef PARSE_LOG
    if (nullptr == cur) {
      error_log << QObject::tr("Ошибка раскодирования дескриптора '%1'").arg(descr);
    }
#endif
    if (ctype == meteo::COMPLEX_CONVERT) {
      if (!MetaData::instance()->dataElements()->parseComplex(descr, astr, cur)) {
#ifdef PARSE_LOG
	error_log << QObject::tr("Ошибка раскодирования дескриптора '%1'").arg(descr);
#endif
      }
    } else {
      control::QualityControl quality = control::NO_CONTROL;
      float val = BAD_METEO_ELEMENT_VAL;
      QString str = astr;

      if (ctype == meteo::TRANSLIT_CONVERT) {
	str = global::translitFuncKoi7(astr);
	val = MetaData::instance()->bufrcode(str);
      } else {
      
	if (MetaData::instance()->dataElements()->outName(descr) != "text") {
	  str = str.trimmed();
	}
	if (str.isEmpty() && elem.hasAttribute("default")) {
	  str = elem.attribute("default");
	}
	//численная интерпретация строкового значения в коде (давление PPPP, температура SnTTT...)  
	val = MetaData::instance()->dataElements()->getValue(descr, str, &quality); 
	
	if (quality == control::MISTAKEN) {
	  //warning_log << QObject::tr("Ошибка семантического контроля. Сводка '%1'").arg(_caption); TODO обратно раскомментить
	}	
      }
#ifdef PARSE_DEBUG_LOG
      debug_log<<" DESCR=" << MetaData::instance()->dataElements()->getDescriptor(descr) << descr << str << val << quality;
#endif
      //--

      cur->add(MetaData::instance()->dataElements()->getDescriptor(descr), str, val, quality);
    }
    

  } else {
    error_log << QObject::tr("Нет описания локального дескриптора") << descr;
  }
  return true;
}

//! Возвращает родительскую или одну из дочерних TMeteoData в соотв с параметрами xml-строки 
TMeteoData* XmlLineDecoder::currentMeteoData(const QString& code, QDomElement elem, TMeteoData* data)
{
  bool ok = true;
  int curNum = -1;
  QMap<int, TMeteoData*>* curChild = nullptr;

  //связывание параметров в разных блоках
  if (elem.hasAttribute("num")) {
    curNum = elem.attribute("num").toInt(&ok);
    curChild = &_childs;
  } else if (elem.hasAttribute("innum")) {
    curNum = elem.attribute("innum").toInt(&ok);
    curChild = &_inchilds;
  }
  if (!ok) {
    error_log<<QObject::tr("Ошибка xml %1. Неверный номер для связи значений=%2 (number=%3, text=%4)").
      arg(code).arg(curNum).arg(elem.attribute("number")).arg(elem.attribute("text"));
    return nullptr;
  }

 
  if (nullptr != curChild) {
    if (!curChild->contains(curNum)) {
      curChild->insert(curNum, &data->addChild());
    }
    if (nullptr != curChild->value(curNum)) {
      return curChild->value(curNum); //!< найден (создан) дочерний объект с глобальным номером для связывания равным num
    }
  }
  
  return data;
}
