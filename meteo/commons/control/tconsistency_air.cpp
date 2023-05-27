#include "tconsistency_air.h"

#include "tmeteocontrol.h"
#include <commons/meteo_data/meteo_data.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <qstring.h>
#include <qvector.h>

#include <math.h>

//template<> tmcontrol::TConsistAir* TConsistAir::_instance = 0;

namespace {
  bool registerMethod() 
  {
    tmcontrol::TConsistAir* m = new tmcontrol::TConsistAir(CONTROL_SETTINGS_DIR + "consistency_air.xml");
    return TMeteoControl::instance()->registerMethod(control::CONSISTENCY_CTRL, m);
  }
  static bool regresult = registerMethod();
}


namespace tmcontrol {

  TConsistAir::TConsistAir(const QString& name): TControlTemplateBase<control::ConsistTableSettings, control::ConsistTableGroup>(name)
  {
    _adiabat = new QList<control::ConsistAdiabatSettings>;
  }
  
  TConsistAir::~TConsistAir()
  {
    if (_adiabat) delete _adiabat;
  }
  
  void TConsistAir::readGroupSettings(control::ConsistTableGroup& group, QDomNode& node)
  {
    while (!node.isNull()) {
      QDomElement el = node.toElement();
      if (el.tagName() == "adiabat") {
	readAdiabatSettings(el.firstChild());
      } else {
	control::ConsistTableCheck ch;
	if  (el.tagName() == "codeform") {
	  ch.type = control::CODEFORM_CHECK;
	} else if (el.tagName() == "phys") {
	  ch.type = control::PHYSVAL_CHECK;
	}
	
	if (readValueSettings(ch, el.firstChild())) {
	  group.append(ch);
	}
      }
      node = node.nextSibling();
    }
  }

  bool TConsistAir::readValueSettings(control::ConsistTableCheck& check, QDomNode node)
  {
    while (!node.isNull()) {
      QDomElement el = node.toElement();
      if (el.tagName() == "match") {
	control::ConsistMatch match;
	if (readMatch(match, el)) {
	  check.match.append(match);
	  // qDebug()<<"new="<<match.descr<<match.segment;
	  // debug_log<<"cnt="<<check.match.count();
	  // for (int k=0;k<check.match.count(); k++) {
	  //   qDebug()<<check.match.at(k).descr<<check.match.at(k).segment;
	  // }
	}
      } else if (el.tagName() == "table") {
	check.descr = TMeteoDescriptor::instance()->descriptor(el.attribute("name"));
	check.descrMatch = TMeteoDescriptor::instance()->descriptor(el.attribute("namematch"));
	control::ConsistTableRange val;
	QDomNode child = el.firstChild();
	while (!child.isNull()) {
	  if (readValue(val, child.toElement())) {
	    check.expr.append(val);
	  }
	  child = child.nextSibling();
	}
      }
      node = node.nextSibling();
    }

    return true;
  }

  bool TConsistAir::readMatch(control::ConsistMatch& match, const QDomElement& el)
  {
    match.descr = TMeteoDescriptor::instance()->descriptor(el.attribute("name"));
    if (match.descr == BAD_DESCRIPTOR_NUM) return false;

    QStringList vals = el.attribute("segment").split(";");
    TControlMethodBase::parseSegments(vals, match.segment);

    return true;
  }

  bool TConsistAir::readValue(control::ConsistTableRange& range, const QDomElement& el)
  {
    bool ok = false;
    QString match = el.attribute("match");

    range.segment.first = match.section(',', 0, 0).toFloat(&ok);
    if (!ok) return false;
    range.segment.second = match.section(',', 1, 1).toFloat(&ok);
    if (!ok) return false;

    range.diff = el.attribute("diff").toInt(&ok);
    if (!ok) return false;
    range.step = el.attribute("step").toFloat(&ok);
    if (!ok) return false;
    range.max = el.attribute("max").toFloat(&ok);
    if (!ok) return false;

    return true;
  }

