#include <qdom.h>
#include <qstringlist.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>

#include "tcontrolmethods.h"
#include "tmeteocontrol.h"

namespace control {
  struct LimitRange {
    QStringList str;  //!< допустимые строковые значения
    QList<float> val; //!< допустимые числовые значение
    QList<QPair<float, float> > segment; //!< допустимые диапазоны [min, max]
    QList<QPair<float, float> > doubtsegment; //!< диапазоны сомнительных величин [min, max]
  };
}

namespace tmcontrol {

template <class S, class G> class TControlTemplateBase : public TControlMethodBase {
public:
  TControlTemplateBase(const QString& dirName):TControlMethodBase(dirName) {
  }
  virtual ~TControlTemplateBase() { for (int i=0; i<_settings.count(); i++) { delete _settings.at(i);} _settings.clear(); }

  bool control(TMeteoData* data);
  virtual bool makeControl(const QList<G>& rules, TMeteoData* data) =0;

  bool readSettings(QDomDocument* xml);
  void readDomDocument(QDomDocument* xml, S* one);
  virtual void readGroupSettings(G& group, QDomNode& node) =0;

private:
  QList<S*> _settings;

};

  //! Для использующих LimitRange
  template <class S, class G, class R> class TControlTemplate : public TControlTemplateBase<S,G> {
public:
    TControlTemplate(const QString& dirName):TControlTemplateBase<S,G>(dirName) {
    }
    virtual ~TControlTemplate() {}

    void readGroupSettings(G& group, QDomNode& node);
    virtual bool readValueSettings(R& check, const QDomElement& el) =0;
    bool readLimitRanges(QMap<descr_t, control::LimitRange> & values, const QDomElement& el, const QString& keyAttr="name");

    bool makeControl(const QList<G>& rules, TMeteoData* data);
    bool limitRulesMatch(const R& rule, TMeteoData* data);
    virtual void limitRulesCheck(const R& rule, TMeteoData* data) =0;

    control::QualityControl checkLimit(float value, const control::LimitRange& range);
    void setQuality(TMeteoParam& param, const control::LimitRange& range, control::CheckValueType type) ;
  };

  //! отдельно каждый узел (родитель, его дочерние)
  template <class S, class G> bool TControlTemplateBase<S,G>::control(TMeteoData* data)
  {
    if (! isReadSettings()) {
      TControlMethodBase::readSettings();
    }

    for (int i=0; i<_settings.count(); i++) { //несколько файлов
      if (rulesMatch(_settings.at(i)->type, data)) {
	makeControl(_settings.at(i)->rules, data);
      }
    }


    for (int idx = 0; idx < data->childsCount(); idx++) {
      control(data->child(idx));
    }

    return true;
  }

  template <class S, class G> bool TControlTemplateBase<S,G>::readSettings(QDomDocument* xml)
  {
    if (!xml) return false;

    S* one = new S;
    _settings.append(one);
    readDomDocument(xml, one);

    //   for (int gr=1; gr<one->rules.count(); gr++) {
    //   for (int i=0; i<one->rules.at(gr).count(); i++) {
    //     debug_log<<"type"<<one->rules.at(gr).at(i).type;
    //     QMapIterator<uint, control::LimitRange*> it(one->rules.at(gr).at(i).match);
    //     while (it.hasNext()) {
    // 	it.next();
    // 	debug_log<<"descr="<<it.key();
    // 	debug_log<<"str="<<it.value()->str;
    // 	qDebug()<<"val"<<it.value()->val;
    // 	qDebug()<<"segm"<<it.value()->segment;
    // 	qDebug()<<"doubt"<<it.value()->doubtsegment;
    //     }
    //   }
    // }


    return true;
  }

  template<class S, class G> void TControlTemplateBase<S,G>::readDomDocument(QDomDocument* xml, S* one)
  {
    QDomNodeList nodeList = xml->elementsByTagName("category");
    QString str = nodeList.at(0).toElement().attribute("num");
    descr_t descr = TMeteoDescriptor::instance()->descriptor(nodeList.at(0).toElement().attribute("name"));
    one->type = QPair<descr_t, QStringList>(descr ,str.split(','));

    nodeList = xml->elementsByTagName("group");
    for (int i =0; i< nodeList.count(); i++) {
      QDomNode node = nodeList.at(i).firstChild();
      G group;

      while (!node.isNull()) {
	readGroupSettings(group, node);
	node = node.nextSibling();
      }

      one->rules.append(group);
    }
  }

  //---------

  //отдельно каждый узел
  template <class S, class G, class R> bool TControlTemplate<S,G,R>::makeControl(const QList<G>& rules, TMeteoData* data)
  {
    if (0 == data) return false;
    
    for (int group=0; group < rules.count(); group++) {
      for (int i=0; i< rules.at(group).count(); i++) {
	if (limitRulesMatch(rules.at(group).at(i), data)) {
	  //debug_log<<"group"<<group<<"rule"<<i<<"match";
	  limitRulesCheck(rules.at(group).at(i), data);
	  break;
	} else {
	  //debug_log<<"group"<<group<<"rule"<<i<<"mismatch";
	}
      }
    }

    return true;
  }

