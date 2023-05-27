#include "tbufrlist.h"
#include "tbufrtransform.h"

#include <cross-commons/debug/tlog.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/proto/surface_service.pb.h>

#include <commons/mathtools/mnmath.h>

#include <qregexp.h>
#include <qstringlist.h>
#include <qvector.h>

#include <float.h>
#include <math.h>


//! Создание пустого элемента списка
/*! 
  \param subNum номер поднабора
*/
TBufrList::TBufrList(uint subNum) 
{
  _subNum = subNum;
  _w = 0;
  _wAssoc = 0;
  _prev = 0;
  _next = 0;
  _isRetry = false;
  _group = 0;
  _meteoDescr = -1;
  _prev = 0;
  _next = 0;
  _isSymbol = false;
  _isReplStart = false;
  _isReplEnd = 0;
}

//! Создание копии элемента списка
/*! 
  \param vs Источник копии
*/
TBufrList::TBufrList(const TBufrList& vs):
  _chop(vs._chop),
  _bufrDescr(vs._bufrDescr)
{
  _subNum = vs._subNum;
  _isRetry = vs._isRetry;
  _isSymbol = vs._isSymbol;
  _isReplStart = vs._isReplStart;
  _isReplEnd = vs._isReplEnd;
  _group = vs._group;
  _meteoDescr = vs._meteoDescr;
  _w = vs._w;
  _wAssoc = vs._wAssoc;
  _prev = 0;
  _next = 0;
}

TBufrList::~TBufrList() 
{
  clear();
}

//! Установка дескриптора для текущего элемента
/*! 
  \param adescr Дескриптор
*/
void TBufrList::set( const BufrDescriptor &adescr ) 
{
  int atable_index;
  atable_index  = adescr.F*100000;
  atable_index += adescr.X*1000;
  atable_index += adescr.Y;
  set(atable_index);
}

//! Установка описателя данных для текущего элемента
/*!   
  \param adescr Описатель данных
*/
void TBufrList::set(const DataDescriptor &adescr) 
{
  if (!(adescr.descriptor.tableIndex() == 0 && _chop.setLocWidth != 0) ) {
    _bufrDescr = adescr;
  } 
  calcWidth();
}

//! Установка дескриптора для текущего элемента
/*! 
  \param atable_index Дескриптор
*/
void TBufrList::set(int atable_index) 
{
  _bufrDescr.descriptor.setIndex(atable_index);
}


//! Добавление нового элемента в список
/*! 
  \param adescr Дескриптор BUFR
  \param subNum Номер поднабора
  \param descr  Дескриптор TMeteoData
  \param group  Номер группы для descr
*/
void TBufrList::add(const BufrDescriptor &adescr, uint subNum, int descr, int group)
{
  int atable_index;
  atable_index  = adescr.F*100000;
  atable_index += adescr.X*1000;
  atable_index += adescr.Y;
  add(atable_index, subNum, descr, group);
}

//! Добавление нового элемента в список
/*! 
  \param atable_index  Дескриптор
  \param subNum        Номер поднабора
  \return true в случае успеха
*/
bool TBufrList::add( int atable_index, uint subNum, int descr, int group) 
{
  if ( _next == 0 &&
       _bufrDescr.descriptor.F ==  _bufrDescr.descriptor.Y && 
       _bufrDescr.descriptor.F ==  _bufrDescr.descriptor.X ) {
    set ( atable_index );
    calcWidth();
    _meteoDescr = descr;
    _group = group;
    return true;
  }
  
  TBufrList *lastvs=last();
  TBufrList *vs=0;

  vs=new TBufrList(subNum);
  if ( !vs ) return false;
  lastvs->_next=vs;
  vs->_prev=lastvs;
  //  vs->_num=lastvs->num() +1;
  vs->set ( atable_index );
  vs->_chop=vs->_prev->_chop;
  if (descr != -1) {
    vs->_meteoDescr = descr;
    vs->_group = group;
  }
  
  vs->calcWidth();
  return true;
}

 
//! Расчёт длины данных с учётом операторов
void TBufrList::calcWidth()
{
  _w = _wAssoc = 0;

  if ( _bufrDescr.descriptor.F == 1 || _bufrDescr.descriptor.F == 2) return;

  if (_chop.setLocWidth != 0) {
    _w = _chop.setLocWidth;
  } else if (isSymbol()) {
    if (_chop.setCharWidth != 0) {
      _w = _chop.setCharWidth;
    } else {
      _w = _bufrDescr.width;
    }
  } else {
    _w = _bufrDescr.width + _chop.addWidth;
  }
  
  if (!(_bufrDescr.descriptor.F == 0 && _bufrDescr.descriptor.X == 31)) {
    _wAssoc = _chop.aswidth;
  }
}

