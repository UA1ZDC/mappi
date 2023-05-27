#include "sxfexchange.h"
#include "passportbasis.h"
#include "sxfstructures.h"
#include "metasxf.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/ui/map/geomrl.h>
#include <meteo/commons/ui/map/weatherfigure.h>
#include <meteo/commons/punchrules/punchrules.h>
#include <meteo/commons/proto/sxfcodes.pb.h>

#include <QByteArray>
#include <QColor>
#include <QList>
#include <QPen>
#include <QStringList>
#include <QPixmap>

namespace meteo {

  Sxf::Sxf() :
    sxfdoc_(0),
    isSaveDegree_(false),
    onlyVis_(false),
    isTtfSupported_(true)
  {

    init();  
  }

  Sxf::~Sxf()
  {
    if (0 != sxfdoc_) {
      delete sxfdoc_;
      sxfdoc_ = 0;
    }
  }

  bool Sxf::init()
  {
    QPixmap p(1000,1000);
    pixScale_ = p.widthMM();
    if (pixScale_ <= 0) {
      pixScale_ = 200;
    }

    return true;
  }

  const QString& Sxf::lastError() const
  {
    return lastError_;
  }

  void Sxf::setPassportBasis(const QSharedPointer<sxf::PassportBasis>& basis)
  {
    basis_ = basis;
  }


  void Sxf::prepareSxfDocument()
  {
    if (sxfdoc_ != 0) {
      delete sxfdoc_;
      sxfdoc_ = 0;
    }
    sxfdoc_ = new sxf::Document();
    sxfdoc_->setCoordType(isSaveDegree_);
  }



  const QByteArray Sxf::exportDocument(const map::Document& doc, bool onlyVis /* = false*/)
  {
    onlyVis_ = onlyVis;
    QStringList layerUuids;
    foreach (map::Layer* layer, doc.layers()) {
      if (layer != 0) {
	layerUuids.append(layer->uuid());
      }
    }
    return exportDocument(layerUuids, doc, onlyVis);
  }

  const QByteArray Sxf::exportDocument(const QStringList& layerUuids, const map::Document& doc, bool onlyVis /* = false*/)
  {
    onlyVis_ = onlyVis;
    prepareSxfDocument();
    if (basis_.isNull() == true) {
      error_log << QObject::tr("Ошибка формирования паспорта данных SXF");
      return QByteArray();
      //basis_ = makePassportBasis(doc);
    }

    foreach (const QString& uuid, layerUuids) {
      map::Layer* layer = doc.layerByUuid(uuid);
      if (layer != 0) {
	if (layer->type() == kLayerFigure) { //чтоб расставить по приоритету
	  QMap<tablo::Color, QList<map::Object*> > objp;
	  foreach (map::Object* obj, layer->objects()) {
	    if (obj != 0 && obj->onDocument() /*&& (!onlyVis_ || obj->wasdrawed())*/) {
	      if (obj->type() == kFigure) {
		map::WeatherFigure* fig = mapobject_cast<map::WeatherFigure*>(obj);
		if (0 != fig) {
		  objp[fig->color(fig->pattern(), fig->meteodata())].append(fig);
		}
	      } else {
		objp[meteo::tablo::kNoColor].append(obj);
	      }
	    }
	  }	
	  QMapIterator<tablo::Color, QList<map::Object*> > it(objp);
	  while (it.hasNext()) {
	    it.next();
	    foreach (map::Object* obj, it.value()) {
	      foreach (const sxf::Object& sxfobj, convertMapObject(*obj)) {
		sxfdoc_->addObject(sxfobj);
	      }
	    }
	  }

	} else {
	  foreach (map::Object* obj, layer->objects()) {
	    if (obj != 0 && obj->onDocument() /*&& (!onlyVis_ || obj->wasdrawed())*/) {
	      foreach (const sxf::Object& sxfobj, convertMapObject(*obj)) {
		sxfdoc_->addObject(sxfobj);
	      }
	    }    
	  } 
	}
      }
    }
    return serializeToByteArray();
  }


