#include "meteo_data.h"
#include "tmeteodescr.h"

#include <cross-commons/debug/tlog.h>
#include <commons/mathtools/mnmath.h>

#include <QtDebug>
#include <qglobal.h>
#include <qdatastream.h>

void meteodescr::MeteoDateTime::setIfEmpty(const MeteoDateTime& m)
{
  if (sec   == std::numeric_limits<float>::max()) { sec  = m.sec;  }
  if (min   == -1) { min  = m.min;  }
  if (hour  == -1) { hour = m.hour; }
  if (day   == -1) { day  = m.day;  }
  if (month == -1) { month = m.month; }
  if (year  == -1) { year  = m.year;  }
}

void meteodescr::MeteoDateTime::readUp(const TMeteoData& src, int dy, int dmo, int dd, int dh, int dm, int ds, int add_descr)
{
  sec	= src.getValueUp(ds, add_descr, sec);
  min	= src.getValueUp(dm, add_descr, min);
  hour	= src.getValueUp(dh, add_descr, hour);
  day	= src.getValueUp(dd, add_descr, day);
  month = src.getValueUp(dmo, add_descr, month);
  if (day > 31) day -= 50;

  int ny = src.getValueUp(dy, add_descr, -1);
  if (ny != -1) {
    year = ny;
    if (year < 1000) {
      year += 2000;
    }     
  } 
}

void meteodescr::MeteoDateTime::readCur(const TMeteoData& src, int dy, int dmo, int dd, int dh, int dm, int ds, int add_descr)
{
  sec	= src.getValueCur(ds, add_descr, sec);
  min	= src.getValueCur(dm, add_descr, min);
  hour	= src.getValueCur(dh, add_descr, hour);
  day	= src.getValueCur(dd, add_descr, day);
  month = src.getValueCur(dmo, add_descr, month);
  if (day > 31) day -= 50;

  int ny = src.getValueCur(dy, add_descr, -1);
  if (ny != -1) {
    year = ny;
    if (year < 1000) {
      year += 2000;
    }
    if (year > QDate::currentDate().year()) {
      year -= 1000;
    }
  }
}

bool meteodescr::MeteoDateTime::toQDateTime(QDateTime* dt)
{
  if (nullptr == dt) {
    return false;
  }
  
  float second = 0;
  float msec = 0;
  if (sec != std::numeric_limits<float>::max()) {
    second = static_cast<int>(sec);
    msec = (sec - second) * 1000;
  }

  dt->setDate(QDate(year, month, day));
  dt->setTime(QTime(hour, min == -1 ? 0 : min, second, msec));

  if (hour == 24) {
    *dt = dt->addDays(1);
  }
  
  return true;
}

/*! 
  \param ac Кодовое значение
  \param av Числовое значение
  \param aq Показатель качества
*/
TMeteoParam::TMeteoParam(const QString& ac, float av, control::QualityControl aq, const QString& descrip /*= QString()*/): 
  _code(ac),_value(av), _quality(aq), _description(descrip),_controlFlag(0)
{
}

TMeteoParam::TMeteoParam(const QString& ac, float av, control::QualityControl aq,
			 ctrlfl_t acontrolFlag, const QString& descrip):
  _code(ac), _value(av), _quality(aq), _description(descrip), _controlFlag(acontrolFlag)
{
  
}

TMeteoParam::TMeteoParam()
{
  _value = BAD_METEO_ELEMENT_VAL;
  _code = "";
  _quality = control::NO_OBSERVE;
  _description = QString();
  _controlFlag = 0;
}

TMeteoParam::~TMeteoParam()
{
}


//! Проверка показателя качества
/*! 
  \return true - значение ошибочное, false - иначе
*/
bool TMeteoParam::isInvalid() const
{
  if (_quality == control::MISTAKEN || 
      _quality == control::NO_OBSERVE || 
      (MnMath::isEqual(_value, BAD_METEO_ELEMENT_VAL) && _code.isEmpty())) {
    return true;
  }

  QString code = _code;
  code.remove('/');
  if (MnMath::isEqual(_value, BAD_METEO_ELEMENT_VAL) && code.isEmpty()) {
    return true;
  }

  return false;
}

//! Проверка показателя качества
/*! 
  \return true - значение не ошибочное и не сомнитльное, false - значение верное или не подвергалось контролю
*/
bool TMeteoParam::isValid() const
{
  if (isInvalid() || _quality == control::DOUBTFUL) {
    return false;
  }

  return true;
}

//! Установка показателя качества
/*! 
  \param qual Новое значение показателя качества
  \param anyway если false - показатель качества устанавливается только при ухудшении значения; true - всегда
*/
void TMeteoParam::setQuality(QChar qual, ctrlfl_t ctrlFlag, bool anyway)
{
  setQuality(control::QualityControl(qual.cell()), ctrlFlag, anyway);
}

//! Установка показателя качества
/*! 
  \param qual Новое значение показателя качества
  \param controlFlag - флаг типа проведённого контроля
  \param anyway если false - показатель качества устанавливается только при ухудшении значения; true - всегда
*/
void TMeteoParam::setQuality(control::QualityControl qual, ctrlfl_t controlFlag, bool anyway)
{
  _controlFlag |= controlFlag;
  
  if (anyway || _quality == control::NO_CONTROL) {
    _quality = qual;
    return;
  }

  switch (qual) {
  case control::RIGHT:
  case control::DOUBTFUL:
  case control::MISTAKEN:
    if (_quality < qual) {
      _quality = qual;
    }
    break;
  default:
    _quality = qual;
  }
}
 
//! Проверка на равенство только по числовому значению
bool TMeteoParam::equal( const float& other ) const 
{
  return ((_value + 1E-6) >= other) && ((_value - 1E-6) <= other);
}

bool TMeteoParam::operator==( const TMeteoParam& other ) const 
{
  return ((_value + 1E-6) >= other._value) && 
         ((_value - 1E-6) <= other._value) && 
         (_code == other._code) &&
         (_quality == other._quality);
}

QString TMeteoParam::toString() const
{
  QString out;
  out += "val=" + _code + " (" + QString::number(_value) + ") " + " q=" + QString::number(_quality);
  return out;
}

QDebug& operator<<(QDebug& out, const TMeteoParam& mtp)
{
  out << "val=" << mtp.code() << "(" << mtp.value() << ')'
      << " q=" << mtp.quality() << "(" << bin << mtp.controlFlag() << ")";
  if (!mtp.description().isEmpty()) {
    out << mtp.description();
  }
  return out;
}

//----


TMeteoData::TMeteoData()
{
}

TMeteoData::TMeteoData(const TMeteoData& src)
{
  copyFrom(src);
}

TMeteoData::~TMeteoData()
{
  while (!_childs.isEmpty()) {
    delete _childs.takeFirst();
  }
  _childs.clear();
}



TMeteoData& TMeteoData::addChild()
{
  TMeteoData* md = new TMeteoData;
  _childs.append(md);
  _childs.last()->_parent = this;
  
  return *(_childs.last());
}

//! Добавить новый метеопараметр
/*! 
 *  \param descr Номер дескриптора
 *  \param param Метеопараметр
 *  \param add_descr индекс (если надо выделить набор метеопараметров
 *  с одним дескриптором от
 *  других с тем же номером дескриптора)
 */
void TMeteoData::add(descr_t descr, const TMeteoParam& param, int add_descr)
{
  if (!_data.contains(descr)) {
    QMap<int, TMeteoParam> met;
    met.insert(add_descr, param);
    _data.insert(descr, met);
  } else {
    if (!param.isInvalid() && 
	(!_data[descr].contains(add_descr) || param.quality() <= _data[descr].value(add_descr).quality())) {
      _data[descr].insert(add_descr, param);
    }
  }
}

//! Добавить новый метеопараметр
/*! 
 *  \param descr Номер дескриптора
 *  \param param Метеопараметр
 *  \param add_descr индекс (если надо выделить набор метеопараметров
 *  с одним дескриптором от других с тем же номером дескриптора)
 *  \param wichoutCheck - true - без проверка на валидность
 */
void TMeteoData::add(descr_t descr, const TMeteoParam& param, int add_descr, bool withoutCheck)
{
  if (withoutCheck == false) {
    add(descr, param, add_descr);
    return;
  }
  
  if (!_data.contains(descr)) {
    QMap<int, TMeteoParam> met;
    met.insert(add_descr, param);
    _data.insert(descr, met);
  } else {
    _data[descr].insert(add_descr, param);
  }
}

//! Добавить новый метеопараметр
/*! 
 *  \param descr Номер дескриптора
 *  \param param Метеопараметр
 *  \param add_descr индекс (если надо выделить набор метеопараметров
 *  с одним дескриптором от
 *  других с тем же номером дескриптора)
 */