//! Поиск первого элемента списка
/*! 
  \return указатель на первый элемент
*/
TBufrList *TBufrList::first() {
    TBufrList *vs=this;
    while ( vs->prev() ) vs=vs->prev();
    return vs;
}


//! Поиск последнего элемента списка
/*! 
  \return указатель на последний элемент
*/
 TBufrList *TBufrList::last() {
    TBufrList *vs=this;
    while ( vs->next() ) vs=vs->next();
    return vs;
}

//! Обнуление данных текущего элемента
void TBufrList::clear() 
{
  _bufrDescr.descriptor.F=0;
  _bufrDescr.descriptor.X=0;
  _bufrDescr.descriptor.Y=0;
  _isRetry = false;
  _w = 0;
  _wAssoc = 0;
  _data.clear();
}

//! Удаление всего списка
void TBufrList::clearDelete() 
{
  TBufrList *vs = last()->prev();
  while ( vs && vs->next() ) {
    delete vs->next();
    vs->_next=0;
    vs=vs->prev();
  }
  clear();
}

//! Копирование элемента
/*! 
  \param item Узел, который копируется
  \param cnt    Количество добавляемых элементов
  \return true в случае успеха
*/
TBufrList* TBufrList::copy(TBufrList* item, uint cnt, bool withData /*= false*/)
{
  if (!item) return item;

  TBufrList* parent = item;
  TBufrList* last = item->next();
  TBufrList* part;

  for (uint i=0; i< cnt; i++) {
    // var(i);
    // var(parent->descr()->tableIndex());
    // var(next->descr()->tableIndex());

    part = new TBufrList(*item);
    if (withData) {
      part->_data = item->_data;
      // var(part);
      // var(part->_data.fdata);
    }

    parent->_next = part;
    part->_prev = parent;
    
    parent = part;
  }

  part->_next = last;

  return part;
}

//! Вставка элементов в список
/*! 
  \param parent Элемент, после которого добавляются новые элементы
  \param first  Элементы, которые копируются для вставки
  \param cnt    Количество добавляемых элементов
  \return true в случае успеха
*/
bool TBufrList::insert(TBufrList* parent, TBufrList* first, uint cnt, bool withData /*= false*/)
{
  if (!parent) return false;
  TBufrList* next = first;
  TBufrList* part;

  for (uint i=0; i< cnt; i++) {
    if (0 == next) {
#ifdef PARSE_LOG
      error_log << QObject::tr("Требуется повторение %1 дескрипторов, тогда как в закодированном BUFR их %2").arg(cnt).arg(i + 1);
#endif
      return false;
    }

    // var(i);
    // var(parent->descr()->tableIndex());
    // var(next->descr()->tableIndex());

    part = new TBufrList(*next);
    if (withData) {
      part->_data = next->_data;
      // var(part);
      // var(part->_data.fdata);
    }

    parent->_next = part;
    part->_prev = parent;
    
    next = next->_next;
    parent = part;
  }

  if (part->_isReplEnd > 1) {
    --part->_isReplEnd;
  }
  part->_next = first;
  first->_prev = part;

  return true;
}

//! Замена элемента (дескриптор последовательности на саму посдедовательность)
/*! 
  \param parent Заменяемый элемент
  \param atable_index Набор дескрипторов для формирования новых элементов
  \param exp  Описание последовательности
  \return true в случае успеха
*/
bool TBufrList::replace(TBufrList* parent, QVector<int>& atable_index, const BufrExpand& exp)
{
  if (!parent) return false;

  int isReplEnd = parent->_isReplEnd;

  TBufrList* next = parent->_next;
  TBufrList* vs = parent;

  vs->set(atable_index[0]);
  vs->_isReplEnd = 0;
  if (exp.group != -1) {
    vs->_meteoDescr = atable_index[0];
    vs->_group = exp.group;
  } else {
    vs->_meteoDescr = (exp.seq.descr.size() != 0) ? exp.seq.descr.at(0).second : atable_index[0];
    vs->_group = 0;
  }
  for (int i=1; i< atable_index.size(); i++) {
    vs->_next = new TBufrList(_subNum);
    vs->_next->_prev = vs;
    vs = vs->_next;
    if (exp.group != -1) {
      vs->_meteoDescr = atable_index[i];
      vs->_group = exp.group;
    } else {
      vs->_meteoDescr = (i < exp.seq.descr.size()) ? exp.seq.descr.at(i).second : atable_index[i];
      vs->_group = 0;
    }
    vs->set(atable_index[i]);
    vs->_chop=vs->_prev->_chop;
  }

  vs->_isReplEnd = isReplEnd;
  
  vs->_next = next;
  if ( next ) next->_prev = vs;

  return true;
}

//! Вставка элемента
/*! 
  \param parent Родитель, после которого проиводится вставка
  \param atable_index Дескриптор для нового элемента
  \return true в случае успеха
*/
// bool TBufrList::insert(TBufrList* parent, int atable_index)
// {
//   if ( !parent ) return add(atable_index, 0);

