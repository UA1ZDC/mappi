#include "tconsistency_time.h"

#include "tmeteocontrol.h"
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <qdom.h>

#include <math.h>

//template<> tmcontrol::TConsistTime* TConsistTime::_instance = 0;

namespace {
  bool registerMethod() 
  {
    tmcontrol::TConsistTime* m = new tmcontrol::TConsistTime(CONTROL_SETTINGS_DIR + "consistency_time.xml");
    return TMeteoControl::instance()->registerMethod(control::TIME_CTRL, m);
  }
  static bool regresult = registerMethod();
}


namespace tmcontrol {

  TConsistTime::TConsistTime(const QString& dirName):TControlTemplateBase<control::TimeSettings, control::TimeGroup>(dirName)
{
}
 
TConsistTime::~TConsistTime()
{
}
    
bool TConsistTime::control(TMeteoData* cur, TMeteoData* prev)
{
  _prev = prev;
  bool ok = TControlTemplateBase::control(cur);

  return ok;
}

bool TConsistTime::makeControl(const QList<control::TimeGroup>& rules, TMeteoData* data)
{
  for (int group=0; group < rules.count(); group++) {
    for (int i=0; i< rules.at(group).check.count(); i++) {
      if (limitRulesMatch(rules.at(group).check.at(i).match, data, _prev)) {
	//debug_log<<"group"<<group<<"rule"<<i<<"match";
	if (rules.at(group).isCustom) {
	  limitPressueCheck(rules.at(group).check.at(i), data, _prev);
	} else {
	  limitRulesCheck(rules.at(group).check.at(i).expr, data, _prev);
	}
	break;
      } else {
	//	debug_log<<"group"<<group<<"rule"<<i<<"mismatch";
      }
    }
  }
  
  return true;
}

bool TConsistTime::limitRulesMatch(const QMap<QString, control::TimeRange>& match, TMeteoData* cur, TMeteoData* prev)
{
  QMapIterator<QString, control::TimeRange> it(match);
  bool ok = false;

  while (it.hasNext()) {
    it.next();

    if (it.key() == "time") {
      ok = true;
      QDateTime dtCur = TMeteoDescriptor::instance()->dateTime(*cur);
      QDateTime dtPrev = TMeteoDescriptor::instance()->dateTime(*prev);
      //debug_log << dtCur << dtPrev;
      float hours = dtPrev.secsTo(dtCur) / 3600;
      if (! checkLimits(it.value(), hours)) {
	return false;
      }
    } else {
      // descr_t descr = TMeteoDescriptor::instance()->descriptor(it.key());
      // if (descr == BAD_DESCRIPTOR_NUM) return false;
      // float val = cur->getValue(descr, BAD_METEO_ELEMENT_VAL);
      // if (! checkLimits(it.value(), val)) {
      // 	return false;
      // }
    }
  }

  return ok;
}

bool TConsistTime::checkLimits(const control::TimeRange& range, float val)
{
 // if (range.val.contains(val)) return true;

 // if (segmentMatch(range.segment, val)) return true;

  if (range.min.first) {
    if (val > range.min.second) {
      return true;
    }
  }

  if (range.max.first) {
    if (val <= range.max.second) {
      return true;
    }
  }

  return false;
}

void TConsistTime::limitRulesCheck(const QMap<descr_t, control::TimeRange>& rule, TMeteoData* cur, TMeteoData* prev)
{
  QMapIterator<descr_t, control::TimeRange> it(rule);
  bool check = false;

  while (it.hasNext()) {
    it.next();

    if (!cur->hasParam(it.key())) continue;

    QMap<int,TMeteoParam>& vals = cur->operator[](it.key());
    QMap<int,TMeteoParam>::iterator itdata = vals.begin();
    while (itdata != vals.end()) {
      TMeteoParam& param = itdata.value();
      float prevVal = prev->getValue(it.key(), BAD_METEO_ELEMENT_VAL);
      if (prevVal != BAD_METEO_ELEMENT_VAL && param.value() != BAD_METEO_ELEMENT_VAL) {
	float val = fabs(param.value() - prevVal);
	// var(cur.value());
	// var(prevVal);
	// var(val);
	// var(it.value().min.second);
	check = checkLimits(it.value(), val);
	if (check) {
	  param.setQuality(control::DOUBTFUL, control::TIME_CTRL, false);
	} else {
	  param.setQuality(control::RIGHT, control::TIME_CTRL, false);
	}
	//	debug_log << "setQual descr=" << it.key() << " qual=" << param.quality();
      }

      ++itdata;
    }
  }
}


void TConsistTime::limitPressueCheck(const control::TimeCheck& check, TMeteoData* cur, TMeteoData* prev)
{
  meteodescr::TMeteoDescriptor* d = TMeteoDescriptor::instance();  

  QMapIterator<descr_t, control::TimeRange> it(check.expr);
  bool checkOk = false;

  while (it.hasNext()) {
    it.next();

    TMeteoParam* P = cur->paramPtr(it.key());
    float Pprev = prev->getValue(it.key(), BAD_METEO_ELEMENT_VAL);
    TMeteoParam* p = cur->paramPtr(d->descriptor("p"));
    float pprev = prev->getValue(d->descriptor("p"), BAD_METEO_ELEMENT_VAL);
    if (!p || !P) return;

    if (Pprev != BAD_METEO_ELEMENT_VAL && P->value() != BAD_METEO_ELEMENT_VAL && 
	pprev != BAD_METEO_ELEMENT_VAL && p->value() != BAD_METEO_ELEMENT_VAL) {
      float val;
      if (check.match["time"].max.second == 3) {
	val = fabs(P->value() - Pprev - p->value());
      } else {
	val = fabs(P->value() - Pprev - 0.5*pprev - 1.5*p->value());
      }
      // var(check.match["time"].max.second);
      // var(p.value());
      // var(P.value());
      // var(Pprev);
      // var(val);
      // var(it.value().min.second);
      checkOk = checkLimits(it.value(), val);
      if (checkOk) {
	P->setQuality(control::DOUBTFUL, control::TIME_CTRL, false);
	p->setQuality(control::DOUBTFUL, control::TIME_CTRL, false);
      } else {
	P->setQuality(control::RIGHT, control::TIME_CTRL, false);
	p->setQuality(control::RIGHT, control::TIME_CTRL, false);
      }
      //      debug_log << "setQual descr=" << it.key() << " qual=" << P->quality();
    }
  }
}

//--- settings

