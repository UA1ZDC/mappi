#include <QApplication>
#include <QByteArray>
#include <QFile>
#include <QTextCodec>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/loader.h>
#include <meteo/commons/ui/map/dataexchange/sxfexchange.h>
#include <meteo/commons/ui/map/dataexchange/passportbasis.h>
#include <meteo/commons/ui/map/dataexchange/sxfstructures.h>
#include <meteo/commons/ui/map/dataexchange/rscobjects.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/proto/sxfcodes.pb.h>

using namespace meteo;
using namespace sxf;

void fillPassport(QSharedPointer<meteo::sxf::PassportBasis> basis,  map::Document* document)
{
  QPoint pnt;

  basis->northWest = document->screen2coord(pnt);
  basis->northEast = document->screen2coord(pnt);
  basis->southWest = document->screen2coord(pnt);
  basis->southEast = document->screen2coord(pnt);
  basis->band = document->documentRect();

  switch (document->projection()->type()) {
  case STEREO:
    basis->projection = 23;
    break;
  case MERCAT:
    basis->projection = 36;
    break;
  case CONICH:
    basis->projection = 20;
    break;
  default: {
    basis->projection = 36;
  }
  }

  basis->scale = 500000;
  basis->mainPoint = document->center(); //GeoPoint(0, view->mapscene()->document->center().lon());
  basis->firstMajorParallel = document->center().lat();//0;
  basis->secondMajorParallel = 0;
  basis->northOffset = 0;
  basis->eastOffset = 0;
}

void addObjects(map::Layer* lay)
{
  //линия с подписью
  map::GeoPolygon* gp = new map::GeoPolygon(lay);
  GeoVector gv;
  gv.append(GeoPoint::fromDegree(60, 30));
  gv.append(GeoPoint::fromDegree(60, 31));
  gp->setSkelet(gv);
  map::GeoText* gt = new map::GeoText(gp);
  gt->setText("Hello, мир!");
  gt->setSkelet(gv);
}

void addTextClassif(Sxf* sxfdoc,  const meteo::GeoPoint& /*mapCenter*/)
{
  QList<sxf::Metric> metric;

  // если шрифт добавлен как объект
  // metric.append(sxf::Object::metricFromGeoPoint(GeoPoint::fromDegree(59.9, 30)));
  // metric.append(sxf::Object::metricFromGeoPoint(GeoPoint::fromDegree(59.9, 30.1)));
  // sxf::Semantic sem("Sign", 9);
  // sxf::Object obj;
  // obj.addMetric(metric);
  // obj.addSemantic(sem);
  // int32_t code = 221; //код объекта шрифта
  // obj.addTextWithClassificator("qer", code);

  // sxfdoc->addObject(obj);

  //----

  metric.clear();
  metric.append(sxf::Object::metricFromGeoPoint(GeoPoint::fromDegree(59.9, 30.3)));
  //  metric.append(sxf::Object::metricFromGeoPoint(GeoPoint::fromDegree(59.9, 30.4)));

  sxf::Semantic sem1("TrueType", 9);
  
  sxf::Object obj1;
  obj1.addMetric(metric);
  obj1.addSemantic(sem1);
  
  obj1.setContract(false);
  int32_t signNum = 93;  
  int fontCode = 3;
  obj1.addTrueTypeText(signNum, fontCode, QFont("Synop"), Qt::blue, 256);

  sxfdoc->addObject(obj1);
  
}