//   TBufrList *vs=0;
//   vs=new TBufrList(_subNum);
//   if ( !vs ) return false;
//   vs->_prev=parent;
//   vs->_next=parent->_next;
//   if ( parent->_next ) parent->_next->_prev=vs;
//   parent->_next=vs;
//   vs->set ( atable_index );
//   vs->_chop=vs->_prev->_chop;
//   return true;
// }

//! Удаление элементов
/*! 
  \param vs  Элемент, начиная с которого происходит удаление
  \param cnt Количество удаляемых элементов
  \return true в случае успеха
*/
//! удалить начиная с vs. cnt - количество элементов для удаления
bool TBufrList::del( TBufrList *vs, uint cnt)
{
  if ( !vs ) return false;

  TBufrList* prev = vs->prev();
  TBufrList* next;

  bool ok = true;

  for (uint i=0; i< cnt; i++) {
    next = vs->next();
    if (0 == next && i != cnt - 1) {
#ifdef PARSE_LOG
      error_log << QObject::tr("Требуется повторение %1 дескрипторов, тогда как в закодированном BUFR их %2").arg(cnt).arg(i + 1);
#endif
      vs = 0;
      ok = false;
      break;
    }
    delete vs;
    vs = next;
  }
  
  if (prev) {
    prev->_next = vs;
  } 

  if (vs) {
    vs->_prev = prev;
  }

  return ok;
}

//! Установка значения
/*! 
  \param adata Строковое значение
*/
void TBufrList::setData(const QString& adata)
{
  _data.quality = '0';
  _data.sdata = adata;
}

//! Установка значения
/*! 
  \param adata Численное значение
  \param aqual Показатель качества значения
*/
void TBufrList::setData (int adata, char aqual ) 
{
  _data.quality = aqual;
  if (_data.quality != '0') {
    // var(_data.fdata);
    return;
  }
  
  if (adata != (2<<(nBit() - 1)) - 1) { //all '1' => undefined
    _data.fdata = ( adata + (_bufrDescr.ref_value + _chop.setRefval)*_chop.mulRef ) / 
      ( pow ( 10., _bufrDescr.scale + _chop.addScale ) );
  }
  //   var(_data.fdata);
}

//! Установка значения
/*! 
  \param adata Численное значение
*/
void TBufrList::setData(int adata) 
{
  if (adata != (2<<(nBit()- 1)) - 1) { //all '1' => undefined
    setData(adata, '0');
  }
}

//! Печать в консоль списка, начиная с текущего значения
void TBufrList::print()
{
  TBufrList* vs = this;
  int num = 0;
  uint oldSub = vs->_subNum;
  QString res;

  // int tmpCnt1 = 0;
  // int tmpCnt2 = 0;

  while (vs) {
    if (oldSub != vs->_subNum) {
      num = 0;
      oldSub = vs->_subNum;
    }

    if (vs->isSymbol()) {
      res = vs->_data.sdata;
    } else {
      res = QString::number(vs->_data.fdata);
    }
    

    // if (vs->descr()->tableIndexStr() == "001007" || vs->descr()->tableIndexStr() == "002019") {
    debug_log << vs->_subNum << num << "   " 
	      << vs->descr()->tableIndexStr()<< "   " 
	      << QString::number(vs->_group).rightJustified(2, '0') 
	      << QString::number(vs->_meteoDescr).rightJustified(5, '0') << "   " 
	      << vs->nBit() << vs->_wAssoc << "   "  << vs->_isReplStart << vs->_isReplEnd << "   "
	      << vs->_bufrDescr.name_elem << "   " 
	      << res<<"   " << vs->_bufrDescr.unit  << vs->_data.assoc <<vs->_data.quality;

    //  }

    // if (vs->descr()->tableIndexStr() == "021022") {
    //   tmpCnt1++;
    //   if (res != "0") {
    // 	tmpCnt2++;
    //   }
    // }

    vs = vs->next();
    num++;
  }

  // var(tmpCnt1);
  // var(tmpCnt2);
}

//! Заполнение значений из указанного узла списка BUFR
/*! 
  \param vs Узел
  \param strVal  Строковое значение
  \param val     Число
  \param qual    Показатель качества
*/
void TBufrList::setValue(const TBufrList* vs, QString* strVal, float* val, 
			 control::QualityControl* qual) const
{
  if (vs->isSymbol()) {
    *strVal = vs->_data.sdata;
    *val = BAD_METEO_ELEMENT_VAL;
  } else {
    *strVal = "";
    if (vs->_data.fdata == FLT_MAX) {
      *val = BAD_METEO_ELEMENT_VAL;
    } else {
      *val = vs->_data.fdata;
    }
  }
  
  if (vs->_data.quality == '1') {
    *qual = control::NO_OBSERVE;
  } else {
    *qual = control::NO_CONTROL;
  }
}

