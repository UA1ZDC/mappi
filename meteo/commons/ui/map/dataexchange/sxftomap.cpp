#include "sxftomap.h"
#include "rscobjects.h"

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/object.h>

#include <qfile.h>

using namespace meteo;

SxfToMap::SxfToMap():
  sxfdoc_(0),
  rscdoc_(0)
{
  sxfdoc_ = new sxf::Document();
}

SxfToMap::~SxfToMap()
{
  clear();
}


void SxfToMap::clear()
{
  if (0 != sxfdoc_) {
    delete sxfdoc_;
    sxfdoc_ = 0;
  }
  if (0 != rscdoc_) {
    delete rscdoc_;
    rscdoc_ = 0;
  }  
}

bool SxfToMap::isValid()
{
  return (0 != sxfdoc_ && sxfdoc_->isValid() && 0 != rscdoc_ && rscdoc_->isValid());
}

bool SxfToMap::readSxf(const QByteArray& ba)
{
  QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
  if (codec == 0) return false;

  if (0 == sxfdoc_) {
    sxfdoc_ = new sxf::Document();
  }

  bool ok = sxfdoc_->setData(ba);

  return ok;
}

bool SxfToMap::readDescription(const QByteArray& ba, SxfDescription* descr)
{
  if (0 == descr) return false;

  bool ok = readSxf(ba);
  if (0 == sxfdoc_ || !ok) return false;

  descr->date = sxfdoc_->date();
  descr->name = sxfdoc_->name();
  descr->objCnt = sxfdoc_->objectsCount();
  
  return ok;
}


//! определение слоев в классификаторе, к которым относятся объекты карты
bool SxfToMap::fillLegend(QMap<uint8_t, sxf::LayersDescription>* legend)
{
  if (!isValid() || legend == 0) return false;

  QMap<sxf::ObjectKey, sxf::ObjectDescription> objCodes;

  QMap<uint8_t, sxf::LayersDescription> graphicLegend;
  bool ok = sxfdoc_->findObjects(&objCodes, &graphicLegend);
  //  var(graphicLegend.count());
  if (!ok) return false;
  
  //var(objCodes.count());

  ok = rscdoc_->fillObjectsLayers(objCodes, &objLayers_, legend);
  legend->unite(graphicLegend);

  return ok;
}


bool SxfToMap::setRscData(const QByteArray& ba)
{
  if (0 != rscdoc_) {
    delete rscdoc_;
    rscdoc_ = 0;
  }
  rscdoc_ = new rsc::Document(ba);
  if (0 == rscdoc_ || !rscdoc_->isValid()) return false;

  return true;  
}

