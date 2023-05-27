#include "tmeteodescr.h"
#include "meteo_data.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/geobasis/geopoint.h>

#include <qdom.h>
#include <qfile.h>
#include <qdatetime.h>


#define DESCRIPTORS_FILE MnCommon::etcPath("meteo") + "descriptors.xml"

template<> meteodescr::TMeteoDescriptor* TMeteoDescriptor::_instance = 0;

// QDebug& operator<<(QDebug& out, const meteodescr::VerticType& vt)
// {
//   out << vt.type << vt.level << vt.height;
//   return out;
// }

//TODO
//удалять дескрипторы времени при заполнении?


namespace meteodescr {

//! добавление номера группы к дескриптору
descr_t create(uint group, descr_t descr)
{
  return group*100000 + descr;
}

bool hasGroup( descr_t descr )
{
  if ( 100000 > descr ) {
    return false;
  }
  return true;
}

int group( descr_t descr )
{
  return descr/100000;
}

descr_t descrWithoutGroup( descr_t descr )
{
  return descr%100000;
}

TMeteoDescriptor::TMeteoDescriptor()
{
  QDomDocument* xml = new QDomDocument;
  bool ok = readFile(xml, DESCRIPTORS_FILE);
  if (ok) {
    read(xml->documentElement().firstChild());
  } else {
    error_log << QObject::tr("Ошибка чтения файла с описанием дескрипторов '%1'").arg(DESCRIPTORS_FILE);
  }
  
  // ok = readFile(xml, DESCR_LINKS_FILE);
  // if (ok) {
  //   readLinks(xml->documentElement().firstChild());
  // }

  delete xml;
}

TMeteoDescriptor::~TMeteoDescriptor()
{
}

  //! загрузка файла в xml
bool TMeteoDescriptor::readFile(QDomDocument* xml, const QString& fileName)
{
  bool ok = false;
  if (!xml) return false;

  QFile file(fileName);
  if ( ! file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка чтения файла '%1'").arg(fileName);
    return false;
  }

  if (xml->setContent(&file)) {
    ok = true;
  }

  file.close();
  return ok;
}

//! парсинг xml с дескрипторами
void TMeteoDescriptor::read(QDomNode node)
{
  bool ok;
  QDomNode parent = node;

  //основные
  while (!node.isNull()) {
    if (node.isElement()) {
      QDomElement el = node.toElement();
      if (el.tagName() == "descr") {
	if (el.hasAttribute("num") && el.hasAttribute("name")) {
	  if (_descr.contains(el.attribute("name"))) {
	    error_log << QObject::tr("Ошибка. Повторение дескриптора с именем '%1'").arg(el.attribute("name"));
	  } else {
	    descr_t num = el.attribute("num").toUInt(&ok);
	    if (!ok) {
	      error_log << QObject::tr("Ошибка номера дескриптора name=%1 num=%2").
		arg(el.attribute("name")).arg(el.attribute("num"));
	    } else {
	      bool anyGroup = el.attribute("group") == "ANY";
	      uint group = el.attribute("group").toUInt();
	      num = create(group, num);
	      QString unit = el.attribute("unit");
	      if (unit.isEmpty()) {
		error_log << QObject::tr("Нет единиц измерения у дескриптора '%1'").arg(el.attribute("name"));
	      }
	      QString unitRu = el.attribute("unit_ru");
	      if (unitRu.isEmpty()) {
		error_log << QObject::tr("Нет русских единиц измерения у дескриптора '%1'").arg(el.attribute("name"));
	      }

	      bool single = false;
	      if (el.attribute("single") == "true") {
		single = true;
	      }
	      QString description;
	      description = el.attribute("description");
	      _descr.insert(el.attribute("name"), num);
	      //debug_log << el.attribute("name") << num << anyGroup;
	      //if (unit != "NO" && !unit.isEmpty()) {
	    
	      _prop.insert(num, Property(unit, unitRu, single, anyGroup, description));
	      // }
	    }
	  }
	} else {
	  error_log << QObject::tr("Ошибка описания дескриптора name=%1 num=%2").
	    arg(el.attribute("name")).arg(el.attribute("num"));
	}
      }
    }
    
    node = node.nextSibling();
  }
  
  //дополнительные
  node = parent;
  while (!node.isNull()) {
    if (node.isElement()) {
      QDomElement el = node.toElement();
      if (el.tagName() == "complex") {
  	readComplex(el.firstChild());
      } else if (el.tagName() == "additional") {
  	readAdditional(el.firstChild());
  	break;
      }
    }
    node = node.nextSibling();
  }
}

void TMeteoDescriptor::readComplex(QDomNode node)
{
  while (!node.isNull()) {
    if (node.isElement()) {
      QDomElement el = node.toElement();
      if (el.hasAttribute("descr") && el.hasAttribute("name") && el.hasAttribute("childs")) {
	if (_descr.contains(el.attribute("name"))) {
	  error_log << QObject::tr("Ошибка. Повторение дескриптора с именем '%1'").arg(el.attribute("name"));
	} else {
	  descr_t descr = el.attribute("descr").toULongLong();
	  if (!isValid(descr)) {
	    error_log << QObject::tr("Ошибка номера дескриптора name=%1 descr=%2").
	      arg(el.attribute("name")).arg(el.attribute("descr"));
	  } else {

	    QStringList childs = el.attribute("childs").split(',');
	    if (childs.size() == 0) {
	      error_log << QObject::tr("Пустой список составного дескриптора name=%1 descr=%2").
		arg(el.attribute("name")).arg(el.attribute("descr"));
	    } else {
	      
	      _descr.insert(el.attribute("name"), descr);
	      _prop.insert(descr, Property());
	      
	      for (int idx = 0; idx < childs.size(); idx++) {
		descr_t chdescr = descriptor(childs.at(idx).trimmed());
		if (!isValid(chdescr)) {
		  error_log << QObject::tr("Ошибка номера дескриптора name=%1 num=%2").
		    arg(childs.at(idx).trimmed()).arg(chdescr);
		} else if (!_descr.contains(childs.at(idx).trimmed())) {
		  error_log << QObject::tr("Дескриптор %1 из списка составного дескриптора %2 не существует").
		    arg(chdescr).arg(el.attribute("name"));
		} else if (_complex.contains(chdescr)) {
		  error_log << QObject::tr("Дескриптор %1 из списка составного дескриптора %2 уже добавлен").
		    arg(chdescr).arg(el.attribute("name"));
		} else {
		  
		  _complex.insert(chdescr, descr);
		  
		}
	      }
	      
	    }
	  }
	}
	
      } else {
	error_log << QObject::tr("Ошибка описания составного дескриптора name=%1 descr=%2").
	  arg(el.attribute("name")).arg(el.attribute("descr"));
      }      
      
    }
    node = node.nextSibling();
  }

}
  
//! чтение параметров дполонительных имен дескрипторов
void TMeteoDescriptor::readAdditional(QDomNode node)
{
  bool ok = true;
  while (!node.isNull()) {
    if (node.isElement()) {
      QDomElement el = node.toElement();
      if (el.hasAttribute("inname") && el.hasAttribute("name")) {
	if (_add.contains(el.attribute("name"))) {
	  error_log << QObject::tr("Ошибка. Повторение дескриптора с именем '%1'").arg(el.attribute("name"));
	} else {
	  descr_t descr = descriptor(el.attribute("inname"));
	  if (!isValid(descr)) {
	    error_log << QObject::tr("Ошибка номера дескриптора name=%1 num=%2").
	      arg(el.attribute("inname")).arg(el.attribute("num"));
	  } else {
	    int idx = -1;
	    int min = -1;
	    int max = -1;
	    if (el.hasAttribute("idx")) {
	      idx = el.attribute("idx").toInt(&ok);
	      if (!ok) {
		idx = -1;
		error_log << QObject::tr("Ошибка номера индекса дескриптора '%1'").arg(el.attribute("name"));
	      }
	    } else if (el.hasAttribute("code")) {
	      min = el.attribute("code").section(',', 0, 0).toInt(&ok);
	      if (!ok) {
		min = -1;
		error_log << QObject::tr("Ошибка диапазона дескриптора '%1'").arg(el.attribute("name"));
	      } else {
		max = el.attribute("code").section(',', 1, 1).toInt(&ok);
		if (!ok) {
		  max = -1;
		  error_log << QObject::tr("Ошибка диапазона дескриптора '%1'").arg(el.attribute("name"));
		}
	      }
	    } else {
	      ok = false;
	      error_log << QObject::tr("Ошибка описания доп. дескриптора '%1'. Не задан ни индекс, ни диапазон "
					   "кодового значения").arg(el.attribute("name"));
	    }
	    if (ok) {
	      _add.insert(el.attribute("name"), Additional(descr, el.attribute("description"), idx, min, max));
	      _addParent << el.attribute("inname");
	    }
	  }
	}
      } else {
	error_log << QObject::tr("Ошибка описания дескриптора name=%1 num=%2").
	  arg(el.attribute("name")).arg(el.attribute("num"));
      }
    }
    
    node = node.nextSibling();
  }
}

//! парсинг xml со связями дескрипторов
  // void TMeteoDescriptor::readLinks(QDomNode node)
  // {
  //   while (!node.isNull()) {
  //     if (node.isElement()) {
  // 	QDomElement el = node.toElement();
  // 	if (el.tagName() == "descr") {
  // 	  QStringList descrs = el.text().split(',');
  // 	  VerticType linkType;
  // 	  linkType.type = el.attribute("type").toInt();
  // 	  linkType.height = el.attribute("height");
  // 	  linkType.heightLo = el.attribute("height_lo");
  // 	  linkType.level = el.attribute("level");
  // 	  linkType.levelLo = el.attribute("level_lo");
	  
