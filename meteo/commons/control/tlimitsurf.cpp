#include "tlimitsurf.h"
#include "tmeteocontrol.h"

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <qdom.h>
#include <qstringlist.h>


//template<> tmcontrol::TLimitSimple* TLimitSimple::_instance = 0;

namespace {
  bool registerMethod(const QString& filename)
  {
    tmcontrol::TLimitSimple* m = new tmcontrol::TLimitSimple(CONTROL_SETTINGS_DIR + filename);
    return TMeteoControl::instance()->registerMethod(control::LIMIT_CTRL, m);
  }
  
  static bool regresult = registerMethod("limit_surface.xml");
  static bool regresult1 = registerMethod("limit_depth.xml");
}


namespace tmcontrol {

  TLimitSimple::TLimitSimple(const QString& dirName):TControlTemplate<control::LimitSettings, control::LimitCheckGroup, control::LimitCheck>(dirName)
  {
  }

  TLimitSimple::~TLimitSimple()
  {

  }

  bool TLimitSimple::readValueSettings(control::LimitCheck& check, const QDomElement& el)
  {
    QDomNodeList nodeList = el.elementsByTagName("match");
    for (int i =0; i< nodeList.count(); i++) {
      readLimitRanges(check.match, nodeList.at(i).toElement());
    }

    nodeList = el.elementsByTagName("val");
    for (int i =0; i< nodeList.count(); i++) {
      readLimitRanges(check.values, nodeList.at(i).toElement());
    }

    return true;
  }

void TLimitSimple::limitRulesCheck(const control::LimitCheck& rule, TMeteoData* data)
{
  QMapIterator<descr_t, control::LimitRange> it(rule.values);
  while (it.hasNext()) {
    it.next();

    if (data->hasParam(it.key(), false)) {
      QMap<int, TMeteoParam>& oneData = data->operator[](it.key());
      QMap<int, TMeteoParam>::iterator itdata  = oneData.begin();
      while (itdata != oneData.end()) {
	if (itdata.value().value() != BAD_METEO_ELEMENT_VAL) {
	  //debug_log<<"check descr"<<it.key();
	  setQuality(itdata.value(), it.value(), rule.type);
#ifdef PARSE_DEBUG_LOG
	  if (itdata.value().quality() != control::RIGHT) {
	    debug_log << "set quality" << itdata.value().quality() 
		      << "rule=" << rule.number <<  TMeteoDescriptor::instance()->name(it.key())
		      << "descr=" << it.key() << itdata.value().value();
	  }
#endif
	}
	++itdata;
      }
    }
  }
}

}
