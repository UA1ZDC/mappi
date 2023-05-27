#include "complexmeteo.h"

#include "tmeteodescr.h"

#include <commons/mathtools/mnmath.h>

//NOTE для данных типа sigwx не подойдёт, там нельзя отбрасывать часть дескрипторов с координатами

ComplexMeteo::ComplexMeteo()
{
}

ComplexMeteo::~ComplexMeteo()
{
}

//! Создание дочернего с заполнением идентификации
ComplexMeteo* ComplexMeteo::createChild(descr_t complDescr)
{
  ComplexMeteo* child = new ComplexMeteo;
  child->_descriptor = complDescr;
  child->_levelType  = _levelType;
  child->_levelLo = _levelLo;
  child->_levelHi = _levelHi;
  child->_coord   = _coord;
  child->_dtType  = _dtType;
  child->_dt1 = _dt1;
  child->_dt2 = _dt2;

  if (_levelType == meteodescr::kTropoLevel || _levelType == meteodescr::kMaxWindLevel) {
    child->set(TMeteoDescriptor::instance()->descriptor("Ps"), "", _levelLo, control::NO_CONTROL);
  }
  return child;
}

ComplexMeteo& ComplexMeteo::addChild()
{
  ComplexMeteo* one = createChild();
  if (0 != one) {
    TMeteoData::addChild(this, one);
    return *one;
  }
  
  return *this;
}

//! Формирование из TMeteoData
bool ComplexMeteo::fill(const TMeteoData& md)
{
  bool ok = false;

  clear();
  meteodescr::TimeParam tp_cur;
  fillIdentification(md, &tp_cur);
  
  //const QList<descr_t> descrs = md.getDescrList();
  QMap<descr_t, ComplexMeteo*> cmlist;

  bool curexist;
  fillMeteo(md, &curexist, &cmlist);

  //var(cmlist.size());
  QMap<descr_t, ComplexMeteo*>::iterator it = cmlist.begin();
  while (it != cmlist.end()) {
    TMeteoData::addChild(this, it.value());
    ++it;
  }  
  
  for(const TMeteoData* ch : md.childs()) {
    ComplexMeteo* child = new ComplexMeteo;
    if (!child->fillChild(*ch, &tp_cur, this) && 0 == child->childs().size()) {
      delete child;
    } else {
      TMeteoData::addChild(this, child);
    }
  }

  return ok;
}

//! Заполнение дочернего
bool ComplexMeteo::fillChild(const TMeteoData& md, meteodescr::TimeParam* tpParent, ComplexMeteo* parent)
{
  bool okCur = false;
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

  clear();
  meteodescr::TimeParam tp = *tpParent;
  fillIdentification(md, &tp);
  // debug_log << "parent" << tpParent->dt0.day << tpParent->dt0.hour
  // 	    << tpParent->dt0.min << tp.dt1.day << tp.dt1.hour
  // 	    << tp.dt1.min;
  
  descr_t complDescr = BAD_DESCRIPTOR_NUM;
  bool allComplex = d->isAllCompex(md, &complDescr);
  QMap<descr_t, ComplexMeteo*> cmlist;
  
  if (allComplex) {
    _descriptor = complDescr;
    okCur = copyDataFrom(md);
    if (okCur) {
      d->copyChildIdentification(this, &md);
    }
  } else {
    bool curexist;
    fillMeteo(md, &curexist, &cmlist);
    
    if (curexist || _coord.isValid()) {
      okCur = true;
      d->copyChildIdentification(this, &md);
    }


    QMap<descr_t, ComplexMeteo*>::iterator it = cmlist.begin();
    while (it != cmlist.end()) {
      d->copyChildIdentification(it.value(), &md);
      TMeteoData::addChild(parent, it.value());
      ++it;
    }
  }

  for(const TMeteoData* ch : md.childs()) {
    ComplexMeteo* child = new ComplexMeteo;
    bool okfill = false;
    // if (tpParent->dt0.day == -1 && tpParent->dt0.hour == -1 && tpParent->dt0.min == -1) {
    okfill = child->fillChild(*ch, &tp, this);
    // } else {
    //   okfill = child->fillChild(*ch, tpParent, this);
    // }
    if (!okfill) {
      delete child;
    } else if (okCur) {
      TMeteoData::addChild(this, child);
    } else if (cmlist.size() != 0) {
      TMeteoData::addChild(cmlist.last(), child);
    } else {
      TMeteoData::addChild(parent, child);
    }	   
  }
 
  return okCur;
}