void TMeteoData::add(descr_t descr, const TMeteoParam& param)
{
  if (!_data.contains(descr)) {
    QMap<int, TMeteoParam> met;
    met.insert(0, param);
    _data.insert(descr, met);
  } else {
    int idx  = _data.value(descr).count() * 100;
    if (TMeteoDescriptor::instance()->isSingle(descr)) {
      //idx = (_data.value(descr).count() - 1) * 100;
      if (_data.value(descr).value(idx).quality() < param.quality()) {
        return;
        }
    } else {
//      QList<int> keys = _data.value(descr).keys();
//      qSort(keys);
//      idx = keys.last() + 100;
      // int max = -1;
      // QHashIterator<int, TMeteoParam> it(_data[descr]);
      // while ( it.hasNext() ) {
      //   max = qMax(max, it.next().key());
      // }
      // idx = max + 100;
    }
    //   if (!param.isInvalid()) { с этим условием нарушается порядок индексов в аэрологических и др, где несколько значений
    _data[descr].insert(idx, param);
    //}
  }
}

//! Добавить новый метеопараметр
/*! 
 *  \param descr Номер дескриптора
 *  \param param Метеопараметр
 *  \param add_descr индекс (если надо выделить набор метеопараметров
 *  с одним дескриптором от
 *  других с тем же номером дескриптора)
 */
void TMeteoData::addNoSingle(descr_t descr, const TMeteoParam& param)
{
  if (!_data.contains(descr)) {
    QMap<int, TMeteoParam> met;
    met.insert(0, param);
    _data.insert(descr, met);
  } else {
    int idx;
    // QList<int> keys = _data.value(descr).keys();
    // qSort(keys);
    // idx = keys.last() + 100;
    idx = (_data.value(descr).count()) * 100 ;

    //   if (!param.isInvalid()) { с этим условием нарушается порядок индексов в аэрологических и др, где несколько значений
    _data[descr].insert(idx, param);
    //}
  }
}



//! Добавление нового параметра по имени, в т.ч. и дополнительные
void TMeteoData::add(const QString& name, const TMeteoParam& param)
{
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  if (d->contains(name)) {
    add(d->descriptor(name), param);
  } else if (d->isAdditional(name)) {
    meteodescr::Additional prop = d->additional(name);
    if (prop.index == -1) {
      add(prop.descr, param);
    } else {
      add(prop.descr, param, prop.index * 100);
    }
//  } else { закоментировано, потому что при получении старых данных с hgr замедляется работа
//    error_log << QObject::tr("Неизвестное название дескриптора '%1'").arg(name);
  }
}


//! Заменить последний метеопараметр (если не было, добавить)
/*! 
  \param descr Номер дескриптора
  \param param Метеопараметр
*/
void TMeteoData::set(descr_t descr, const TMeteoParam& param)
{
  if (!_data.contains(descr)) {
    QMap<int, TMeteoParam> met;
    met.insert(0, param);
    _data.insert(descr, met);
  } else {
    int idx = (_data.value(descr).count() - 1) * 100 ;
    _data[descr].insert(idx, param);
  }  
}

//! Заменить последний метеопараметр (если не было, добавить), с учетом доп имен
/*! 
  \param name Название дескриптора
  \param param Метеопараметр
*/
void TMeteoData::set(const QString& name, const TMeteoParam& param)
{
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  if (d->contains(name)) { //базовый дескриптор
    set(d->descriptor(name), param);
  } else if (d->isAdditional(name)) {
    meteodescr::Additional prop = d->additional(name);
    if (prop.index == -1) {
      //имя с диапазоном, заменяем тот параметр, который соответствует диапазону, т.е.
      //если введут CL, но значение для CH, заменяется CH
      QString addName = TMeteoDescriptor::instance()->findAdditional(prop.descr, -1, param.value());
      if (name != addName) {
    warning_log << QObject::tr("Значение %1 не соответствует диапазону метеопараметра %2").arg(param.value()).arg(name);
      }
      TMeteoParam* mp = meteoParamPtr(addName, false);
      if (0 == mp) {
	add(prop.descr, param);
      } else {
	*mp = param;
      }	
    } else {
      add(prop.descr, param, prop.index * 100);
    }
  } else {
    error_log << QObject::tr("Неизвестное название дескриптора '%1'").arg(name);
  }
}

//! Добавляет (с заменой) параметры в родительских данных, добавляет всех child-ов
void TMeteoData::set(const TMeteoData& data)
{
  QMap<QString, TMeteoParam> parent = data.allByNames(false);

  QMapIterator<QString, TMeteoParam> it(parent);
  while (it.hasNext()) {
    it.next();
    set(it.key(), it.value());
  }
  
  for (int idx = 0; idx < data._childs.size(); idx++) {
    TMeteoData& ch = addChild();
    ch.copyFrom(*(data._childs.at(idx)));
  }
}

void TMeteoData::setStation(int station_number)
{
  int II, iii;
  II = station_number / 1000;
  iii = station_number - II*1000;
  //set center
  set(TMeteoDescriptor::instance()->descriptor("II"),  TMeteoParam(QString::number(II), II, control::RIGHT));
  set(TMeteoDescriptor::instance()->descriptor("iii"), TMeteoParam(QString::number(iii), iii, control::RIGHT));  
  set(TMeteoDescriptor::instance()->descriptor("station_index"),  
      TMeteoParam(QString::number(station_number), station_number, control::RIGHT));
}

void TMeteoData::setStation(const QString& station)
{
  bool ok = false;
  int index = station.toInt(&ok);
  if ( ok ) {
    setStation(index);
  }
  else {
    setCCCC(station);
  }
}

void TMeteoData::setCCCC( const QString& CCCC )
{
  set(TMeteoDescriptor::instance()->descriptor("CCCC"),  TMeteoParam( CCCC, 0, control::RIGHT));
}

void TMeteoData::setDateTime(const QDateTime &adt ){
  if(!adt.isValid()) return;
  //День месяца
  add(TMeteoDescriptor::instance()->descriptor("YY"),  TMeteoParam(QString::number(adt.date().day()), adt.date().day(),control::RIGHT), 0);
  //Месяц года
  add(TMeteoDescriptor::instance()->descriptor("MM"),  TMeteoParam(QString::number(adt.date().month()), adt.date().month(),control::RIGHT), 0);
  //Год
  add(TMeteoDescriptor::instance()->descriptor("JJJ"),  TMeteoParam(QString::number(adt.date().year()), adt.date().year(),control::RIGHT), 0);
  //Срок наблюдения, час
  add(TMeteoDescriptor::instance()->descriptor("GG"),  TMeteoParam(QString::number(adt.time().hour()), adt.time().hour(),control::RIGHT), 0);
  //Срок наблюдения, минут
  add(TMeteoDescriptor::instance()->descriptor("gg"),  TMeteoParam(QString::number(adt.time().minute()), adt.time().minute(),control::RIGHT), 0);
  //Срок наблюдения, секунд
  add(TMeteoDescriptor::instance()->descriptor("sec"),  TMeteoParam(QString::number(adt.time().second()), adt.time().second(),control::RIGHT), 0);
}

//! Установить координаты. Широта: +/-90. Долгота: +/-180
void TMeteoData::setCoord(float fi, float la)
{
  set(TMeteoDescriptor::instance()->descriptor("La"),  TMeteoParam("",fi,control::RIGHT));
  set(TMeteoDescriptor::instance()->descriptor("Lo"),  TMeteoParam("",la,control::RIGHT));
}

//! Установить координаты. Широта: +/-90. Долгота: +/-180
void TMeteoData::setCoord(float fi, float la, float h)
{
  set(TMeteoDescriptor::instance()->descriptor("La"), TMeteoParam("", fi, control::RIGHT));
  set(TMeteoDescriptor::instance()->descriptor("Lo"), TMeteoParam("", la, control::RIGHT));
  set(TMeteoDescriptor::instance()->descriptor("h0_station"),  TMeteoParam("", h, control::RIGHT));
}

//! Добавить новый метеопараметр
/*! 
  \param descr Номер дескриптора
  \param acode Кодовое значение
  \param aval Числовое значение
  \param aquality Показатель качества
  \param unit Единицы измерения (для проверки и пересчёте при необходимости)
  \param baseindex Значение для формирования индекса (если надо выделить набор метеопараметров с одним дескриптором от
  других с тем же номером дескриптора)
*/
void TMeteoData::add(descr_t descr, const QString& acode, float aval, control::QualityControl aquality, 
		     const QString unit)
{
  float val = aval;
  if (aquality < control::MISTAKEN) {
    val = TMeteoDescriptor::instance()->convertUnit(descr, aval, unit);
  }
  
  add(descr,TMeteoParam(acode, val, aquality));
}

//! все имена с дополнительными
QStringList TMeteoData::allNames() const
{
  QStringList names;
  
  QHashIterator<descr_t, QMap<int, TMeteoParam> > it(_data);
  while (it.hasNext()) {
    it.next();
    
    descr_t descr = it.key();
    QList<int> keys = it.value().keys();
    qSort(keys);
    
    for (int idx = 0; idx < keys.count(); idx++) {
      QString name = TMeteoDescriptor::instance()->findAdditional(descr, idx, it.value().value(keys.at(idx)).value());
      if (name.isEmpty()) {
	name = TMeteoDescriptor::instance()->name(descr);
      }
      names << name;
    }
  }
  
  for (int idx = 0; idx < _childs.size(); idx++) {
    names << _childs.at(idx)->allNames();
  }

  names.removeDuplicates();
  return names;
}