  // void Sxf::convertMapObjects(const QStringList& objectUuids, const map::Layer& layer)
  // {
  //   QStringListIterator it(objectUuids);
  //   while (it.hasNext() == true) {
  //     map::Object* obj = layer.objectByUuid(it.next());
  //     if (obj != 0) {
  //       foreach (const sxf::Object& sxfobj, convertMapObject(*obj)) {
  //         sxfdoc_->addObject(sxfobj);
  //       }
  //     }
  //   }
  // }

  void Sxf::addObject(const sxf::Object& obj)
  {
    sxfdoc_->addObject(obj);
  }

  const QList<sxf::Object> Sxf::convertMapObject(const map::Object& obj) const
  {
    QList<sxf::Object> result;

    //  var(obj.type());

    if (obj.type() == kGroup) {
      parseGroupObject(obj, &result);
    } else {
      if (!onlyVis_ || obj.wasdrawed() ) {
	QList<meteo::GeoVector> lv = obj.skeletInRect(basis_->band, obj.layer()->document()->transform());
	//    debug_log << lv.at(0).at(0).lat() << lv.at(0).at(0).lon() ;
      
	//несколько частей объекта
	for (int num =0; num < lv.count(); num++) {
	  QList<sxf::Metric> metric;
	  QList<sxf::Semantic> semantic;
	  parseMetric(lv.at(num), &metric);
	  parseSemantic(obj, &semantic);
	  parseObject(obj, &metric, semantic, &result);
	}
      }
    
      // Рекурсивный парсинг подобъектов
      if (obj.hasChilds() == true) {
        foreach(map::Object* child, obj.childs()){
	  if (child != 0) {
	    result.append(convertMapObject(*child));
	  }
	}
      } 
    

    }

    return result;
  }

  const QList<sxf::Object> Sxf::convertMapObject(const map::Object& obj, const QList<sxf::Metric>& parentMetric) const
  {
    QList<sxf::Object> result;

    if (onlyVis_ && !obj.wasdrawed() && !obj.hasChilds()) {
      return result;
    }

    QList<meteo::GeoVector> lv = obj.skeletInRect(basis_->band, obj.layer()->document()->transform());
  
    //несколько частей объекта
    for (int num =0; num < lv.count(); num++) {
      QList<sxf::Metric> metric;
      QList<sxf::Semantic> semantic;
      parseMetric(lv.at(num), parentMetric, &metric);
      parseSemantic(obj, &semantic);
      parseObject(obj, &metric, semantic, &result);
    }

    return result;
  }

  void Sxf::parseMetric(const GeoVector& geo, const QList<sxf::Metric>& pmetric, QList<sxf::Metric>* ometric) const
  {
    if (ometric != 0) {
      QVectorIterator<GeoPoint> it(geo);
      while (it.hasNext() == true) {
	const GeoPoint& gp = it.next();
	if (gp.isValid() == true) {
	  int idx = findParentPoint(gp, pmetric);
	  if (idx == -1 || idx >= pmetric.count()) {
	    ometric->append(sxf::Object::metricFromGeoPoint(gp));
	  } else {
	    ometric->append(pmetric.at(idx));
	  }
	}
      }
    }
  }

  //! возвращает индекс одной из близких к gp точек родителя
  int Sxf::findParentPoint(const GeoPoint& gp, const QList<sxf::Metric>& pmetric) const
  {
    if (pmetric.count() == 0) {
      return -1;
    }

    float lim = 1e-6;
    int curIdx = 0;
    float curD = pow((pmetric.at(curIdx).x - gp.lat()), 2) + pow((pmetric.at(curIdx).y - gp.lon()), 2);
    if (curD < lim) { return curIdx; }

    for (int idx = 1; idx < pmetric.count(); idx++) {
      float d = pow((pmetric.at(idx).x - gp.lat()), 2) + pow((pmetric.at(idx).y - gp.lon()), 2);
      if (d < curD) {
	curD = d;
	curIdx = idx;
	if (curD < lim) { return idx; }
      }
    }
  
    return curIdx;
  }

  //! Метрика объекта
  void Sxf::parseMetric(const GeoVector& geo, QList<sxf::Metric>* ometric) const
  {
    if (ometric != 0) {
      QVectorIterator<GeoPoint> it(geo);
      while (it.hasNext() == true) {
	const GeoPoint& gp = it.next();
	if (gp.isValid() == true) {
	  //	debug_log << gp.lat() << gp.lon() ;
	  ometric->append(sxf::Object::metricFromGeoPoint(gp));
	}
      }
    }
  }