void addVector(Sxf* sxfdoc,  const meteo::GeoPoint& /*mapCenter*/)
{
  QList<sxf::Metric> metric;
  metric.append(sxf::Object::metricFromGeoPoint(GeoPoint::fromDegree(60, 30)));
  metric.append(sxf::Object::metricFromGeoPoint(GeoPoint::fromDegree(60, 30.1)));
  sxf::Semantic sem("Vector", 9);
  //  sxf::Semantic sem1("[]", 17001, 0);
  sxf::Semantic semff("10", meteo::sxf::kffSemantic, 2);
  sxf::Semantic semdd("234", meteo::sxf::kddSemantic, 2);
  sxf::Semantic name("Puanson kn01", meteo::sxf::kNameSemantic);
  sxf::Semantic lay("test", meteo::sxf::kLayerSemantic);

  sxf::Object obj;
  obj.addMetric(metric);
  obj.addSemantic(sem);
  obj.addSemantic(semff);
  obj.addSemantic(semdd);
  obj.addSemantic(name);
  obj.addSemantic(lay);

  //  obj.addSemantic(sem1);
  //  sxf::VectorSign* sign = obj.createVectorObject();
  QSharedPointer<sxf::VectorSign> sign = QSharedPointer<sxf::VectorSign>(new sxf::VectorSign);
  obj.addGraphicPrimitive(sign, sxf::kVector);

  QSharedPointer<sxf::VectorObject> fr3 = sign->addObject(68, sxf::TEXT_VECT);
  sxf::VectorTextGraphicObject* tobj = static_cast<sxf::VectorTextGraphicObject*>(fr3->obj.data());
  tobj->text[0] = ']';
  tobj->height = 3000;
  tobj->fontcode = 3;
  tobj->reserved1 = 3;
  tobj->color = 0x00ff00;
  fr3->xy << QPair<int32_t, int32_t>(0,-3000) << QPair<int32_t, int32_t>(0,-3000);
  fr3->calcLength();

  QSharedPointer<sxf::VectorObject> fr3_1 = sign->addObject(68, sxf::TEXT_VECT);
  sxf::VectorTextGraphicObject* tobj3_1 = static_cast<sxf::VectorTextGraphicObject*>(fr3_1->obj.data());
  tobj3_1->text[0] = 'q';
  tobj3_1->height = 4000;
  tobj3_1->fontcode = 2;
  tobj3_1->reserved1 = 2;
  tobj3_1->color = 0xff0fff;
  fr3_1->xy << QPair<int32_t, int32_t>(0,-500) << QPair<int32_t, int32_t>(0,-500);
  fr3_1->calcLength();

  QSharedPointer<sxf::VectorObject> fr1 = sign->addObject(1, sxf::SOLID_LINE);
  fr1->xy << QPair<int32_t, int32_t>(0,0) << QPair<int32_t, int32_t>(2000,2000) << QPair<int32_t, int32_t>(0,2000);
  fr1->calcLength();

  QSharedPointer<sxf::VectorObject> fr1_1 = sign->addObject(1, sxf::SOLID_LINE);
  fr1_1->xy << QPair<int32_t, int32_t>(4500,2000) << QPair<int32_t, int32_t>(2500,2000) << QPair<int32_t, int32_t>(2500,0);
  fr1_1->calcLength();

  QSharedPointer<sxf::VectorObject> fr2 = sign->addObject(0, sxf::AREA_OBJECT);
  fr2->xy << QPair<int32_t, int32_t>(3000,500) << QPair<int32_t, int32_t>(3800,700) << QPair<int32_t, int32_t>(3000,1500);
  fr2->calcLength();

  //текст по семантике не работает, в том числе, если добавлять его в интеграции 
  // QSharedPointer<sxf::Object::VectorObject> fr4 = sign->addObject(69, sxf::TEXT_VECT);
  // sxf::Object::VectorTextGraphicObject* tobj4 = static_cast<sxf::Object::VectorTextGraphicObject*>(fr4->obj.data());
  // tobj4->height = 4000;
  // tobj4->fontcode = 2;
  // tobj4->reserved1 = 2;
  // tobj4->semcode = 17001;
  // tobj4->color = 0x0000ff;
  // fr4->xy << QPair<int32_t, int32_t>(2500,-1000) << QPair<int32_t, int32_t>(2700,-1000);
  // fr4->calcLength();

  
  sign->pnt_vertic = 5000;
  sign->pnt_hor = 5000;
  sign->pnt_base = 5000;
  sign->mark_vertic_end = 10000;
  sign->size_vertic = 10000;
  sign->mark_hor_end = 10000;
  sign->size_hor = 10000;
  sign->max_dim = 557057;
  sign->calcLength();

  sxfdoc->addObject(obj);
}