//! Импорт слоёв из sxf
/*! 
  \param layerNums Номер (из классификатора для не графических объектов), название слоя
  \param checked Выбранные объекты
  \param doc Документ
*/
bool SxfToMap::import(const QList<QPair<QString,QString> >& layerNums, const QList<sxf::ObjectKey>& checked, map::Document* doc)
{
  if (0 == doc || 0 == doc->projection() || 
      0 == sxfdoc_ || 0 == sxfdoc_->passport() ||
      !isValid()) return false;

 
  sxf::Object sxfobj;

  int kk = 0;
  int ll = 0;
  int sub = 0;

  QMap<QString, QString> layersUuids; //(номер слоя в sxf для слоев из классификатора или название слоя, uuid на карте)
  addLayers(layerNums, &layersUuids, doc);

  float origScale = sxfdoc_->getMapScaleMatch();
  if (origScale == 0) {
    origScale = doc->scale();
  }

  //TODO вернуть
  // Projection* proj = doc->projection();
  // if (proj->type() == meteo::MERCAT) {
  //   GeoVector ramka = proj->ramka();
  //   for (int i =0; i < ramka.size(); i++) {
  //     if (ramka[i].lat() > 0) {
  // 	ramka[i].setLat(sxfdoc_->passport()->north_west_lat);
  //     } else {
  // 	ramka[i].setLat(sxfdoc_->passport()->south_west_lat);
  //     }
  //   }
  //   proj->setRamka(ramka);
  // }

  while (sxfdoc_->readNextObject(&sxfobj, &checked)) {
    if (checkIfMissObject(sxfobj.header().classification_code, sxfobj.header().localization)) {
      continue;
    }

    QString layName;
    for (int idx = 0; idx < sxfobj.semantic().length(); idx++) {
      if (sxfobj.semantic().at(idx).code == meteo::sxf::kLayerSemantic) {
	layName = sxfobj.semantic().at(idx).value;
	break;
      }
    }
    
    //debug_log << sxfobj.header().classification_code << sxfobj.header().localization;
    
    if (sxfobj.header().classification_code != 0) {
      const rsc::Object& rscobj = rscdoc_->object(sxfobj.header().classification_code, 
						  sxfobj.header().localization,
						  sxfobj.semantic());
      if (rscobj.code == 0) {
	warning_log << QObject::tr("Не найдено описание объекта в классификаторе").
	  arg(sxfobj.header().classification_code).arg(sxfobj.header().localization);
	continue;
      }
      
      map::Layer* lay = doc->layer(layersUuids.value(QString::number(rscobj.layer)));
      
      if (0 == lay) {
	continue;
      }
      
      int ss = 0;
      if (0 != (ss = rscdoc_->drawObject(sxfobj, rscobj, lay, origScale))) {
	ll++;
      }
      sub += ss;
    } else {
      
      map::Layer* lay = 0;
      if (layersUuids.contains(layName)) {
	lay = doc->layer(layersUuids.value(layName));
      } 

      if (0 == lay) {
	continue;
      }
      
      int ss = 0;
      //var(origScale);
      if (0 != (ss = rscdoc_->drawObject(sxfobj, lay, origScale))) {
	ll++;
      }
      sub += ss;
    }
    
    //debug
    kk++;
    // if (kk % 5000 == 0) {
    // var(kk);
    // var(sub);
    // }
    // if (kk > 1) {
    //   break;
    // }
    //--

    if (kk % 100 == 0) {
      // var(kk);
      // var(sub);
      emit importedObjects(kk, objLayers_.count());
    }
  }  
  
  // var("end");
  // var(kk);
  // var(ll);
  // var(sub);

 
  QMap<QString, QString>::const_iterator itl = layersUuids.constBegin();
  while (itl != layersUuids.constEnd()) {
    map::Layer* lay = doc->layer(itl.value());
    lay->show();
    //  debug_log << itl.key();
    ++itl;
  }
  return true;
}

//! добавление слоев в соответствии с порядком, указанным в классификаторе
bool SxfToMap::addLayers(const QList<QPair<QString,QString> >& layerNums, QMap<QString, QString>* layersUuids, map::Document* doc)
{
  QTextCodec *codec = QTextCodec::codecForName("Windows-1251");

  QMap<int, QString> layOrder;//порядок и номер слоя для слоев из классификатора, порядок и название для других
  for (int idx = 0; idx < layerNums.count(); idx++) {
    if (rscdoc_->hasLayer(layerNums.at(idx).first.toUInt())) {
      layOrder.insert(rscdoc_->layer(layerNums.at(idx).first.toUInt()).order, layerNums.at(idx).first);
    } else {
      layOrder.insert(layerNums.at(idx).first.toUInt(), layerNums.at(idx).second);
    } 
  }

  QList<int> keys = layOrder.keys();
  qSort(keys);

  for (int idx = 0; idx < keys.size(); idx++) {
    bool ok = false;
    int num = layOrder.value(keys.at(idx)).toUInt(&ok);
    QString name;
    if (ok) {
      name = codec->toUnicode(rscdoc_->layer(num).name, 
			      strlen(rscdoc_->layer(num).name));
    }
    if (name.isEmpty()) {
      name = layOrder.value(keys.at(idx));
    }
    map::Layer* lay = new map::Layer(doc, QString::fromUtf8("%1").arg(name)); 
    if (lay != 0) {
      layersUuids->insert(layOrder.value(keys.at(idx)), lay->uuid());
      lay->hide();
    }
  }

  return true;
}

