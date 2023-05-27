#include "tbufrtransform.h"
#include "tbufrmeta.h"

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <qdom.h>
#include <qfile.h>
#include <qpair.h>
#include <qdebug.h>

#define TBUFR_DESCRIPTORS_FILE MnCommon::etcPath("meteo") + "/bufrtransform.xml"

//находим последовательность, заменяем дескрипторы в ней указанные
//раскрываем дескприпторы D, заменяя номера, добавляя группы
//заменяем дескрипторы B, преобразуем значения, в соответствии с единицами измерения

QDebug& operator<<(QDebug& out, const DescrSeq& d) {
  out << "{" << d.bufrCateg << ", " << d.X << "}";
  return out;
}



TBufrTransform::TBufrTransform()
{
  init();
}


TBufrTransform::~TBufrTransform()
{
}


void TBufrTransform::init()
{
  QFile file(TBUFR_DESCRIPTORS_FILE);
  if ( ! file.open(QIODevice::ReadOnly) ) {
    error_log<<QString("Ошибка открытия файла %1").arg(TBUFR_DESCRIPTORS_FILE);
    return;
  }

  QDomDocument* xml = new QDomDocument;
  if (xml->setContent(&file)) {
    readSettings(xml->documentElement().firstChild());
  }

  file.close();
  delete xml;

  if (_sequence.count() == 0) {
    _sequence.append(BufrSequence());
  }
}


//! чтение настройки из xml
void TBufrTransform::readSettings(QDomNode node)
{
  while (! node.isNull()) {
    QDomElement el = node.toElement();
    if (el.tagName() == "sequence") {
      readSequence(el);
    } else if (el.tagName() == "d_descrtype") {
      readDescrType(el, &_dDescrType);
    } else if (el.tagName() == "b_descrtype") {
      readDescrType(el, &_bDescrType);
    }
    else if (el.tagName() == "b_descr") {
      readTransform(el.firstChild());
    } else if (el.tagName() == "signgroup_descr") {
      readSignGroup(el.firstChild());
    }
    
    node = node.nextSibling();
  }

}

void TBufrTransform::readSequence(const QDomElement& pel)
{
  QDomNode node = pel.firstChild();

  while (! node.isNull()) {
    if (node.isElement()) {
      QDomElement el = node.toElement();
      if (el.tagName() == "descr"  && el.hasAttribute("num") &&
	  el.hasAttribute("bufr_category") && el.hasAttribute("data_type")) {
	bool ok;
	uint bufrDescr = el.attribute("num").toUInt(&ok);
	int bufrCateg = el.attribute("bufr_category", "-1").toUInt();
	meteo::surf::DataType category = meteo::surf::DataType(el.attribute("data_type", "-1").toInt());
	if (!ok || category == -1) {
	  error_log << QObject::tr("Ошибка номера дескриптора num=%1 data_type=%2").arg(el.attribute("num")).arg(el.attribute("data_type")) << bufrDescr << category;
	} else {
	  _dSeqType.insert(bufrDescr, SequenceType(bufrCateg, category, 0));
	}
      } else {
	error_log << QObject::tr("Ошибка номера дескриптора num=%1").arg(el.attribute("num"));
      }
    }
    
    node = node.nextSibling(); 
  }
}

void TBufrTransform::readDescrType(const QDomElement& pel, QMap<DescrSeq, SequenceType>* descrType)
{
  if (nullptr == descrType) return;
  
  QDomNode node = pel.firstChild();

  while (! node.isNull()) {
    if (node.isElement()) {
      QDomElement el = node.toElement();
      if (el.tagName() == "descr" && el.hasAttribute("bufr_category") && el.hasAttribute("X") &&
	  el.hasAttribute("prior") && el.hasAttribute("data_type")) {
	int prior = el.attribute("prior", "-1").toUInt();
	int bufrCateg = el.attribute("bufr_category", "-1").toUInt();
	int X = el.attribute("X", "-1").toUInt();
	meteo::surf::DataType category = meteo::surf::DataType(el.attribute("data_type", "-1").toUInt());
	descrType->insert(DescrSeq(bufrCateg, X), SequenceType(bufrCateg, category, prior));
      } else {
	error_log << QObject::tr("Ошибка descrtype") << el.tagName() << el.hasAttribute("bufr_category")
		  << el.hasAttribute("X") << el.hasAttribute("prior") << el.hasAttribute("date_type");
      }
    }
    node = node.nextSibling(); 
  }
}




//! Чтение исходных наборов дескрипторов из xml
// void TBufrTransform::readSequence(QDomNode node, BufrSequence& seq)
// {
//   bool ok;