//! Замена дескриптора таблицы B
/*! 
  \param bDescr Список дескрипторов для замены
  \param vs Узел
*/
void TBufrList::convertBDescr(const QHash<uint, BufrConvert>& bDescr, TBufrList* vs)
{
  if (bDescr.contains(vs->_meteoDescr)) {
    //	debug_log<<"contain" << vs->_meteoDescr;
    BufrConvert conv = bDescr.value(vs->_meteoDescr);
    vs->_meteoDescr = conv.descr;
  } else {
    BufrDescriptor* descr = vs->descr();
    if (descr->F == 2 && descr->X == 5) {
      vs->_meteoDescr = TMeteoDescriptor::instance()->descriptor("text");
    }
  }
}


//! Составление номеров групп для текущего узла, в соответствии с значением описателя данных
// void TBufrList::setCurrentSignGroup(const QHash<uint, QMap<int, uint> >& signGroup, const TBufrList* vs, 
// 				    QList<int>* curSignGroup) const
// {
//   curSignGroup->clear();
//   // debug_log<< "Sign group val=" << signGroup;
//   // printf("sign=0x%x %f q=%c\n", (uint)vs->_data.fdata, vs->_data.fdata, vs->_data.quality);
//   if (vs->_data.quality == '1') {
//     return;
//   }

//   int curVal = static_cast<int>(vs->_data.fdata);
        
//   if (curVal == 0) { 
//     if (signGroup.value(vs->_meteoDescr).contains(-1)) {
//       //нулевое значение и существует для него маска
//       curSignGroup->append(signGroup.value(vs->_meteoDescr).value(-1));
//     }
//   } else {
//     QMapIterator<int, uint> it(signGroup.value(vs->_meteoDescr));
//     while (it.hasNext()) {
//       it.next();
//       if (it.key() == -1) {
// 	continue; //пропускаем маску для нулевого значения
//       }
      
//       int mask = 1 << it.key();
//       //debug_log << mask << it.value() << "curVal=" << curVal << "res =" << (mask & curVal);
      
//       if (0 != (mask & curVal) && !curSignGroup->contains(it.value())) {
// 	curSignGroup->append(it.value());
//       }
//     }
//   }
  
//   if (curSignGroup->isEmpty()) {
//     curSignGroup->append(-1);
//     error_log << QObject::tr("Ошибка вертикальной значимости группы");
//   }

//   //  var(curSignGroup);

// }