  void TConsistAir::readAdiabatSettings(QDomNode parent)
  {
    control::ConsistAdiabatSettings cas;

    cas.descrLevel = TMeteoDescriptor::instance()->descriptor(parent.toElement().attribute("namematch"));
    cas.descr = TMeteoDescriptor::instance()->descriptor(parent.toElement().attribute("name"));
    if (cas.descr == BAD_DESCRIPTOR_NUM ||
	cas.descrLevel == BAD_DESCRIPTOR_NUM) {
      error_log << QObject::tr("Не верно задан дескриптор температура='%1', уровень='%2'").
	arg(parent.toElement().attribute("name")).
	arg(parent.toElement().attribute("namematch"));
      return;
    }

    QDomNode node = parent.firstChild();
    control::ConsistTableRange val;
    while (!node.isNull()) {
      if (readValue(val, node.toElement())) {
	cas.expr.append(val);
      }
      
      node = node.nextSibling();
    }

    _adiabat->append(cas);
  }
  
  bool TConsistAir::rulesMatch(const QPair<descr_t,QStringList>& type, TMeteoData* data)
  {
    if (0 == data) return false;
    
    QString num; // = data->getCodeUp(type.first);
    // if (num.isEmpty()) {
    //   debug_log<<QObject::tr("Нет дескриптора %1").arg(type.first);
    //   return false;
    // }
    
    // if (type.second.contains(num)) {
    //   return true;
    // }
    
    //проверяем есть ли child с аэрологическими данными. чтоб сравнивать потом несколько child одного родителя
    for (int idx = 0; idx < data->childsCount(); idx++) {
      num = data->child(idx)->getCode(type.first, false);
      if (!num.isEmpty()) {
	if (type.second.contains(num)) {
	  return true;
	}
      }
    }

    return false;  
  }

  bool TConsistAir::makeControl(const QList<control::ConsistTableGroup>& rules , TMeteoData* data)
  {
    for (int r = 0; r< rules.count(); r++) {
      for (int g = 0; g< rules.at(r).count(); g++) {
	if (rulesMatch(rules.at(r).at(g).match, data)) {
	  controlInversion(rules.at(r).at(g), data);
	  break;
	}
      }
    }

    for (int i = 0; i < _adiabat->count(); i++) {
      controlSuperAdiabatic(data, _adiabat->at(i));
    }

    //controlHydroBalance(data);
    
    return true;
  }

  bool TConsistAir::rulesMatch(QList<control::ConsistMatch> match, TMeteoData* data)
  {
    bool ok = false;

    for (int i=0; i<match.count(); i++) {
      const TMeteoParam& param = data->getParamUp(match.at(i).descr);
      if (param.quality() == control::NO_OBSERVE) {
	//	debug_log<< "Нет дескриптора в данных"<<match.at(i).descr;
	return false;
      }
      //QString code = param.code();
      //      debug_log<<"d"<<match.at(i).descr<<param._value;
      ok = false;
      for (int segm=0; segm<match.at(i).segment.count(); segm++) {
	//debug_log<<"segm="<< match.at(i).segment.at(segm).first<< match.at(i).segment.at(segm).second;
	if (param.value() <= match.at(i).segment.at(segm).second && param.value() >= match.at(i).segment.at(segm).first) {
	  ok = true;
	  break;
	}
      }

      //      debug_log<<"rule"<<i<<ok;

      if (ok == false) break;
    }
    
    return ok;
  }