  template<class S, class G, class R>  bool TControlTemplate<S,G,R>::limitRulesMatch(const R& rule, TMeteoData* data)
  {
    if (0 == data) return false;
    if (rule.match.size() == 0) return true;
    
  QMapIterator<descr_t, control::LimitRange> it(rule.match);
  bool ok = false;
 
  while (it.hasNext()) {
    it.next();
    //debug_log<<"check match"<<it.key();

    const TMeteoParam& param = data->getParamUp(it.key());
    if (param.quality() == control::NO_OBSERVE) {
      //debug_log<< "Нет дескриптора в данных"<<it.key();
      return false;
    }

    QString code = param.code();
    const control::LimitRange& range = it.value();
    if (!range.str.isEmpty()) {
      if (range.str.contains(code)) {
	//	debug_log<<"Совпадение. str="<<code;
	ok = true;
      }
    }

    float fval;
    if (rule.type == control::CODEFORM_CHECK) {
      bool valOk = false;
      fval = code.toFloat(&valOk);
      if (!valOk) return false;
    } else {
      fval = param.value();
    }

    control::QualityControl cc = checkLimit(fval, range);
    if (cc == control::RIGHT || cc == control::DOUBTFUL) {
      //      debug_log<<"Попадание в segment="<<fval;
      ok = true;
    } else {
      return false;
    }

  }

  return ok;
}

  template <class S, class G, class R> control::QualityControl TControlTemplate<S,G,R>::checkLimit(float value, const control::LimitRange& range)
{
  for (int idx = 0; idx < range.val.size(); idx++) {
    if (qFuzzyCompare(range.val.at(idx), value)) {
      return control::RIGHT;
    }
  }

  //  if (range.val.contains(value)) return control::RIGHT;

  if (!range.segment.isEmpty()) {
    for (int i=0; i< range.segment.count(); i++) {
      //debug_log<<"check right"<<value<<range.segment.at(i).first<<range.segment.at(i).second;
      if (value >= range.segment.at(i).first && value <= range.segment.at(i).second) {
	//debug_log<<"right";
	return control::RIGHT;
      }
    }
  }

  if (!range.doubtsegment.isEmpty()) {
    for (int i=0; i< range.doubtsegment.count(); i++) {
      //debug_log<<"check doubt"<<value<<range.doubtsegment.at(i).first<<range.doubtsegment.at(i).second;
      if (value >= range.doubtsegment.at(i).first && value <= range.doubtsegment.at(i).second) {
	//debug_log<<"doubt";
	return control::DOUBTFUL;
      }
    }
  } 
  //debug_log<<"mistaken"<<value;

  return control::MISTAKEN;
}

  template <class S, class G, class R> void TControlTemplate<S,G,R>::setQuality(TMeteoParam& param, const control::LimitRange& range, control::CheckValueType type)
{
  control::QualityControl cc =control::MISTAKEN;

  if (!range.str.isEmpty() && range.str.contains(param.code())) {
      cc = control::RIGHT;
  } else {
    float fval;
    bool ok = true;
    if (type == control::CODEFORM_CHECK) {
      fval = param.code().toFloat(&ok);
    } else {
      fval = param.value();
    }
    if (!ok) {
      cc = control::MISTAKEN;
    } else {
      cc = checkLimit(fval, range);
    }
  }

  param.setQuality(cc, control::LIMIT_CTRL, false);
  //debug_log << "set quality" << cc;
}

  //чтение элемента внтури группы
  template<class S, class G, class R> void TControlTemplate<S,G,R>::readGroupSettings(G& group, QDomNode& node)
  {
    R ch;
    QDomElement el = node.toElement();

    if (el.tagName() == "codeform") {
      ch.type = control::CODEFORM_CHECK;
    } else if (el.tagName() == "phys") {
      ch.type = control::PHYSVAL_CHECK;
    } else {
      error_log << QObject::tr("Не указан тип проверки - кодовая форма или значение");
      ch.type = control::CODEFORM_CHECK;
    }
    ch.number = el.attribute("rule").toInt();
    if (readValueSettings(ch, el)) {
      group.append(ch);
      // qDebug()<<ch.type;
      // qDebug()<<ch.match;
      // qDebug()<<ch.values;
    }
  }

  template<class S, class G, class R> bool TControlTemplate<S,G,R>::readLimitRanges(QMap<descr_t, control::LimitRange> & values, const QDomElement& el, const QString& keyAttr/*="name"*/)
{
  descr_t key;

  if (keyAttr == "name") {
    key = TMeteoDescriptor::instance()->descriptor(el.attribute(keyAttr));

    if (key == BAD_DESCRIPTOR_NUM) {
      error_log<< QObject::tr("Ошибка xml-файла. Нет дескриптора %1").arg(el.attribute(keyAttr));
      return false;
    }
  } else {
    bool ok;
    key = el.attribute(keyAttr).toUInt(&ok);
    if (!ok) {
      return false;
    }
  }

  control::LimitRange range;
  if (!el.attribute("str").isEmpty()) {
    range.str = el.attribute("str").split(",");
  }

  QStringList vals = el.attribute("num").split(",");
  TControlMethodBase::parseNum(vals, range.val);
  // for (int i=0; i<vals.count(); i++) {
  //   float v = vals.at(i).toFloat(&ok);
  //   if (ok) {
  //     range->val.append(v);
  //   }
  // }

  vals = el.attribute("segment").split(";");
  TControlMethodBase::parseSegments(vals, range.segment);

  vals = el.attribute("doubtsegment").split(";");
  TControlMethodBase::parseSegments(vals, range.doubtsegment);

  values.insert(key, range);

  return true;
}


}
