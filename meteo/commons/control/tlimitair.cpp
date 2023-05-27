#include "tlimitair.h"
#include "tmeteocontrol.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/meteo_data/meteo_data.h>

//template<> tmcontrol::TLimitTable* TLimitTable::_instance = 0;

namespace {
  bool registerMethod() 
  {
    tmcontrol::TLimitTable* m = new tmcontrol::TLimitTable(CONTROL_SETTINGS_DIR + "limit_air.xml");
    return TMeteoControl::instance()->registerMethod(control::LIMIT_CTRL, m);
  }
  static bool regresult = registerMethod();
}

namespace tmcontrol {

  TLimitTable::TLimitTable(const QString& dirName):TControlTemplate<control::LimitTableSettings, control::LimitTableGroup, control::LimitTableCheck>(dirName)
  {
  }

  TLimitTable::~TLimitTable()
  {
  }


bool TLimitTable::readValueSettings(control::LimitTableCheck& check, const QDomElement& root)
{
  QDomNode node = root.firstChild();

  while (!node.isNull()) {
    QDomElement el = node.toElement();
    if (el.tagName() == "match") {
      readLimitRanges(check.match, el);      
    } else if (el.tagName() == "table") {
      check.descr = TMeteoDescriptor::instance()->descriptor(el.attribute("name"));
      check.descrMatch = TMeteoDescriptor::instance()->descriptor(el.attribute("namematch"));
      QDomNodeList nList = el.elementsByTagName("val");
      for (int i =0; i< nList.count(); i++) {
	readLimitRanges(check.values, nList.at(i).toElement(), "match");
      }
    }
    node = node.nextSibling();
  }
  return true;
}

//! установка показателя качества, в соответствии с правилом
void TLimitTable::limitRulesCheck(const control::LimitTableCheck& rule, TMeteoData* data)
{
  int pmatch;
  control::QualityControl qual;
  bool ok = data->getValue(rule.descrMatch, &pmatch, &qual);
  TMeteoParam* val = data->paramPtr(rule.descr, false);

  if (0 == val || !ok || qual >= control::MISTAKEN || val->value() == BAD_METEO_ELEMENT_VAL) {
    return;
  }

  //  debug_log << rule.descrMatch << pmatch << qual << rule.descr << val->value() << rule.values.contains(pmatch);
  if (rule.values.contains(pmatch)) {
    setQuality(*val, rule.values.value(pmatch), rule.type);
#ifdef PARSE_DEBUG_LOG
    if (val->quality() != control::RIGHT) {
      debug_log << "set quality" << val->quality() 
		<< "rule=" << rule.number <<  TMeteoDescriptor::instance()->name(rule.descr)
		<< "descr=" << rule.descr << val->value();
    }
#endif
  }
}

}