void addDecoratedLine(Sxf* sxfdoc,  const meteo::GeoPoint& /*mapCenter*/)
{
  QList<sxf::Metric> metric;
  metric.append(sxf::Object::metricFromGeoPoint(GeoPoint::fromDegree(60.2, 30)));
  metric.append(sxf::Object::metricFromGeoPoint(GeoPoint::fromDegree(60.2, 30.8)));
  sxf::Semantic sem("Decor", 9);

  sxf::Object obj;
  obj.addMetric(metric);
  obj.addSemantic(sem);

  QSharedPointer<sxf::DecoratedLine> dl = QSharedPointer<sxf::DecoratedLine>(new sxf::DecoratedLine);
  obj.addGraphicPrimitive(dl, sxf::kLinear);

  dl->base = QSharedPointer<sxf::GraphicPrimitive>(new sxf::LineGraphicObject);
  dl->base_num = sxf::SOLID_LINE;
  dl->base_length = 12;
  dl->calcLength();

  //TODO надо заполнить что-нить из объектов

  sxfdoc->addObject(obj);
}

void addLineFromClassif(Sxf* sxfdoc,  const meteo::GeoPoint&)
{
  QList<sxf::Metric> metric;
  metric.append(sxf::Object::metricFromGeoPoint(GeoPoint::fromDegree(60.2, 30)));
  metric.append(sxf::Object::metricFromGeoPoint(GeoPoint::fromDegree(60.2, 31)));
  sxf::Semantic sem("Decor", 9);

  sxf::Object obj;
  obj.addMetric(metric);
  obj.addSemantic(sem);
  int32_t code = 1154;
  obj.addWithClassificator(code, sxf::kLinear);

  sxfdoc->addObject(obj);

  //---

  metric.clear();
  metric.append(sxf::Object::metricFromGeoPoint(GeoPoint::fromDegree(60.1, 30)));
  metric.append(sxf::Object::metricFromGeoPoint(GeoPoint::fromDegree(60.1, 31)));
  sxf::Semantic sem1("NaborLine", 9);

  sxf::Object obj1;
  obj1.addMetric(metric);
  obj1.addSemantic(sem);
  code = 1;
  obj1.addWithClassificator(code, sxf::kLinear);

  sxfdoc->addObject(obj1);

}

// void addObject(Sxf* sxfdoc,  const meteo::GeoPoint& mapCenter)
// {  
//   sxfdoc->prepareSxfDocument();

//   //addTextClassif(sxfdoc, mapCenter);
//   addVector(sxfdoc, mapCenter);

//   //  addDecoratedLine(sxfdoc, mapCenter);

//   //addLineFromClassif(sxfdoc, mapCenter);
// }

void createObject(QByteArray* obj, QByteArray* v)
{
  rsc::Object object;
  object.length = 112;
  object.code = 90161;
  object.incode = 372;
  object.ident = 400;
  strcpy(object.short_name, "test");
  strcpy(object.name, "test");
  object.localization = 0;
  object.layer = 1;
  object.scale_flag = 0;
  object.vis_lo = 0;
  object.vis_hi = 0;
  object.ext_loc = 0;
  object.direct = 0;
  object.sem_flag = 0;
  object.snumber = 0;
  object.text_cnt = 0;
  object.contract_flag = 0;
  object.max_scale = 0;
  object.min_scale = 0;
  object.visible_flag = 0;
  object.reserved = 0;
  *obj = object.serializeToByteArray() + QByteArray(16, 0);//ещё 16 байт для связанной подписи
  var(obj->size());

  rsc::ObjectView view;
  view.length = 16;
  view.incode = 372;
  view.primitive = sxf::SOLID_LINE;//128

  sxf::LineGraphicObject* gobj = new sxf::LineGraphicObject();
  gobj->color = 0xf000000d;
  gobj->width = 250;
  //  view.obj = QSharedPointer<sxf::GraphicPrimitive>(gobj);
  *v = view.serializeToByteArray() + gobj->serializeToByteArray(false);
  var(v->size());

 delete gobj;
}