//! Загрузка основы из sxf
/*! 
  
  \param layerNums Номера слоев из классификатора
  \param map Карта
*/
bool SxfToMap::import(const QStringList& layerNums, map::Document* doc )
{
  if (0 == doc || 0 == doc->projection() || 
      0 == sxfdoc_ || 0 == sxfdoc_->passport() ||
      !isValid()) return false;

 
  sxf::Object sxfobj;

  int kk = 0;
  int ll = 0;
  int sub = 0;


  QMap<QString, map::Layer*> layersUuids; //(номер слоя в sxf для слоев из классификатора или название слоя, uuid на карте)
  addLayers(layerNums, &layersUuids, doc );

  float origScale = sxfdoc_->getMapScaleMatch();
  // if (origScale == 0) {
  //   origScale = doc->scale();
  // }

  Projection* proj = doc->projection();
  if (proj->type() == meteo::MERCAT) {
    GeoVector ramka = proj->ramka();
    for (int i =0; i < ramka.size(); i++) {
      if (ramka[i].lat() > 0) {
  	ramka[i].setLat(sxfdoc_->passport()->north_west_lat);
      } else {
  	ramka[i].setLat(sxfdoc_->passport()->south_west_lat);
      }
    }
    proj->setRamka(ramka);
  }

  while (sxfdoc_->readNextObject(&sxfobj, 0)) {
    if (checkIfMissObject(sxfobj.header().classification_code, sxfobj.header().localization)) {
      continue;
    }

    QString layName;
    for (int idx = 0; idx < sxfobj.semantic().length(); idx++) {
      if (sxfobj.semantic().at(idx).code == meteo::sxf::kLayerSemantic) {
	layName = sxfobj.semantic().at(idx).value;
	break;
      } else if (sxfobj.semantic().at(idx).code == meteo::sxf::kShortLayerSemantic) {
	layName = sxfobj.semantic().at(idx).value;
	break;
      }
    }
    
    //debug_log << sxfobj.header().classification_code << sxfobj.header().localization;
    
    if (sxfobj.header().classification_code != 0) {
      const rsc::Object& rscobj = rscdoc_->object(sxfobj.header().classification_code, 
						  sxfobj.header().localization,
						  sxfobj.semantic());
      if (rscobj.code == 0) {
	warning_log << QObject::tr("Не найдено описание объекта в классификаторе").
	  arg(sxfobj.header().classification_code).arg(sxfobj.header().localization);
	continue;
      }
      
      map::Layer* lay = layersUuids.value(QString::number(rscobj.layer));
      
      if (0 == lay) {
	continue;
      }
      
      int ss = 0;
      if (0 != (ss = rscdoc_->drawObject(sxfobj, rscobj, lay, origScale))) {
	ll++;
      }
      sub += ss;
    } else {

      map::Layer* lay = 0;
      if (layersUuids.contains(layName) && 0 != layersUuids.value(layName)) {
	lay = layersUuids.value(layName);
      } else {
	lay = new map::Layer(doc, layName);
        lay->setBase(true);
//	lay = map->createBasisLayer(layName); 
	if (lay != 0) {
	  layersUuids.insert(layName, lay);
	}
      }

      if (0 == lay) {
	continue;
      }
      
      int ss = 0;
      if (0 != (ss = rscdoc_->drawObject(sxfobj, lay, origScale))) {
	ll++;
      }
      sub += ss;
    }
    
    //debug
    kk++;
    // if (kk % 5000 == 0) {
    // var(kk);
    // var(sub);
    // }
    // if (kk > 1) {
    //   break;
    // }
    //--

    if (kk % 100 == 0) {
      // var(kk);
      // var(sub);
    }
  }  
  
  // var("end");
  // var(kk);
  // var(ll);
  // var(sub);

  return true;
}

//! добавление слоев в соответствии с порядком, указанным в классификаторе
bool SxfToMap::addLayers(const QStringList& layerNums, QMap<QString, map::Layer*>* layersUuids, map::Document* doc )
{
  QTextCodec *codec = QTextCodec::codecForName("Windows-1251");

  QMultiMap<int, QString> layOrder;
  for (int idx = 0; idx < layerNums.count(); idx++) {
    if (rscdoc_->hasLayer(layerNums.at(idx).toUInt())) {
      layOrder.insert(rscdoc_->layer(layerNums.at(idx).toUInt()).order, layerNums.at(idx));
    } 
  }

  QMapIterator<int, QString> it(layOrder);
  while (it.hasNext()) {
    it.next();
    QString name = codec->toUnicode(rscdoc_->layer(it.value().toUInt()).name, strlen(rscdoc_->layer(it.value().toUInt()).name));
     if (name.isEmpty()) {
      name = it.value();
    }
//    map::Layer* lay = map->createBasisLayer(QString::fromUtf8("%1").arg(name)); 
    map::Layer* lay = new map::Layer( doc, QString::fromUtf8("%1").arg(name) ); 
    lay->setBase(true);
    if (lay != 0) {
      layersUuids->insert(it.value(), lay);
    }
  }

  return true;
}

//! Пропуск ненужных объектов слоя (для отладки)
bool SxfToMap::checkIfMissObject(uint32_t code, uint8_t localization)
{
  Q_UNUSED(code);
  Q_UNUSED(localization);

  // if (code == 92170000 /*|| code == 92171000 || code == 92172000*/) {
  //   return true;
  // }

  return false;
}