void TBufrList::fill(QList<TMeteoData>& dataList, const QHash<uint, BufrConvert>& bDescr, int category)
{
#ifdef DEBUG_BUFR_PARSER
  trc;
#endif

  TBufrList* vs = this;
  if (vs == 0) {
    return;
  }

  uint oldSub = vs->_subNum;

  TMeteoData data;
  dataList.append(data);
  TMeteoData* current = &dataList.last(); //текущая заполняемая
  QList<TMeteoData*> replParent; //родитель для повторяющихся наборов (replication), м.б. вложенными

  QList<bool> hasMeteo; // есть ли дескрипторы данных (список, чтоб у родителя запоминать)
  bool isCancel = false; // последний дескриптор был отменой значения (для нескольких отмен подряд)
  QList<descr_t> childsIdent; // набор идентификационных дескрипторов текущей TMeteoData
  QList<QList<descr_t>> replChildsIdent;

  int linkCnt = 0; // количество оставшихся связанных дескрипторов с предыдущим

  QString res;
  float val;
  control::QualityControl qual;
  hasMeteo << false;

  //debug_log << current << current->parent();

  //TODO если инкремент, он фиг знает что.

  while (vs) {
    if (oldSub != vs->_subNum) {
      //каждый поднабор - отдельная TMeteoData
      //      num = 0;
      oldSub = vs->_subNum;
      TMeteoData data;
      dataList.append(data);
      current = &dataList.last();
      hasMeteo.clear();
      hasMeteo << false;
      isCancel = false;
      linkCnt = 0;
      childsIdent.clear();
      replParent.clear();
      replChildsIdent.clear();
      //var(oldSub);
    }

    if (isMeteoDataDescr(vs->descr())) {
      setValue(vs, &res, &val, &qual);
      convertBDescr(bDescr, vs);
      descr_t descr = vs->_meteoDescr;
      
      //debug_log << descr << val << res;

      //повторяющиеся наборы
      if (vs->_isReplStart && 0 == vs->_isReplEnd) {
	// if (hasMeteo) { //NOTE нельзя, ломается sigwx. надо проверять набор идентификаторов, но только раз для набора
	//   current = current->parent() != 0 ? current->parent() : &dataList.last();
	// }
	replParent.append(current);
	replChildsIdent.append(childsIdent);
	current = &current->addChild();
	//debug_log << "start repl" << current << current->parent() << hasMeteo.last();
	hasMeteo << false;
	isCancel = false;
	childsIdent.clear();
      } 

      if (linkCnt == 0) {
	if (category <= meteo::surf::kEndSynopType || category == meteo::surf::kFlightSigWx ||
	    category == meteo::surf::kOceanType || category == meteo::surf::kSatGnss) {
	  linkCnt = linkedDescrCount(vs);
	  if (0 != linkCnt) {
	    current = &current->addChild();
	    hasMeteo << false;
	  }
	}
      }
      //debug_log << descr;
      if (0 != linkCnt) {
	isCancel = false;
      } else if (!isIdentDataDescr(vs->descr())) {
	hasMeteo.last() = true;
	isCancel = false;
      } else {
	//var(hasMeteo.last());
	if (hasMeteo.last() || isNeedIsolate(vs->descr(), childsIdent)) {
	  if ((isCanceledDescr(vs->descr()) && val == BAD_METEO_ELEMENT_VAL && res.isEmpty() ) ) {//отмена дескриптора
	    if (childsIdent.contains(descr)) {
	      childsIdent.removeAll(descr);
	      if (!isCancel) { //подряд отмена нескольких дескрипторов
		isCancel = true;
		current = current->parent() != 0 ? current->parent() : &dataList.last();
		if (hasMeteo.size() > 1) {
		  hasMeteo.removeLast();
		}
		//debug_log << "cancel";
	      }
	    }
	    //    debug_log << "parent" << current << current->parent();
	  } else {
	    isCancel = false;
	    //debug_log << "ident" << childsIdent << isReplacedDescr(vs, descr, childsIdent);
	    if (isReplacedDescr(vs, descr, childsIdent)) {
	      current = current->parent() != 0 ? current->parent() : &dataList.last();
	      if (hasMeteo.size() > 1) {
		hasMeteo.removeLast();
	      }
	      current = &current->addChild();
	    } else if (val != BAD_METEO_ELEMENT_VAL) {
	      current = &current->addChild();
	    }
	    //debug_log << current << current->parent();
	    hasMeteo << false; //новый child
	    childsIdent.clear();
	  }
	}

	if (vs->descr()->X != 2 && !isCancel) {
	  //если два дескриптора одинаковых, то должны идти подряд
	  if (!(val == BAD_METEO_ELEMENT_VAL && res.isEmpty()) || 
	      (vs->next() != 0 && vs->next()->_meteoDescr == descr)) {//считаем, что только первый может быть пустым
	    childsIdent << descr;
	  }
	}
      }
      
      if (!isCancel /*&& !(val == BAD_METEO_ELEMENT_VAL && res.isEmpty())*/) { //нельзя пропускать пустые, напр. может отсутствовать значение нижнего уровня, а верхнего быть
	current->add(descr, res, val, qual, vs->_bufrDescr.unit);
	//      dataList[vs->_subNum].add(descr, res, val, qual, vs->_bufrDescr.unit);
      }

      if (!vs->_isReplStart && vs->_isReplEnd != 0) {
	int replCnt =  vs->_isReplEnd; //дескриптор может быть окончанием вложенного повторения
	while (0!= replCnt--) {
	  if (0 != replParent.size()) {
	    current = replParent.takeLast();
	    childsIdent = replChildsIdent.takeLast();
	  } else {
	    current = &dataList.last();
	  }
	  if (hasMeteo.size() > 1) {
	    hasMeteo.removeLast();
	  }
	}
	//debug_log << "end repl" << current << current->parent();
      }
      
      if (0 != linkCnt) {
	if (1 == linkCnt) {
	  linkCnt = 0;
	  current = current->parent() != 0 ? current->parent() : &dataList.last();
	  if (hasMeteo.size() > 1) {
	    hasMeteo.removeLast();
	  }
	} else {
	  --linkCnt;
	}
	if (category > meteo::surf::kEndSynopType && category != meteo::surf::kOceanType) {
	  hasMeteo.last() = false;
	}
      }

    } else {
      isCancel = false; //TODO или не надо

      //TODO копия то, что выше
      if (!vs->_isReplStart && vs->_isReplEnd != 0) {
	int replCnt =  vs->_isReplEnd; //дескриптор может быть окончанием вложенного повторения
	while (0!= replCnt--) {
	  if (0 != replParent.size()) {
	    current = replParent.takeLast();
	    childsIdent = replChildsIdent.takeLast();
	  } else {
	    current = &dataList.last();
	  }
	  if (hasMeteo.size() > 1) {
	    hasMeteo.removeLast();
	  }
	  //debug_log << "end repl" << current << current->parent();
	}
      }
      
    }

    vs = vs->next();
  }


  // var(dataList.at(0).childsCount());
}