  // 	  for (int idx = 0; idx < descrs.count(); idx++) {
  // 	    descr_t descr = descriptor(descrs.at(idx).trimmed());
  // 	    if (isValid(descr)) {
  // 	      _links.insert(descr, linkType);
  // 	    }
  // 	  }
  // 	}
  //     }
  //     node = node.nextSibling();
  //   }
  // }

//-------------

bool TMeteoDescriptor::isExist(descr_t descr) const
{ 
  if (_prop.contains(descr)) {
    return true;
  }
  
  descr_t d = descr %100000;
  if (_prop.contains(d) && _prop.value(d).anyGroup) {
    return true;
  }
  
  return false; 
}

  //! дескриптор-идентификатор
  bool TMeteoDescriptor::isIdentDescr(descr_t descr) const
  {
    descr_t d = descr % 100000;
    if (d < 9000 || (d >= 29000 && d < 33000) || (d >= 25000 && d < 26000)) return true;
    if (d == 22055 || d == 22056 || d == 22060 || d == 22067 || d == 22068) return true;    
    
    return false;
  }

  //! дескриптор-идентификатор, кроме местоположения (гориз, вертик, времени)
  bool TMeteoDescriptor::isNonLocateIdentDescr(descr_t descr) const
  {
    descr_t d = descr % 100000;
    if (d >= 4000 && d < 8000) return false;
    
    if (d < 9000 || (d >= 29000 && d < 33000) || (d >= 25000 && d < 26000)) return true;
    if (d == 22055 || d == 22056 || d == 22060 || d == 22067 || d == 22068) return true;
    
    return false;
  }

//! метео, показатели значимости 
bool TMeteoDescriptor::isMeteo(descr_t descr) const
{
  descr_t d = descr % 100000;
  if (/*(d >= 8000 && d < 9000) ||*/ d >= 10000 && !(d >= 25000 && d < 26000)) return true;
 
  return false;
}


//! проверка является ли дескриптор частью составного, complDescr - номер сосатвного
bool TMeteoDescriptor::isComponent(descr_t descr, descr_t* complDescr) const
{
  if (nullptr == complDescr) return false;
  
  bool ok = false;
  
  if (_complex.contains(descr)) {
    *complDescr = _complex.value(descr);
    ok = true;
  }
    
  return ok;
}

//! проверка является ли дескриптор частью составного, complName - название сосатвного
bool TMeteoDescriptor::isComponent(const QString& dname, QString* complName) const
{
  if (nullptr == complName) return false;
  
  bool ok = false;
  descr_t complDescr = BAD_DESCRIPTOR_NUM;

  descr_t descr;
  if (isAdditional(dname)) {
    descr = additional(dname).descr;
  } else {
    descr = descriptor(dname);
  }

  ok = isComponent(descr, &complDescr);
  if (!ok) return false;
  
  *complName = name(complDescr);

  return ok;
}

//! проверка является ли дескриптор частью составного, complName - название сосатвного
bool TMeteoDescriptor::isComponent(const QString& dname, descr_t* complDescr) const
{
  if (nullptr == complDescr) return false;
  
  bool ok = false;
  *complDescr = BAD_DESCRIPTOR_NUM;

  descr_t descr;
  if (isAdditional(dname)) {
    descr = additional(dname).descr;
  } else {
    descr = descriptor(dname);
  }

  ok = isComponent(descr, complDescr);

  return ok;
}

//! проверка является ли дескриптор составным
bool TMeteoDescriptor::isComplex(const QString& complName) const
{
  if (isAdditional(complName)) {
    return false;
  }

  return _complex.key(descriptor(complName), BAD_DESCRIPTOR_NUM) != BAD_DESCRIPTOR_NUM;
}

//! Проверка все ли дескрипторы md относятся к составному дескриптору (без дочерних)
bool TMeteoDescriptor::isAllCompex(const TMeteoData& md, descr_t* complDescr) const
{
  bool allComplex = true; //TODO считаем, что не могут встретиться вместе из разных наборов
  bool hasMeteo = false;
  const QList<descr_t> descrs = md.getDescrList();
  for (int idx = 0; idx < descrs.size(); idx++) {
    if (!isIdentDescr(descrs.at(idx))) {
      hasMeteo = true;
      if (!isComponent(descrs.at(idx), complDescr)) {
	allComplex = false;
	break;
      }
    }
  }

  return (allComplex && hasMeteo);
}

//! номер дескриптора по имени
descr_t TMeteoDescriptor::descriptor(const QString& name) const
{
  if ( true == isAdditional(name) ) {
    auto add = additional(name);
    return add.descr;
  }
  if (!_descr.contains(name)) {
    return BAD_DESCRIPTOR_NUM;
//    закоментировано, потому что при получении старых данных с hgr замедляется работа
//    error_log << QObject::tr("Неизвестное название дескриптора '%1'").arg(name);
  }
  return _descr.value(name, BAD_DESCRIPTOR_NUM); 
}

descr_t TMeteoDescriptor::descriptor(const char* name) const
{
  return descriptor( QString::fromUtf8(name) );
}

descr_t TMeteoDescriptor::descriptor(const std::string& name) const
{
  return descriptor( QString::fromStdString(name) );
}


//! получение имени дескриптора по номеру
QString TMeteoDescriptor::name(descr_t descr) const
{
  if (_prop.contains(descr)) {
    return _descr.key(descr);    
  }

  descr_t d = descr %100000;
  if (_prop.contains(d) && _prop.value(d).anyGroup) {
    return _descr.key(d);
  }

  return "";
}

//! Получение свойств по имени дескриптора (в том числе дополнительные)
bool TMeteoDescriptor::property(const QString& name,  Property* prop) const
{
  bool ok = false;
  
  if (isAdditional(name)) {
    prop->operator=(_prop[additional(name).descr]);
    prop->description = additional(name).description;
    ok = true;
  } else if (contains(name)) {
    prop->operator=(_prop[descriptor(name)]);
    ok = true;
  }

  return ok;
}

QString TMeteoDescriptor::description(const QString& name) const
{
  if (isAdditional(name)) {
    return additional(name).description;
  }

  return _prop[descriptor(name)].description;
}

//! Поиск описания с учетом дополнительных дескрипторов
/*! 
  \param descr Номер дескриптора
  \param order Порядковый номер, если несколько параметров с таким дескриптором (индекс в наборе)
  \param value Значение величины  
  \return Описание параметра
*/
QString TMeteoDescriptor::description(descr_t descr, int order, float value) const
{
  QString aname = findAdditional(descr, order, value);

  if (aname.isEmpty()) {
    return property(descr).description;
  }
  return description(aname);
  
//  return additional(aname).description;
}

//! Поиск имени дополнительного дескриптора
/*! 
  \param descr Номер дескриптора
  \param order Порядковый номер, если несколько параметров с таким дескриптором (индекс в наборе)
  \param value Кодовое значение величины  
  \return Имя дополнительного дескриптора
*/
QString TMeteoDescriptor::findAdditional(descr_t descr, int order, float value) const
{
  QHashIterator<QString, Additional> it(_add);
  while (it.hasNext()) {
     it.next();

     if ( it.value().descr == descr
       && ( (it.value().index != -1 && it.value().index == order) || ( value >= it.value().min && value <= it.value().max) ) ) {
       return it.key();
     }
  }

  return name(descr);
}

// const QHash<descr_t, VerticType>& TMeteoDescriptor::links()
// {
//   return _links;
// }


//! Идентификатор станции (индекс, ИКАО, позывной или др.)
QString TMeteoDescriptor::stationIdentificator(const TMeteoData& data) const
{
  int number = station(data);
  
  if (number != BAD_METEO_ELEMENT_VAL) {
    return QString::number(number);//.rightJustified(5, '0');
  }

  if (data.hasParam(descriptor("CCCC"), false)) {
    return data.getCode(descriptor("CCCC"), false);
  }

  number = buoy(data); //TODO не помню может ли в bufr быть пустое (-9999) значение для буя, когда идёт позывной судна
  if (number != BAD_METEO_ELEMENT_VAL && number > 1000) {
    return QString::number(number).rightJustified(5, '0');
  }

  QString str = data.getCode(descriptor("D____D"), false);
  if (!str.isEmpty()) {
    return str;
  }

  // название может оказаться не уникальным
  // str = data.getCode(descriptor("station"), false);
  // if (!str.isEmpty()) {
  //   return str;
  // }  
  // str = data.getCode(descriptor("station_short"), false);
  // if (!str.isEmpty()) {
  //   return str;
  // }

  if (data.childsCount() != 0 && nullptr != data.child(0)) {
    return stationIdentificator(*data.child(0));
  }
  
  return QString();
}

//! Идентификатор станции (индекс, ИКАО, позывной или др.)
// TODO переделать для ComplexMeteo (ток у него внутри нет данных о станции)
/*
  \param gp - если нет идентификатора заполнить координатами
 */
QString TMeteoDescriptor::stationIdentificator(const TMeteoData& data, const meteo::GeoPoint& gp, bool withName /* = false*/) const
{
  QString station = stationIdentificator(data);

  // название может оказаться не уникальным, но иногда нужно именно оно
  if (station.isEmpty() && withName) {
    station = data.getCode(descriptor("station"), false);
  }
  
  if (station.isEmpty()) {
    if (gp.isValid()) {
      station = QObject::tr("%1, %2").arg(gp.strLat().trimmed()).arg(gp.strLon().trimmed());
    }
  }
  
  return station;
}

QString TMeteoDescriptor::stationName(const TMeteoData& data) const
{
  return data.getCode(descriptor("station"), false);    
}

QString TMeteoDescriptor::stationShortName(const TMeteoData& data) const
{
  return data.getCode(descriptor("station_short"), false);    
}

QString TMeteoDescriptor::stationFullName(const TMeteoData& data) const
{
  return data.getCode(descriptor("station_long"), false);    
}