QList<QPair<descr_t, TMeteoParam> > TMeteoData::allParams() const
{
  QList<QPair<descr_t, TMeteoParam> > mp;

  QHashIterator<descr_t, QMap<int, TMeteoParam> > it(_data);
  while (it.hasNext()) {
    it.next();
    
    descr_t descr = it.key();
    QList<int> keys = it.value().keys();
    qSort(keys);

    for (int idx = 0; idx < keys.count(); idx++) {
      // QString description = TMeteoDescriptor::instance()->property(descr).description;
      // if (!description.isEmpty() && !it.value().value(keys.at(idx)).isInvalid()) {
      mp << QPair<descr_t, TMeteoParam>(descr, it.value().value(keys.at(idx)));
      // }
    }
  }
  
  for (int idx = 0; idx < _childs.size(); idx++) {
    mp << _childs.at(idx)->allParams();
  }

  return mp;
}

QMap<QString, TMeteoParam> TMeteoData::allByNames(bool withChilds /* = true*/) const
{
  QMap<QString, TMeteoParam> mp;

  QHashIterator<descr_t, QMap<int, TMeteoParam> > it(_data);
  while (it.hasNext()) {
    it.next();
    
    descr_t descr = it.key();
    QList<int> keys = it.value().keys();
    qSort(keys);
    
    for (int idx = 0; idx < keys.count(); idx++) {
      // QString description = TMeteoDescriptor::instance()->property(descr).description;
      // if (!description.isEmpty() && !it.value().value(keys.at(idx)).isInvalid()) {
      QString name = TMeteoDescriptor::instance()->findAdditional(descr, idx, it.value().value(keys.at(idx)).value());
      if (name.isEmpty()) {
	name = TMeteoDescriptor::instance()->name(descr);
      }
      mp.insertMulti(name, it.value().value(keys.at(idx)));
      // }
    }
  }
  
  if (withChilds) {
    for (int idx = 0; idx < _childs.size(); idx++) {
      mp.unite(_childs.at(idx)->allByNames());
    }
  }

  return mp;
}

//! Получение кодового значения (первого записанного, если несколько), с учетом дочерних
/*! 
  \param descr Номер дескриптора
  \return Кодовое значение
*/
QString TMeteoData::getCode(descr_t descr, bool withChilds /*= true*/) const
{
  if( hasParam(descr, false)) {
    return first(descr).code();
  }
  
  if (withChilds) {
    for (int idx = 0; idx < _childs.count(); idx++) {
      QString code = _childs.at(idx)->getCode(descr, withChilds);
      if (code != QString()) return code;
    }
  }

  return QString();  
}

//! Получение кодового значения (первого записанного, если несколько), с поиском у родителей
/*! 
  \param descr Номер дескриптора
  \return Кодовое значение
*/
QString TMeteoData::getCodeUp(descr_t descr) const
{
  const TMeteoData* data = findParent(descr);
  if (0 != data) {
    return data->first(descr).code();
  }
    
  return QString();
}

//! Получение кодового значния и показателя качества (первого записанного, если несколько), с учетом дочерних
/*! 
  \param descr Номер дескриптора
  \param code  Кодовое значение
  \param qual  Показатель качества

  \return true - если найден
*/
bool TMeteoData::getCode(descr_t descr, QString* code, control::QualityControl* qual) const
{
  if(! code || !qual) {
    return false;
  }
  if (!hasParam(descr, false)) {
    bool ok = false;
    for (int idx = 0; idx < _childs.count(); idx++) {
      ok = _childs.at(idx)->getCode(descr, code, qual);
      break;
    }
    return ok;
  }
  
  *code = first(descr).code();
  *qual = first(descr).quality();
  return true;  
}


//! Получение кодового значния и показателя качества (первого записанного, если несколько), с поиском у родителей
/*! 
  \param descr Номер дескриптора
  \param code  Кодовое значение
  \param qual  Показатель качества

  \return true - если найден
*/
bool TMeteoData::getCodeUp(descr_t descr, QString* code, control::QualityControl* qual) const
{
  if(! code || !qual) {
    return false;
  }

  bool ok = false;
  const TMeteoData* data = findParent(descr);
  if (0 != data) {
    ok = true;
    *code = data->first(descr).code();
    *qual = data->first(descr).quality();
  }
  
  return ok;  
}

//! Получение числового значения метеопараметра (первого записанного, если несколько), с учетом дочерних
/*!   
  \param descr Номер дескриптора
  \param defValue Значение по умолчанию, если величина не найдена

  \return Значение метеопараметра
*/
float TMeteoData::getValue(descr_t descr, float defValue, bool withChilds /*= true*/) const
{
  if(hasParam(descr, false)) {
    return first(descr).value();
  }

  float val = defValue;

  if (withChilds) {
    bool ok; 
    control::QualityControl qual;
    for (int idx = 0; idx < _childs.count(); idx++) {
      float childVal;
      ok = _childs.at(idx)->getValue(descr, &childVal, &qual);
      if (ok && qual < control::MISTAKEN) {
	val = childVal;
	break;
      }
    }
  }

  return val;
}


//! Получение числового значения и показателя качества (первого записанного, если несколько), с учетом дочерних
/*! 
  \param descr Номер дескриптора
  \param code  Значение метеопараметра
  \param qual  Показатель качества

  \return true - если найден
*/
bool TMeteoData::getValue(descr_t descr, float *ret_val, control::QualityControl *qual, bool withChilds /*= true*/) const
{
  if(! ret_val || !qual) return false;

  if (hasParam(descr, false)) {
    *ret_val = first(descr).value();
    *qual = first(descr).quality();
    return true;
  }
  
  bool ok = false;

  if (withChilds) {
    for (int idx = 0; idx < _childs.count(); idx++) {
      ok = _childs.at(idx)->getValue(descr, ret_val, qual);
      if (ok) {
	break;
      }
    }
  }
  
  return ok;
}

//! Получение числового значения и показателя качества (первого записанного, если несколько), с поисклом у родителей
/*! 
  \param descr Номер дескриптора
  \param code  Значение метеопараметра
  \param qual  Показатель качества

  \return true - если найден
*/
bool TMeteoData::getValueUp(descr_t descr, float *ret_val, control::QualityControl *qual) const
{
  if(! ret_val || !qual) {
    return false;
  }

  bool ok = false;
  const TMeteoData* data = findParent(descr);
  if (0 != data) {
    *ret_val = data->first(descr).value();
    *qual = data->first(descr).quality();
  }
  
  return ok;
}

//! Получение числового значния и показателя качества (первого записанного, если несколько), с учетом дочерних
/*! 
  \param descr Номер дескриптора
  \param code  Значение метеопараметра
  \param qual  Показатель качества

  \return true - если найден
*/
bool TMeteoData::getValue(descr_t descr, int* val, control::QualityControl* qual) const
{
  if(! val || !qual) {
    return false;
  }

  float fv;
  bool ok = getValue(descr, &fv, qual, false);
  *val = static_cast<int>(fv);

  return ok;
}

//! Получение числового значения и показателя качества (первого записанного, если несколько), только текущий
/*! 
  \param descr Номер дескриптора
  \param index Индекс
  \param qual  Показатель качества

  \return true - если найден
*/
bool TMeteoData::getValue(descr_t descr, int index, float* val, control::QualityControl* qual, bool withChilds /*= true*/) const
{
  if (!val || ! qual) return false;

  bool ok;
  const TMeteoParam& p = getParam(descr, index, &ok, withChilds);
  *val = p.value();
  *qual = p.quality();    

  return ok;
}


//! Получение числового значения по дескриптору и индексу, только текущий узел
/*! 
  \param descr Номер дескриптора
  \param index Индекс
  \param defValue Значение по умолчанию, если величина не найдена

  \return найденное значение или defValue
*/
float TMeteoData::getValueCur(descr_t descr, int index, float defValue) const
{
  const TMeteoParam& p = getParam(descr, index, 0, false);
  if (!p.isValid()) return defValue;

  return p.value();
}

//! Поиск значения от текущего узла до корня, с поиском у родителей
/*!   
  \param descr Номер дескрипторв
  \param defValue Значение по умолчанию, если величина не найдена
  
  \return 
*/
float TMeteoData::getValueUp(descr_t descr, float defValue) const
{
  float val;
  control::QualityControl qual;
  bool ok = getValue(descr, &val, &qual, false);
  if ((!ok || qual >= control::MISTAKEN) && _parent != 0) {
    return _parent->getValueUp(descr, defValue);
  }
  
  if ((!ok || qual >= control::MISTAKEN)) {
    val = defValue;
  }
  return val;
}

float TMeteoData::getValueUp(descr_t descr, int index, float defValue) const
{
  float val;
  control::QualityControl qual = control::NO_OBSERVE;
  bool ok = getValue(descr, index, &val, &qual, false);
  if ((!ok || qual >= control::MISTAKEN) && _parent != 0) {
    return _parent->getValueUp(descr, index, defValue);
  }
  
  if ((!ok || qual >= control::MISTAKEN)) {
    val = defValue;
  }
  return val;
}


