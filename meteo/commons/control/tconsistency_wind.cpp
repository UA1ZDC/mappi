#include "tconsistency_wind.h"

#include "tmeteocontrol.h"
#include <commons/meteo_data/meteo_data.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/mathtools/mnmath.h>

#include <qdebug.h>

//template<> tmcontrol::TConsistWind* TConsistWind::_instance = 0;

namespace {
  bool registerMethod() 
  {
    tmcontrol::TConsistWind* m = new tmcontrol::TConsistWind(CONTROL_SETTINGS_DIR + "consistency_wind.xml");
    return TMeteoControl::instance()->registerMethod(control::CONSISTENCY_CTRL, m);
  }
  static bool regresult = registerMethod();
}

namespace tmcontrol {

  TConsistWind::TConsistWind(const QString& dirName): 
    TControlTemplateBase<control::ConsistWindSettings, control::ConsistWindGroup>(dirName)
  {
  }

  TConsistWind::~TConsistWind()
  {
  }
  
  bool TConsistWind::makeControl(const QList<control::ConsistWindGroup>& rules, TMeteoData* data)
  {
    for (int r = 0; r< rules.count(); r++) {
      controlWind(rules.at(r), data);
    }
    return true;
  }


  bool TConsistWind::controlWind(const control::ConsistWindGroup& rule, TMeteoData* parent)
  {
    bool ok = false;
    QList<TMeteoData*> dataList = parent->findChilds(rule.descr, meteodescr::kIsobarLevel);
    QMap<float, TMeteoData*> data;
    QList<float> levels;

    QList<TMeteoData*>::iterator it = dataList.begin();
    while (it != dataList.end()) {
      if ((*it)->getValue(rule.descr, BAD_METEO_ELEMENT_VAL, false) == BAD_METEO_ELEMENT_VAL || 
	  (*it)->getValue(rule.descrMatch, BAD_METEO_ELEMENT_VAL, false) == BAD_METEO_ELEMENT_VAL) {
      } else {
	levels << (*it)->getValue(rule.descrLevel, BAD_METEO_ELEMENT_VAL, false);
	data.insert((*it)->getValue(rule.descrLevel, BAD_METEO_ELEMENT_VAL, false), *it);
      }
      ++it;
    }
    if (data.size() == 0) {
      return true;
    }

    qSort(levels.begin(), levels.end(), qGreater<float>());

    for (int ii = 0; ii < levels.size() - 1; ii++) {
      TMeteoParam* ff1 = data.value(levels.at(ii))->paramPtr(rule.descr);
      TMeteoParam* ff2 = data.value(levels.at(ii+1))->paramPtr(rule.descr);
      TMeteoParam* dd1 = data.value(levels.at(ii))->paramPtr(rule.descrMatch);
      TMeteoParam* dd2 = data.value(levels.at(ii+1))->paramPtr(rule.descrMatch);

      float limSpeed = 20.6 + 0.275 * (ff1->value() + ff2->value());
      if (fabs(ff1->value() - ff2->value()) > limSpeed) {
	ff1->setQuality(control::DOUBTFUL, control::CONSISTENCY_CTRL, false);
	ff2->setQuality(control::DOUBTFUL, control::CONSISTENCY_CTRL, false);
      }

      float direct = fabs(MnMath::M180To180(dd1->value() - dd2->value()));
      for (int r=0; r < rule.range.count(); r++) {
	if (segmentMatch(rule.range.at(r).matchSegment, direct)) {
	  float lim = getRangeLimit(rule.range.at(r).expr, levels.at(ii));
	  if (lim == -1) {
	    warning_log<<QObject::tr("Значение уровня %1 гПа не соответствует ни одному интервалу "
					 "из файла с правилами").arg(levels.at(ii));
	    continue;
	  }

	  //debug_log<<"speed+direct rule" << lim;
	  if (fabs(ff1->value() + ff2->value()) > lim) {
	    ff1->setQuality(control::DOUBTFUL, control::CONSISTENCY_CTRL, false);
	    ff2->setQuality(control::DOUBTFUL, control::CONSISTENCY_CTRL, false);
	    dd1->setQuality(control::DOUBTFUL, control::CONSISTENCY_CTRL, false);
	    dd2->setQuality(control::DOUBTFUL, control::CONSISTENCY_CTRL, false);
	  } else {
	    ff1->setQuality(control::RIGHT, control::CONSISTENCY_CTRL, false);
	    ff2->setQuality(control::RIGHT, control::CONSISTENCY_CTRL, false);
	    dd1->setQuality(control::RIGHT, control::CONSISTENCY_CTRL, false);
	    dd2->setQuality(control::RIGHT, control::CONSISTENCY_CTRL, false);
	  }
	}
      }

    }
    

    // QHash<int, TMeteoParam>& windDirect = (*data)[rule.descrMatch];
    // QHash<int, TMeteoParam>& windSpeed  = (*data)[rule.descr];
    // const QHash<int, TMeteoParam>& press = data->getParamList(rule.descrLevel);
    
    // QList<int> keys = windSpeed.keys();
    //  qSort(keys);

    // for (int ii = 0; ii < keys.count()-1; ii++) {
    //   //var(press.value(keys.at(ii)).value());
   
    //   float limSpeed = 20.6 + 0.275 * (windSpeed.value(keys.at(ii)).value() + windSpeed.value(keys.at(ii+1)).value());
      
    //   //debug_log << "limspeed" << limSpeed << fabs(windSpeed.value(keys.at(ii)).value() - windSpeed.value(keys.at(ii+1)).value());
    //   if (fabs(windSpeed.value(keys.at(ii)).value() - windSpeed.value(keys.at(ii+1)).value()) > limSpeed) {
    // 	windSpeed[keys.at(ii)].setQuality(control::DOUBTFUL);
    // 	windSpeed[keys.at(ii+1)].setQuality(control::DOUBTFUL);
    // 	// debug_log << "speed rule";
    // 	// debug_log << "level=" << press.value(keys.at(ii)).value() << "wind qual=" << windSpeed.value(keys.at(ii)).quality();
    //   }

    //   for (int r=0; r < rule.range.count(); r++) {
    // 	float direct = fabs(MnMath::M180To180(windDirect.value(keys.at(ii)).value() - windDirect.value(keys.at(ii+1)).value()));
    // 	//var(direct);
    // 	//qDebug() << rule.range.at(r).matchSegment;
    // 	if (segmentMatch(rule.range.at(r).matchSegment, direct)) {
    // 	  float lim = getRangeLimit(rule.range.at(r).expr, press.value(keys.at(ii)).value());
    // 	  if (lim == -1) {
    // 	    warning_log<<QObject::tr("Значение уровня %1 гПа не соответствует ни одному интервалу "
    // 					 "из файла с правилами").arg(press.value(keys.at(ii)).value());
    // 	    continue;
    // 	  }

    // 	  //debug_log<<"speed+direct rule" << lim;
    // 	  if (fabs(windSpeed.value(keys.at(ii)).value() + windSpeed.value(keys.at(ii+1)).value()) > lim) {
    // 	    windSpeed[keys.at(ii)].setQuality(control::DOUBTFUL);
    // 	    windSpeed[keys.at(ii+1)].setQuality(control::DOUBTFUL);
    // 	    windDirect[keys.at(ii)].setQuality(control::DOUBTFUL);
    // 	    windDirect[keys.at(ii+1)].setQuality(control::DOUBTFUL);
    // 	  } else {
    // 	    windSpeed[keys.at(ii)].setQuality(control::RIGHT);
    // 	    windSpeed[keys.at(ii+1)].setQuality(control::RIGHT);
    // 	    windDirect[keys.at(ii)].setQuality(control::RIGHT);
    // 	    windDirect[keys.at(ii+1)].setQuality(control::RIGHT);
    // 	  }

    // 	}
    //   }

    //   //debug_log<<"level="<<press.value(keys.at(ii)).value()<<"wind qual="<<windSpeed[keys.at(ii)].quality();

    // }
    
  
    return ok;
  }
  