//   QDomElement pel = node.toElement();
//   if (pel.hasAttribute("number") && pel.hasAttribute("data_type")) {
//     seq.number = pel.attribute("number", "-1").toInt();
//     seq.stationType = meteo::surf::DataType(pel.attribute("data_type", QString::number(meteo::surf::kUnknownDataType)).toInt());
//   }

//   node = node.firstChild();

//   while (! node.isNull()) {
//     QDomElement el = node.toElement();
//     if (el.tagName() == "descr" && el.hasAttribute("num")) {
//       int descr = -1; 
//       if (!el.text().isEmpty()) {
// 	descr = el.text().toInt(&ok);
// 	if (!ok) {
// 	  error_log << QObject::tr("Ошибка номера дескриптора num=%1 val=%2").arg(el.attribute("num").arg(el.text()));
// 	  node = node.nextSibling();
// 	  continue;
// 	}
//       }
      
//       uint bufrDescr = el.attribute("num").toUInt(&ok);
//       if (!ok) {
// 	error_log << QObject::tr("Ошибка номера дескриптора num=%1").arg(el.attribute("num"));
//       } else {
// 	descr = (descr == -1) ? bufrDescr : descr;
// 	seq.descr.append(QPair<uint, int>(bufrDescr, descr));
//       }
//     }    
//     node = node.nextSibling();
//   }
// }



//! чтение дескрипторов, требующих преобразования
void TBufrTransform::readTransform(QDomNode node)
{
  bool ok;
  while (! node.isNull()) {
    QDomElement el = node.toElement();
    if (el.tagName() == "descr" && el.hasAttribute("num")) {
      int num = el.attribute("num").toUInt(&ok);
      if (!ok) {
	error_log << QObject::tr("Ошибка номера дескриптора num=%1 val=%2").arg(el.attribute("num").arg(el.text()));
	node = node.nextSibling();
	continue;
      }      
      if (_bDescr.contains(num)) {
	error_log << QObject::tr("Ошибка. Повтор номера дескриптора num=%1 val=%2").arg(el.attribute("num").arg(el.text()));
	node = node.nextSibling();
	continue;
      }
      BufrConvert conv;
      conv.descr = el.text().toInt(&ok);
      if (!ok) {
	error_log << QObject::tr("Ошибка номера дескриптора num=%1 val=%2").arg(el.attribute("num").arg(el.text()));
	node = node.nextSibling();
	continue;
      }
      _bDescr.insert(num, conv);
    }
  
    node = node.nextSibling();
  }    
}

//! чтение дескрипторов описателей значимости
void TBufrTransform::readSignGroup(QDomNode node)
{  
  bool ok;
  while (! node.isNull()) {
    QDomElement el = node.toElement();
    if (el.tagName() == "descr" && el.hasAttribute("num")) {
      int descr = el.attribute("num").toUInt(&ok);
      if (!ok) {
	error_log << QObject::tr("Ошибка номера дескриптора num=%1 val=%2").arg(el.attribute("num").arg(el.text()));
	node = node.nextSibling();
	continue;
      }      
      if (_signGroup.contains(descr)) {
	error_log << QObject::tr("Ошибка. Повтор номера дескриптора num=%1 val=%2").arg(el.attribute("num").arg(el.text()));
	node = node.nextSibling();
	continue;
      }
      QMap<int, uint> descrGroups;
      QStringList gr = el.text().split(';');
      for (int i =0; i < gr.count(); i++) {
	int val = gr.at(i).section(',', 0,0).toInt(&ok);
	int group;
	if (ok) {
	  group = gr.at(i).section(',', 1,1).toInt(&ok);
	}
	if (!ok || descrGroups.contains(val)) {
	  error_log << QObject::tr("Неверная последовательность дескриптора %1 значение-группа = %2").arg(el.attribute("num")).arg(el.text());
	  continue;
	}
	descrGroups.insert(val, group);
      }
      _signGroup.insert(descr, descrGroups);

    }

    node = node.nextSibling();
  }
}


//! Поиск заданной последовательности дескрипторов
/*! 
  \param descr заданная последовательность
  \param cnt количество дескрипторов, которые надо проверять в descr (если несколько наборов, то они повторяются)
  \param ok true - найдена, false - нет
  \return последовательность с правилами для последующего преобразования
*/
// const BufrSequence& TBufrTransform::findSequence(const QList<BufrDescriptor>& descr, int cnt, 
// 						 bool* ok, int* number, meteo::surf::DataType* type) const
// {
//   int idx = 0;
//   *ok = false;
//   *number = -1;
//   *type = meteo::surf::kUnknownDataType;
//   //  var(_sequence.count());