//! Получении метеопараметра (первого записанного, если несколько), с учётом дочерних
/*! 
  \param descr Номер дескриптора

  \return Метеопараметр
*/
const TMeteoParam& TMeteoData::getParam(descr_t descr, bool withChilds /*= true*/) const
{
  if(hasParam(descr, false)) {
    return first(descr);
  }

  if (withChilds) {
    for (int idx = 0; idx < _childs.count(); idx++) {
      const TMeteoParam& mp = _childs.at(idx)->getParam(descr);
      if (!mp.isInvalid()) return mp;
    }
  }

  return _empty;  
}

const TMeteoParam& TMeteoData::getParamUp(descr_t descr) const
{
  const TMeteoData* data = findParent(descr);
  if (0 != data) {
    return data->first(descr);
  }
  
  return _empty;  
}

//!  Получение списка метеопараметров по дескриптору у первого встреченного узла
/*! 
  \param descr Дескриптор
  \param aok true, если найдены значения

  \return Список метеопараметров
*/
const QMap<int, TMeteoParam> TMeteoData::getParamList(descr_t descr, bool* aok /* = 0*/) const 
{ 
  if (_data.contains(descr)) {
    if (aok) *aok = true;
    return _data.value(descr); 
  }

  bool ok = false;
  QMap<int, TMeteoParam> mp;
  for (int idx = 0; idx < _childs.count(); idx++) {
    mp = _childs.at(idx)->getParamList(descr, &ok);
    if (ok) {
      break;
    }
  }

  return mp;
}

//! Получении указателя на метеопараметр (первого записанного, если несколько), с учётом дочерних
/*! 
  \param descr Номер дескриптора

  \return Метеопараметр, 0 - если не найден
*/
TMeteoParam* TMeteoData::paramPtr(descr_t descr, bool withChilds /*= true*/)
{ 
  if (hasParam(descr, false)) {
    return &(_data[descr].begin().value()); 
  }
  
  TMeteoParam* mp = 0;
  if (withChilds) {
    for (int idx = 0; idx < _childs.count(); idx++) {
      mp = _childs[idx]->paramPtr(descr, withChilds);
      if (0 != mp) break;
    }
  }

  return mp;
} 

//! Получении указателя на метеопараметр (первого записанного, если несколько), с учётом дочерних
/*! 
  \param descr Номер дескриптора
  \param owner Ссылка на указатель владельца параметра
  \return Метеопараметр, 0 - если не найден
*/
TMeteoParam* TMeteoData::paramPtr(descr_t descr, TMeteoData*& owner, bool withChilds)
{ 
  if (hasParam(descr, false)) {
    owner = this;
    return &(_data[descr].begin().value()); 
  }
  
  TMeteoParam* mp = 0;
  if (withChilds) {
    for (int idx = 0; idx < _childs.count(); idx++) {
      mp = _childs[idx]->paramPtr(descr, owner, withChilds);
      if (0 != mp) break;
    }
  }

  return mp;
} 


// TMeteoParam* TMeteoData::paramPtr(descr_t descr)
// { 
//   if (withParent) {
//     TMeteoData* data = findParent(descr);
//     if (0 != data) {
//       return &(data->_data[descr].begin().value()); 
//     }
//   } else if (hasParam(descr, false)) {
//     return &(_data[descr].begin().value()); 
//   }

//   return 0;
// } 

//! Получении указателя на метеопараметр (первого записанного, если несколько), с учётом дочерних
/*! 
  \param descr Номер дескриптора
  \param index Индекс

  \return Метеопараметр, 0 - если не найден
*/
TMeteoParam* TMeteoData::paramPtr(descr_t descr, int index, bool withChilds /* = true*/) 
{ 
  if (hasParam(descr, false) && _data.value(descr).contains(index)) {
    return &(_data[descr][index]); 
  }

  TMeteoParam* mp = 0;
  if (withChilds) {
    for (int idx = 0; idx < _childs.count(); idx++) {
      mp = _childs[idx]->paramPtr(descr, index, withChilds);
      if (0 != mp) break;
    }
  }

  return mp;
}


//! Получении метеопараметра (первого записанного, если несколько)
/*! 
  \param descr Номер дескриптора
  \param index Индекс

  \return Метеопараметр, 0 - если не найден
*/
const TMeteoParam& TMeteoData::getParam(descr_t descr, int index, bool* ok /*= 0*/, bool withChilds /* = true*/) const 
{
  if (hasParam(descr, false)) {
    QMap<int, TMeteoParam>::const_iterator it = _data.value(descr).find(index);
    if (it == _data.value(descr).constEnd()) {
      if (0 != ok) *ok = false;
      return _empty;
    }
    
    if (0 != ok) *ok = true;
    return it.value();
  }

  if (withChilds) {
    for (int idx = 0; idx < _childs.count(); idx++) {
      if (_childs.at(idx)->hasParam(descr, false)) {
	return _childs.at(idx)->getParam(descr, index, ok, withChilds);
      }
    }
  }

  return _empty;
}


//! Получение метеопараметра по названию дескриптора (проверяются так же дополнительные имена), с учётом дочерних
/*! 
  \param name название дескриптора

  \return метеопараметр
*/
const TMeteoParam& TMeteoData::meteoParam(const QString& name, bool* ok /*= 0*/) const
{
  if (TMeteoDescriptor::instance()->contains(name)) {
    descr_t descr = TMeteoDescriptor::instance()->descriptor(name);
    if(hasParam(descr, true)) {
      if (0 != ok) *ok = true;
      return getParam(descr);
    }
  }

  if (!TMeteoDescriptor::instance()->isAdditional(name)) {
    if (0 != ok) *ok = false;
    return _empty;
  }

  meteodescr::Additional addProp = TMeteoDescriptor::instance()->additional(name);
  if (! hasParam(addProp.descr, true)) {
    if (0 != ok) *ok = false;
    return _empty;
  }
  
  if (addProp.index != -1) {
    return getParam(addProp.descr, addProp.index * 100, ok);
  }
  
  int idx = -1;
  QMap<int, TMeteoParam>::const_iterator it = _data.value(addProp.descr).constBegin();
  while (it != _data.value(addProp.descr).constEnd()) {
    if (it.value().value() >= addProp.min && it.value().value() <= addProp.max) {
     idx = it.key();
     break;
    }
    ++it;
  }
  
  if (idx != -1) {
    if (0 != ok) *ok = true;
    return getParam(addProp.descr, idx);
  }

  if (0 != ok) *ok = false;
  return _empty;
}

//! Получение указателя на метеопараметр по названию дескриптора (проверяются так же дополнительные имена), с учётом дочерних
/*! 
  \param name название дескриптора

  \return метеопараметр
*/
TMeteoParam* TMeteoData::meteoParamPtr(const QString& name, bool withChilds /*= true*/)
{
  if (TMeteoDescriptor::instance()->contains(name)) {
    return paramPtr(TMeteoDescriptor::instance()->descriptor(name), withChilds);
  }

  if (!TMeteoDescriptor::instance()->isAdditional(name)) {
    return 0;
  }

  meteodescr::Additional addProp = TMeteoDescriptor::instance()->additional(name);
  if (! hasParam(addProp.descr, true)) {
    return 0;
  }
  
  if (addProp.index != -1) {
    return paramPtr(addProp.descr, addProp.index * 100, withChilds);
  }
  
  int idx = -1;
  QMap<int, TMeteoParam>::iterator it = _data[addProp.descr].begin();
  while (it != _data.value(addProp.descr).end()) {
    if (it.value().value() >= addProp.min && it.value().value() <= addProp.max) {
     idx = it.key();
     break;
    }
    ++it;
  }
  
  if (idx != -1) {
    return paramPtr(addProp.descr, idx, withChilds);
  }

  return 0;
}

//! Получение списка метеопараметров по названию дескриптора, с учётом дочерних
/*! 
  \param name название дескриптора

  \return список метеопараметров
*/
const QMap<int, TMeteoParam> TMeteoData::meteoParamList(const QString& name) const
{
  return getParamList(TMeteoDescriptor::instance()->descriptor(name));
}

//! Удаление параметра с учетом доп имен. может нарушить целостность данных, нет реализации последующего безопасного добавления параметров (см. autotest, f2())
int TMeteoData::remove(const QString& name, bool withChilds /*= true*/)
{
  int cnt = 0;

  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  if (d->contains(name)) { //базовый дескриптор
    descr_t descr = TMeteoDescriptor::instance()->descriptor(name);
    cnt = countParam(descr, false);
    if (cnt != 0) {
      _data.remove(descr);
    }
  } else if (d->isAdditional(name)) { //дополнительные имена
    meteodescr::Additional prop = d->additional(name);
    if (_data.contains(prop.descr)) {
      if (prop.index == -1) { //имена с интервалом значений
	QMap<int, TMeteoParam>::iterator it = _data[prop.descr].begin();
	while (it != _data.value(prop.descr).end()) {
	  if (it.value().value() >= prop.min && it.value().value() <= prop.max) {
	    it = _data[prop.descr].erase(it);
	    ++cnt;
	  } else {
	    ++it;
	  }
	}
      } else { //имена с индексами
	_data[prop.descr].remove(prop.index * 100);
	++cnt;
      }
    }   
  } else {
    error_log << QObject::tr("Неизвестное название дескриптора '%1'").arg(name);
  } 

  if (withChilds) {
    for (int idx = 0; idx < _childs.size(); idx++) {
      cnt += _childs[idx]->remove(name);
    }
  }

  return cnt;  
}