  //! возвращает номер станции
int TMeteoDescriptor::station(const TMeteoData& data, int defaultValue /*= BAD_METEO_ELEMENT_VAL*/) const
{
  if (data.hasParam(descriptor("station_index"), false)) {
    return data.getValueCur(descriptor("station_index"), defaultValue);
  }

  const TMeteoParam& center = data.getParam(descriptor("II"));

  if (center.isInvalid()) return defaultValue;

  const TMeteoParam& subCenter = data.getParam(descriptor("iii"));
  if (subCenter.isInvalid()) return defaultValue;

  if (center.value() <= 0 || subCenter.value() < 0) {
    return defaultValue;
  }
  
  //  debug_log << "center=" << center.value()*1000 << subCenter.value();
  return static_cast<int>(center.value()*1000 + subCenter.value());
}

//номер буя
 int TMeteoDescriptor::buoy(const TMeteoData& data, int defaultValue /*= BAD_METEO_ELEMENT_VAL*/) const
{
  const TMeteoParam& A1 = data.getParam(descriptor("A1"));
  const TMeteoParam& bw = data.getParam(descriptor("bw"));
  const TMeteoParam& nb = data.getParam(descriptor("nb"));

  if (A1.isInvalid() || bw.isInvalid() || nb.isInvalid()) {
    if (nb.isValid()) {
      return static_cast<int>(nb.value());
    }
    return defaultValue;
  }

  return static_cast<int>(((A1.value() * 10) + bw.value()) * 1000 + nb.value());
} 

//! местоположение данных
  bool TMeteoDescriptor::getCoord(const TMeteoData& data, float *fi, float *la, float* h /* = 0*/) const
{
  control::QualityControl qual1, qual2;
  if (!data.getValue(descriptor("La"), fi, &qual1) || 
      !data.getValue(descriptor("Lo"), la, &qual2)) return false;
  if (qual1 >= control::MISTAKEN || 
      qual2 >= control::MISTAKEN) return false;
  //Qc проверять не надо, преобразование происходит в раскодировщике  

  if (0 != h) {
    if (!data.getValue(descriptor("h0_station"), h, &qual1)) {
      *h = BAD_METEO_ELEMENT_VAL;
      return true;
    }
    if (qual1 == control::MISTAKEN) {
      return false;
    }
  }

  return true;
}

//! местоположение данных
bool TMeteoDescriptor::getCoord(const TMeteoData& data, meteo::GeoPoint* point) const
{
  if (point != 0) {
    float lat = 0.0;
    float lon = 0.0;
    float h = 0.0;
    if (getCoord(data, &lat, &lon, &h) == true) {
      *point = meteo::GeoPoint::fromDegree(lat, lon, h);
      return true;
    }
  }
  return false;
}
//! местоположение данных
bool TMeteoDescriptor::getDegCoord(const TMeteoData& data, meteo::GeoPoint* point) const
{
  if (point != 0) {
    float lat = 0.0;
    float lon = 0.0;
    float h = 0.0;
    if (getCoord(data, &lat, &lon, &h) == true) {
      *point = meteo::GeoPoint(lat, lon, h);
      return true;
    }
  }
  return false;
}

void TMeteoDescriptor::setType(TMeteoData* data, meteodescr::LevelType type) const
{
  if (0 == data) return;

  data->add(descriptor("level_type"), TMeteoParam(QString::number(type), type , control::RIGHT), 0);
}

void TMeteoDescriptor::fillTypeLevel(TMeteoData* data) const
{
  if (0 == data) return;

  int levelType = -1;
  float lo = -1, hi = -1;
  findVertic(*data, &levelType, &lo, &hi);
  if (levelType == -1) levelType = meteodescr::kSurfaceLevel;
  
  data->set(descriptor("level_type"), QString::number(levelType), levelType, control::RIGHT);
  
  for (int idx = 0; idx < data->childsCount(); idx++) {
    fillTypeLevel(data->child(idx));
  }
}

//возвращает уровень, если высокогорная станция
int TMeteoDescriptor::mountainLevel(const TMeteoData& data) const
{
  int category = data.getValueCur(descriptor("category"), BAD_METEO_ELEMENT_VAL);
  if (category == BAD_METEO_ELEMENT_VAL || category > 39 ) { //только синоптические
    return BAD_METEO_ELEMENT_VAL;
  }

  QList<const TMeteoData*> ch = data.findChildsConst(descriptor("hh"));

  for (int idx = 0; idx < ch.size(); idx++) {
    float hh = ch.at(idx)->getValueCur(descriptor("hh"), BAD_METEO_ELEMENT_VAL);
    if (hh != BAD_METEO_ELEMENT_VAL) {
      float p1 = ch.at(idx)->getValueCur(descriptor("P1"), BAD_METEO_ELEMENT_VAL);
      return round(p1);
    }
  }

  return BAD_METEO_ELEMENT_VAL;
}

//! Поиск уровня вертикальной координаты данных с учетом родителей
  /*! 
    \param src Данные
    \param levelType Тип уровня
    \param levelLo   Нижняя граница
    \param levelHi   Верхняя граница
    \return true - если значение определено
  */
bool TMeteoDescriptor::findVertic(const TMeteoData& src, int* levelType, float* levelLo, float* levelHi) const
{
  if (0 == levelType || 0 == levelLo || 0 == levelHi) return false;

  //TODO
  //P1(7004) + hh(10009) - высокогорная станция
  //P1(7004) - изобарические поверхности, уровень 6, 7, 100, 101
  //ha(7006) - высота, уровень 103 и для слоя
  //hgpm(7009) - геопотенциальная высота, уровень 
  //zn(7062) - глубина, уровень 160  
  //h0(7002) - высота над уровнем станции
  //hmsl(7007) - высота на УМ, уровень 102
  //Pd(7065) - давление воды
  bool ok = false;
  bool levset = false;

  if (-1 != *levelType) {
    levset = true;
  }
  
  if (!levset && src.hasParam(descriptor("level_type"), false)) {
    *levelType = src.getValueCur(descriptor("level_type"), -1);
    levset = true;
  }

  //debug_log << "level" << levset << *levelType;
  
  float p1 = src.getValueCur(descriptor("P1"), BAD_METEO_ELEMENT_VAL);
  if (p1 != BAD_METEO_ELEMENT_VAL) {
    float hh = src.getValueCur(descriptor("hh"), BAD_METEO_ELEMENT_VAL);
    if (hh != BAD_METEO_ELEMENT_VAL && *levelType == -1) {
      if (!levset) *levelType = kIsobarLevel;
      *levelLo = p1;
    } else if (src.countParam(descriptor("P1"), false) > 1) {
      *levelLo = src.getValue(descriptor("P1"), 0, -1);
      *levelHi = src.getValue(descriptor("P1"), 100, -1);
      if (!levset) *levelType = kIsobarLevel;
    } else {
      *levelLo = p1; //src.getValue(descriptor("P1"), -1, false);
      if (!levset) *levelType = kIsobarLevel;
    }
    if (*levelLo != -1) ok = true;
  }

  if (!ok && src.hasParam(descriptor("hgpm"), false)) {
    *levelLo = src.getValue(descriptor("hgpm"), -1);
    if (!levset) *levelType = kGeopotentialLevel;
    if (*levelLo != -1) ok = true;
  }
  
  if (!ok && src.hasParam(descriptor("ha"), false)) {
    if (src.countParam(descriptor("ha"), false) > 1) {
      *levelLo = src.getValue(descriptor("ha"), 0, -1);
      *levelHi = src.getValue(descriptor("ha"), 100, -1);
      if (!levset) *levelType = kHeightLevel;
    } else {
      *levelLo = src.getValue(descriptor("ha"), -1);
      if (!levset) *levelType = kHeightLevel;
    }
    if (*levelLo != -1) ok = true;
  }
  
  if (!ok && src.hasParam(descriptor("zn"), false)) {
    *levelLo = src.getValue(descriptor("zn"), -1);
    if (*levelLo != -1 && *levelLo != -9999) { //TODO тут надо бы побольше таких данных посмореть
      if (!levset) *levelType = kDepthLevel;
    }
    if (*levelLo == -9999) {
      *levelLo = 0; //NOTE: в bufr идут данные с дескриптором zn с отсутствующим значением (есть ещё до неск. см от поверхности)
    }
    
    if (*levelLo != -1) ok = true;
  }

  if (!ok && src.hasParam(descriptor("h0"), false)) {
    *levelLo = src.getValueCur(descriptor("h0"), -1);
    *levelHi = src.getValueCur(descriptor("h0"), 100, -1);
    if (!levset) {
      if (-1 == *levelHi) {
	if (0 == *levelLo) {
	  *levelType = kSurfaceLevel;
	} else {
	  *levelType = kHeightLevel;
	}
      } else {
	*levelType = kHeightLevel;
      }
    }
    
    if (*levelLo != -1) ok = true;
  }


 if (!ok && src.hasParam(descriptor("hmsl"), false)) {
    *levelLo = src.getValueCur(descriptor("hmsl"), -1);
    *levelHi = src.getValueCur(descriptor("hmsl"), 100, -1);
    if (!levset) {
      if (-1 == *levelHi) {
	*levelType = kHeightMslLevel;
      } else {
	*levelType = kHeightMslLevel;
      }
    }
    
    if (*levelLo != -1) ok = true;
  }

 if (!ok && src.hasParam(descriptor("Pd"), false)) {
   *levelLo = src.getValue(descriptor("Pd"), -1);
   if (!levset) *levelType = kDepthPressLevel;
   if (*levelLo != -1) ok = true;
 }
  

  //статические данные 1-го порядка, данные о разностях
  if (!ok && (src.hasParam(descr_t(8023), false) || src.hasParam(descr_t(8024), false))) {
    if (!levset) *levelType = kUnknownLevel;
    *levelLo = -1;
    ok = true;
  }

  if (!ok && src.hasParam(descriptor("zs"), false)) {
    *levelLo = src.getValue(descriptor("zs"), -1);
    if (!levset) *levelType = kDepthSurfLevel;
    
    if (*levelLo != -1) ok = true;
  }

  //debug_log << ok << "parent=" << src.hasParent();
  if (!ok && src.hasParent()) {
    ok = findVertic(*src.parent(), levelType, levelLo, levelHi); 
  }
  
  //  debug_log << "type=" << *levelType << "level=" << *levelLo << *levelHi;

  return ok;
}

//! Разделение по уровням (память необходимо очистить самостоятельно)
QList<TMeteoData*> TMeteoDescriptor::splitWithLevel(TMeteoData* data, int ltype, float levLo, float levHi) const
{
  QList<TMeteoData*> fragmList;  

  QMutableListIterator<TMeteoData*> it(data->_childs);
  while (it.hasNext()) {
    TMeteoData* md = it.next();
    float v1 = -1, v2 = -1;
    int curltype = -1;
    findVertic(*md, &curltype, &v1, &v2);
    if (curltype != ltype || levLo != v1 || levHi != v2) {
      //debug_log << curltype << ltype << levLo << v1 << levHi << v2;
      TMeteoData* fragm = new TMeteoData;
      fragm->copyFrom(*md);
      setFullIdentification(fragm, md->parent());
      //fragm->set(descriptor("level_type"), md.getParam(descriptor("level_type")));
      //TMeteoData& child = fragm->addChild();
      //child.copyFrom(md);
      //fragm->_childs.last()._parent = fragm;
      fragmList.append(fragm);      
      it.remove();
    } else {
      fragmList << splitWithLevel(md, curltype, v1, v2);
    }
  }

  return fragmList;
}

//! Получение временной координаты с учетом родителей
/*! 
  \param src   Данные
  \param type  Тип времени
  \param start Первое значение времени
  \param end   Второе значение времени
  \return  true - если значение определено
*/
//TODO убрать секуды, если время прогноза
// bool TMeteoDescriptor::findTime(const TMeteoData& src, int* type, QDateTime* start, QDateTime* end) const
// {
//   if (type == 0 || start == 0 || end == 0) return false;

//   start->setTimeSpec(Qt::UTC);
//   end->setTimeSpec(Qt::UTC);
//   bool ok = true;
//   bool timeset = false;
  
//   if (-1 != *type) {
//     timeset = true;
//   }
  
//   if (!timeset && src.hasParam(descriptor("t_sign"))) {
//     *type = src.getValueCur(descriptor("t_sign"), -1);
//     timeset = true;
//   }

//   //первое значение времени
//   int add_descr = 0;
//   int tf_sign = src.getValueUp(descriptor("tf_sign"), -1);
  
//   if (tf_sign != -1) {
//     *type = kForecastTime;
//   }
//   MeteoDateTime dt1;
//   MeteoDateTime dtp;
//   if (tf_sign == 0) {
//     MeteoDateTime dt;
//     dt.readUp(src, descriptor("JJJ"),  descriptor("MM"), descriptor("YY"),
// 	      descriptor("GG"), descriptor("gg"), descriptor("sec"), add_descr);
//     add_descr += 100;
  
//     dt1.set(dt);
//     dt1.readUp(src, descriptor("JJJ"),  descriptor("MM"), descriptor("YY"),
// 	       descriptor("GG"), descriptor("gg"), descriptor("sec"), add_descr);

//     start->setDate(QDate(dt1.year, dt1.month, dt1.day));
//     start->setTime(QTime(dt1.hour, dt1.min == -1 ? 0 : dt1.min, dt1.sec == -1 ? 0 : dt1.sec));
//   } else {
//     MeteoDateTime dti(true);
//     //только у текущего узла
//     dti.readCur(src, descriptor("JJJi"), descriptor("MMi"), descriptor("YYi"),
// 		descriptor("GGi"),  descriptor("ggi"), descriptor("seci"), add_descr);
//     dtp.set(dti);   
//     //с учетом родителей
//     dtp.readUp(src, descriptor("JJJp"), descriptor("MMp"), descriptor("YYp"), 
// 	       descriptor("GGp"),  descriptor("ggp"), descriptor("secp"), add_descr);
    
//     dt1.readUp(src, descriptor("JJJ"),  descriptor("MM"), descriptor("YY"),
// 	       descriptor("GG"), descriptor("gg"), descriptor("sec"), add_descr);

//     start->setDate(QDate(dt1.year, dt1.month, dt1.day));
//     start->setTime(QTime(dt1.hour, dt1.min == -1 ? 0 : dt1.min, dt1.sec == -1 ? 0 : dt1.sec));
//     *start = start->addYears(dtp.year).addMonths(dtp.month).addDays(dtp.day).addSecs(dtp.sec + dtp.min*60 + dtp.hour*3600);
//   }

//   const TMeteoData* ttParent = src.findParent("TT");
//   int tt = kUnkTT;
//   if (ttParent != 0) {
//     tt = ttParent->getValueCur(descriptor("TT"), kUnkTT);
//   }
//   if (tt != kUnkTT) {
//     *type = kForecastTime;
//   }
//   int tttt = src.getValueUp(descriptor("TTTTT"), -1);

//   //debug_log << "tt" << tt<< tttt << tf_sign;


//   //второе значение
//   if (tt == kUnkTT && tttt != -1 && tf_sign == -1) { //тренд
//     *type = kForecastTime;
//     *end = start->addSecs(2*3600);
//   } else if (tf_sign == 3 || tf_sign == 4 || 
//       (tf_sign == -1 && tt == kAT)) { //прогноз на время, или от без указания окончания
//     *end = *start;
//   } else if (tt == kTL && tf_sign == -1) { //прогноз до времени
//     *end = *start;
//     //от родителя
//     if (0 != ttParent && 0 != ttParent->parent()) {
//       dt1.readUp(*(ttParent->parent()), descriptor("JJJ"),  descriptor("MM"), descriptor("YY"),
// 		 descriptor("GG"), descriptor("gg"), descriptor("sec"), add_descr);
//     }
//     start->setDate(QDate(dt1.year, dt1.month, dt1.day));
//     start->setTime(QTime(dt1.hour, dt1.min == -1 ? 0 : dt1.min, dt1.sec == -1 ? 0 : dt1.sec));
//   } else if (tf_sign == 5 && src.parent() != 0 && 
// 	     src.countParam(descriptor("tf_sign"), false) <= 1) { //указано начало изменения, поиск окончания
//     add_descr = 0;
//     MeteoDateTime dt;
//     dt.readUp(*(src.parent()), descriptor("JJJ"),  descriptor("MM"), descriptor("YY"),
// 	      descriptor("GG"), descriptor("gg"), descriptor("sec"), add_descr);

//     add_descr = 200;
//     MeteoDateTime dt1(dt);
//     dt1.readUp(src, descriptor("JJJ"),  descriptor("MM"), descriptor("YY"),
// 	       descriptor("GG"), descriptor("gg"), descriptor("sec"), add_descr);
 
//     end->setDate(QDate(dt1.year, dt1.month, dt1.day));
//     end->setTime(QTime(dt1.hour, dt1.min == -1 ? 0 : dt1.min, dt1.sec == -1 ? 0 : dt1.sec));
//     *end = end->addYears(dtp.year).addMonths(dtp.month).addDays(dtp.day).addSecs(dtp.sec + dtp.min*60 + dtp.hour*3600);
//   } else {
//     add_descr += 100;

//     //только у текущего узла
//     MeteoDateTime dtp1(true);
//     //с учетом родителей
//     dtp1.readUp(src, descriptor("JJJp"), descriptor("MMp"), descriptor("YYp"), 
// 		descriptor("GGp"),  descriptor("ggp"), descriptor("secp"), add_descr);
//     dt1.readUp(src, descriptor("JJJ"),  descriptor("MM"), descriptor("YY"),
// 	       descriptor("GG"), descriptor("gg"), descriptor("sec"), add_descr);
    
//     end->setDate(QDate(dt1.year, dt1.month, dt1.day));
//     end->setTime(QTime(dt1.hour, dt1.min == -1 ? 0 : dt1.min, dt1.sec == -1 ? 0 : dt1.sec));
    
//     *end = end->addYears(dtp1.year).addMonths(dtp1.month).addDays(dtp1.day).addSecs(dtp1.sec + dtp1.min*60 + dtp1.hour*3600);

//     if (*type == kForecastTime && dt1.hour == 24) {
//       *end = end->addDays(1);      
//     }
//   }

//   if (*end < *start) {
//     *end = end->addDays(1);
//   }

//   return ok;
// }


//! Получение временной координаты с учетом родителей
/*! 
  \param src   Данные
  \param type  Тип времени
  \param start Первое значение времени
  \param end   Второе значение времени
  \return  true - если значение определено
*/
bool TMeteoDescriptor::findTime(const TMeteoData& src, TimeParam* tp, int* type, QDateTime* start, QDateTime* end) const
{
  if (type == 0 || start == 0 || end == 0 || tp == 0) return false;
  bool ok = true;
  bool timeset = false;
  start->setTimeSpec(Qt::UTC);
  end->setTimeSpec(Qt::UTC);

  if (-1 != *type) {
    timeset = true;
  }  
  if (!timeset && src.hasParam(descriptor("t_sign"))) {
    float val = src.getValueCur(descriptor("t_sign"), -1);
    if (val != -1 && !qFuzzyCompare(val, BAD_METEO_ELEMENT_VAL)) {
      *type = val;
      timeset = true;
    }
  }
  
  //первое значение времени
  int add_descr = 0;
  int tf_sign = src.getValueCur(descriptor("tf_sign"), tp->tf_sign);
  tp->tf_sign = tf_sign;
  tp->tf_count = src.countParam(descriptor("tf_sign"), false);
  
  if (tf_sign != -1) {
    *type = kForecastTime;
  }

  //-
  MeteoDateTime dtc0(tp->dt0);
  dtc0.readCur(src, descriptor("JJJ"),  descriptor("MM"), descriptor("YY"),
	       descriptor("GG"), descriptor("gg"), descriptor("sec"), 0);
  MeteoDateTime dtc1;
  if (tp->dt1.year > 0) {
    dtc1.set(tp->dt1);
  } else {
    dtc1.set(dtc0);
  }
  dtc1.readCur(src, descriptor("JJJ"),  descriptor("MM"), descriptor("YY"),
	       descriptor("GG"), descriptor("gg"), descriptor("sec"), 100);
  

  MeteoDateTime dt1;
  MeteoDateTime dtp;
  if (tf_sign == 0) { //время выпуска прогноза
    add_descr += 100;
    dt1.set(dtc0);
    dt1.readCur(src, descriptor("JJJ"), descriptor("MM"), descriptor("YY"),
    		descriptor("GG"), descriptor("gg"), descriptor("sec"), add_descr);
    dt1.setIfEmpty(tp->dt1);
    dtc0.set(dt1);

    dt1.toQDateTime(start);
    // start->setDate(QDate(dt1.year, dt1.month, dt1.day));
    // start->setTime(QTime(dt1.hour, dt1.min == -1 ? 0 : dt1.min, dt1.sec == -1 ? 0 : dt1.sec));
  } else {
    MeteoDateTime dti(true);
    //только у текущего узла
    dti.readCur(src, descriptor("JJJi"), descriptor("MMi"), descriptor("YYi"),
		descriptor("GGi"),  descriptor("ggi"), descriptor("seci"), add_descr);
    dtp.set(dti);
    //с учетом родителей
    dtp.readUp(src, descriptor("JJJp"), descriptor("MMp"), descriptor("YYp"), 
	       descriptor("GGp"), descriptor("ggp"), descriptor("secp"), add_descr);

    dtc0.toQDateTime(start);
    // start->setDate(QDate(dtc0.year, dtc0.month, dtc0.day));
    // start->setTime(QTime(dtc0.hour, dtc0.min == -1 ? 0 : dtc0.min, dtc0.sec == -1 ? 0 : dtc0.sec));
    
    //debug_log << *start << dtp.year << dtp.month << dtp.day;
    *start = start->addYears(dtp.year).addMonths(dtp.month).addDays(dtp.day).addSecs(dtp.sec + dtp.min*60 + dtp.hour*3600);
  }

  if (*type == kForecastTime && dtc0.hour == 24) {
    start->setTime(QTime(0, start->time().minute(), start->time().second(), start->time().msec()));
    *start = start->addDays(1);
  }

  int tt = kUnkTT;
  tt = src.getValueCur(descriptor("TT"), kUnkTT);
  if (tt != kUnkTT) {
    *type = kForecastTime;
    tp->tt = tt;
    tp->dt_tt.set(tp->dt0);
    if (tp->dt_tt.min == -1) { tp->dt_tt.min = 0; }
    if (tp->dt_tt.sec == -1) { tp->dt_tt.sec = 0; }    
  } else {
    tt = tp->tt;
    if (tt != kUnkTT) {
      *type = kForecastTime;
    }
  }
   
  int tttt = src.getValueCur(descriptor("TTTTT"), tp->tttt);
  tp->tttt = tttt;
  
  //debug_log << "tt" << tt<< tttt << tf_sign;

  //второе значение
  if (tt == kUnkTT && tttt != -1 && tf_sign == -1) { //тренд
    *type = kForecastTime;
    *end = start->addSecs(2*3600);
  } else if (tf_sign == 3 || tf_sign == 4 || 
	     (tf_sign == -1 && tt == kAT)) { //прогноз на время, или от без указания окончания
    *end = *start;
  } else if (tt == kTL && tf_sign == -1) { //прогноз до времени
    *end = *start;
    //от родителя
    tp->dt_tt.toQDateTime(start);
    // start->setDate(QDate(tp->dt_tt.year, tp->dt_tt.month, tp->dt_tt.day));
    // start->setTime(QTime(tp->dt_tt.hour, tp->dt_tt.min, tp->dt_tt.sec));
  } else if (tf_sign == 5 && tp->tf_count == 1) { //указано начало изменения, поиск окончания
    add_descr = 200;
    MeteoDateTime dt1(dtc0);
    dt1.readUp(*(src.parent()), descriptor("JJJ"),  descriptor("MM"), descriptor("YY"),
	       descriptor("GG"), descriptor("gg"), descriptor("sec"), add_descr);

    dt1.toQDateTime(end);
    // end->setDate(QDate(dt1.year, dt1.month, dt1.day));
    // end->setTime(QTime(dt1.hour, dt1.min == -1 ? 0 : dt1.min, dt1.sec == -1 ? 0 : dt1.sec));
    *end = end->addYears(dtp.year).addMonths(dtp.month).addDays(dtp.day).addSecs(dtp.sec + dtp.min*60 + dtp.hour*3600);
  } else if (tf_sign == 5 && tp->tf_count == 0) {
    dtc1.toQDateTime(end);
    // end->setDate(QDate(dtc1.year, dtc1.month, dtc1.day));
    // end->setTime(QTime(dtc1.hour, dtc1.min == -1 ? 0 : dtc1.min, dt1.sec == -1 ? 0 : dtc1.sec));
  } else if (tf_sign == 0 && tp->tf_count == 3) {
    dt1.set(dtc0);
    dt1.readCur(src, descriptor("JJJ"),  descriptor("MM"), descriptor("YY"),
		descriptor("GG"), descriptor("gg"), descriptor("sec"), 200);
    dtc1.set(dt1);

    dt1.toQDateTime(end);

  } else {
    add_descr += 100;
    dtc1.set(dtc0);
    dtc1.readCur(src, descriptor("JJJ"),  descriptor("MM"), descriptor("YY"),
		 descriptor("GG"), descriptor("gg"), descriptor("sec"), 100);

    
    //только у текущего узла
    MeteoDateTime dtp1(true);
    //с учетом родителей
    dtp1.readUp(src, descriptor("JJJp"), descriptor("MMp"), descriptor("YYp"), 
		descriptor("GGp"),  descriptor("ggp"), descriptor("secp"), add_descr);

    dtc1.toQDateTime(end);
    *end = end->addYears(dtp1.year).addMonths(dtp1.month).addDays(dtp1.day).addSecs(dtp1.sec + dtp1.min*60 + dtp1.hour*3600);
  }

  if (*type == kForecastTime && dtc1.hour == 24) {
    end->setTime(QTime(0, end->time().minute(), end->time().second()));
    *end = end->addDays(1);      
  }

  if (*end < *start) {
    //    *end = end->addDays(1); //TODO почему?
    *end = end->addMonths(1); //переход через месяц
  }

  tp->dt0.set(dtc0);
  tp->dt1.set(dtc1);
  return ok;
}


//! дата/время создания данных
QDateTime TMeteoDescriptor::dateTime(const TMeteoData& data) const
{
  int day, hour;
  control::QualityControl quality1, quality2;  
  QDate date = QDate::currentDate();
  TMeteoParam param;
  QString code1, code2;

  if(!getDayHour(data, &day, &hour, &quality1, &quality2) || 
     quality1 >= control::MISTAKEN || quality2 >= control::MISTAKEN) {
    // error_log << QObject::tr("Ошибка получения дня и часа. Показатель качества") 
    // 	      << quality1 << quality2;
  }

  int year, month, min;
  float sec, msec;

  TMeteoParam minParam = data.getParam(descriptor("gg"));
  if (!minParam.isValid()) {
    min = 0;
  } else {
    min = static_cast<int>(minParam.value());
  }

  TMeteoParam secParam = data.getParam(descriptor("sec"));
  if (!secParam.isValid()) {
    sec = 0;
  } else {
    sec = static_cast<int>(secParam.value());
    msec = (secParam.value() - sec) * 1000;
  }

  TMeteoParam yearParam = data.getParam(descriptor("JJJ"));
  if (!yearParam.isValid()) {
    year = date.year();
  } else {
    year = static_cast<int>(yearParam.value());
  }

  if (year < 1000) {
    year += 2000;
  }
  if (year > QDate::currentDate().year()) {
    year -= 1000;
  }

  TMeteoParam monthParam = data.getParam(descriptor("MM"));
  if (!monthParam.isValid()) {
    month = date.month();
  } else {
    month = static_cast<int>(monthParam.value());
  }

  QDateTime dt(QDate(year, month, day), QTime(hour, min, sec, msec), Qt::UTC);
  if (hour == 24) {
    dt = dt.addDays(1);
  }
  
  //  var(dt);

  int diffutc = static_cast<int>(data.getValue(descriptor("diff_utc"), 0));

  return dt.addSecs(-diffutc * 3600);
}