  void TConsistAir::controlInversion(const control::ConsistTableCheck& check, TMeteoData* parent)
  {
    QList<TMeteoData*> data = parent->findChilds(check.descr, meteodescr::kIsobarLevel);

    QList<TMeteoData*>::iterator it = data.begin();
    while (it != data.end()) {
      if ((*it)->getValue(check.descr, BAD_METEO_ELEMENT_VAL, false) == BAD_METEO_ELEMENT_VAL) {
	it = data.erase(it);
      } else {
	++it;
      }
    }

    if (data.size() == 0) {
      return;
    }

    QVector<int> consist(data.count());

    for (int idx = 0; idx < data.size(); idx++) {
      if (idx != 0) {
	float temp = data.at(idx)->getValue(check.descr, -9999, false);
	float tempPrev = data.at(idx-1)->getValue(check.descr, -9999, false);
	if (temp != -9999 && tempPrev != -9999) {
	  float lim = getLimit(data.at(idx)->getValue(check.descrMatch, -9999, false),
			       data.at(idx-1)->getValue(check.descrMatch, -9999, false), check.expr);
	  // debug_log << "prev" << temp << tempPrev << (temp - tempPrev) << lim 
	  // 	    << data.at(idx)->getValue(check.descrMatch, -9999, false) 
	  // 	    << data.at(idx-1)->getValue(check.descrMatch, -9999, false);
	  if (lim !=0 && temp - tempPrev <= lim) {
	    ++consist[idx];
	    //var(consist[idx]);
	  }	
	}
      }
      if (idx != data.size() -1) {
	float temp = data.at(idx)->getValue(check.descr, -9999, false);
	float tempNext = data.at(idx+1)->getValue(check.descr, -9999, false);
	if (temp != -9999 && tempNext != -9999) {
	  float lim = getLimit(data.at(idx)->getValue(check.descrMatch, -9999, false),
			       data.at(idx+1)->getValue(check.descrMatch, -9999, false), check.expr);
	  // debug_log << "next"  << tempNext << temp << (tempNext - temp) << lim 
	  // 	    << data.at(idx)->getValue(check.descrMatch, -9999, false) 
	  // 	    << data.at(idx+1)->getValue(check.descrMatch, -9999, false);
	  if (lim != 0 && tempNext - temp <= lim) {
	    ++consist[idx];
	    //var(consist[idx]);
	  }
	}
      }
      //debug_log;
    }

    //debug_log << "consist air" << consist;
    QVector<control::QualityControl> quality(data.count(), control::NO_CONTROL);

    bool need = false;
    do {
      need = false;
      //если разница с обоими соседями в пределах нормы
      for (int l=0; l< data.count(); l++) {
	//var(l);
	if (consist.at(l) == 2 && quality[l] == control::NO_CONTROL) {
	  quality[l] = control::RIGHT;
	  //debug_log << "val=" << temp.value(indexes.at(l)).value() << quality[l];
	  need = true;
	}
	//если разница с соседом, прошедшим контроль в пределах нормы
	if (consist.at(l) == 1 && 
	    quality[l] == control::NO_CONTROL && 
    	    ((l!=0 && (quality[l-1] == control::RIGHT)) || 
	     (l != data.count()-1 && (quality[l+1] == control::RIGHT)) || quality.size() <= 2)) {
	  quality[l] = control::RIGHT;
	  //debug_log << "val=" << temp.value(indexes.at(l)).value() << quality[l];
	  need = true;
	}
      }
    } while (need);

    for (int l=0; l< data.count(); l++) {
      TMeteoParam* mp = data[l]->paramPtr(check.descr);
      if (0 != mp && mp->value() != BAD_METEO_ELEMENT_VAL) {
	if (quality[l] != control::RIGHT) {
	  mp->setQuality(control::MISTAKEN, control::CONSISTENCY_CTRL, false);
#ifdef PARSE_DEBUG_LOG
	  debug_log << "set quality" << control::MISTAKEN << "rule= inversion" 
		    << TMeteoDescriptor::instance()->name(check.descr) << "descr="
		    << check.descr << mp->value();
#endif
	} else {
	  mp->setQuality(control::RIGHT, control::CONSISTENCY_CTRL, false);
	  //debug_log << "val=" << temp[indexes.at(l)].value() << temp[indexes.at(l)].quality();
	}
      } 
    }   
  }

  float TConsistAir::getLimit(float p1, float p2, const QList<control::ConsistTableRange>& range)
  {
    float lim = 0;
    if (p1 <= 0 || p2 <= 0) return lim;

    float diff = fabs(p2-p1);
    for (int r=0; r<range.count(); r++) {
      if (p2 > range.at(r).segment.first && p2 <= range.at(r).segment.second) {
	//debug_log << p2 << range.at(r).segment.first << range.at(r).segment.second;
	if (diff >= range.at(r).diff) {
	  lim = range.at(r).max;
	} else {
	  lim = range.at(r).step * diff;
	}
	//debug_log << diff << lim;
	
	break;
      }
    }
    return lim;
  }