  //! чтение элемента группы
void TConsistTime::readGroupSettings(control::TimeGroup& group, QDomNode& node)
{
  QDomElement el = node.toElement();
  if (el.tagName() == "phys" || el.tagName() == "tend") {
    control::TimeCheck check;
    if (readValueSettings(check, el.toElement())) {
      group.isCustom = (el.tagName() != "phys");
      group.check.append(check);
    }
  }
}  

  //! чтение элемента группы с физическими величинами 
bool TConsistTime::readValueSettings(control::TimeCheck& check, QDomElement el)
{
  QString name;

  control::TimeRange range;
  QDomNodeList nodeList = el.elementsByTagName("match");
  readRanges(range, name, nodeList.at(0).toElement());
  check.match.insert(name, range);
    
  
  nodeList = el.elementsByTagName("val");
  for (int i =0; i< nodeList.count(); i++) {
    control::TimeRange range;
    readRanges(range, name, nodeList.at(i).toElement());
    descr_t descr = TMeteoDescriptor::instance()->descriptor(name);
    if (descr == BAD_DESCRIPTOR_NUM) {
      error_log << QObject::tr("Не найден дескриптор %1").arg(name);
    } else {
      check.expr.insert(descr, range);
    }
  } 

  return true;
}

  //! чтение правил
void TConsistTime::readRanges(control::TimeRange& range, QString& name, QDomElement el)
{
  name = el.attribute("name");
  if (name.isEmpty()) {
    error_log << QObject::tr("Ошибка xml файла (согласованность по времени). Нет имени дескриптора");
    return;
  }

  // if (el.hasAttribute("sub")) {
  //   range.subDescr =  TMeteoDescriptor::instance()->descriptor(el.attribute("sub"));
  // }

  // if (el.hasAttribute("add")) {
  //   range.addDescr =  TMeteoDescriptor::instance()->descriptor(el.attribute("add"));
  // }
  
  // QStringList vals;
  // vals = el.attribute("num").split(",");
  // TControlMethodBase::parseNum(vals, range.val);

  // vals = el.attribute("segment").split(";");
  // TControlMethodBase::parseSegments(vals, range.segment);

  if (el.hasAttribute("min")) {
    range.min.second = el.attribute("min").toFloat(&range.min.first);
  }
  if (el.hasAttribute("max")) {
    range.max.second = el.attribute("max").toFloat(&range.max.first);
  }
}

}