//! Заполнение идентификации из TMeteoData. tp_cur - возвращаемые текущие параметры времени
void ComplexMeteo::fillIdentification(const TMeteoData& md, meteodescr::TimeParam* tp_cur)
{
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

  d->findVertic(md, &_levelType, &_levelLo, &_levelHi);
  if (_levelType == meteodescr::kSurfaceLevel && _levelLo == -1) {
    _levelLo = 0;
  }

  if (_levelType == meteodescr::kTropoLevel || _levelType == meteodescr::kMaxWindLevel) {
    set(TMeteoDescriptor::instance()->descriptor("Ps"), "", _levelLo, control::NO_CONTROL);
  }
  
  d->findTime(md, tp_cur, &_dtType, &_dt1, &_dt2);
  _dtType = _dtType != -1 ? _dtType : meteodescr::kNormalTime;
  //var(_dt1);
  

  float lat, lon;
  float h = 0;
  control::QualityControl qual1, qual2;
  if (md.getValue(d->descriptor("La"), &lat, &qual1, false) && 
      md.getValue(d->descriptor("Lo"), &lon, &qual2, false) && 
      qual1 < control::MISTAKEN && 
      qual2 < control::MISTAKEN) {
    h = md.getValue(d->descriptor("h0_station"), 0, false);
    
    _coord = meteo::GeoPoint::fromDegree(lat, lon, h);
  }
}

bool ComplexMeteo::copyDataFrom(const TMeteoData& md)
{
  bool dataexist = false;
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();  
  const QList<descr_t> descrs = md.getDescrList();
  
  for (int idx = 0; idx < descrs.size(); idx++) {
    if (!d->isIdentDescr(descrs.at(idx))) {
      const QMap<int, TMeteoParam> plist = md.getParamList(descrs.at(idx));
      const QList<int> indexes = plist.keys();
      bool exist = false;
      for (int ki = indexes.size() - 1; ki >= 0 ; ki--) {
	const TMeteoParam& param = plist.value(indexes.at(ki));
	if (exist || !param.isInvalid()) {
	  add(descrs.at(idx), param, indexes.at(ki), true);
	  exist = true;
	  dataexist = true;
	}
      }
    }
  }
  return dataexist;
}

//! Формирование метеоданных с выделение составных, при необходимости
void ComplexMeteo::fillMeteo(const TMeteoData& md, bool* curexist, QMap<descr_t, ComplexMeteo*>* cmlist)
{
  *curexist = false;
 
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

  const QList<descr_t> descrs = md.getDescrList();
 
  for (int idx = 0; idx < descrs.size(); idx++) {
    //if (!d->isIdentDescr(descrs.at(idx)) || d->isNonLocateIdentDescr(descrs.at(idx))) {
    if (!d->isIdentDescr(descrs.at(idx))) {
      const QMap<int, TMeteoParam> plist = md.getParamList(descrs.at(idx));
      const QList<int> indexes = plist.keys();
      
      bool exist = false;
      for (int ki = indexes.size() - 1; ki >= 0 ; ki--) {
	const TMeteoParam& param = plist.value(indexes.at(ki));
	if (exist || !param.isInvalid()) {
	  //если встретили complex - создаем новый
	  descr_t complDescr = BAD_DESCRIPTOR_NUM;
	  if (d->isComponent(descrs.at(idx), &complDescr)) {
	    if (cmlist->contains(complDescr)) {
	      cmlist->operator[](complDescr)->add(descrs.at(idx), param, indexes.at(ki), true);
	    } else {
	      ComplexMeteo* child = createChild(complDescr);
	      cmlist->insert(complDescr, child);
	      child->add(descrs.at(idx), param, indexes.at(ki), true);
	    }	    
	  } else {
	    add(descrs.at(idx), param, indexes.at(ki), true);
	    *curexist = true;
	  }
	  exist = true;
	}	
      }
    }
  }
}


//! Поиск дочерних, с составным дескриптором complDescr
const QList<ComplexMeteo*> ComplexMeteo::findComplChilds(descr_t complDescr) const
{
  QList<ComplexMeteo*> complch;

  const QList<TMeteoData*>& child = childs();
  for (auto ch : child) {
    ComplexMeteo* cm = static_cast<ComplexMeteo*>(ch);
    if (nullptr != cm) {
      if (cm->descriptor() == complDescr) {
	complch << cm;
      } else {
	complch << cm->findComplChilds(complDescr);
      }
      
    }
  }
  
  return complch;
}

//! Печать данных
/*! 
  \param lev     Уровень вложенности - для сдвига
  \param maxQual Максимальное значение качества выводимых данных
 */