//! Количество параметров у первого найденного узла, имеющего descr
/*! 
  \param descr Дескриптор
  \param withChilds С учетом дочерних
  
  \return Количество параметров
*/
uint TMeteoData::countParam(descr_t descr, bool withChilds) const
{ 
  if (hasParam(descr, false)) {
    return _data.value(descr).count(); 
  }

  uint cnt = 0;
  if (withChilds) {
    for (int idx = 0; idx < _childs.count(); idx++) {
      if ( _childs.at(idx)->hasParam(descr, false)) {
	cnt = _childs.at(idx)->_data.value(descr).count(); 
      }
    }
  }

  return cnt;
}

//! Проверка наличия метеопараметра
/*! 
  \param descr Номер дескриптора
  \param withChilds true - с проверкой дочерних
  
  \return true - если найден
*/
bool TMeteoData::hasParam(descr_t descr, bool withChilds /*= true*/) const
{ 
  if ( _data.contains(descr) && !_data.value(descr).isEmpty()) {
    return true;
  }

  bool ok = false;
  if (withChilds) {
    for (int idx = 0; idx < _childs.count(); idx++) {
      ok = _childs.at(idx)->hasParam(descr, true);
      if (ok) break;
    }
  }

  return ok;
}

/*! 
  \brief Проверка наличия метеопараметра. 
  Если после проверки параметр необходимо считать, лучше пользоваться функцией 
  void meteoParam( const QString&, bool* ok = 0 )

  \param name Основное или дополнительное имя
  \param withChilds true - с проверкой дочерних

  \return true - если есть
*/
bool TMeteoData::hasParam(const QString& name, bool withChilds /*= true*/) const
{
  if (TMeteoDescriptor::instance()->contains(name)) {
    descr_t descr = TMeteoDescriptor::instance()->descriptor(name);
    if(hasParam(descr, withChilds)) {
      return true;
    }
  }

  if (!TMeteoDescriptor::instance()->isAdditional(name)) {
    return false;
  }

  meteodescr::Additional addProp = TMeteoDescriptor::instance()->additional(name);
  if (! hasParam(addProp.descr, withChilds)) {
    return false;
  }

  if (addProp.index != -1) {
    QMap<int, TMeteoParam>::const_iterator it = _data.value(addProp.descr).find(addProp.index * 100);
    if (it == _data.value(addProp.descr).constEnd()) return false;
    return true;
  }

  int idx = -1;
  QMap<int, TMeteoParam>::const_iterator it = _data.value(addProp.descr).constBegin();
  while (it != _data.value(addProp.descr).constEnd()) {
    if (it.value().value() >= addProp.min && it.value().value() <= addProp.max) {
      idx = it.key();
      break;
    }
    ++it;
  }
  
  if (idx != -1) {
    return true;
  }

  return false;
}

//! Поиск узла содержащего дескриптор descr от текущего по корневого
/*! 
  \param descr Дескриптор
  \return Адрес узли, иначе 0
*/
TMeteoData* TMeteoData::findParent(descr_t descr)
{
  if (hasParam(descr, false)) return this;
  if (0 != _parent) return _parent->findParent(descr);

  return 0;
}

//! Поиск узла содержащего дескриптор name от текущего по корневого, с учетом доп имен
/*! 
  \param name Название метеопараметра
  \return Адрес узли, иначе 0
*/
TMeteoData* TMeteoData::findParent(const QString& name)
{
  if (hasParam(name, false)) return this;
  if (0 != _parent) return _parent->findParent(name);
  
  return 0;
}

QList<const TMeteoData*> TMeteoData::findChildsConst(descr_t descr) const
{
  QList<const TMeteoData*> childs;

  if (hasParam(descr, false)) {
    childs << this;
  }

  for (int idx = 0; idx < _childs.count(); idx++) {
    QList<const TMeteoData*> ch = _childs.at(idx)->findChildsConst(descr);
    childs << ch;
  }

  return childs;
}

QList< TMeteoData*> TMeteoData::findChilds(descr_t descr)
{
  QList< TMeteoData*> childs;
  
  if (hasParam(descr, false)) {
    childs << this;
  }
  
  for (int idx = 0; idx < _childs.count(); idx++) {
    childs << _childs[idx]->findChilds(descr);
  }
  
  return childs;
}

QList<TMeteoData*> TMeteoData::findChilds(const QString& name)
{
  QList<TMeteoData*> childs;

  if (hasParam(name, false)) {
    childs << this;
  }

  for (int idx = 0; idx < _childs.count(); idx++) {
    childs << _childs[idx]->findChilds(name);
  }

  return childs;  
}

//! Удаление дочерних, содержащих дескриптор descr
/*! 
  \param descr Дескриптор
  \return Удаленные дочерние
*/
QList<TMeteoData*> TMeteoData::takeChilds(descr_t descr)
{
  QList<TMeteoData*> fragmList;

  QMutableListIterator<TMeteoData*> it(_childs);
  while (it.hasNext()) {
    TMeteoData* md = it.next();
    if (md->hasParam(descr, false)) {
      TMeteoData* fragm = new TMeteoData;
      TMeteoDescriptor::instance()->setFullIdentification(fragm, md->parent());
      fragm->set(descr, md->getParam(descr));
      TMeteoData& child = fragm->addChild();
      child.copyFrom(*md);
      fragm->_childs.last()->_parent = fragm;
      fragmList.append(fragm);

      it.remove();
    } else {
      fragmList << md->takeChilds(descr);
    }
  }

  return fragmList;
}


//! Наличие параметра по дескриптору и типу уровня
bool TMeteoData::hasParam(descr_t descr, meteodescr::LevelType type) const
{
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

  meteodescr::LevelType curType = static_cast<meteodescr::LevelType>(getValueCur(d->descriptor("level_type"), 
										 meteodescr::kSurfaceLevel));//meteodescr::kUnknownLevel TODO
  if (curType == type && hasParam(descr, false)) {
    return true;
  }

  for (int idx = 0; idx < _childs.count(); idx++) {
    if (_childs.at(idx)->hasParam(descr, type)) {
      return true;
    }
  }

  return false;
}

//! Список дочерних, имеющих параметр descr и тип уровеня type
QList<TMeteoData*> TMeteoData::findChilds(descr_t descr, meteodescr::LevelType type) 
{
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  QList<TMeteoData*> md;

  meteodescr::LevelType curType = static_cast<meteodescr::LevelType>(getValueCur(d->descriptor("level_type"), meteodescr::kSurfaceLevel));//meteodescr::kUnknownLevel TODO
  if (curType == type && hasParam(descr, false)) {
    // if (lo == -1)  {
    //   return this;
    // }

    switch (curType) {
    case meteodescr::kSurfaceLevel:
    case meteodescr::kMaxWindLevel:
    case meteodescr::kTropoLevel:
      md << this;
      break;
    case meteodescr::kIsobarLevel: {
      // int levelLo = getValueCur(d->descriptor("P1"), -1);
      // if (levelLo == lo) {
	md << this;
	//      }
    }
      break;
    // case kHeightLayLevel:
    //   break;
    default:{}
  
    }
  }

  for (int idx = 0; idx < _childs.count(); idx++) {
    md << _childs[idx]->findChilds(descr, type);
  }
  
  return md;
}

//! Поиск дочернего по уровню
/*! 
  \param type Тип уровня
  \param lo Значение уровня или нижней границы
  \param hi Значение верхней границы
  \return метеоданные
*/
const TMeteoData* TMeteoData::findChild(meteodescr::LevelType type, int lo, int hi) const
{
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
  const TMeteoData* md = 0;

  int curType = getValueCur(d->descriptor("level_type"), meteodescr::kSurfaceLevel);//meteodescr::kUnknownLevel TODO
  if (curType == type) {
    if (lo == -1)  {
      return this;
    }

    switch (curType) {
    case meteodescr::kSurfaceLevel:
    case meteodescr::kMaxWindLevel:
    case meteodescr::kTropoLevel:
      md = this;
      break;
    case meteodescr::kIsobarLevel: {
      int levelLo = getValueCur(d->descriptor("P1"), -1);
      if (levelLo == lo) {
	md = this;
      }
    }
      break;
    // case kHeightLayLevel:
    //   break;
    default:{}
  
    }
  }
  if (0 == md) {
    for (int idx = 0; idx < _childs.count(); idx++) {
      md = _childs.at(idx)->findChild(type, lo, hi);
      if (0 != md) break;
    }
  }
  
  return md;
}

//! Поиск дочернего по уровню
/*! 
  \param type Тип уровня
  \param lo Значение уровня или нижней границы
  \return метеоданные
*/
const TMeteoData* TMeteoData::findChild(meteodescr::LevelType type, int lo) const
{
  return findChild(type, lo, -1);
}

//! Поиск дочернего по уровню
/*! 
  \param type Тип уровня
  \return метеоданные
*/
const TMeteoData* TMeteoData::findChild(meteodescr::LevelType type) const
{
  return findChild(type, -1, -1);
}