  //! Преобразование объекта карты в объект SXF
  void Sxf::parseObject(const map::Object& from, QList<sxf::Metric>* metric,
			const QList<sxf::Semantic>& semantic, QList<sxf::Object>* to) const
  {
    if (to == 0) return;
  
    switch (from.type()) {
    case kText:
      parseTextObject(from, *metric, semantic, to);
      break;
    case kPolygon:
    case kIsoline:
    case kCloudWx:
      parsePolygonObject(from, metric, semantic, to);
      break;
    case kPuanson:
      parsePuansonObject(from, *metric, semantic, to);
      break;
    case kRadar:
      parseRadarObject(from, *metric, semantic, to);
      break;
      // case map::kGroup:
      //   parseGroupObject(from, metric, semantic, to);
      //   break;
    case kFigure:
      parseCircleObject(from, *metric, semantic, to);
      break;
    default: {
    }
    }
  
    return;
  }

  void Sxf::parseGroupObject(const map::Object& from,
			     QList<sxf::Object>* to) const
  {
    if (from.hasChilds() == true) {
      foreach(map::Object* child, from.childs()){
	if (child != 0) {
	  to->append(convertMapObject(*child));
	}
      }
    }
  }

  //! Преобразование полигонов и линий
  void Sxf::parsePolygonObject(const map::Object& from,  QList<sxf::Metric>* metric,
			       const QList<sxf::Semantic>& semantic, QList<sxf::Object>* to) const
  {
    if (to == 0) return;

    const meteo::Property& prop = from.property();
    sxf::Object obj;

    int clcode = -1;
    if (prop.has_ornament()) {
      clcode = meteo::sxf::MetaSxf::instance()->ornamentCode(prop.ornament());
    } else if (prop.has_arrow_place()) {
      if (prop.arrow_place() == kEndArrow) {
	clcode = meteo::sxf::MetaSxf::instance()->ornamentCode("endarrow");//пока так, у нас обратно не загрузится
      }
    }

    if (-1 != clcode) {
      obj.setClassificationCode(clcode, sxf::kLinear);      
    } else {
      QPen pen = pen2qpen(prop.pen());
      QBrush brush = brush2qbrush(prop.brush());
      if ( prop.closed() &&  brush.style() != Qt::NoBrush && brush.color().alpha() != 0) {
	obj.addGraphicPrimitive(sxf::AREA_OBJECT, brush.color());
	metric->append(metric->first());
      }
      else if (pen.style() == Qt::SolidLine) {
	int w = pen.width()*pixScale_;
	obj.addGraphicPrimitive(sxf::SOLID_LINE, pen.color(), w > 0 ? w:pixScale_);
      }
      else if (pen.style() != Qt::NoPen) {
	int w = pen.width()*pixScale_;
	obj.addGraphicPrimitive(sxf::DASH_LINE, pen.color(), w > 0 ? w:pixScale_);
      }
    }

    obj.setSpline(prop.splinefactor() > 1);

    obj.setGeneralization(from.general().lowLimit(), from.general().highLimit());
    obj.addMetric(*metric);
    obj.addSemantic(semantic);

    if (obj.hasMetric() == true) {
      to->append(obj);
    }

    // var(prop.generalization.lowLimit());
    // var(prop.generalization.highLimit());
  }

  //! Преобразование данных радара
  void Sxf::parseRadarObject(const map::Object& from, const QList<sxf::Metric>& /*metric*/,
			     const QList<sxf::Semantic>& semantic, QList<sxf::Object>* to) const
  { 
    if (to == 0) return;
    const map::GeoMrl* mrl = mapobject_cast<const map::GeoMrl*>(&from);
    if (0 == mrl) return;

    QList< QPair< GeoVector, meteo::Property > > sqs = mrl->getSquares();
    
    for (int idx = 0; idx < sqs.size(); idx++) {
      sxf::Object obj;
      const meteo::Property& prop = sqs.at(idx).second;
      QPen pen = pen2qpen(prop.pen());
      QBrush brush = brush2qbrush(prop.brush());
      obj.addGraphicPrimitive(sxf::AREA_OBJECT, brush.color());
      obj.setGeneralization(from.general().lowLimit(), from.general().highLimit());
      QList<sxf::Metric> sqmetric;
      parseMetric(sqs.at(idx).first, &sqmetric);
      //metric->append(sqmetric->first());
      obj.addMetric(sqmetric);
      obj.addSemantic(semantic);

      if (obj.hasMetric() == true) {
    	to->append(obj);
      }
    }

   
  }