  float TConsistAir::getAdiabatLimit(float p, const QList<control::ConsistTableRange>& range) 
  {
    float lim = 9999;
    for (int r=0; r<range.count(); r++) {
      if (p > range.at(r).segment.first && p <= range.at(r).segment.second) {
	lim = range.at(r).max;
	break;
      }
    }

    return lim;
  }

  void TConsistAir::controlSuperAdiabatic(TMeteoData* parent, const control::ConsistAdiabatSettings& adiabat)
  {
    QList<TMeteoData*> data = parent->findChilds(adiabat.descr);
    if (data.size() == 0) {
      return;
    }

    float m = control::R/control::Cp;
    float e;
   
    for (int kk = 0; kk < data.count()-1; kk++) {
      TMeteoParam* temp = data[kk]->paramPtr(adiabat.descr, false);
      float press = data[kk]->getValue(adiabat.descrLevel, -9999, false);
      float pressNext = data[kk+1]->getValue(adiabat.descrLevel, -9999, false);
      if ( press == -9999 || pressNext == -9999) {
	continue;
      }
      float nTempNext = temp->value()* (pressNext/ press) * m;

      e = getAdiabatLimit(press, adiabat.expr);
      if (nTempNext - temp->value() >= e) {
	temp->setQuality(control::MISTAKEN, control::CONSISTENCY_CTRL, false);
#ifdef PARSE_DEBUG_LOG
	debug_log << "set quality" << control::MISTAKEN << "rule= superadiabat" 
		  << TMeteoDescriptor::instance()->name(adiabat.descr) << "descr=" 
		  << adiabat.descr << temp->value();
#endif
      }
    }
  }
  
  void TConsistAir::controlHydroBalance(TMeteoData* parent)
  {
    trc;
    meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
    
    const TMeteoData* tropo =  parent->findChild(meteodescr::kTropoLevel);
    if (0 == tropo) {
      return;
    }

    const TMeteoParam& Ttr = parent->meteoParam("T");
    const TMeteoParam& Ptr = parent->meteoParam("P1");
    if (!Ttr.isValid() || !Ptr.isValid()) {
      return;
    } 
    
    QList<TMeteoData*> data = parent->findChilds(d->descriptor("T"), meteodescr::kIsobarLevel);

    QList<TMeteoData*>::iterator it = data.begin(); //TODO можно сразу в цикле ниже проверять
    while (it != data.end()) {
      if ((*it)->getValue(d->descriptor("T"), BAD_METEO_ELEMENT_VAL, false) == BAD_METEO_ELEMENT_VAL) {
	it = data.erase(it);
      } else {
	++it;
      }
    }

    if (data.size() == 0) {
      return;
    }

    QList<float> F;
    QList<control::QualityControl> qual;
    float E = 1, Eprev = 1;
    for (int idx = 0; idx < data.size() - 1; idx++) {
      control::QualityControl qual_cur;
      controlHydroBalance(data[idx], data[idx + 1], Ttr.value() + 273.15, Ptr.value(), &E, &qual_cur);
      qual.append(qual_cur);
      if (idx > 0) {
	debug_log << "F" << Eprev / E;
	F.append(Eprev / E);
      }
      Eprev = E;
      debug_log;
    }
    qual.append(qual.last());
    F.append(F.last());
    
    bool allMistaken = false;
    for (int idx = 0; idx < data.size(); idx++) {
      if (allMistaken) {
	data[idx]->paramPtr(d->descriptor("T"))->setQuality(control::MISTAKEN, control::CONSISTENCY_CTRL, false);
	data[idx]->paramPtr(d->descriptor("hh"))->setQuality(control::MISTAKEN, control::CONSISTENCY_CTRL, false);
	continue;
      }

      if (qual[idx] == control::RIGHT) {
	data[idx]->paramPtr(d->descriptor("T"))->setQuality(control::RIGHT, control::CONSISTENCY_CTRL, false);
	data[idx]->paramPtr(d->descriptor("hh"))->setQuality(control::RIGHT, control::CONSISTENCY_CTRL, false);
      } else {
	data[idx]->paramPtr(d->descriptor("T"))->setQuality(control::DOUBTFUL, control::CONSISTENCY_CTRL, false);
	data[idx]->paramPtr(d->descriptor("hh"))->setQuality(control::DOUBTFUL, control::CONSISTENCY_CTRL, false);
	if (idx < data.size() - 2) {
	  if (F[idx] > 0.5 && F[idx] <2.0) {
	    data[idx + 1]->paramPtr(d->descriptor("T"))->setQuality(control::MISTAKEN, control::CONSISTENCY_CTRL, false);
	  } else if (F[idx] < -0.5 && F[idx] > -2.0) {
	    data[idx + 1]->paramPtr(d->descriptor("hh"))->setQuality(control::MISTAKEN, control::CONSISTENCY_CTRL, false);
	  } else if (F[idx] >= 2.0) {
	    allMistaken = true;
	    data[idx]->paramPtr(d->descriptor("T"))->setQuality(control::MISTAKEN, control::CONSISTENCY_CTRL, false);
	    data[idx]->paramPtr(d->descriptor("hh"))->setQuality(control::MISTAKEN, control::CONSISTENCY_CTRL, false);
	  }
	}
      }
    }
    
  }