//! Поиск узла содержащего дескриптор descr от текущего по корневого
/*! 
  \param descr Дескриптор
  \return Адрес узли, иначе 0
*/
const TMeteoData* TMeteoData::findParent(descr_t descr) const
{
  if (hasParam(descr, false)) return this;
  if (0 != _parent) return _parent->findParent(descr);

  return 0;
}

//! Поиск узла содержащего дескриптор name от текущего по корневого, с учетом доп имен
/*! 
  \param name Название метеопараметра
  \return Адрес узли, иначе 0
*/
const TMeteoData* TMeteoData::findParent(const QString& name) const
{
  if (hasParam(name, false)) return this;
  if (0 != _parent) return _parent->findParent(name);
  
  return 0;
}

//! Печать данных
/*! 
  \param lev     Уровень вложенности - для сдвига
  \param maxQual Максимальное значение качества выводимых данных
 */
// void TMeteoData::printData_old(int lev /*= 0*/, int  /*= -1*/, control::QualityControl maxQual /*= NO_OBSERVE*/) const
// {
//   //  var(_data.size());
//   QString tab;
//   for (int l = 0; l < lev; l++) {
//     tab += '\t';
//   }
  
//   int type = -1;
//   float lo = -1, hi = -1;
//   TMeteoDescriptor::instance()->findVertic(*this, &type, &lo, &hi);
//   qDebug() << tab << "level=" << type << lo << hi;
//   QDateTime dt1, dt2;
//   type = -1;
//   TMeteoDescriptor::instance()->findTime(*this, &type, &dt1, &dt2);
//   qDebug() << tab << " time=" << type << dt1.toString("dd hh:mm") << dt2.toString("dd hh:mm");

//   QHashIterator<descr_t, QMap<int, TMeteoParam> > it(_data);
//   while (it.hasNext()) {
//     it.next();

//     descr_t descr = it.key();
//     QList<int> keys = it.value().keys();
//     qSort(keys);

//     if (keys.count() == 0) {
//       debug_log << "Empty data descr=" << descr;
//     }

//     for (int idx = 0; idx < keys.count(); idx++) {
//       if (it.value().value(keys.at(idx)).quality() <= maxQual) {
// 	qDebug() << tab << TMeteoDescriptor::instance()->name(descr) << "(" << descr << ") : \t" 
// 		 << "idx" << keys.at(idx) << ":\t" << it.value().value(keys.at(idx));
//       }
//     }
//   }

  
//   if (_childs.size() != 0) {
//     qDebug() << tab + "/------------- childs_count = " + QString::number(_childs.size());
//   }
//   for (int idx = 0; idx < _childs.size(); idx++) {
//     qDebug() << tab + "childs_num = " + QString::number(idx);
//     // var(idx);
//     _childs.at(idx)->printData_old(lev + 1, idx, maxQual);
//   }
//   if (_childs.size() != 0) {
//     qDebug() << tab + "\\------------";
//   }
// }

//! Печать данных
/*! 
  \param lev     Уровень вложенности - для сдвига
  \param maxQual Максимальное значение качества выводимых данных
 */
void TMeteoData::printData(meteodescr::TimeParam* tp /*= 0*/, int lev /*= 0*/, int  /*= -1*/, control::QualityControl maxQual /*= NO_OBSERVE*/) const
{
  //trc;
  //  var(_data.size());
  QString tab;
  for (int l = 0; l < lev; l++) {
    tab += "    ";
  }
  
  int type = -1;
  float lo = -1, hi = -1;
  TMeteoDescriptor::instance()->findVertic(*this, &type, &lo, &hi);
  qDebug() << tab << "level=" << type << lo << hi;
  QDateTime dt1, dt2;
  type = -1;
  meteodescr::TimeParam tp_cur;
  if (tp != 0) {
    tp_cur = *tp;
  }
  TMeteoDescriptor::instance()->findTime(*this, &tp_cur, &type, &dt1, &dt2);
  qDebug() << tab << "time=" << type << dt1.toString("yyyy-MM-dd hh:mm:ss.zzz") << dt2.toString("yyyy-MM-dd hh:mm:ss.zzz");

  QHashIterator<descr_t, QMap<int, TMeteoParam> > it(_data);
  while (it.hasNext()) {
    it.next();

    descr_t descr = it.key();
    QList<int> keys = it.value().keys();
    qSort(keys);

    if (keys.count() == 0) {
      debug_log << "Empty data descr=" << descr;
    }

    for (int idx = 0; idx < keys.count(); idx++) {
      if (it.value().value(keys.at(idx)).quality() <= maxQual) {
	qDebug() << tab << TMeteoDescriptor::instance()->name(descr) << "(" << descr << ") : \t" 
		 << "idx" << keys.at(idx) << ":\t" << it.value().value(keys.at(idx));
      }
    }
  }

  
  if (_childs.size() != 0) {
    qDebug() << tab + "/------------- childs_count = " + QString::number(_childs.size());
  }
  for (int idx = 0; idx < _childs.size(); idx++) {
    qDebug() << tab + "childs_num = " + QString::number(idx);
    // var(idx);
    _childs.at(idx)->printData(&tp_cur, lev + 1, idx, maxQual);
  }
  if (_childs.size() != 0) {
    qDebug() << tab + "\\------------";
  }
}


QString TMeteoData::toString(int lev /*= 0*/) const
{
  QString res;
  
  QString tab;
  for (int l = 0; l < lev; l++) {
    tab += "    ";
  }
  
  QHashIterator<descr_t, QMap<int, TMeteoParam> > it(_data);
  while (it.hasNext()) {
    it.next();

    descr_t descr = it.key();
    QList<int> keys = it.value().keys();
    qSort(keys);
    
    for (int idx = 0; idx < keys.count(); idx++) {
      res += tab + TMeteoDescriptor::instance()->name(descr) + " (" + QString::number(descr) + ") : \t" +
	"idx " + QString::number(keys.at(idx)) + " :\t" + it.value().value(keys.at(idx)).toString() + "\n";
    }
  }
  
  for (int idx = 0; idx < _childs.size(); idx++) {
    res += _childs.at(idx)->toString(lev + 1) + "---\n";
  }

  return res;
}

void TMeteoData::toText(QTextStream* ts) const
{
  QHashIterator<descr_t, QMap<int, TMeteoParam> > it(_data);
  while (it.hasNext()) {
    it.next();
    
    descr_t descr = it.key();
    QList<int> keys = it.value().keys();
    qSort(keys);

    for (int idx = 0; idx < keys.count(); idx++) {
      // ts << TMeteoDescriptor::instance()->name(descr) << "(" << descr << ") : \t" 
      // 	       << "idx" << keys.at(idx) << ":\t" << it.value().value(keys.at(idx));
      QString description = TMeteoDescriptor::instance()->property(descr).description;
      if (!description.isEmpty() && !it.value().value(keys.at(idx)).isInvalid()) {
	*ts << TMeteoDescriptor::instance()->name(descr) << "\t"
	    << "idx" << keys.at(idx)/100 << ":\t" 
	    << it.value().value(keys.at(idx)).value() << "\t"
	    << description << "\n";
      }
    }
  }
  
  for (int idx = 0; idx < _childs.size(); idx++) {
    _childs.at(idx)->toText(ts);
  }
}

void TMeteoData::toDataStream(QDataStream& out) const
{
  out << _data;
  int size = _childs.size();
  out << size;
  for (int idx = 0; idx < _childs.size(); ++idx) {
    _childs.at(idx)->toDataStream(out);
  }
}

void TMeteoData::fromDataStream(QDataStream& in)
{
  in >> _data;
  int size;
  in >> size;
  for (int idx = 0; idx < size; idx++) {
    TMeteoData& child = addChild();
    child.fromDataStream(in);
    child._parent = this;
  }
}


const TMeteoData& operator>>(const TMeteoData& data, QByteArray& out)
{
  QDataStream ds(&out, QIODevice::WriteOnly);
  ds << data;
  return data;
}

TMeteoData& operator<<(TMeteoData& data, const QByteArray& ba)
{
  QDataStream ds(ba);
  ds >> data;
  return data;
}


TMeteoData& TMeteoData::operator=(const TMeteoData& src)
{
  copyFrom(src);

  return *this;
}

//! Скопировать данные с построением иерархии
void TMeteoData::copyFrom(const TMeteoData& src)
{
  _data = src._data;

  _childs.clear();
  for (int idx = 0; idx < src._childs.size(); idx++) {
    TMeteoData& ch = addChild();
    ch.copyFrom(*(src._childs.at(idx)));
  }
}