void ComplexMeteo::printData(meteodescr::TimeParam* tp /*= 0*/, int lev /*= 0*/, int  /*= -1*/, control::QualityControl maxQual /*= NO_OBSERVE*/) const
{
  //  trc;
  //  var(_data.size());
  QString tab;
  for (int l = 0; l < lev; l++) {
    tab += "    ";
  }
  
  qDebug() << tab << "level=" << _levelType << _levelLo << _levelHi;
  if (_coord.isValid()) {
    qDebug() << tab << "coord=" << _coord.latDeg() << _coord.lonDeg();
  }
  qDebug() << tab << "time=" << _dtType << _dt1.toString("yyyy-MM-dd hh:mm:ss.zzz") << _dt2.toString("yyyy-MM-dd hh:mm:ss.zzz");
  if (TMeteoDescriptor::instance()->isValid(_descriptor)) {
    qDebug() << tab << "descriptor=" << _descriptor;
  }  

  QHashIterator<descr_t, QMap<int, TMeteoParam> > it(data());
  while (it.hasNext()) {
    it.next();

    descr_t descr = it.key();
    QList<int> keys = it.value().keys();
    qSort(keys);

    if (keys.count() == 0) {
      qDebug() << "Empty data descr=" << descr;
    }

    for (int idx = 0; idx < keys.count(); idx++) {
      if (it.value().value(keys.at(idx)).quality() <= maxQual) {
	qDebug() << tab << TMeteoDescriptor::instance()->name(descr) << "(" << descr << ") : \t" 
		 << "idx" << keys.at(idx) << ":\t" << it.value().value(keys.at(idx));
      }
    }
  }

  const QList<TMeteoData*>& child = childs();
  
  if (child.size() != 0) {
    qDebug() << tab + "/------------- childs_count = " + QString::number(child.size());
  }
  for (int idx = 0; idx < child.size(); idx++) {
    qDebug() << tab + "childs_num = " + QString::number(idx);
    // var(idx);
    child.at(idx)->printData(tp, lev + 1, idx, maxQual);
  }
  if (child.size() != 0) {
    qDebug() << tab + "\\------------";
  }
}

QString ComplexMeteo::identJson(const QDateTime& ) const
{
  bool flag = false;
  
  // QString sdttype =  QString::number(_dtType);
  
  QString ident = "\n\"ident\" : {";

  // if (dt != _dt1 || dt != _dt2) {
  //   flag = true;
  //   ident += QString("\"dtType\" : %1, ").arg(sdttype);
  //   ident += QString("\"dtBeg\" : { \"$date\": \"%1\" }, ").arg(_dt1.toUTC().toString(Qt::ISODate));
  //   ident += QString("\"dtEnd\" : { \"$date\": \"%1\" },\n").arg(_dt2.toUTC().toString(Qt::ISODate));
  // }

  for (auto it = data().begin();  it != data().end(); ++it) {
    if (!TMeteoDescriptor::instance()->isNonLocateIdentDescr(it.key())) { continue; }
    
    QMap<int, TMeteoParam> values = it.value();
    QMap<int, TMeteoParam>::iterator iv = values.begin();
    
    QString key = QString("\"") +  QString::number(it.key()) + ("\" : ");
    int cnt = 0;
    QString value;

    for (; iv != values.end(); ++iv) {
      if(!(qFuzzyCompare(iv.value().value(), BAD_METEO_ELEMENT_VAL) && iv.value().code().isEmpty()) 
      	 && iv.value().quality() < control::DOUBTFUL) {
	value += QString("{ \"val\" : %1, \"code\" : \"%2\"},\n")
	  .arg(iv.value().value())
	  .arg(iv.value().code());
	cnt++;
	flag = true;
      }
    }
    
    if (cnt == 1) {
      ident += key + value;
    } else if (cnt > 1) {
      value.remove(-2, 1);
      ident += key + "[\n" + value + " ],\n";
    }
  }

  ident.remove(QRegExp(",\n$"));
  ident += "}";

  if (flag == false) {
    return QString();
  }
  
  return ident;
}

void ComplexMeteo::toDataStream(QDataStream& out) const
{
  out << _levelType;
  out << _levelLo;
  out << _levelHi;
  out << _coord;
  out << _dtType;
  out << _dt1;
  out << _dt2;
  out << _descriptor;
  TMeteoData::toDataStream(out);
}

void ComplexMeteo::fromDataStream(QDataStream& in)
{
  in >> _levelType;
  in >> _levelLo;
  in >> _levelHi;
  in >> _coord;
  in >> _dtType;
  in >> _dt1;
  in >> _dt2;
  in >> _descriptor;

  TMeteoData::fromDataStream(in);
}



const ComplexMeteo& operator>>(const ComplexMeteo& data, QByteArray& out)
{
  QDataStream ds(&out, QIODevice::WriteOnly);
  data.toDataStream(ds);
  
  return data;
}

ComplexMeteo& operator<<(ComplexMeteo& data, const QByteArray& ba)
{
  QDataStream ds(ba);
  data.fromDataStream(ds);
  
  return data;
}