//изменение структуры классификатора meteoznaki_v1.rsc, чтоб дописываемая часть при добавлении объектов была в конце файла
void refillRsc(const QString& rscFile)
{
  QByteArray ba;
  QFile file(rscFile);
  file.open(QIODevice::ReadOnly);
  ba = file.readAll();
  file.close();


  rsc::Document rscdoc(ba);
  if (!rscdoc.isValid()) {
    error_log << QObject::tr("Error rsc");
    return;
  }

  QByteArray bheader(ba.left(328)); //.
  QByteArray btab(ba.mid(328, 4 + 72 + 8)); //. идентификатор + размер секции + отступ
  QByteArray bobj(ba.mid(412, 4 + 41552 + 12));

  QByteArray bcls(ba.mid(41980, 4 + 580 + 8));
  QByteArray bdef(ba.mid(42572, 4 + 96 + 12));
  QByteArray bsem(ba.mid(42684, 4 + 2772 + 8));
  QByteArray bpos(ba.mid(45468, 4 + 936 + 4));

  QByteArray blay(ba.mid(46412, 4 + 1800 + 4));
  //  QByteArray bparam(ba.mid(48220, 91024)); //.
  QByteArray bparam(ba.mid(48220, 4 + 90660 + 360)); //.
  QByteArray blim(ba.mid(139244, 4 + 0 + 12));
  QByteArray bprn(ba.mid(139260, 4 + 94416 + 364)); 
  QByteArray bcmy(ba.mid(234044, 4 + 1056 + 1532)); //1056 - странно, д.б. 1024
  QByteArray bfnm(ba.mid(236636, 4 + 1974));

  debug_log << "read parts";

  rsc::Header header(bheader);
  rsc::Tables tab(btab.right(btab.size() - 4));

  QList<rsc::Semantics*> sem;
  for (uint idx = 0 ; idx < header.sem_cnt; idx++) {
    rsc::Semantics* one = new rsc::Semantics(bsem.mid(4 + idx*one->size, one->size));
    if (one->sem_offset != 0) {
      one->sem_offset -= bobj.size();
    }
    if (one->defsem_offset != 0) {
      one->defsem_offset -= bobj.size();
    }
    sem << one;
  }

  var(header.length);
  var(tab.color_offset);
  var(tab.color_length);
  var(tab.color_cnt);
  var(tab.unk_offset);
  var(tab.unk_length);
  var(tab.unk_cnt);


  header.sem_offset -= bobj.size();
  header.clsem_offset -= bobj.size();
  header.default_offset -= bobj.size();
  header.probsem_offset -= bobj.size();
  header.layers_offset -= bobj.size();
  header.lim_offset -= bparam.size() + bobj.size();
  header.print_offset -= bparam.size() + bobj.size();

  header.pal_offset -= bparam.size() + bobj.size();//pal
  header.lib_offset -= bparam.size() + bobj.size();//iml
  header.font_offset -= bparam.size() + bobj.size(); //txt
  header.imsem_offset -= bparam.size() + bobj.size();//grs
  tab.color_offset -= bobj.size() + bparam.size();//cmy
  tab.unk_offset -= bobj.size() + bparam.size();  //fnm

  header.obj_offset += bcls.size() + bdef.size() + bsem.size() + bpos.size() + 
    blay.size() + blim.size() + bprn.size() + bcmy.size() + bfnm.size() + 6;
  header.param_offset += blim.size() + bprn.size() + bcmy.size() + bfnm.size() + 6;//6 - для выравнивания

  QByteArray res;
 
  res.append(btab.left(4));
  res.append(tab.serializeToByteArray() + QByteArray(8, 0));

  res.append(bcls);
  res.append(bdef);

  //res.append(bsem);
  res.append(bsem.left(4));
  for (uint idx = 0; idx < header.sem_cnt; idx++) {
    res.append(sem.at(idx)->serializeToByteArray());
  }
  res.append(QByteArray(8, 0));

  res.append(bpos);
  res.append(blay);

  res.append(blim);

  res.append(bprn);

  res.append(bcmy);

  res.append(bfnm);
  res.append(QByteArray(6, 0));

  QByteArray obj;
  QByteArray view;
  createObject(&obj, &view);

  res.append(bobj.left(bobj.size() - 12));
  res.append(obj);
  res.append(QByteArray(12, 0));

  res.append(bparam.left(bparam.size() - 360));
  res.append(view);
  //  res.append(bparam.right(360));

  header.obj_len += obj.size();
  header.obj_cnt += 1;
  header.param_offset += obj.size();
  header.param_len += view.size();
  header.param_cnt += 1;

  int size = header.serializeToByteArray().size();
  header.length = res.length() + size;
  res.prepend(header.serializeToByteArray());


  file.setFileName(rscFile + ".tmp.rsc");
  file.open(QIODevice::WriteOnly);
  file.write(res);
  file.close();

  return;
}