//! Формирование TMeteoData из списка BUFR
/*! 
  \param dataList  Формируемый список TMeteoData
  \param bDescr    Описание для преобразования BUFR
  \param signGroup Список дескрипторов описателей BUFR 
*/
// void TBufrList::fill(QList<TMeteoData>& dataList, 
// 		      const QHash<uint, BufrConvert>& bDescr, 
// 		      const QHash<uint, QMap<int, uint> >& signGroup)
// {
// #ifdef DEBUG_BUFR_PARSER
//   trc;
// #endif
//   TBufrList* vs = this;
//   // int num = 0; для вывода отладки

//   if (vs == 0) {
//     return;
//   }
//   uint oldSub = vs->_subNum;
//   QString res;
//   float val;
//   control::QualityControl qual;
//   QList<int> curSignGroup; //!< номер группы, в соответствии с значением описателя данных

//   TMeteoData data;
//   dataList.append(data);

//   while (vs) {
//     if (oldSub != vs->_subNum) {
//       //каждый поднабор - отдельная TMeteoData
//       //      num = 0;
//       oldSub = vs->_subNum;
//       TMeteoData data;
//       dataList.append(data);
//     }

//     //    var(vs->_meteoDescr);

//     if (isMeteoDataDescr(vs->descr())) {

//       setValue(vs, &res, &val, &qual);
//       convertBDescr(bDescr, vs);

//       if (signGroup.contains(vs->_meteoDescr)) {
// 	setCurrentSignGroup(signGroup, vs, &curSignGroup);
//       }

//       if (vs->_group != 0 || curSignGroup.isEmpty()) {
// 	descr_t descr = meteodescr::create(vs->_group, vs->_meteoDescr);
// 	if (TMeteoDescriptor::instance()->isExist(descr)) {
// 	  dataList[vs->_subNum].add(descr, res, val, qual, vs->_bufrDescr.unit);
// 	  //debug_log << "add " << meteodescr::create(vs->_group, vs->_meteoDescr) << res << val;
// 	} else {
// 	  //debug_log << "not exist descr" << descr << vs->_bufrDescr.name_elem;
// 	}
//       } else if (curSignGroup.at(0) != -1) {
// 	for (int idx=0; idx < curSignGroup.count(); idx++) {
// 	  uint curGroup = curSignGroup.at(idx);
// 	  descr_t descr = meteodescr::create(curGroup, vs->_meteoDescr);
// 	  if (TMeteoDescriptor::instance()->isExist(descr)) {
// 	    dataList[vs->_subNum].add(descr, res, val, qual, vs->_bufrDescr.unit);
// 	    //debug_log << "add " << meteodescr::create(curGroup, vs->_meteoDescr) << res << val;
// 	  } else {
// 	    //debug_log << "not exist descr" << descr << vs->_bufrDescr.name_elem << vs->_meteoDescr;
// 	  }
// 	}
//       }
      
//       // debug_log<<vs->_subNum<< num << "   " << vs->descr()->tableIndexStr()<< "   " << 
//       //   QString::number(vs->group()).rightJustified(2, '0') << "   " << vs->nBit() << vs->_wAssoc << "   " << 
//       //   vs->_bufrDescr.name_elem << "   " << res<<"   "<<vs->_bufrDescr.unit<<vs->_data.assoc <<vs->_data.quality;
//     }
      
//     vs = vs->next();
//     //    num++;
//   }
// }