  //! день и час создания данных
  bool TMeteoDescriptor::getDayHour(const TMeteoData& data, int *day, int *hour, 
				    control::QualityControl* qual1, control::QualityControl* qual2) const
{
  const TMeteoParam& phour = data.getParam(descriptor("GG"), 0);
  *hour = static_cast<int>(phour.value());
  *qual2 = phour.quality(); 
  if(!data.getValue(descriptor("YY"), day, qual1 ) || 
     !phour.isValid())
    {
      return false;
    }
  
  if(*day > 31) *day-= 50;  

  return true;
}

void TMeteoDescriptor::setFullIdentification(TMeteoData* data, TMeteoData* parent) const
{
  if (0 == data || 0 == parent) return;

  const QList<descr_t>  dlist = parent->getDescrList();
  for (int idx = 0; idx < dlist.size(); idx++) {
    descr_t descr = dlist.at(idx) % 100000;
    if (descr >= 9000 && 
	(descr < 29000 || descr >= 55000)) {
      continue;
    }
    if (!data->hasParam(dlist.at(idx), false)) {
      const QMap<int, TMeteoParam> plist = parent->getParamList(dlist.at(idx));
      QMap<int, TMeteoParam>::const_iterator it = plist.constBegin();
      while (it != plist.constEnd()) {
	data->add(dlist.at(idx), it.value(), it.key());
	++it;
      }
    }
  }

  setFullIdentification(data, parent->parent());
  
}

void TMeteoDescriptor::copyChildIdentification(TMeteoData* data, const TMeteoData* parent) const
{
  if (0 == data || 0 == parent) return;

  const QList<descr_t>  dlist = parent->getDescrList();
  for (int idx = 0; idx < dlist.size(); idx++) {
    descr_t descr = dlist.at(idx) % 100000;
    if (descr >= 8000 && descr < 9000) {
      if (!data->hasParam(dlist.at(idx), false)) {
	const QMap<int, TMeteoParam> plist = parent->getParamList(dlist.at(idx));
	QMap<int, TMeteoParam>::const_iterator it = plist.constBegin();
	while (it != plist.constEnd()) {
	  data->add(dlist.at(idx), it.value(), it.key());
	  ++it;
	}
      }
    }
  }
}

//! преобразование величины в соответствии с единицами измерения
float TMeteoDescriptor::convertUnit(descr_t descr, float val, const QString& srcUnit) const
{
  if (!_prop.contains(descr) || srcUnit.isEmpty()) {
    return val;
  }
  
  return convertUnit(val, srcUnit, _prop.value(descr).units);
}

//! преобразование величины в соответствии с единицами измерения
float TMeteoDescriptor::convertUnit(float val, const QString& asrcUnit, const QString& dstUnit) const
{
  QString srcUnit = asrcUnit.toUpper();
  if (srcUnit == dstUnit.toUpper() || srcUnit.isEmpty() || dstUnit.isEmpty()) {
    return val;
  }
  
  if (srcUnit == "PA" && dstUnit == "hPa") {
    return val / 100.;
  }

  if (srcUnit == "PA/S" && dstUnit == "hPa/h") {
    return val / 100. * 360;
  }

  if (srcUnit == "K" && dstUnit == "C") {
    return val - 273.15;
  }

  if ((srcUnit == "KG M-2" || srcUnit == "KG/M**2") && dstUnit == "mm") { //осадки, испарение
    return val;
  }
 
  if (srcUnit == "M/S" && dstUnit == "km/h") {
    return val * 3.6;
  }

  if (srcUnit == "M" && dstUnit == "cm") {
    return val * 100;
  }

  if (srcUnit == "M" && dstUnit == "km") {
    return val / 1000;
  }

  if (srcUnit == "J M-2" && dstUnit == "J cm-2") {
    return val / 10000.;
  }

  if (srcUnit == "J M-2" && dstUnit == "kJ m-2") {
    return val /= 1000.;
  }

  if (srcUnit == "KG M-2 H-1" && dstUnit == "mm/h") {
    return val;
  }

  if ((srcUnit == "KG M-2 S-1" || srcUnit == "KG/(M**2)S") && dstUnit == "mm/h") {
    return val * 3600;
  }
  
  if ((srcUnit == "M S-1" || srcUnit == "M/S") && dstUnit == "mm/h") {
    return val * 1000 * 3600;
  }

  if (srcUnit == "KG M-3" && dstUnit == "g/sm3") {
    return val / 1000;
  }

  if ( (srcUnit == "FEET" || srcUnit == "FOOT" || srcUnit == "FT" ) && dstUnit == "m") {
    return val / 3.28084;
  }
  if (srcUnit == "KNOT" && dstUnit == "m/s") {
    return val * 0.514;
  }

  if (srcUnit == "KM/H" && dstUnit == "m/s") {
    return val / 3.6;
  }

  if (srcUnit == "M/S" && dstUnit == "sm/s") {
    return val *100;
  }

  if (srcUnit == "CODE TABLE 20011" && dstUnit == "num") {
    if (val < 2)  return val;
    if (val == 2) return 3;
    if (val < 6)  return val + 1;
    if (val == 6) return 8;
    return val + 2; 
  }

  // if (srcUnit == "" && dstUnit == "") {
  // }

  // if (srcUnit == "" && dstUnit == "") {
  // }

  // if (srcUnit == "" && dstUnit == "") {
  // }
  
  
  return val;
}



// void TMeteoDescriptor::modifyForSave(TMeteoData* data)
// {
//   if (nullptr == data) return;
  
//   QList< TMeteoData*> mdList = data->findChilds(descriptor("Nh"));

//   for (auto  md : mdList) {
//     if (nullptr == md) continue;
//     TMeteoParam* mp = md->paramPtr(descriptor("Nh"));
//     if (nullptr == mp) continue;
//     float val = mp->value();
//     if (val < 2) { val = val; }
//     else if (val == 2) { val = 3; }
//     else if (val < 6)  { val += 1; }
//     else if (val == 6) { val =  8; }
//     else { val += 2; }

//     mp->setValue(val);
//   }
  
// }

}