// возвращает дескрипторы и значения величин в формате массивов sql
int TMeteoData::getParamsForDb(ParamsForDb* pdb) const
{
  if (0 == pdb) return 0;
  
  pdb->descriptor = "{";
  pdb->group = "{";
  pdb->index = "{";
  pdb->val = "{";
  pdb->code = "{";
  pdb->qual = "{";
  pdb->levelType = "{";
  pdb->levelLo = "{";
  pdb->levelHi = "{";
  pdb->dtType = "{";
  pdb->dtStart = "{";
  pdb->dtEnd = "{";
  pdb->number = "{";
  pdb->parent = "{";
  
  float fi,la,height;
  if(TMeteoDescriptor::instance()->getCoord(*this, &fi,&la,&height)){
    pdb->fi = QString::number(fi);
    pdb->la = QString::number(la);
    pdb->height = QString::number(height);
  } 
  
// debug_log << pdb->fi<<pdb->la<<pdb->height<<fi<<la<<height;
  int num = 0;
  int mountainLev = TMeteoDescriptor::instance()->mountainLevel(*this);
  addChildParamsForDb(*this, 0, 0, &num, mountainLev, pdb);

  pdb->index.remove(-1,1);
  pdb->index += "}";
  pdb->descriptor.remove(-1,1);
  pdb->descriptor += "}";
  pdb->group.remove(-1,1);
  pdb->group += "}";
  pdb->val.remove(-1,1);
  pdb->val += "}";
  pdb->code.remove(-1,1);
  pdb->code += "}";
  pdb->qual.remove(-1,1);
  pdb->qual += "}";
  pdb->levelType.remove(-1,1);
  pdb->levelType += "}";
  pdb->levelLo.remove(-1,1);
  pdb->levelLo += "}";
  pdb->levelHi.remove(-1,1);
  pdb->levelHi += "}";
  pdb->dtType.remove(-1,1);
  pdb->dtType += "}";
  pdb->dtStart.remove(-1,1);
  pdb->dtStart += "}";
  pdb->dtEnd.remove(-1,1);
  pdb->dtEnd += "}";
  pdb->number.remove(-1,1);
  pdb->number += "}";
  pdb->parent.remove(-1,1);
  pdb->parent += "}";

  
  return num; 
}

void TMeteoData::addChildParamsForDb(const TMeteoData& data, meteodescr::TimeParam* tp, uint parent, int* num, int mountainLev, ParamsForDb* pdb) const
{
  if (0 == pdb) return;

  QString stype, slevLo, slevHi;
  QString sdttype, sdt1, sdt2;
  int vtype = -1;
  float v1 = -1, v2 = -1;
  TMeteoDescriptor::instance()->findVertic(data, &vtype, &v1, &v2);
  stype = vtype != -1 ? QString::number(vtype) : QString::number(meteodescr::kSurfaceLevel);
  slevLo = v1 != -1 ? QString::number(v1) : "0";
  slevHi = v2 != -1 ? QString::number(v2) : "NULL";
  vtype = -1;

  meteodescr::TimeParam tp_cur;
  if (tp != 0) {
    tp_cur = *tp;
  }
  QDateTime dt1, dt2;
  TMeteoDescriptor::instance()->findTime(data, &tp_cur, &vtype, &dt1, &dt2);
  sdttype = vtype != -1 ? QString::number(vtype) : QString::number(meteodescr::kNormalTime);
  sdt1 = dt1.toString("yyyy-MM-dd hh:mm:ss");
  sdt2 = dt2.toString("yyyy-MM-dd hh:mm:ss");

  
  QByteArray keyData;
  keyData.append(stype.leftJustified(3));
  keyData.append(slevLo.leftJustified(6));
  keyData.append(slevHi.leftJustified(6));
  keyData.append(sdttype.leftJustified(3));
  keyData.append(sdt1);
  keyData.append(sdt2);
  keyData.append(QString::number(*num).leftJustified(3));

  uint16_t key = MnMath::makeCrc(keyData.data(), keyData.size());
  //qHash(stype + slevLo + slevHi + sdttype + sdt1 + sdt2 + QString::number(*num));

  //debug_log << parent << *num << key;

  QHash<descr_t, QMap<int, TMeteoParam> >::const_iterator it = data._data.begin();
  int i = 0;
  for (; it != data._data.end(); ++it) {
    //debug_log <<"descriptor "<<it.key();
    QMap<int, TMeteoParam> values = it.value();
    QMap<int, TMeteoParam>::iterator iv = values.begin();
    for (; iv != values.end(); ++iv){
      if(!(qFuzzyCompare(iv.value().value(), BAD_METEO_ELEMENT_VAL) && iv.value().code().isEmpty()) 
	 && iv.value().quality() != control::NO_OBSERVE) {
        pdb->descriptor += QString::number(it.key() % 100000) + ",";
        pdb->group += QString::number(it.key() / 100000) + ",";
        pdb->index += QString::number(iv.key()) + ",";
        pdb->val  += QString::number(iv.value().value()) + ",";
	if (iv.value().code().isEmpty()) {
	  pdb->code += "NULL,";
	} else {
	  pdb->code += "'" + iv.value().code().replace(",", "\\,") + "',";
	}
	pdb->qual += QString::number(iv.value().quality()) + ",";
	if (BAD_METEO_ELEMENT_VAL != mountainLev && 
	    (it.key() == TMeteoDescriptor::instance()->descriptor("T")  || 
	     it.key() == TMeteoDescriptor::instance()->descriptor("Td") || 
	     it.key() == TMeteoDescriptor::instance()->descriptor("dd") || 
	     it.key() == TMeteoDescriptor::instance()->descriptor("ff"))) {
	  pdb->levelType += QString::number(meteodescr::kIsobarLevel) + ",";
	  pdb->levelLo   += QString::number(mountainLev) + ",";
	  pdb->levelHi   += slevHi + ",";
	} else {
	  pdb->levelType += stype + ",";
	  pdb->levelLo   += slevLo + ",";
	  pdb->levelHi   += slevHi + ",";
	}
	pdb->dtType  += sdttype + ",";
	pdb->dtStart += "'" + sdt1 + "'" + ",";
	pdb->dtEnd   += "'" + sdt2 + "'" + ",";	
    
	pdb->number += QString::number(key) + ",";
	pdb->parent += QString::number(parent) + ",";
	//debug_log << it.key() << iv.value().value() << stype << slevLo;

      } 
      ++i;
     // debug_log <<"quality " <<iv.value().quality();
    }
  }

  //  var(data._childs.size());
  for (int idx = 0; idx <data. _childs.size(); idx++) {
    (*num) += 1;
    addChildParamsForDb(*(data._childs.at(idx)), &tp_cur, key, num, mountainLev, pdb);
  }
}


// возвращает дескрипторы и значения величин в формате массивов sql для указанного набора дескрипторов (descrList)
int TMeteoData::getParamsForDb(const QStringList &descrList, QString *adescr, QString *aadd_descr,
			       QString *aval, QString *aqual)const
{  
  QString add_descr,descriptors, val, qual;
  
  descriptors = "{";
  add_descr = "{";
  val = "{";
  qual = "{";

  int cnt = 0;

  for (int i = 0; i < descrList.count(); i++) {
    descr_t cur = TMeteoDescriptor::instance()->descriptor(descrList.at(i));
    QMap<int, TMeteoParam> values = _data.value(cur);
    QMap<int, TMeteoParam>::iterator iv = values.begin();
    for (; iv != values.end(); ++iv){
      if( !qFuzzyCompare( iv.value().value() ,BAD_METEO_ELEMENT_VAL) && iv.value().quality() != control::NO_OBSERVE) {
        descriptors += QString::number(cur)+",";
        add_descr += QString::number(iv.key())+",";
        val += QString::number(iv.value().value())+",";
        qual += QString::number(iv.value().quality())+",";
      } 
      ++cnt;
      // debug_log <<"quality " <<iv.value().quality();
    }
  }
  
  add_descr.remove(-1,1);
  add_descr += "}";
  descriptors.remove(-1,1);
  descriptors += "}";
  val.remove(-1,1);
  val += "}";
  qual.remove(-1,1);
  qual += "}";
  (*aadd_descr) = add_descr;
  (*adescr) = descriptors;
  (*aval) = val;
  (*aqual) = qual;
  return cnt; 
}

// QString TMeteoData::identJson(const QDateTime& dt) const
// {
//   bool flag = false;
  
//   int vtype = -1;
//   QDateTime dt1, dt2;
//   TMeteoDescriptor::instance()->findTime(*this, &vtype, &dt1, &dt2);
//   QString sdttype = vtype != -1 ? QString::number(vtype) : QString::number(meteodescr::kNormalTime);
  
//   QString ident = "\n\"ident\" : {";

//   if (dt != dt1 || dt != dt2) {
//     flag = true;
//     ident += QString("\"dtType\" : %1, ").arg(sdttype);
//     ident += QString("\"dt1\" : { \"$date\": \"%1\" }, ").arg(dt1.toUTC().toString(Qt::ISODate));
//     ident += QString("\"dt2\" : { \"$date\": \"%1\" },\n").arg(dt2.toUTC().toString(Qt::ISODate));
//   }

//   for (auto it = _data.begin();  it != _data.end(); ++it) {
//     if (!TMeteoDescriptor::instance()->isNonLocateIdentDescr(it.key())) { continue; }
    
//     QMap<int, TMeteoParam> values = it.value();
//     QMap<int, TMeteoParam>::iterator iv = values.begin();
    
//     QString key = QString("\"") +  QString::number(it.key()) + ("\" : ");
//     int cnt = 0;
//     QString value;