//! Формирование TMeteoData из списка BUFR для радиолокационных данных на карте 
// (Проще формирование дескрипторов, указывается тип продукции, заполняются только не пустые квадраты)
/*! 
  \param dataList  Формируемый список TMeteoData
*/
void TBufrList::fillRadar(QList<TMeteoData>& dataList, const QHash<uint, BufrConvert>& bDescr, bool onlyHeader)
{
  #ifdef DEBUG_BUFR_PARSER
  trc;
#endif
  TBufrList* vs = this;
  // int num = 0; для вывода отладки

  if (vs == 0) {
    return;
  }
  uint oldSub = vs->_subNum;
  QString res;
  float val;
  control::QualityControl qual;
  QList<int> curSignGroup; //!< номер группы, в соответствии с значением описателя данных

  TMeteoData data;
  dataList.append(data);
  bool isEmpty = false;
  bool isMap = false;
  int idx = 0;
  int cols = -1;
  int rows = -1;

  while (vs) {
    if (oldSub != vs->_subNum) { 
      //каждый поднабор - отдельная TMeteoData
      //      num = 0;
      oldSub = vs->_subNum;
      TMeteoData data;
      dataList.append(data);
    }

    if (isMeteoDataDescr(vs->descr())) {

      isMap = isMapData(vs->descr(), vs->bufrDescr(), vs->_data.fdata, &isEmpty);

      if (isMap && !dataList[vs->_subNum].hasParam(TMeteoDescriptor::instance()->descriptor("product"))) {
	dataList[vs->_subNum].set(TMeteoDescriptor::instance()->descriptor("product"), 
				  vs->descr()->tableIndexStr(),
				  vs->descr()->tableIndex(), control::RIGHT); //category	
      }
    
      if (isMap) {
	if (!onlyHeader) {
	  convertBDescr(bDescr, vs); 
	  if (!isEmpty && vs->_data.quality != '1') {
	    setValue(vs, &res, &val, &qual);
	    dataList[vs->_subNum].add(vs->descr()->tableIndex(), res, val, qual, vs->_bufrDescr.unit);
	    dataList[vs->_subNum].add(TMeteoDescriptor::instance()->descriptor("NeNe"), QString::number(idx), idx,
				      control::RIGHT);
	  }
	  ++idx;

	  if (rows != 0 && cols != 0 && idx > rows*cols) {
	    dataList.clear();
#ifdef PARSE_LOG
	    error_log << QObject::tr("Количество распакованных данных радара превышает значение, указанное в заголовке");
#endif
	    return;
	  }

	}
      } else {
	//    var(vs->_meteoDescr);
	setValue(vs, &res, &val, &qual);
	convertBDescr(bDescr, vs); 
	
	if (vs->descr()->tableIndex() == TMeteoDescriptor::instance()->descriptor("cols")) {
	  cols = val;
	}
	if (vs->descr()->tableIndex() == TMeteoDescriptor::instance()->descriptor("rows")) {
	  rows = val;
	}
	
	if (TMeteoDescriptor::instance()->isExist(vs->_meteoDescr)) {
	  dataList[vs->_subNum].add(vs->_meteoDescr, res, val, qual, vs->_bufrDescr.unit);
	  //debug_log << "add " << meteodescr::create(vs->_group, vs->_meteoDescr) << res << val;
	} else {
#ifdef PARSE_LOG
	  error_log << "not exist descr" << vs->_meteoDescr << vs->_bufrDescr.name_elem << res << val;
#endif
	}
	
	// debug_log<<vs->_subNum<< num << "   " << vs->descr()->tableIndexStr()<< "   " << 
	//   QString::number(vs->group()).rightJustified(2, '0') << "   " << vs->nBit() << vs->_wAssoc << "   " << 
	//   vs->_bufrDescr.name_elem << "   " << res<<"   "<<vs->_bufrDescr.unit<<vs->_data.assoc <<vs->_data.quality;
      }
    }
        
    vs = vs->next();
    //    num++;
  }

  //debug_log << "size=" << cols << rows << idx;
}

//! Проверка дескриптора BUFR на наличие соответствия дескриптора TMeteoData
/*! 
  \param descr Дескриптор BUFR
  \return true - подходит, false - нет
*/
bool TBufrList::isMeteoDataDescr(const BufrDescriptor* descr) const
{
  //! не нужны операторы, описатели
  bool ok = false;

  switch (descr->F) {
  case 0: 
    if (descr->X == 0 || descr->X == 31) {
      ok = false;
    } else if (descr->X == 33 && descr->Y != 50){
      ok = false;
    } else {
      ok = true;
    }
    break;
  case 2:
    if (descr->X == 5) {
      ok = true;
    }
    break;
  case 3:
    ok = false;
    break;
  }
  
  return ok;
}

//! Является ли дескриптором данных на карте
/*! 
  \param descr Дескриптор
  \param empty Пустое значение для этого типа данных
  \return true - если дескриптор данных
*/
bool TBufrList::isMapData(const BufrDescriptor* descr, const DataDescriptor& bufrDescr, float value, bool* isEmpty) const
{
  bool ok = false;
  *isEmpty = false;

  switch (descr->X) {
  case 13:
    if ((descr->Y >=  19 && descr->Y <= 23) ||
	descr->Y == 55) {
      ok = true;
      if (MnMath::isZero(value) || value < 0) {
	*isEmpty = true;
      }
    }
    break;
  case 21: 
    if ( (descr->Y ==  1 || descr->Y ==  2 ||
    	  descr->Y ==  3 || descr->Y == 14 ||
	  descr->Y == 17 || 
    	  descr->Y == 21 || descr->Y == 22 || 
    	  descr->Y == 36))
      {
      ok = true;
      float emptyVal = bufrDescr.ref_value / pow(10, bufrDescr.scale);   
      if (MnMath::isEqual(emptyVal, value, float(pow(10, -bufrDescr.scale - 1)))) {
	*isEmpty = true;
      }
      //debug_log << value << emptyVal << *isEmpty << pow(10, bufrDescr.scale);
    }
    break;
  }

  return ok;
}

//! Идентификационные данные (координаты горизонтальные, вертикальные, время)
bool TBufrList::isIdentDataDescr(const BufrDescriptor* descr) const
{
  if (descr->F == 2 && descr->X == 5) {
    return true;
  }

  if (descr->F != 0) return false;

  if (descr->X < 9) return true;

  // if (descr->X >= 29) return true;
  if (descr->X >= 25 && descr->X < 40) return true;

  if (descr->X == 22 &&
      (descr->Y == 55 || descr->Y == 56 || descr->Y == 60 ||
       descr->Y == 67 || descr->Y == 68)) {
    return true;
  }
  
  return false;
}