  void Sxf::parseCircleObject(const map::Object& from, const QList<sxf::Metric>& metric,
			      const QList<sxf::Semantic>& semantic, QList<sxf::Object>* to) const
  {
    Q_UNUSED(metric);

    if (to == 0) return;

    const map::WeatherFigure* fig = mapobject_cast<const map::WeatherFigure*>(&from);
    if (0 == fig) return;

    bool ok = false;
    QPair< GeoVector, meteo::Property > prop =  fig->getCircle( &ok );
    if (!ok) return;

    sxf::Object obj;
    QBrush brush = brush2qbrush(prop.second.brush());
    obj.addGraphicPrimitive(sxf::AREA_OBJECT, brush.color());
    obj.setSpline(prop.second.splinefactor() > 1);

    obj.setGeneralization(from.general().lowLimit(), from.general().highLimit());
    
    QList<sxf::Metric> ometric;
    parseMetric(prop.first, &ometric);
    ometric.append(ometric.first());
    obj.addMetric(ometric);

    obj.addSemantic(semantic);

    if (obj.hasMetric() == true) {
      to->append(obj);
    }

  }

  //! Преобразование текста
  void Sxf::parseTextObject(const map::Object& from, const QList<sxf::Metric>& metric,
			    const QList<sxf::Semantic>& semantic, QList<sxf::Object>* to) const
  {
    if (to == 0) return;

    const meteo::Property& prop = from.property();
    //  QList<QRect> positions(from.boundingRect(from.layer()->document()->transform()));

    //несколько подписей на линии
    for (int idx = 0; idx < metric.count(); idx++) {

      sxf::Object obj;    

      obj.addTextPrimitive(static_cast<const map::GeoText&>(from).text(), font2qfont(prop.font()), pen2qpen(prop.pen()).color(),
      			   pixScale_);

      obj.addMetric(metric.at(idx));
      obj.addMetric(metric.at(idx));
      
      obj.addSemantic(semantic);

      obj.setGeneralization(from.general().lowLimit(), from.general().highLimit());
      
      if (obj.hasMetric() == true) {
	to->append(obj);
      }
    }
    
  }
  
  //! Создание объекта ветра
  bool Sxf::parsePuansonWind(const map::Object& from, const map::Puanson* puan, 
			     const QList<sxf::Semantic>& semantic, QList<sxf::Object>* to) const
  {
    if (0 == to || 0 == puan) return false;

    bool isWind = false;
    GeoVector gv = puan->windSkelet();
    if (gv.size() == 2 ) {
      sxf::Object obj;
      int code = meteo::sxf::MetaSxf::instance()->windCode(round(puan->ff().value()), puan->lenft());
      if (-1 != code) {
	obj.setClassificationCode(code, sxf::kVector);
	obj.addMetric(sxf::Object::metricFromGeoPoint(gv[0]));
	obj.addMetric(sxf::Object::metricFromGeoPoint(gv[1]));
	//debug_log << MnMath::rad2deg(metric.at(0).x) << MnMath::rad2deg(metric.at(0).y);
	// debug_log << MnMath::rad2deg(gv[0].lat()) << MnMath::rad2deg(gv[0].lon()) 
	// 	  << MnMath::rad2deg(gv[1].lat()) << MnMath::rad2deg(gv[1].lon());
	// debug_log << metric.at(0).x << metric.at(0).y;
	// debug_log << gv[0].lat() << gv[0].lon() 
	// 	  << gv[1].lat() << gv[1].lon();
	obj.addSemantic(semantic);
	obj.addSemantic(sxf::Semantic(QString("Puanson ") + QString::fromStdString(puan->punch().code()), meteo::sxf::kNameSemantic));
	obj.addSemantic(sxf::Semantic(QString::number(puan->ff().value(), 'f', 0), meteo::sxf::kffSemantic, 2));
	obj.addSemantic(sxf::Semantic(QString::number(puan->dd().value(), 'f', 0), meteo::sxf::kddSemantic, 2));
	isWind = true;
	obj.setGeneralization(from.general().lowLimit(), from.general().highLimit());
	
	if (obj.hasMetric() == true) {
	  to->append(obj);
	}
      }
    }    

    return isWind;
  }