//чтоб воспользоваться, надо заменить в rsc::Document::Document(const QByteArray& ba) readViews(ba) на readAndChangeViews(ba)
void changeRscView(const QString& rscFile)
{
  QByteArray ba;
  QFile file(rscFile);
  file.open(QIODevice::ReadOnly);
  ba = file.readAll();
  file.close();


  rsc::Document rscdoc(ba);
  if (!rscdoc.isValid()) {
    error_log << QObject::tr("Error rsc");
    return;
  }

  QByteArray bheader(ba.left(0xc9cc)); //.
  QByteArray bview(ba.mid(0xc9cc, 0x242fc-0xc9cc)); //. идентификатор + размер секции + отступ. Секция PAR
  QByteArray btail(ba.right(0x25bc3 + 1 - 0x242fc)); //от секции LIM

  QByteArray nview = rscdoc.viewba();
  
  debug_log << bview.size() << nview.size();

  QByteArray res;
  res.append(bheader);
  //res.append(bview);
  res.append(bview.left(4));
  res.append(nview);
  if (bview.size() > nview.size() + 4) {
    // res.append(bview.right(bview.size() - nview.size() - 4));
    res.append(QByteArray(bview.size() - nview.size() - 4, 0));
    debug_log << bview.size() - nview.size() - 4;
  }
  res.append(btail);

  file.setFileName(rscFile + ".tmp.rsc");
  file.open(QIODevice::WriteOnly);
  file.write(res);
  file.close();
}

int main(int argc, char** argv)
{
  TAPPLICATION_NAME("meteo");

  /*
  //так больше не работает
  map::Map* map =  map::singleton::Map::instance();
  meteo::GeoPoint mapCenter = meteo::GeoPoint::fromDegree(60.0, 30.0);
  map->setCurrentParams(meteo::MERCAT, mapCenter);

  map::Document* doc = map->createDocument(QSize(1024, 1024), mapCenter);
  doc->setScale(14);

  map::Layer* lay = new map::Layer(doc, QString::fromUtf8("Имя слоя"));
  addObjects(lay);
  QStringList layerUuids(  lay->uuid());

  Sxf sxfdoc;
  QSharedPointer<meteo::sxf::PassportBasis> basis = QSharedPointer<sxf::PassportBasis>(new sxf::PassportBasis());
  fillPassport(basis, doc);
  sxfdoc.setPassportBasis(basis);

  addObject(&sxfdoc, mapCenter);

  QByteArray exported(sxfdoc.serializeToByteArray()); //набор объектов
  //QByteArray exported(sxfdoc.exportDocument(layerUuids, *doc)); //текст и линия

  QString errorMsg(sxfdoc.lastError());

  if (!errorMsg.isEmpty()) {
    debug_log << QString::fromUtf8("Сообщение об ошибках: '%1'").arg(errorMsg);
  }
  debug_log << QString::fromUtf8("Результат экспорта:") << (exported.isEmpty() ? "False" : "True");

  QFile f("export.sxf");
  if (f.open(QIODevice::WriteOnly) == true) {
    f.write(exported);
  }
  f.close();
  */

 
  QString fileName;
  if (argc < 2) {
    error_log<<"Need point file name. \""<<argv[0]<<"-h\" for help";
    exit(-1);
  } else {
    fileName = argv[1];
  }
  
  QApplication app(argc, argv);

  //refillRsc(fileName);
  changeRscView(fileName);

 Q_UNUSED(app);
 
  return EXIT_SUCCESS;
}