//   for (int i = 0; i < _sequence.count(); i++) {
//     if (_sequence.at(i).descr.size() != cnt) { continue; }
//     if (equalSequence(descr, _sequence.at(i))) {
//       *ok = true;
//       idx = i;
//       *number = _sequence.at(i).number;
//       *type = _sequence.at(i).stationType;
//       break;
//     }
//   }

//   return _sequence.at(idx);
// }


//! Сравнение двух наборов дескрипторов
/*! 
  \param seq    набор дескрипторов из настроек
  \param descr  проверяемый набор дескрипторов
  \return true - если равны, иначе false
*/
// bool TBufrTransform::equalSequence(const QList<BufrDescriptor>& descr, const BufrSequence& seq) const
// {
//   for (int idx = 0; idx < seq.descr.size(); idx++) {
//     //    debug_log << seq.at(idx).first << (uint)descr.at(idx).tableIndex();
//     if (seq.descr.at(idx).first != (uint)descr.at(idx).tableIndex()) return false;
//   }

//   return true;
// }

//cnt - чтоб не проверять по всем subset
meteo::surf::DataType TBufrTransform::findType(const QList<BufrDescriptor>& descrlist, int cnt, int bufrCateg) const
{
  meteo::surf::DataType type = meteo::surf::kUnknownDataType;
  int priority = 9999;
  // BufrDescriptor tmpdescr;

  //var(_dSeqType.keys());

  if (bufrCateg == 7) {
    return meteo::surf::kFlightSigWx;
  }

  if (bufrCateg == 5) {
    return meteo::surf::kSatAeroOne;
  }
  
  if (cnt > descrlist.size()) {
#ifdef PARSE_LOG
    error_log << QObject::tr("Ошибка структуры BUFR");
#endif
    return type;
  }
    
  for (int idx = 0; idx < cnt; idx++) {
    auto descr = descrlist.at(idx);
    meteo::surf::DataType curs = findSequenceType(descr, bufrCateg);
    if (curs != meteo::surf::kUnknownDataType) {
      type = curs;

      //debug_log << "seq" << descr.tableIndexStr() << type;
      return type;
    }

    if (descr.F == 3) {
      SequenceType curst = findDescrType(descr, bufrCateg, _dDescrType);
      if (curst.category != meteo::surf::kUnknownDataType && curst.prior < priority) {
	type = curst.category;
	priority = curst.prior;
	//	tmpdescr = descr;
      }
    } else if (descr.F == 0) {
      SequenceType curst = findDescrType(descr, bufrCateg, _bDescrType);
      if (curst.category != meteo::surf::kUnknownDataType && curst.prior < priority) {
	type = curst.category;
	priority = curst.prior;
	//tmpdescr = descr;
	if (priority == 0) {
	  return type;
	}
      }      
    }

    //debug_log << descr.tableIndexStr() << type << priority;    
  }

  if (type == meteo::surf::kUnknownDataType && bufrCateg == 0) {
    type = meteo::surf::DataType(meteo::surf::kSynopBufr);
  }

  if (type == meteo::surf::kUnknownDataType && bufrCateg == 2) {
    type = meteo::surf::DataType(meteo::surf::kAeroUnk);
  }

  if (type == meteo::surf::kUnknownDataType && bufrCateg == 3) {
    type = meteo::surf::DataType(meteo::surf::kSatAero);
  }
  if (type == meteo::surf::kUnknownDataType && bufrCateg == 12) {
    type = meteo::surf::DataType(meteo::surf::kSatSurfOther);
  }

  // debug_log << tmpdescr.tableIndexStr() << type << priority;    
  return type;
}

meteo::surf::DataType TBufrTransform::findSequenceType(const BufrDescriptor& descr, int bufrCateg) const
{
  uint full = descr.tableIndex();
  // var(full);
  // var(_dSeqType.keys());
   
  
  if (_dSeqType.contains(full)) {
    const SequenceType& stype = _dSeqType[full];
    if (bufrCateg != stype.bufrCateg) {
#ifdef PARSE_LOG
      warning_log << QObject::tr("Не совпадают ожидаемая (%1) и раскодированная категории данных (%2) seq=%3")
	.arg(stype.bufrCateg).arg(bufrCateg).arg(full);
#endif
    }
    return stype.category;
  }

  return meteo::surf::kUnknownDataType;
}

SequenceType TBufrTransform::findDescrType(const BufrDescriptor& descr, int bufrCateg,
					   const QMap<DescrSeq, SequenceType>& descrType) const
{
  DescrSeq key(bufrCateg, descr.X);
  if (descrType.contains(key)) {
    return descrType.value(key);
  }

  return SequenceType();
}

