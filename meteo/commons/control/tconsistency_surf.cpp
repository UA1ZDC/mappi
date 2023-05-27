#include "tconsistency_surf.h"
#include "tmeteocontrol.h"

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <qdom.h>

#include <math.h>

//TODO
//controlPress - высоты на уровне станции и на уровне моря

//template<> tmcontrol::TConsistency* TConsistency::_instance = 0;

namespace {
  bool registerMethod() 
  {
    tmcontrol::TConsistency* m = new tmcontrol::TConsistency(CONTROL_SETTINGS_DIR + "consistency_surface.xml");
    return TMeteoControl::instance()->registerMethod(control::CONSISTENCY_CTRL, m);
  }
  static bool regresult = registerMethod();
}


namespace tmcontrol {

  TConsistency::TConsistency(const QString& dirName):TControlTemplateBase<control::ConsistencySettings,control::ConsistencyGroup>(dirName)
{
}

TConsistency::~TConsistency()
{
}

void TConsistency::readGroupSettings(control::ConsistencyGroup& group, QDomNode& node)
{
  QDomNodeList nodeList;
  
  while (!node.isNull()) {
    QDomElement el = node.toElement();
    if (el.tagName() == "check") {
      readCheckSettings(group, el);
    } else if (el.tagName() == "match") {
      readRanges(group.match, el, -1);
    } else if (el.tagName() == "expression") {
      int number = el.toElement().attribute("rule").toInt();
      nodeList = el.elementsByTagName("val");
      control::ConsistencyCheck expr;
      for (int i =0; i< nodeList.count(); i++) {
	readRanges(expr, nodeList.at(i).toElement(), number);
      }
      group.expr.append(expr);
    }
    node = node.nextSibling();
  }
}

void TConsistency::readCheckSettings(control::ConsistencyGroup& group, const QDomElement& el)
{
  if (el.attribute("qual") == "4") {
    group.qual = control::DOUBTFUL;
  } else if (el.attribute("qual") == "5") {
    group.qual = control::MISTAKEN;
  } else if (el.attribute("qual") == "6") {
    group.qual = control::NO_OBSERVE;
  } else if (el.attribute("qual") == "7") {
    group.qual = control::SPECIAL_VALUE;
  } else {
    group.qual = control::DOUBTFUL;
    error_log<< QObject::tr("Ошибка xml-файла (согласованность приземных данных). Нет указателя контроля качества");
  }
  QStringList vals = el.attribute("name").split(',');
  for (int i=0; i<vals.count(); i++) {
    descr_t d = TMeteoDescriptor::instance()->descriptor(vals.at(i));
    if (d != BAD_DESCRIPTOR_NUM) {
      group.descrList.append(d);
    }
  }
}

void TConsistency::readRanges(control::ConsistencyCheck& values, const QDomElement& el, int number)
{
  // descr_t key = TMeteoDescriptor::instance()->descriptor(el.attribute("name"));
  // if (key == BAD_DESCRIPTOR_NUM) {
  //   error_log<< QObject::tr("Ошибка xml-файла. Нет дескриптора %1").arg(el.attribute("name"));
  //   return;
  // }

  QString key = el.attribute("name");

  control::ConsistencyRange range;
  range.number = number;

  if (el.hasAttribute("index")) {
    range.index = el.attribute("index").toUInt();
  }
  if (el.hasAttribute("subindex")) {
    range.subIndex = el.attribute("subindex").toUInt();
  }
  if (el.hasAttribute("sub")) {
    range.subDescr = TMeteoDescriptor::instance()->descriptor(el.attribute("sub"));
  }
  if (!el.attribute("str").isEmpty()) {
    range.str = el.attribute("str").split(",");
  }
  QStringList vals = el.attribute("num").split(",");
  parseNum(vals, range.val);
  vals = el.attribute("segment").split(";");
  parseSegments(vals, range.segment);
  range.negation = (el.attribute("negation") == "true");
  
  if (el.hasAttribute("min")) {
    range.min.first = true;
    range.min.second = el.attribute("min").toFloat();
  }

  if (el.hasAttribute("max")) {
    range.max.second = el.attribute("max").toFloat(&range.max.first);
  }

  if (el.hasAttribute("type")) {
    if (el.attribute("type") == "code") {
      range.type = control::CODEFORM_CHECK;
    } else if (el.attribute("type") == "phys") {
      range.type = control::PHYSVAL_CHECK;
    } else if (el.attribute("type") == "valid") {
      range.type = control::VAL_VALID;
    }

  } else {
    error_log << QObject::tr("Ошибка xml");
  }

  values.insert(key, range);
}