// bool TBufrList::isMeteoParamDescr(const BufrDescriptor* descr) const
// {
//   if (descr->F == 2 && descr->X == 5) {
//     return true;
//   }

//   if (descr->F != 0)  return false;

//   if (descr->X <=  9) return false;
//   if (descr->X >= 29 && descr->X != 40) return false;

//   return true;
// }

//новый набор идентификационных дескрипторов заменяет предыдущий
bool TBufrList::isReplacedDescr(TBufrList* start, descr_t descr, const QList<descr_t>& childsIdent) const
{
  if (childsIdent.size() == 1 && descr == childsIdent.at(0)) {
    return true;
  }

  TBufrList* vs = start;

  QList<descr_t> ident;
  for (int idx = 0; idx < childsIdent.size(); idx++) {//временные периоды заменяют друг друга
    if ((childsIdent.at(idx) >= 4021 && childsIdent.at(idx) <= 4032) || 
	(childsIdent.at(idx) >= 4073 && childsIdent.at(idx) <= 4086)) {
      ident.append(4020);
    } else {
      ident.append(childsIdent.at(idx));
    }
  }

  while (0 != vs && isIdentDataDescr(vs->descr())) {
    if ((vs->_meteoDescr >= 4021 && vs->_meteoDescr <= 4032) || 
	(vs->_meteoDescr >= 4073 && vs->_meteoDescr <= 4086)) {
      ident.removeAll(4020);
    } else {
      ident.removeAll(vs->_meteoDescr);
    }
    vs = vs->next();
  }

  if (ident.isEmpty()) return true;

  //var(ident);

  return false;
}

//дескриптор, который может быть отменен. или должен
bool TBufrList::isCanceledDescr(const BufrDescriptor* descr) const
{
  if (descr->F != 0) return false;

  if (descr->X == 5) {
    if (descr->Y == 21) {
      return true;
    }
  }

  if (descr->X == 7) {
    // if (descr->Y == 21 || descr->Y == 32 || 
    // 	descr->Y == 33 || descr->Y == 61 || descr->Y == 63)
      {
      return true;
    }
  }

  if (descr->X == 8) {
    return true;
  }

  return false;
}

bool TBufrList::isNeedIsolate(const BufrDescriptor* descr, QList<descr_t> childsIdent) const
{
  if (descr->tableIndex() != 8007) {
    return false;
  }
  for (int idx = 0; idx < childsIdent.size(); idx++) {
    if (childsIdent.at(idx) < 8000 || childsIdent.at(idx) > 8999 || childsIdent.at(idx) == 8007) {
      return true;
    }
  }

  return false;
}

//! связанные дескрипторы выделяем отдельным поднабором
int TBufrList::linkedDescrCount(TBufrList* vs) const
{
  if (vs == 0 || vs->next() == 0) return 0;

  if (vs->descr()->tableIndex() == 7004 && 
      vs->next()->descr()->tableIndex() == 10009) {
    return 2;
  }

  if (vs->descr()->tableIndex() == 8080 && 
      vs->next()->descr()->tableIndex() == 33050) {
    return 2;
  }

  if (vs->descr()->tableIndex() == 27031) {
    TBufrList* next = vs->next();
    int cnt = 1;
    while (next != nullptr) {
      if (next->descr()->tableIndex() == 28031 ||
	  next->descr()->tableIndex() == 10031 ||
	  next->descr()->tableIndex() == 1041  ||
	  next->descr()->tableIndex() == 1042  ||
	  next->descr()->tableIndex() == 1043) {
	cnt++;
      } else {
	break;
      }
      next = next->next();
    }
    if (cnt > 1) {
      return cnt;
    }
  }
  
  return 0;
}

void TBufrList::findIndex(int* index)
{
  TBufrList* vs = this;
  int II = 0;
  int iii = 0;

  while (vs) {
    if (vs->descr()->tableIndex() == 1001) {
      II = vs->_data.fdata;
    }
    if (vs->descr()->tableIndex() == 1002) {
      iii = vs->_data.fdata;
    }

    if (II !=0 && iii != 0) {
      break;
    }
    vs = vs->next();
  }

  *index = II*1000 + iii;
}

bool TBufrList::setReplicationFlag(int cnt)
{
  //  trc;
  _isReplStart = true;
  TBufrList* next = this;
  for (int i=0; i< cnt - 1; i++) {
    next = next->next();
    if (0 == next) {
#ifdef PARSE_LOG
      error_log << QObject::tr("Требуется повторение %1 дескрипторов, тогда как в закодированном BUFR их %2").arg(cnt).arg(i + 1);
#endif
      return false;
    }
  }
  next->_isReplEnd++;
  return true;
}