  //! Добавление фрагментов пуансона
  void Sxf::parseMeteoWithTtf(const map::Puanson* puan, const puanson::proto::CellRule& rule, 
			      sxf::Object* obj, sxf::VectorSign* sign) const
  {
    if (0 == puan || 0 == obj || 0 == sign) return;

    bool res = false;
    const TMeteoParam& mp = puan->meteodata().meteoParam(QString::fromStdString(rule.id().name()), &res);
    //if (res == false) continue;
    
    QString value = map::stringFromRuleValue(mp.value(), rule, mp.code());
    //debug_log << punch.rule(idx).id().name()<< mp.value() << mp.code() << mp.quality() << value << value[0].cell();
    
    if (!value.isEmpty()) {
      
      obj->addSemantic(sxf::Semantic(QString("%1;%2;'%3';%4").arg(QString::fromStdString(rule.id().name()))
				     .arg(mp.value(), 0, 'f', 3).arg(mp.code()).arg(mp.quality()),
				     meteo::sxf::kMeteoSemantic));
      
      QSharedPointer<sxf::VectorObject> fr = sign->addObject(68, sxf::TEXT_VECT); // 68 - текст постоянный
      sxf::VectorTextGraphicObject* tobj = static_cast<sxf::VectorTextGraphicObject*>(fr->obj.data());
      if (0 == tobj) { return; }
      
      QFont font = map::fontFromRuleValue(mp.value(), rule);
      tobj->setText(value, meteo::sxf::MetaSxf::instance()->getFont(font),
		    map::colorFromRuleValue(mp.value(), rule));
      
      QFontMetrics fm(font);
      
      tobj->scaleFlag = 1;
      tobj->align = 8;
      tobj->horizontal = 0;
      
      QRect rect = puan->boundingRect(rule);
      
      if (font.weight() == QFont::Bold) {	
	tobj->weight = 600; 
	tobj->height = (fm.height()) * pixScale_;
	fr->xy << QPair<int32_t, int32_t>((rect.left())*pixScale_, (rect.bottom() + 3)*(pixScale_)) 
	       << QPair<int32_t, int32_t>((rect.right())*pixScale_, (rect.bottom() + 3)*(pixScale_));
	
      } else {
	tobj->height = (fm.height()) * pixScale_;
	fr->xy << QPair<int32_t, int32_t>(rect.left()*pixScale_, rect.bottom()*(pixScale_)) 
	       << QPair<int32_t, int32_t>((rect.right())*pixScale_, rect.bottom()*(pixScale_));
      }
      
      fr->calcLength();	
      //debug_log << value << "xy" << rect.left() << rect.right() << rect.bottom();
    }
  }