  float TConsistWind::getRangeLimit(const QList<control::ConsistWindRange>& range, float level)
  {
    for (int i=0; i< range.count(); i++) {
      if (segmentMatch(range.at(i).levelMatch, level)) {
	return range.at(i).max;
      }
    }
    return -1;
  }

  void TConsistWind::readGroupSettings(control::ConsistWindGroup& group, QDomNode& node)
  {
    while (!node.isNull()) {
      QDomElement el = node.toElement();
      if  (el.tagName() == "codeform") {
	group.type = control::CODEFORM_CHECK;
      } else if (el.tagName() == "phys") {
	group.type = control::PHYSVAL_CHECK;
      }
     
      if (readValueSettings(group, el.firstChild().toElement())) {
      }
      node = node.nextSibling();
    }
  }

  bool TConsistWind::readValueSettings(control::ConsistWindGroup& group, QDomElement parent)
  {
    group.descr = TMeteoDescriptor::instance()->descriptor(parent.attribute("name"));
    group.descrLevel = TMeteoDescriptor::instance()->descriptor(parent.attribute("nameLevel"));
    group.descrMatch = TMeteoDescriptor::instance()->descriptor(parent.attribute("nameMatch"));

    QDomNode node = parent.firstChild();
    
    while (!node.isNull()) {
      QDomElement el = node.toElement();
      control::ConsistWindCheck check;
      if (el.tagName() == "match") {
	QStringList vals = el.attribute("segment").split(";");
	TControlMethodBase::parseSegments(vals, check.matchSegment);
	readRange(check, el.firstChild());
      } 
    
      group.range.append(check);      
      node = node.nextSibling();
    }
    return true;
  }

  void TConsistWind::readRange(control::ConsistWindCheck& check, QDomNode node)
  {
    while (!node.isNull()) {
      QDomElement el = node.toElement();
      if (el.tagName() == "val") {
	control::ConsistWindRange range;
      	range.max = el.attribute("max").toInt();
      	QStringList vals = el.attribute("level").split(";");
      	TControlMethodBase::parseSegments(vals, range.levelMatch);
	check.expr.append(range);
      }
      node = node.nextSibling();
    }
  }

}