  void TConsistAir::controlHydroBalance(TMeteoData* data, TMeteoData* dataNext, float Ttr, float Ptr,
					float* E, control::QualityControl* qual)
  {
    if (nullptr == data || nullptr == dataNext) {
      return;
    }
    meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();

    float P     =  data->getValue(d->descriptor("P1"), BAD_METEO_ELEMENT_VAL);
    float Pn = dataNext->getValue(d->descriptor("P1"), BAD_METEO_ELEMENT_VAL);
    float h  =     data->getValue(d->descriptor("hh"), BAD_METEO_ELEMENT_VAL);
    float hn = dataNext->getValue(d->descriptor("hh"), BAD_METEO_ELEMENT_VAL);
    float Tvirt  =     data->getValue(d->descriptor("T"), BAD_METEO_ELEMENT_VAL) + 273.15;
    float Tvirtn = dataNext->getValue(d->descriptor("T"), BAD_METEO_ELEMENT_VAL) + 273.15;

    if (tropopauseLevel(Tvirt, h, Tvirtn, hn)) {
      debug_log << "tropo";
      Tvirtn = Tvirtn + Ttr - (Tvirt + (log(P) - log(Ptr)) / (log(P) - log(Pn)));
    }
    
    float Di, Da, Db;
    
    calcHeight(Tvirt, P, Tvirtn, Pn, &Di, &Da, &Db);
    //  float lim = 3. / 8. * fabs(Da-Db);
    float lim = 0.5 * fabs(Da-Db);
    debug_log << "h" << h << hn << hn - h << fabs(hn - h - Di);
    //TODO пределы для lim
    *E = fabs(hn - h - Di);
    if (*E < lim) {
      *qual = control::RIGHT;
    } else {
      *qual = control::MISTAKEN;
    }
    
    var(Da);
    var(Db);
    var(Di);
    var(lim);
    var(*E);
    var(*qual);

  }

  

  bool TConsistAir::tropopauseLevel(float t1, float h1, float t2, float h2)
  {
    //убывание температуры с высотой становится меньше 2(°)С/км
    if ((t1 - t2)/(h2 - h1) < 0.002) return true;

    return false;
  }
  
  //T - температура (желательно виртуальная), P - давление. Tnext, Pnext - следующая точка
  void TConsistAir::calcHeight(float T, float P, float Tnext, float Pnext, float* Di, float* Da, float* Db)
  {
    float Rg = control::Rd/control::g;
    *Di = Rg *  (T + Tnext) / 2  * log(P / Pnext);
    *Da = Rg / 2 * ((1 + pow(Pnext / P, control::R/control::Cp)) * T * log(P / Pnext));
    *Db = Rg / 2 * ((1 + pow(P / Pnext, control::R/control::Cp)) * Tnext * log(P / Pnext));

  }
  
}