//     for (; iv != values.end(); ++iv) {
//       if(!(qFuzzyCompare(iv.value().value(), BAD_METEO_ELEMENT_VAL) && iv.value().code().isEmpty()) 
//       	 && iv.value().quality() < control::DOUBTFUL) {
// 	value += QString("{ \"val\" : %1, \"code\" : \"%2\"},\n")
// 	  .arg(iv.value().value())
// 	  .arg(iv.value().code());
// 	cnt++;
// 	flag = true;
//       }
//     }
    
//     if (cnt == 1) {
//       ident += key + value;
//     } else if (cnt > 1) {
//       value.remove(-2, 1);
//       ident += key + "[\n" + value + " ],\n";
//     }
//   }

//   ident.remove(QRegExp(",\n$"));
//   ident += "}";

//   if (flag == false) {
//     return QString();
//   }
  
//   return ident;
// }

QString TMeteoData::toJson(int* levelType, bool isChild /*= false*/) const
{
  QString doc;
  if (!isChild) {
    doc += QString("\"data\":");
    
    //printData();

    float v1, v2;
    if (_childs.size() != 0) { //пока так
      TMeteoDescriptor::instance()->findVertic(*(_childs.first()), levelType, &v1, &v2);
    } else {
      TMeteoDescriptor::instance()->findVertic(*this, levelType, &v1, &v2);
    }    
    //var(*levelType);

  } else {
    //  doc += QString("\"child\" :");
  }
  doc += "{\n";
  

  QHash<descr_t, QMap<int, TMeteoParam> >::const_iterator it = _data.begin();
  for (; it != _data.end(); ++it) {
    //    debug_log <<"descriptor " << it.key();
    QMap<int, TMeteoParam> values = it.value();
    QMap<int, TMeteoParam>::iterator iv = values.begin();

    QString key = QString("\"") +  QString::number(it.key()) + ("\" : ");
    int cnt = 0;
    QString value;

    for (; iv != values.end(); ++iv){
      if(!(qFuzzyCompare(iv.value().value(), BAD_METEO_ELEMENT_VAL) && iv.value().code().isEmpty()) 
      	 && iv.value().quality() != control::NO_OBSERVE) 
      {
	value += QString("{ \"val\" : %1, \"code\" : \"%2\", \"qual\" : %3 },\n")
	  .arg(iv.value().value())
	  .arg(iv.value().code())
	  .arg(iv.value().quality());
	cnt++;
      }
    }
    if (cnt == 1) {
      doc += key + value;
    } else if (cnt > 1) {
      value.remove(-2, 1);
      doc += key + "[\n" + value + " ],\n";
    }
  }

  if (_childs.size() != 0) {
    doc += "\"data\" : [\n";
    for (int idx = 0; idx < _childs.size(); idx++) {
      doc += _childs.at(idx)->toJson(levelType, true);
      //debug_log << "child=" << _childs.at(idx).toJson(levelType, true);
    }      
    doc.remove(-2, 1);
    doc += " ]\n";
  } else {
    doc.remove(QRegExp(",\n$"));
  }

  doc += "},\n";

  return doc;
}

QString TMeteoData::toJson(const QString& header, bool isChild /*= false*/) const
{
  QString doc;
  if (!isChild) {
      
    float v1 = -1, v2 = -1;
    int curltype = -1;
    TMeteoDescriptor::instance()->findVertic(*this, &curltype, &v1, &v2);
    
    doc.append(header);
    doc.append(QString("\"level_type\" : %1, ").arg(curltype));
    if (v1 == -1 ) { v1 = 0; }
    doc.append(QString("\"level\" : %1, ").arg(v1));
    if (v2 != -1) {
      doc.append(QString("\"level_hi\" : %1, ").arg(v2));
    }
  
    //  doc += QString("\"data\":");
  
    //  printData();
  } else {
    //  doc += QString("\"child\" :");
  }
  // doc += "{\n";
    

  //var(*levelType);


  QHash<descr_t, QMap<int, TMeteoParam> >::const_iterator it = _data.begin();
  for (; it != _data.end(); ++it) {
    //    debug_log <<"descriptor " << it.key();
    QMap<int, TMeteoParam> values = it.value();
    QMap<int, TMeteoParam>::iterator iv = values.begin();

    QString dname;
    if(false) {
      dname = TMeteoDescriptor::instance()->name(it.key());
      if (dname.isEmpty()) {
        dname = QString::number(it.key());
      }
    }
    else {
      dname = QString::number(it.key());
    }
    QString key = QString("\"") + dname + ("\" : ");
    int cnt = 0;
    QString value;

    for (; iv != values.end(); ++iv){
      if(!(qFuzzyCompare(iv.value().value(), BAD_METEO_ELEMENT_VAL) && iv.value().code().isEmpty()) 
      	 && iv.value().quality() != control::NO_OBSERVE) 
      {
	value += QString("{ \"val\" : %1, \"code\" : \"%2\", \"qual\" : %3, \"name\" : \"%4\" },\n")
	  .arg(iv.value().value())
	  .arg(iv.value().code())
	  .arg(iv.value().quality())
	  .arg(TMeteoDescriptor::instance()->name(it.key()));;
	cnt++;
      }
    }
    if (cnt == 1) {
      doc += key + value;
    } else if (cnt > 1) {
      value.remove(-2, 1);
      doc += key + "[\n" + value + " ],\n";
    }
  }

  if (_childs.size() != 0) {
    doc += "\"data\" : [\n";
    for (int idx = 0; idx < _childs.size(); idx++) {
      doc += _childs.at(idx)->toJson(header, true);
      //debug_log << "child=" << _childs.at(idx).toJson(levelType, true);
    }      
    doc.remove(-2, 1);
    doc += " ]\n";
  } else {
    doc.remove(QRegExp(",\n$"));
  }

  // doc += "},\n";

  return doc;
}

QDataStream &operator>>(QDataStream &in, TMeteoParam &mtp)
{
  int qual;

  in >> mtp._code;
  in >> qual;
  mtp._quality = control::QualityControl(qual);
  in >> mtp._value;
  in >> mtp._description;

  return in;
}

QDataStream &operator<<(QDataStream &out, const TMeteoParam &mtp)
{
  out << mtp.code();
  out << mtp.quality();
  out << mtp.value();
  out << mtp.description();

  return out;
}


QDataStream &operator<<(QDataStream &out, const TMeteoData &mtd)
{
  out << mtd._data;
  int size = mtd._childs.size();
  out << size;
  for (int idx = 0; idx < mtd._childs.size(); ++idx) {
    out << *(mtd._childs.at(idx));
  }
  return out;
}

QDataStream &operator>>(QDataStream &in, TMeteoData &mtd)
{
  in >> mtd._data;
  int size;
  in >> size;
  for (int idx = 0; idx < size; idx++) {
    TMeteoData& child = mtd.addChild();
    in >> child;
    child._parent = &mtd;
  }
  return in;
}


// возвращает 0 - равны; -1, если q1 хуже q2; 1, если q1 хуже q2
// int control::qualityCompare(QualityControl q1, QualityControl q2)
// {
//    if (q1 == q2)
//      return 0;
   
//   switch (q1) {
//     case NO_OBSERVE:
//       if (q2 == MISTAKEN) return 1;
//       return -1;
//     case NO_CONTROL:
//       if (q2 == RIGHT || q2 == DOUBTFUL) return -1;
//       return 1;
//     case RIGHT:
//       return 1;
//     case DOUBTFUL:
//       if (q2 == RIGHT) return -1;
//       return 1;
//     case MISTAKEN:
//       return -1;
//   default: {}
//     }
  
//   return 1; 
// }

QString control::titleForQuality( int qual )
{
  QString str = QObject::tr("Контроль качества не проводился");
  switch ( qual ) {
    case NO_CONTROL:
      str = QObject::tr("Контроль качества не проводился");
      break;
    case RIGHT:
      str = QObject::tr("Правильное значение");
      break;
    case HAND_CORRECTED:
      str = QObject::tr("Вручную введенное значение");
      break;
    case AUTO_CORRECTED:
      str = QObject::tr("Автоматически введенное значение");
      break;
    case ABSENT_CORRECTED:
      str = QObject::tr("Автоматически введенное отсутствующее значение");
      break;
    case DOUBTFUL_CORRECTED:
      str = QObject::tr("Автоматически замененное сомнительное значение");
      break;
    case MISTAKEN_CORRECTED:
      str = QObject::tr("Автоматически замененное ошибочное значение");
      break;
    case HAND_CORR_DOUBTFUL:
      str = QObject::tr("Вручную замененное сомнительное значение");
      break;
    case HAND_CORR_MISTAKEN:
      str = QObject::tr("Вручную замененное ошибочное значение");
      break;
    case HAND_CORR_ABSENT:
      str = QObject::tr("Вручную введенное отсутствующее значение");
      break;
    case SPECIAL_VALUE:
    str = QObject::tr("Специализированное значение");
     break;
    case DOUBTFUL:
      str = QObject::tr("Сомнительное значение");
      break;
    case MISTAKEN:
      str = QObject::tr("Ошибочное значение");
      break;
    case NO_OBSERVE:
      str = QObject::tr("Отсутствующее значение");
      break;
    default:
      break;
  }
  return str;
}

