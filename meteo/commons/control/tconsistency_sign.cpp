#include "tconsistency_sign.h"

#include "tmeteocontrol.h"
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <math.h>


//template<> tmcontrol::TConsistSign* TConsistSign::_instance = 0;

namespace {
  bool registerMethod()
  {
    tmcontrol::TConsistSign* m = new tmcontrol::TConsistSign(CONTROL_SETTINGS_DIR + "consistency_sign.xml");
    return TMeteoControl::instance()->registerMethod(control::CONSISTENCY_SIGN, m);
  }
  static bool regresult = registerMethod();
}


namespace tmcontrol {

  TConsistSign::TConsistSign(const QString& dirName):TControlMethodBase(dirName)
  {
    _settings = new control::ConsistSignSettings;
  }

  TConsistSign::~TConsistSign()
  {
    if (_settings) delete _settings;
  }

  bool TConsistSign::control(TMeteoData* data)
  {
    if (! isReadSettings()) {
      TControlMethodBase::readSettings();
    }

    if (rulesMatch(_settings->type, data)) {
      controlSignificantLevel(data);
    }

    //controlSignificantHeight(data, _settings->ranges.keys()); //TODO основное уравнение статики. что-то не работает
  

    return true;
  }



  bool TConsistSign::readSettings(QDomDocument* xml)
  {
    if (!xml) return false;

    QDomNode node = xml->firstChild().nextSibling();
    if (node.isNull()) {
      return false;
    }
    node = node.firstChild();
    while (!node.isNull()) {
      if (!node.isElement()) {
	node = node.nextSibling();
	continue;
      }
      QDomElement el = node.toElement();

      if (el.tagName() == "category") {
	descr_t descr = TMeteoDescriptor::instance()->descriptor(el.attribute("name"));
	_settings->type = QPair<descr_t, QStringList>(descr, el.attribute("num").split(','));
      } else if (el.tagName() == "level") {
	readLevelSettings(el);
      } else if (el.tagName() == "interpolation") {
	readLimits(el.toElement(), _settings->interpol);
      } else if (el.tagName() == "hydrostatic") {
	readLimits(el.firstChild().toElement(), _settings->hydro);
      }

      node = node.nextSibling();
    }

    return true;
  }

  void TConsistSign::readLevelSettings(const QDomElement& el)
  {
    QDomNodeList nodeList = el.elementsByTagName("val");
    for (int i=0; i<nodeList.count(); i++) {
      QDomElement child = nodeList.at(i).toElement();

      _settings->ranges.insert(child.attribute("level").toUInt(),
			       child.attribute("range").toUInt());
    }

  }

  void TConsistSign::readLimits(const QDomElement& el, QList<control::ConsistSignLimit>& limitList)
  {
    QDomNodeList nodeList = el.elementsByTagName("val");
    for (int i=0; i<nodeList.count(); i++) {
      control::ConsistSignLimit limit;
      readLimits(nodeList.at(i).toElement(), limit);

      limitList.append(limit);
    }
  }

  void TConsistSign::readLimits(const QDomElement& child, control::ConsistSignLimit& limit)
  {
    // limit.descr = TMeteoDescriptor::instance()->descriptor(child.attribute("name"));
    limit.signDescr = TMeteoDescriptor::instance()->descriptor(child.attribute("signName"));
    limit.signLevelDescr = TMeteoDescriptor::instance()->descriptor(child.attribute("signLevel"));
    // limit.levelDescr = TMeteoDescriptor::instance()->descriptor(child.attribute("nameLevel"));
    if (child.hasAttribute("level")) {
      limit.level = child.attribute("level").toInt();
      limit.lowLimit = child.attribute("low_limit").toFloat();
      limit.hiLimit = child.attribute("hi_limit").toFloat();
    } else {
      limit.level = -1;
      limit.lowLimit = child.attribute("limit").toFloat();
      limit.hiLimit = limit.lowLimit;
    }
  }

  void TConsistSign::controlSignificantLevel(TMeteoData* data)
  {
    for (int i=0; i<_settings->interpol.count(); i++) {
      controlSignificantLevel(_settings->ranges, 
			      _settings->interpol.at(i), data);
    }
    
    
  }