  //поиск совместимости между дочерними
bool TConsistency::makeControl(const QList<control::ConsistencyGroup>& rules , TMeteoData* data)
{
  int num = 0;

  //  var(rules.count());

  //TODO в limitrules отдавать child, если Ns и т.д. признак для группы можно

  for (int group = 0; group < rules.count(); group++) {
    if (descrExist(rules.at(group).descrList, data) && 
	limitRulesMatch(rules.at(group).match, data)) {
      //debug_log<<"group"<<group<<"match. expr.cnt="<<rules.at(group).expr.count();
      for (num = 0; num < rules.at(group).expr.count(); num++) {
	// debug_log<<"check expr=" << num << rules.at(group).expr.at(num).count() 
	// 	 << "rule=" << rules.at(group).expr.at(num).begin().value().number;
	if (limitRulesMatch(rules.at(group).expr.at(num), data)) {
#ifdef PARSE_DEBUG_LOG
	  debug_log << "set quality" << rules.at(group).qual << "rule=" 
		    << rules.at(group).expr.at(num).begin().value().number
		    << rules.at(group).descrList << "\n";
#endif
	  setQuality(rules.at(group).descrList, rules.at(group).qual, data);
	  break;
	}
      }
      if (num == rules.at(group).expr.count()) {
	//debug_log<<"all expr check" << rules.at(group).descrList;
	setQuality(rules.at(group).descrList, control::RIGHT, data);
      }

    } else {
      //debug_log<<"group"<<group<<"mismatch";
    }
  }

  //  controlPress(data);

  return true;
}

bool TConsistency::descrExist(QList<descr_t> descrList, TMeteoData* data)
{
  meteodescr::LevelType curType = static_cast<meteodescr::LevelType>(data->getValueCur(TMeteoDescriptor::instance()->descriptor("level_type"), 
										       meteodescr::kSurfaceLevel));
  for (int i=0; i< descrList.count(); i++) {
    if (!data->hasParam(descrList.at(i), curType)) return false;
  }

  return true;
}

bool TConsistency::limitRulesMatch(const control::ConsistencyCheck& rule, TMeteoData* data)
{
  QMapIterator<QString, control::ConsistencyRange> it(rule);
  bool ok = true;

  while (it.hasNext()) {
    it.next();
    //debug_log<<"check match"<<it.key();

    ok = checkCodeMatch(it.key(), it.value(), data);

    if (!ok) return false;
  }

  return ok;
}

bool TConsistency::checkCodeMatch(const QString& descr, const control::ConsistencyRange& range, TMeteoData* parent)
{
  QString code, subCode;

  QList<TMeteoData*> data = parent->findChilds(descr);
  if (data.size() == 0) {
    return false;
  }

  // var(range.subDescr);
  // var(range.subIndex);

  QList<TMeteoData*> subdata;
  if (range.subDescr != BAD_DESCRIPTOR_NUM) {
    if (!parent->hasParam(range.subDescr)) {
      //debug_log<< "Нет дескриптора в данных"<<range.subDescr;
    return false;
    }
    subdata = parent->findChilds(range.subDescr);    
  }

  bool ok = true;
  //var(subdata.size());

  for (int idx = 0; idx < data.size(); idx++) {
    TMeteoParam* param = data.at(idx)->meteoParamPtr(descr, false);
    if (0 == param || param->isInvalid()) {
      if (range.type == control::VAL_VALID) {
	return true;
      } else {
	return false;
      }
    }

    if (range.type == control::CODEFORM_CHECK) {
      if (subdata.size() == 0) {
	ok = checkCodeMatch(range, param->code(), "");
      } else {
	for (int sidx = 0; sidx < subdata.size(); sidx++) {
	  ok = checkCodeMatch(range, param->code(), subdata.at(sidx)->getParam(range.subDescr, false).code());
	  if (!ok) break;
	}
      }
    } else if (subdata.size() == 0) {
      ok = checkLimits(range, param->value());
    } else {
      for (int sidx = 0; sidx < subdata.size(); sidx++) {
	float subVal = subdata.at(sidx)->getParam(range.subDescr, false).value();
	if (subVal == BAD_METEO_ELEMENT_VAL) return false;
	ok = checkLimits(range, param->value() - subVal);
	if (!ok) break;
      }
    }
    if (!ok) return false;
  }

  return ok;
}

// bool TConsistency::checkCodeMatch(descr_t descr, const control::ConsistencyRange& range, TMeteoData* data)
// {
//   QString code, subCode;

//   int count = data->countParam(descr);
//   if (count == 0 || (range.index != -1 && range.index >= count)) {
//     //    debug_log<< "Нет дескриптора в данных"<<descr;
//     return false;
//   }

//   // var(range.subDescr);
//   // var(range.subIndex);
//   if ((range.subDescr != BAD_DESCRIPTOR_NUM && !data->hasParam(range.subDescr))  ||
//       (range.subIndex !=0 && (uint)range.subIndex >= data->countParam(range.subDescr)) ) {
//     //debug_log<< "Нет дескриптора в данных"<<range.subDescr;
//     return false;
//   }

//   int startIndex = 0, endIndex = count;
//   if (range.index != -1) {
//     startIndex = range.index;
//     endIndex = range.index;
//   }
  

//   const TMeteoParam& subParam =  data->getParam(range.subDescr, range.subIndex);

//   bool ok = true;

//   for (int idx= startIndex; idx < endIndex; idx++) {
//     const TMeteoParam& param = data->getParam(descr, idx);
//     if (range.type == control::CODEFORM_CHECK) {
//       ok = checkCodeMatch(range, param.code(), subParam.code());
//     } else {
//       float subVal = 0;
//       if (range.subDescr != BAD_DESCRIPTOR_NUM) {
// 	subVal = subParam.value();
//       }
//       ok = checkLimits(range, param.value() - subVal);
//     }
//     if (!ok) return false;
//   }
  
//   return ok;
// }

bool TConsistency::checkCodeMatch(const control::ConsistencyRange& range, const QString& code, const QString& subCode)
{
  if (!range.str.isEmpty()) {
    if (range.str.contains(code)) {
      //debug_log<<"Совпадение. str="<<code;
      return true;
    }
  }
  
  bool valOk;
  float fval = code.toFloat(&valOk);
  if (!valOk) return false;

  float subVal = 0;
  if (!subCode.isEmpty()) {
    subVal = subCode.toFloat(&valOk);
    if (!valOk) return false;
  }  

  return checkLimits(range, fval-subVal);
}
  
bool TConsistency::checkLimits(const control::ConsistencyRange& range, float value)
{
  //debug_log<<"val="<<value;
  if (!range.val.isEmpty()) {
    bool cok = false;
    for (int idx = 0; idx < range.val.size(); idx++) {
      if (qFuzzyCompare(range.val.at(idx), value)) {
	cok = true;
	break;
      }
    }
    if (cok) {
      if (!range.negation) {
	//debug_log<<"exist num"<<value;
	return true;
      }
    } else if (range.negation) {
      //debug_log<<"exist negation num"<<value;
      return true;
    }
  }

  if (!range.segment.isEmpty()) {
    for (int i=0; i< range.segment.count(); i++) {
      //debug_log<<"right"<<value<<range.segment.at(i).first<<range.segment.at(i).second;
      if (value >= range.segment.at(i).first && value <= range.segment.at(i).second) {
	if (!range.negation) {
	  return true;
	} else {
	  return false;
	}
      }
    }
  }

  if (range.min.first) {
    if (value > range.min.second) {
      //debug_log<<"min value"<<value<<"min.second"<<range.min.second;
      return true;
    }
  }

  if (range.max.first) {
    if (value < range.max.second) {
      //debug_log<<"max";
      return true;
    }
  }

  //debug_log<<"no match";
  return false;
}


void TConsistency::setQuality(const QList<descr_t>& descrList, control::QualityControl qual, TMeteoData* data)
{
  for (int d = 0; d < descrList.count(); d++) {
    QList<TMeteoData*> childs = data->findChilds(descrList.at(d));
    for (int ch = 0; ch < childs.size(); ch++) {
      if (childs[ch]->hasParam(descrList.at(d), false)) {
	QMap<int, TMeteoParam>& vals = childs[ch]->operator[](descrList.at(d));
	QMap<int,TMeteoParam>::iterator itdata = vals.begin();
	while (itdata != vals.end()) {
#ifdef PARSE_DEBUG_LOG
	  if (qual != control::RIGHT) {
	    debug_log << TMeteoDescriptor::instance()->name(descrList.at(d)) << "(" << descrList.at(d) << ")"
		      << itdata.value().value() << itdata.value().code() ;
	  }
#endif
	  //debug_log << "qual" << itdata.value().quality() << qual;
	  itdata.value().setQuality(qual, control::CONSISTENCY_CTRL, false);
	  //debug_log << "qual" << itdata.value().quality() << qual;
	  
	  ++itdata;
	}
      }
    }
  }
}