/* ед-цы измерения bufr
allunits.count() 82            
"00001" "CCITTIA5"             
"01001" "NUMERIC"              
"01012" "DEGREE TRUE"          
"01013" "M/S"                  
"02005" "K"                    
"02026" "M"                    
"02063" "DEGREE"               
"02067" "Hz"                   
"02082" "KG"                   
"02091" "A"                    
"02100" "dB"                   
"02109" "DEGREE/S"             
"02114" "M**2"                 
"02116" "%"                    
"02123" "W"                    
"02126" "S"                    
"02168" "PA"                   
"02173" "DEGREE2"              
"04001" "YEAR"                 
"04002" "MONTH"                
"04003" "DAY"                  
"04004" "HOUR"                 
"04005" "MINUTE"               
"04006" "SECOND"               
"06030" "RAD/M"                
"06232" "RADS/M"               
"07003" "M**2/S**2"            
"07009" "GPM"                  
"10082" "MS-1"                 
"11005" "PA/S"                 
"11021" "1/S"                  
"11023" "M**2/S"               
"11035" "M/S**2"               
"11072" "KM/S"                 
"11075" "M**(2/3)/S"           
"11083" "KM/HOUR"              
"11084" "KNOT"                 
"12023" "C"                    
"12072" "WM**(-2)SR**(-1)"     
"12075" "WM**(-3)SR**(-1)"     
"12194" "W/M**2*STER*M**(-1)"  
"12196" "W/M**2*STER"          
"13001" "KG/KG"                
"13005" "KG/M**3"              
"13011" "KG/M**2"              
"13014" "KG/(M**2)S"           
"13080" "pH"                   
"13081" "S M-1"                
"13084" "LUMEN"                
"13085" "V"                    
"13195" "KG/M**2HOUR"          
"13203" "KG/M**2*HOUR"         
"13208" "KG/M**2*S"            
"13241" "J/KG"                 
"14001" "J/M**2"               
"14017" "W/M**2"               
"14045" "(W/M**2)*(1/SR)*CM"   
"14046" "(W/M**2)*(1/SR)*(1/M)"
"15001" "DU"                   
"15011" "LOG (1/M2)"           
"15026" "MOLMOL-1"             
"15027" "KG/M3"                
"15036" "N-UNITS"              
"15037" "RADIANS"              
"15202" "1/M**2"               
"20091" "FEET"                 
"21152" "dB/DEG"               
"21157" "dB/M"                 
"22059" "PART PER THOUSAND"    
"22069" "M2HZ-1"               
"22081" "1/M"                  
"22082" "M**2S"                
"22083" "M**3"                 
"22092" "M**2S/RAD"            
"22093" "M**4"                 
"22198" "M**2*S/RAD"           
"23017" "M**3/S"               
"24001" "Bq"                   
"24011" "mSv"                  
"24021" "Bq/M**3"              
"24022" "Bq/L"                 
"25076" "LOG (1/M)"            
*/