  void TConsistSign::controlSignificantLevel(const QMap<uint, uint>& ranges,
					     const control::ConsistSignLimit& limit,
					     TMeteoData* parent)
  {
    QList<uint> kPstd = ranges.keys();

    QList<TMeteoData*> data = parent->findChilds(limit.signDescr, meteodescr::kIsobarLevel);
    if (data.size() == 0) return;

    int idxSign1 = 0;
    int idxSign2 = 0;
    int idxStd = 0;
    int  p1, pstd;
    
    //предполагается, что уврони идут по порядку 
    for (int idx = 0; idx < data.size(); ++idx) {
      int cur = round(data.at(idx)->getValue(limit.signLevelDescr, BAD_METEO_ELEMENT_VAL));
      if (cur == BAD_METEO_ELEMENT_VAL) {
	continue;
      }
      if (kPstd.contains(cur)) { //стандартный уровень
	idxStd = idx;
	pstd = cur;

	idxSign2 = -1; //поиск следующей особой точки
	int p2;
	for (int kk = idx; kk < data.size(); ++kk) {
	  p2 = round(data.at(kk)->getValue(limit.signLevelDescr, BAD_METEO_ELEMENT_VAL));	  
	  if (p2 != BAD_METEO_ELEMENT_VAL && !kPstd.contains(p2)) {
	    idxSign2 = kk;
	    break;
	  }
	}
	if (idxSign2 == -1) {
	  break;
	}

	float t1 = data.at(idxSign1)->getValue(limit.signDescr, BAD_METEO_ELEMENT_VAL);
	float t2 = data.at(idxSign2)->getValue(limit.signDescr, BAD_METEO_ELEMENT_VAL);
	float tstd = data.at(idxStd)->getValue(limit.signDescr, BAD_METEO_ELEMENT_VAL);
	if (t1 == BAD_METEO_ELEMENT_VAL || t2 == BAD_METEO_ELEMENT_VAL || 
	    tstd == BAD_METEO_ELEMENT_VAL) {
	  continue;
	}
	if (p2 > p1) {
	  qSwap(p2, p1);
	  qSwap(t2, t1);	  
	}

	float valueCalc = t1 + (log(pstd) - log(p1)) / (log(p2) - log(p1)) * (t2 - t1);

	float lim = 0;
	if (pstd > limit.level) {
	  lim = limit.lowLimit;
	} else {
	  lim = limit.hiLimit;
	}

	if (fabs(tstd - valueCalc) > lim) {
	  data[idxStd]->paramPtr(limit.signDescr)->setQuality(control::DOUBTFUL, control::CONSISTENCY_SIGN, false);
	  data[idxSign1]->paramPtr(limit.signDescr)->setQuality(control::DOUBTFUL, control::CONSISTENCY_SIGN, false);
	  data[idxSign2]->paramPtr(limit.signDescr)->setQuality(control::DOUBTFUL, control::CONSISTENCY_SIGN, false);
	}

      } else {
	idxSign1 = idx;
	p1 = cur;
      }
    }
  }