  void Sxf::parseMeteoObjects(const map::Puanson* puan, bool isWind, const QList<sxf::Metric>& metric,
			      const QList<sxf::Semantic>& semantic, QList<sxf::Object>* to) const
  {
    const puanson::proto::Puanson& punch = puan->punch();

    sxf::Object obj;
    sxf::VectorSign* sign = obj.createVectorObject();
    if (0 == sign) return;

    obj.addMetric(metric);
    obj.addSemantic(semantic);
    obj.setGeneralization(puan->general().lowLimit(), puan->general().highLimit());

    if (isWind) {
      obj.addSemantic(sxf::Semantic(QString::number(puan->ff().value(), 'f', 0), meteo::sxf::kffSemantic, 2));
      obj.addSemantic(sxf::Semantic(QString::number(puan->dd().value(), 'f', 0), meteo::sxf::kddSemantic, 2));
    }
  
    obj.addSemantic(sxf::Semantic(QString("Puanson ") + QString::fromStdString(punch.code()), meteo::sxf::kNameSemantic));

    if (punch.drawrect()) {
      QRect rect = puan->boundingRect();

      QSharedPointer<sxf::VectorObject> fr1 = sign->addObject(0, sxf::AREA_OBJECT);
      sxf::AreaGraphicObject* tobj = static_cast<sxf::AreaGraphicObject*>(fr1->obj.data());
      tobj->color = 0xffffff;
      fr1->xy << QPair<int32_t, int32_t>((rect.left()-1)*(pixScale_), (rect.top()-1)*(pixScale_)) 
	      << QPair<int32_t, int32_t>((rect.left()-1)*(pixScale_), (rect.bottom())*(pixScale_)) 
	      << QPair<int32_t, int32_t>((rect.right()+1)*(pixScale_), (rect.bottom())*(pixScale_))
	      << QPair<int32_t, int32_t>((rect.right()+1)*(pixScale_), (rect.top()-1)*(pixScale_))
	      << QPair<int32_t, int32_t>((rect.left()-1)*(pixScale_), (rect.top()-1)*(pixScale_));
      fr1->calcLength();

      QSharedPointer<sxf::VectorObject> fr2 = sign->addObject(1, sxf::SOLID_LINE);
      fr2->xy << QPair<int32_t, int32_t>((rect.left()-1)*(pixScale_), (rect.top()-1)*(pixScale_)) 
	      << QPair<int32_t, int32_t>((rect.left()-1)*(pixScale_),  rect.bottom()*(pixScale_)) 
	      << QPair<int32_t, int32_t>((rect.right()+1)*(pixScale_), rect.bottom()*(pixScale_))
	      << QPair<int32_t, int32_t>((rect.right()+1)*(pixScale_), (rect.top()-1)*(pixScale_))
	      << QPair<int32_t, int32_t>((rect.left()-1)*(pixScale_), (rect.top()-1)*(pixScale_));
      fr2->calcLength();
      // debug_log << '(' << rect.top() << rect.left() << ") (" << rect.bottom() << rect.right() << ')';
      // debug_log << '(' << rect.top()*pixScale_ << rect.left()*pixScale_  << ") ("
      // 		<< rect.bottom()*pixScale_ << rect.right()*pixScale_ << ')';
    }

    QList<sxf::Object> cllist;

    for (int idx = 0; idx < punch.rule_size(); idx++) {
      if ( punch.rule(idx).align() == puanson::proto::kNoPosition ) {
        continue;
      }
        
      if (false == puan->meteodata().hasParam(QString::fromStdString(punch.rule(idx).id().name())) && 
	  false == punch.rule(idx).has_absent_rule() ) {
	continue;
      }

      QString name = QString::fromStdString(punch.rule(idx).id().name());
      //var(name);

      bool res = false;
      const TMeteoParam& mp = puan->meteodata().meteoParam(name, &res);
      QString family = map::fontFromRuleValue(mp.value(), punch.rule(idx)).family();
      
      if (!isTtfSupported_ && meteo::sxf::MetaSxf::instance()->meteoType(name) != sxf::kUnkClassif &&
	  (family == "wths" || family == "Synop" || family == "wthrNew0  Normal" || 
	   family == "wthrNew1  Normal")) { 
	sxf::Object clobj;
	QString value = map::stringFromRuleValue(mp.value(), punch.rule(idx), mp.code());
	//var(value);
	int code = -1;
	if (punch.code() == "wxiceturb" && name == "B") { //костыль
	  if (value == "q") {
	    code = 1184;
	  } else {
	    code = 1186;
	  }
	} else {
	  code = meteo::sxf::MetaSxf::instance()->meteoCode(name, value);
	}
	//var(code);
	if (code != -1) {
	  clobj.setClassificationCode(code, sxf::kVector);
	  clobj.addSemantic(semantic);
	  clobj.addMetric(metric);
	  clobj.setGeneralization(puan->general().lowLimit(), puan->general().highLimit());
	  if (clobj.hasMetric() == true) {
	    cllist.append(clobj);
	  }
	}
      } else { 
	parseMeteoWithTtf(puan, punch.rule(idx), &obj, sign);
      }
   
      // {//for debug
      // 	QSharedPointer<sxf::VectorObject> fr1 = sign->addObject(1, sxf::SOLID_LINE);
      // 	fr1->xy << QPair<int32_t, int32_t>((rect.left()-1)*(pixScale_), (rect.top()-1)*(pixScale_)) 
      // 		<< QPair<int32_t, int32_t>((rect.left()-1)*(pixScale_), (rect.bottom()+1)*(pixScale_)) 
      // 		<< QPair<int32_t, int32_t>((rect.right()+1)*(pixScale_), (rect.bottom()+1)*(pixScale_))
      // 		<< QPair<int32_t, int32_t>((rect.right()+1)*(pixScale_), (rect.top()-1)*(pixScale_))
      // 		<< QPair<int32_t, int32_t>((rect.left()-1)*(pixScale_), (rect.top()-1)*(pixScale_));

      // 	fr1->calcLength();
      // 	QFontMetrics fm(puan->fontFromRuleValue(mp.value(), punch.rule(idx)));
      // 	debug_log << value << (fm.height()) << '(' << rect.top() << rect.left() << ") (" << rect.bottom() << rect.right() << ')';
      // 	debug_log << (fm.height()-2) * pixScale_ << '(' << rect.top()*pixScale_ << rect.left()*pixScale_  << ") ("
      // 		  << rect.bottom()*pixScale_ << rect.right()*pixScale_ << ')';
      // }
    }

    QRect puanRect = puan->boundingRect();

    sign->pnt_vertic = puanRect.height()*pixScale_ / 2;
    sign->pnt_hor = puanRect.width()*pixScale_ / 2;
    sign->pnt_base = puanRect.width()*pixScale_;
    sign->mark_vertic_end = puanRect.height()*pixScale_;
    sign->size_vertic  = puanRect.height()*pixScale_;
    sign->mark_hor_end = puanRect.width()*pixScale_;
    sign->size_hor = puanRect.width()*pixScale_;
    sign->max_dim  = puanRect.width()*pixScale_;
    sign->align = 0;
    sign->calcLength();
    
    if (punch.drawrect()) {
      if (obj.hasMetric() == true) {
	to->append(obj);
      }
      to->append(cllist);
    } else {
      to->append(cllist);
      if (obj.hasMetric() == true) {
	to->append(obj);
      }
    }

  }
  