  void TConsistency::controlPress(TMeteoData* data)
  {
    TMeteoParam* P = data->paramPtr(TMeteoDescriptor::instance()->descriptor("P"));
    TMeteoParam* Pstn = data->paramPtr(TMeteoDescriptor::instance()->descriptor("P0"));
    float Zred = 0;  //для среднего уровня моря
    const TMeteoParam& Zstn = data->getParam(TMeteoDescriptor::instance()->descriptor("h0_station"));
    TMeteoParam* Tstn = data->paramPtr(TMeteoDescriptor::instance()->descriptor("T"));

    if (!P || !Pstn || !Tstn || Tstn->quality() == control::MISTAKEN) return;
    if (P->value() == BAD_METEO_ELEMENT_VAL || Tstn->value() == BAD_METEO_ELEMENT_VAL ||
	Pstn->value() == BAD_METEO_ELEMENT_VAL || Zstn.value() == BAD_METEO_ELEMENT_VAL) {
      return;
    }

    float Tm = Tstn->value() + control::gamma*(Zstn.value() - Zred)/2.0;
    float alpha = control::g * (Zstn.value() - Zred)/control::R/Tm;
    float Pred = Pstn->value() * exp(alpha);

    // var(Tm);
    // var(alpha);
    // var(Pred);
    // debug_log << Pstn->value() << exp(alpha);

    float lim = 0;
    if (fabs(Zred - Zstn.value()) <= 100) {
      lim = 0.4;
    } else {
      lim = 0.004*fabs(Zred - Zstn.value());
    }

    float diff = fabs(P->value() - Pred);
    if (diff > lim) {
#ifdef PARSE_DEBUG_LOG
      debug_log << "set quality" << control::DOUBTFUL << "rule= controlPress\n" 
		<< "Pred=" << Pred
		<< "P=" << P->value() << "\nPstn=" << Pstn->value() 
		<< "\nZstn=" << Zstn.value() << "\nTstn=" << Tstn->value();
#endif
      P->setQuality(control::DOUBTFUL, control::CONSISTENCY_CTRL, false);
      Pstn->setQuality(control::DOUBTFUL, control::CONSISTENCY_CTRL, false);
      Tstn->setQuality(control::DOUBTFUL, control::CONSISTENCY_CTRL, false);
    } else {
      P->setQuality(control::RIGHT, control::CONSISTENCY_CTRL, false);
      Pstn->setQuality(control::RIGHT, control::CONSISTENCY_CTRL, false);
      Tstn->setQuality(control::RIGHT, control::CONSISTENCY_CTRL, false);
    }
  }

}