  //копия предыдущей
  void TConsistSign::controlSignificantHeight(TMeteoData* parent, const QList<uint>& kPstd)
  {
    float Zstn = parent->getValue(TMeteoDescriptor::instance()->descriptor("h0_station"), BAD_METEO_ELEMENT_VAL); 
    var(Zstn);
    if (qFuzzyCompare(Zstn, BAD_METEO_ELEMENT_VAL)) return;
    
    meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();
    QList<TMeteoData*> data = parent->findChilds(d->descriptor("T"), meteodescr::kIsobarLevel);
    QList<TMeteoData*> dStd;
    QList<TMeteoData*> dSign;
    
    QList<TMeteoData*>::iterator it = data.begin();
    while (it != data.end()) {
      float P = (*it)->getValue(d->descriptor("P1"), BAD_METEO_ELEMENT_VAL, false);
      if ((*it)->getValue(d->descriptor("T"), BAD_METEO_ELEMENT_VAL, false) == BAD_METEO_ELEMENT_VAL ||
	  P == BAD_METEO_ELEMENT_VAL) {
      } else {
	if (kPstd.contains(round(P))) {
	  dStd << *it;
	} else {
	  dSign.prepend(*it); //чтоб в порядке возрастания
	}
      }
      ++it;
    }

    if (dStd.size() == 0 || dSign.size() == 0) {
      return;
    }
    
    for (int idx = 0; idx < dStd.size(); idx++) {
      float Ts = dStd.at(idx)->getValue(d->descriptor("T"), BAD_METEO_ELEMENT_VAL, false);
      float Zs = dStd.at(idx)->getValue(d->descriptor("hh"), BAD_METEO_ELEMENT_VAL, false);
      float Ps = dStd.at(idx)->getValue(d->descriptor("P1"), BAD_METEO_ELEMENT_VAL, false);
      if (Zs == BAD_METEO_ELEMENT_VAL) {
	continue;
      }

      float sum = 0;
      int index = indexLowLevel(Ps, dSign);
      debug_log << "index" << index << "Ps" << Ps;
      for (int sidx = index; sidx < dSign.size() - 2; sidx++) {
	float P1 = dSign.at(sidx)->getValue(d->descriptor("P1"), BAD_METEO_ELEMENT_VAL, false);
	float T1 = dSign.at(sidx)->getValue(d->descriptor("T"), BAD_METEO_ELEMENT_VAL, false);
	float P2 = dSign.at(sidx + 1)->getValue(d->descriptor("P1"), BAD_METEO_ELEMENT_VAL, false);
	float T2 = dSign.at(sidx + 1)->getValue(d->descriptor("T"), BAD_METEO_ELEMENT_VAL, false);
	sum += 100 * control::Rd/control::g * (T1 + T2 + 273.15)/2 * log(P2/P1);
	var(sum);
      }

      float Tn = dSign.at(dSign.size() - 1)->getValue(d->descriptor("T"), BAD_METEO_ELEMENT_VAL, false);
      float Pn = dSign.at(dSign.size() - 1)->getValue(d->descriptor("P1"), BAD_METEO_ELEMENT_VAL, false);
      float Z = Zstn + sum + 100 * control::Rd/control::g * (Tn + Ts+273.15)/2 * log(Pn/Ps);
      debug_log << Zstn << sum << 100*control::Rd/control::g * (Tn + Ts+273.15)/2 * log(Pn/Ps) << Z;
      
      float lim = 0;
      if (Ps > _settings->hydro.level) {
      	lim = _settings->hydro.lowLimit;
      } else {
      	lim = _settings->hydro.hiLimit;
      }

      debug_log << "res" <<  Z << Zs << lim;

      if (fabs(Z - Zs) > lim) {
   	dStd[idx]->paramPtr(d->descriptor("hh"))->setQuality(control::DOUBTFUL, control::CONSISTENCY_SIGN, false);
   	dStd[idx]->paramPtr(d->descriptor("P1"))->setQuality(control::DOUBTFUL, control::CONSISTENCY_SIGN, false);
   	dStd[idx]->paramPtr(d->descriptor("T"))->setQuality(control::DOUBTFUL,  control::CONSISTENCY_SIGN, false);
   	for (int p = index; p < dSign.size(); p++) {
   	  dSign[p]->paramPtr(d->descriptor("P1"))->setQuality(control::DOUBTFUL, control::CONSISTENCY_SIGN, false);
	  dSign[p]->paramPtr(d->descriptor("T"))->setQuality(control::DOUBTFUL,  control::CONSISTENCY_SIGN, false);
   	}
      } else {
	dStd[idx]->paramPtr(d->descriptor("hh"))->setQuality(control::RIGHT, control::CONSISTENCY_SIGN, false);
   	dStd[idx]->paramPtr(d->descriptor("P1"))->setQuality(control::RIGHT, control::CONSISTENCY_SIGN, false);
   	dStd[idx]->paramPtr(d->descriptor("T"))->setQuality(control::RIGHT,  control::CONSISTENCY_SIGN, false);
   	for (int p = index; p < dSign.size(); p++) {
   	  dSign[p]->paramPtr(d->descriptor("P1"))->setQuality(control::RIGHT, control::CONSISTENCY_SIGN, false);
	  dSign[p]->paramPtr(d->descriptor("T"))->setQuality(control::RIGHT,  control::CONSISTENCY_SIGN, false);
   	}
      }
    }
  }
    
  int TConsistSign::indexLowLevel(float level, const QList<TMeteoData*>& dSign)
  {
    for (int idx = 0; idx < dSign.size(); idx++) {
      if (dSign.at(idx)->getValue(TMeteoDescriptor::instance()->descriptor("P1"), BAD_METEO_ELEMENT_VAL, false) > level) {
	return idx;
      }
    }
    
    return dSign.size();
  }




}