  //! Преобразование пуансона
  void Sxf::parsePuansonObject(const map::Object& from, const QList<sxf::Metric>& metric,
			       const QList<sxf::Semantic>& semantic, QList<sxf::Object>* to) const
  {
    if (to == 0) return;

    const map::Puanson* puan = mapobject_cast<const map::Puanson*>(&from);
    if (0 == puan) return;

    bool isWind = false;
    //ветер
    if (puan->hasWind() && puan->ff().isValid() &&  puan->dd().isValid()) {
      isWind = parsePuansonWind(from, puan, semantic, to);
    }
    
    //остальные объекты
    QList<sxf::Metric> meteoMetric;
    //debug_log << puan->bindedToSkelet() << puan->bindedSkelet();
    if (puan->bindedToSkelet()) {
      bool fl = false;
      GeoVector gv = puan->bindedSkelet(&fl);
      if (fl && puan->bindedSkelet().size() >= 2) {
	meteoMetric.append(sxf::Object::metricFromGeoPoint(gv[0]));
	meteoMetric.append(sxf::Object::metricFromGeoPoint(gv[1]));
      }
    } else {
      meteoMetric.append(metric);
      meteoMetric.append(metric);
    }

    parseMeteoObjects(puan, isWind, meteoMetric, semantic, to);
  }
  

  //! Семантика объекта
  void Sxf::parseSemantic(const map::Object& from, QList<sxf::Semantic>* semantic) const
  {
    if (semantic != 0) {
      sxf::Semantic uuid(from.uuid(), meteo::sxf::kUuidSemantic);
      semantic->append(uuid);
      sxf::Semantic lay(from.layer()->name(), meteo::sxf::kLayerSemantic);
      semantic->append(lay); 
    }
  }

  const QByteArray Sxf::serializeToByteArray()
  {
    if (sxfdoc_ == 0) {
      lastError_ = QString::fromUtf8("Ошибка. Не были заданы данные для экпорта");
      error_log << lastError_;
      return QByteArray();
    }
    sxfdoc_->setGeobasis(basis_);

    return sxfdoc_->serializeToByteArray(&lastError_);
  }

} // meteo
