#include "oceandiag_doc.h"

#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/generalproj.h>
#include <commons/geobasis/geopoint.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ocean/placedata.h>
#include <meteo/commons/proto/meteo.pb.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/event.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/view/actions/action.h>
#include <meteo/commons/ui/map/view/actions/infotextaction.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/graph/funcs.h>
#include <meteo/commons/ui/map/graph/ramkagraph.h>
#include <meteo/commons/ui/map/graph/layergraph.h>


const float MAX_H = 2000.;
const float MAX_T = 70.;

namespace meteo {
namespace odiag {

static float maxH = 31000.;

bool OceanDiagDoc::logF2X_one( const Projection& proj, const GeoPoint& geoCoord, QPoint* meterCoord )
{
  //GeoPoint start = GeoPoint(-100,1800,0,LA_GENERAL);
  //GeoPoint end = GeoPoint(50,5,0,LA_GENERAL);
  float lon = geoCoord.lon() - proj.getMapCenter().lon();
  float lat = geoCoord.lat() - proj.getMapCenter().lat();
  int x = kRVSG*lat*proj.xfactor();
  int y = kRVSG*(lon*proj.yfactor());

  meterCoord->setX(x);
  meterCoord->setY(y);

  if ( 0 > x && ::abs(x) == x ) {
    return false;
  }
  if ( 0 > y && ::abs(y) == y ) {
    return false;
  }
  return true;
}

bool OceanDiagDoc::logX2F_one( const Projection& proj, const QPoint& meterCoord, GeoPoint* geoCoord )
{
  if ( nullptr == geoCoord ) {
    error_log << QObject::tr("Нулевой указатель GeoPoint*");
    return false;
  }
  *geoCoord = GeoPoint( 0,0,0, LA_GENERAL );

  float x = meterCoord.x();
  float y = meterCoord.y();
  float lat = proj.getMapCenter().lat() + x/( kRVSG*proj.xfactor() );
  float lon = proj.getMapCenter().lon() + y/( kRVSG*proj.yfactor() ) /* + p */ ;


  geoCoord->setLat(lat);
  geoCoord->setLon(lon);

  return true;
}


bool OceanDiagDoc::logF2X_onef( const Projection& proj, const GeoPoint& geoCoord, QPointF* meterCoord )
{
  //GeoPoint start = GeoPoint(-100,1800,0,LA_GENERAL);
  //GeoPoint end = GeoPoint(50,5,0,LA_GENERAL);
  float lon = geoCoord.lon() - proj.getMapCenter().lon();
  float lat = geoCoord.lat() - proj.getMapCenter().lat();

  float x = kRVSG*lat*proj.xfactor();
  float y = kRVSG*(lon*proj.yfactor());

  meterCoord->setX(x);
  meterCoord->setY(y);
  return true;
}

bool OceanDiagDoc::logX2F_onef( const Projection& proj, const QPointF& meterCoord, GeoPoint* geoCoord )
{
  if ( nullptr == geoCoord ) {
    error_log << QObject::tr("Нулевой указатель GeoPoint*");
    return false;
  }
  *geoCoord = GeoPoint( 0,0,0, LA_GENERAL );

  float x = meterCoord.x();
  float y = meterCoord.y();

  float lat = proj.getMapCenter().lat() + x/( kRVSG*proj.xfactor() );
  float lon = proj.getMapCenter().lon() + y/( kRVSG*proj.yfactor() ) /* + p */ ;


  geoCoord->setLat(lat);
  geoCoord->setLon(lon);

  return true;
}

OceanDiagDoc::OceanDiagDoc(meteo::map::Document* doc, meteo::map::MapScene* scene):
  doc_(doc),
  scene_(scene)
{
  layerT_ = nullptr;
  layerS_ = nullptr;
  layerC_ = nullptr;
  layerGrid_ = nullptr;
  layerPZK_  = nullptr;
  has_data_ = false;
  float Yfactor = 0.02f;
  GeoPoint dc = GeoPoint(0,0,0,LA_GENERAL);
  meteo::GeneralProj* proj = dynamic_cast<meteo::GeneralProj*>(doc_->projection());
  proj->setMapCenter(dc);
  GeoPoint start = GeoPoint(70,0,0,LA_GENERAL);
  GeoPoint end = GeoPoint(0,MAX_H,0,LA_GENERAL);

  maxH = end.la();
  proj->setStart(start);
  proj->setEnd(end);
  proj->setYfactor(Yfactor);

  proj->setFuncTransform( logF2X_one, logX2F_one);
  proj->setFuncTransform( logF2X_onef, logX2F_onef);
  proj->setRamka(start.fi(),start.la(),end.fi(),end.la());

  QString name = QString("Диаграмма состояния моря");
  doc_->setName(name);
  doc_->setScale(16.0);

  RamkaGraph* ramka = new RamkaGraph(doc_);
  doc_->setRamka(ramka);

  if ( nullptr != doc_ && nullptr != doc_->eventHandler() ) {
    connect(doc_->eventHandler(), SIGNAL(layerChanged(map::Layer*, int)), this, SLOT(layerChanged(map::Layer*, int)));
  }
}

OceanDiagDoc::~OceanDiagDoc()
{
}

void OceanDiagDoc::showNoData()
{
  disconnect(doc_->eventHandler(), SIGNAL(layerChanged(map::Layer*, int)), this, SLOT(layerChanged(map::Layer*, int)));

  createEmptyBlank();

  if (! scene_->hasAction("infotextaction")) {
    QPoint pnt = doc_->coord2screen(doc_->screenCenter());
    map::InfoTextAction* act = new map::InfoTextAction(QObject::tr("<НЕТ ДАННЫХ>"), pnt, scene_);
    act->setTextSize(48);
    act->setTextColor(Qt::gray);
    scene_->addAction(act);
  }
  connect(doc_->eventHandler(), SIGNAL(layerChanged(map::Layer*, int)), this, SLOT(layerChanged(map::Layer*, int)));
}

void OceanDiagDoc::hideNoData()
{
  map::Action* act = scene_->takeAction("infotextaction");
  if (nullptr != act) {
    delete act;
  }
}

float OceanDiagDoc::StoT(float S)
{
  return (S-32.5)*10.;
}

float OceanDiagDoc::CtoT(float c)
{
  return (c-1420.)/4.;
}

void OceanDiagDoc::createLayerT()
{
  if ( nullptr != layerT_ ) {
    delete layerT_;
    layerT_ = nullptr;
  }
  layerT_ = new map::LayerGraph(doc_, QObject::tr("Температура"));

  meteo::Property prop;
  prop.mutable_pen()->set_width(2);
  prop.mutable_pen()->set_color(qRgba(255,0,0,255));

  layerT_->setGraphProperty(prop);

  AxisGraph* y = layerT_->yAxis();
  y->setPosition(AxisGraph::kLeft);
  y->setTickLabelFormat("f,0");
  y->setRange(0,2000);
  y->setLabel("Глубина, м",AxisGraph::ktpEnd);

  AxisGraph* x = layerT_->xAxis();
  x->setPosition(AxisGraph::kTop);
  x->setTickLabelFormat("f,1");
 x->setLabel("T,C°",AxisGraph::ktpBegin, AxisGraph::LabelAlign::pLeft);
  x->setRange(-5,70);

  prop.mutable_pen()->set_width(1);
  prop.mutable_pen()->set_color(qRgba(0,128,0,255));
  y->setProperty(prop);

  QVector<float> ticks;
  for ( int i=-5; i<=70; i+=5 ) {
    ticks << i;
  }
  x->setTickVector(ticks);
}

void OceanDiagDoc::createLayerS()
{
  if ( nullptr != layerS_ ) {
    delete layerS_;
    layerS_ = nullptr;
  }

  layerS_ = new map::LayerGraph(doc_, QObject::tr("Солёность"));

  meteo::Property prop;
  prop.mutable_pen()->set_width(2);
  prop.mutable_pen()->set_color(qRgba(0,128,0,255));

  layerS_->setGraphProperty(prop);

  AxisGraph* y = layerS_->yAxis();
  y->setPosition(AxisGraph::kLeft);
  y->setTickLabelFormat("f,0");
  y->setRange(0,2000);
  y->setLabel("Глубина, м",AxisGraph::ktpEnd);

  AxisGraph* x = layerS_->xAxis();
  x->setPosition(AxisGraph::kTop);
  x->setTickLabelFormat("f,1");
  x->setLabel("Солёность, ‰",AxisGraph::ktpBegin, AxisGraph::LabelAlign::pLeft);// ‰ - Знак промилле (U+2030)
  x->setRange(32,39.5);

  prop.mutable_pen()->set_width(1);
  y->setProperty(prop);

  QVector<float> ticks;
  for ( float i=32; i<=39.5; i+=0.5 ) {
    ticks << i;
  }
  x->setTickVector(ticks);
}

void OceanDiagDoc::createLayerC()
{
  if ( nullptr != layerC_ ) {
    delete layerC_;
    layerC_ = nullptr;
  }

  layerC_ = new map::LayerGraph(doc_, QObject::tr("Скорость звука"));

  meteo::Property prop;
  prop.mutable_pen()->set_width(2);
  prop.mutable_pen()->set_color(qRgba(0,0,128,255));

  layerC_->setGraphProperty(prop);

  AxisGraph* y = layerC_->yAxis();
  y->setPosition(AxisGraph::kLeft);
  y->setTickLabelFormat("f,0");
  y->setRange(0,2000);
  y->setLabel("Глубина, м",AxisGraph::ktpEnd);

  AxisGraph* x = layerC_->xAxis();
  x->setPosition(AxisGraph::kTop);
  x->setTickLabelFormat("f,0");
  x->setLabel("Скорость звука, м/с",AxisGraph::ktpBegin, AxisGraph::LabelAlign::pLeft);
  x->setRange(1400,1700);

  prop.mutable_pen()->set_width(1);
  prop.mutable_pen()->set_color(qRgba(0,128,0,255));
  y->setProperty(prop);

  QVector<float> ticks;
  for ( int i=1400; i<=1700; i+=20 ) {
    ticks << i;
  }
  x->setTickVector(ticks);
}

void OceanDiagDoc::createLayerGrid()
{
  if ( nullptr != layerGrid_ ) {
    delete layerGrid_;
    layerGrid_ = nullptr;
  }
  layerGrid_ = new map::LayerGraph(doc_, QObject::tr("Бланк диаграммы состояния моря"));
  layerGrid_->setBase(true);

  meteo::Property prop;
  prop.mutable_pen()->set_color(qRgba(255,0,0,255));

  layerGrid_->setGraphProperty(prop);

  AxisGraph* y = layerGrid_->yAxis();
  y->setPosition(AxisGraph::kRight);
  y->setTickLabelFormat("f,0");
  y->setRange(0,2000);

  AxisGraph* x = layerGrid_->xAxis();
  x->setPosition(AxisGraph::kTop);
  x->setTickLabelFormat("f,1");
  x->setLabel("T, C°",AxisGraph::ktpBegin, AxisGraph::LabelAlign::pLeft);
  x->setRange(-5,70);

  prop.mutable_pen()->set_width(1);
  prop.mutable_pen()->set_color(qRgba(0,128,0,255));
  y->setProperty(prop);

  QVector<float> ticks;
  for ( int i=-5; i<=70; i+=5 ) {
    ticks << i;
  }
  x->setTickVector(ticks);
}

bool OceanDiagDoc::createEmptyBlank()
{
  if ( nullptr != layerPZK_ ) {
    delete layerPZK_;
    layerPZK_ = nullptr;
  }

  createLayerGrid();
  createGrid();

  createLayerT();
  createLayerS();
  createLayerC();

  Projection* proj = doc_->projection();

  GeoPoint beg = proj->start();
  GeoPoint end = proj->end();
  QSizeF sz(qAbs(beg.lat() - end.lat()), qAbs(beg.lon() - end.lon()));

  QVector<float> yTicks;
  for ( int i=0; i<=2000; i+=200 ) {
    yTicks << i;
  }

  QList<AxisGraph*> xAxis;
  QList<AxisGraph*> yAxis;

  xAxis << layerT_->xAxis() << layerS_->xAxis() << layerC_->xAxis() << layerGrid_->xAxis();
  yAxis << layerT_->yAxis() << layerS_->yAxis() << layerC_->yAxis() << layerGrid_->yAxis();

  for ( int i=0,isz=xAxis.size(); i<isz; ++i ) {
    AxisGraph* y = yAxis[i];
    y->setMapSize(sz);
    y->setTickVector(yTicks);

    AxisGraph* x = xAxis[i];
    x->setMapSize(sz);
  }

  return true;
}

bool OceanDiagDoc::createGrid()
{
  if ( nullptr == layerGrid_ ) { return false; }

  AxisGraph* x = layerGrid_->xAxis();
  AxisGraph* y = layerGrid_->yAxis();

  float minH = 0.;
  float maxH = MAX_H;
  float minT = -5.;
  float maxT = 70.;
  float h=-999.,t=-999.;
  meteo::Property prop;

  meteo::Property prop1;
  QPen pen;
  pen.setColor(qRgb(70, 250, 250));
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(2);
  prop1.mutable_pen()->CopyFrom( qpen2pen(pen) );
  meteo::Property prop3;
  pen.setColor(qRgb(100, 240, 240));
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(1);
  prop3.mutable_pen()->CopyFrom( qpen2pen(pen) );

  GeoVector skelet;
  for(h=minH;h<=maxH;h=h+40.){
    skelet.append(GeoPoint(x->scale2coord(minT),y->scale2coord(h),0,LA_GENERAL));
    skelet.append(GeoPoint(x->scale2coord(maxT),y->scale2coord(h),0,LA_GENERAL));
    if((static_cast<int>(h)%200)==0) prop=prop1;
    else prop=prop3;

    map::GeoPolygon* line = new map::GeoPolygon(layerGrid_);
    line->setProperty(prop);
    line->setSkelet(skelet);

    skelet.clear();
  }
  for(t=minT;t<=maxT;t=t+1.f){
    skelet.append(GeoPoint(x->scale2coord(t),y->scale2coord(minH),0,LA_GENERAL));
    skelet.append(GeoPoint(x->scale2coord(t),y->scale2coord(maxH),0,LA_GENERAL));
    if((static_cast<int>(t)%5)==0) prop=prop1;
    else prop=prop3;

    map::GeoPolygon* line = new map::GeoPolygon(layerGrid_);
    line->setProperty(prop);
    line->setSkelet(skelet);

    skelet.clear();
  }

  return true;
}

bool OceanDiagDoc::addChannel(float xe0_)
{
  if ( nullptr == doc_ ) { return false; }

  if ( nullptr != layerPZK_ ) {
    delete layerPZK_;
    layerPZK_ = nullptr;
  }

  layerPZK_ = new map::Layer(doc_);
  layerPZK_->setName(tr("Подводный звуковой канал"));
 // layerPZK_->setShotName(tr("Подводный звуковой канал"));


  QMap<float,float> speeds;
  float h=-999.,t=-999.,s=-999.,c=-999.;
  QMap<float, ocean::Uroven>::const_iterator itb = pd_->ocean().urovenList().begin();
  QMap<float, ocean::Uroven>::const_iterator ite = pd_->ocean().urovenList().end();
  for ( ; itb != ite; ++itb ) {
    if(!itb.value().isGood(ocean::UR_S) || !itb.value().isGood(ocean::UR_T)) continue;
    h = itb.key();
    //if(h>1600.) break;
    t = itb.value().value(ocean::UR_T);
    s = itb.value().value(ocean::UR_S);
    c = 1492.9 + 3 * ( t - 10. ) -
        0.006 * pow((t-10.),2) -
        0.04 * pow((t-18.),2) +
        1.2 * ( s - 35. ) -
        0.01 * ( t - 18. ) * ( s - 35. ) + h/61.;
    speeds.insert(h,c);
  }
  if(speeds.size() < 2) return false;

  QMap<float,float>::const_iterator itb_ = speeds.begin();
  QMap<float,float>::const_iterator ite_ = speeds.end();
  QMap<float,float>::const_iterator it_cur;

  /*
  QMap<float,float>::const_iterator it_prev;
  int sign_dC = 0, sign_dC_prev = 0;
  float dC = 0.;
  //прорежение массива точек скорости звука по глубинам (оставляем только максимумы и минимумы)
  for(it_cur = itb_+1, it_prev = itb_; it_cur != ite_ ; ++it_cur) {
    dC = it_cur.value() - it_prev.value();
    if(dC < 0.) {
      sign_dC = -1;
    }
    else {
      sign_dC = 1;
    }
    if((sign_dC*sign_dC_prev)>0) speeds.remove(it_prev.key());
    it_prev = it_cur;
    sign_dC_prev = sign_dC;
  }
  */

  float ur_min_c = -999., min_c = -999., ur_max_c_up = -999., ur_max_c_down = -999., max_c_up = -999., max_c_down = -999.;
  float ur_gr_up = -999., ur_gr_down = -999., c_gr = -999.;
  itb_ = speeds.begin();
  ite_ = speeds.end();

 // var(speeds);

  //поиск минимума скорости звука
  min_c = itb_.value();
  ur_min_c = itb_.key();
  for(it_cur = itb_+1; it_cur != ite_ ; ++it_cur) {
    if(it_cur.value() < min_c) {
      min_c = it_cur.value();
      ur_min_c = it_cur.key();
    }
  }
  //  debug_log << "min_c = " << min_c << "  ur_min_c = " << ur_min_c;
  QMap<float,float>::const_iterator it_min_c;
  if ( false == MnMath::isEqual(ur_min_c,itb_.key())) {
    it_min_c = speeds.lowerBound(ur_min_c);
  //поиск верхнего максимума скорости звука
    max_c_up = itb_.value();
    ur_max_c_up = itb_.key();
    for(it_cur = itb_+1; it_cur != it_min_c ; ++it_cur) {
      if(it_cur.value() > max_c_up) {
  max_c_up = it_cur.value();
  ur_max_c_up = it_cur.key();
      }
    }
  } else {
    it_min_c = itb_;
    max_c_up = min_c;
    ur_max_c_up = ur_min_c;
  }


  //поиск нижнего максимума скорости звука
  max_c_down = it_min_c.value();
  ur_max_c_down = it_min_c.key();
  for(it_cur = it_min_c+1; it_cur != ite_ ; ++it_cur) {
    if(it_cur.value() > max_c_down) {
      max_c_down = it_cur.value();
      ur_max_c_down = it_cur.key();
    }
  }

  ///если задан угол выхода луча, вместо верхнего и нижнего максимумов скорости звука, считаются верхняя и нижняя границы заданного луча и скорость звука на них
  if( false ==  MnMath::isEqual(xe0_,-999.f)) {
    float max_c = min_c / cos(xe0_);

    //поиск верхней границы луча
    max_c_up = it_min_c.value();
    ur_max_c_up = it_min_c.key();
    if (it_min_c != itb_) {
      for(it_cur = it_min_c-1; it_cur != itb_ ; --it_cur) {
  if(it_cur.value() >= max_c) {
    max_c_up = it_cur.value();
    ur_max_c_up = it_cur.key();
    //debug_log << "max_c_up = " << max_c_up << "  ur_max_c_up = " << ur_max_c_up;
    break;
  }
      }
    }
    //поиск нижней границы луча
    max_c_down = it_min_c.value();
    ur_max_c_down = it_min_c.key();
    if (it_min_c != ite_ - 1) {
      for(it_cur = it_min_c+1; it_cur != ite_ ; ++it_cur) {
  if(it_cur.value() >= max_c) {
    max_c_down = it_cur.value();
    ur_max_c_down = it_cur.key();
    //debug_log << "max_c_down = " << max_c_down << "  ur_max_c_down = " << ur_max_c_down;
    break;
  }
      }
    }

  }

  //определение верхнего и нижнего граничных уровней и скорости звука на них
  if(max_c_up < max_c_down) {
    c_gr = max_c_up;
    ur_gr_up = ur_max_c_up;
    for(it_cur = it_min_c; it_cur != ite_ ; ++it_cur) {
      if(it_cur.value() > c_gr) {
        float a=-999., b=-999.;
        a = ( it_cur.key()-(it_cur-1).key() ) / ( it_cur.value() - (it_cur-1).value() );
        b = ( it_cur.value() * (it_cur-1).key() - (it_cur-1).value() * it_cur.key() ) / ( it_cur.value() - (it_cur-1).value() );
        ur_gr_down = a * c_gr + b;
        break;
      }
    }
  }
  else {
    c_gr = max_c_down;
    ur_gr_down = ur_max_c_down;
    for(it_cur = it_min_c; it_cur != itb_ ; --it_cur) {
      if(it_cur.value() > c_gr) {
        float a=-999., b=-999.;
        a = ( it_cur.key()-(it_cur+1).key() ) / ( it_cur.value() - (it_cur+1).value() );
        b = ( it_cur.value() * (it_cur+1).key() - (it_cur+1).value() * it_cur.key() ) / ( it_cur.value() - (it_cur+1).value() );
        ur_gr_up = a * c_gr + b;
        break;
      }
    }
  }

  if(MnMath::isEqual(ur_min_c, ur_max_c_down)
     || MnMath::isEqual(ur_min_c, ur_max_c_up)
     || MnMath::isEqual(ur_max_c_down, ur_max_c_up)) {
    GeoVector gv;
    gv.append(GeoPoint(70.,0.,0,LA_GENERAL));

    ::meteo::map::GeoText* label = new ::meteo::map::GeoText(layerPZK_);
    label->setPenColor(Qt::gray);
    label->setFontPointSize(24);
    label->setPos(kBottomLeft);
    label->setDrawAlways(true);
    label->setText(QObject::tr("<ПЗК отсутствует>"));
    label->setSkelet(gv);
    return true;
  }

  GeoVector lineUpSkelet;
  GeoVector lineDownSkelet;
  GeoVector axisSkelet;
  GeoVector raySkelet;
  float r0=-999., r=-999., r1=-999.,xe0=-999., xe=-999., z0=-999., zgr=-999., z=-999, z1=-999., c0=-999., cgr=-999., c1=-999;
  float c_=-999, z_=-999.;
  c=-999;

  float a=-999, b=-999.;
  float count = 100.;
  float ri = -999., riSumm = -999;

  //рисуем верхнюю границу ПЗК
  lineUpSkelet.append(GeoPoint(layerT_->xAxis()->scale2coord(CtoT(c_gr)), ur_gr_up, layerT_->yAxis()->scale2coord(0), LA_GENERAL));
  lineUpSkelet.append(GeoPoint(layerT_->xAxis()->scale2coord(MAX_T), layerT_->yAxis()->scale2coord(ur_gr_up), 0, LA_GENERAL));
  map::GeoPolygon* channelUpGr = new map::GeoPolygon(layerPZK_);
  channelUpGr->setPenWidth(2);
  channelUpGr->setPenColor(Qt::darkMagenta);
  channelUpGr->setPenStyle(Qt::DashLine);
  channelUpGr->setSkelet(lineUpSkelet);
  lineUpSkelet.clear();

  //рисуем нижнюю границу ПЗК
  lineDownSkelet.append(GeoPoint(layerT_->xAxis()->scale2coord(CtoT(c_gr)), layerT_->yAxis()->scale2coord(ur_gr_down), 0, LA_GENERAL));
  lineDownSkelet.append(GeoPoint(layerT_->xAxis()->scale2coord(MAX_T), layerT_->yAxis()->scale2coord(ur_gr_down), 0, LA_GENERAL));
  map::GeoPolygon* channelDownGr = new map::GeoPolygon(layerPZK_);
  channelDownGr->setPenWidth(2);
  channelDownGr->setPenColor(Qt::darkMagenta);
  channelDownGr->setPenStyle(Qt::DashLine);
  channelDownGr->setSkelet(lineDownSkelet);
  lineDownSkelet.clear();

  //рисуем ось ПЗК
  axisSkelet.append(GeoPoint(layerT_->xAxis()->scale2coord(CtoT(min_c)), layerT_->yAxis()->scale2coord(ur_min_c), 0, LA_GENERAL));
  axisSkelet.append(GeoPoint(layerT_->xAxis()->scale2coord(MAX_T), layerT_->yAxis()->scale2coord(ur_min_c), 0, LA_GENERAL));
  map::GeoPolygon* channelAxis = new map::GeoPolygon(layerPZK_);
  channelAxis->setPenWidth(2);
  channelAxis->setPenColor(Qt::magenta);
  channelAxis->setPenStyle(Qt::DashLine);
  channelAxis->setSkelet(axisSkelet);
  axisSkelet.clear();

  float rayScale = 1000.;//TODO отношение значений шкалы расстояния к шкале температуры (для красивого отображения на диаграмме)
  r0=CtoT(min_c) * rayScale; //начальное значение в точке источника звукового луча
  r1=r0; //начальное значение в промежуточной точке источника звукового луча

  z=99999.;
  z0=ur_min_c; //горизонт оси ПЗК
  zgr=ur_gr_up; // горизонт верхней границы ПЗК
  z1 = z0; // промежуточное начальное значение глубины

  c0=min_c; //скорость звука на оси ПЗК
  cgr=c_gr; //скорость звука на границе ПЗК
  c1 = c0; // промежуточное начальное значение скорости звука
  xe0=sqrt(2 * (cgr - c0) / c0); //начальный угол выхода звукового луча из источника по отношению к оси (приравнивается к значению угла выхода граничного луча)
  xe=xe0; // промежуточное начальное значение угла выхода звукового луча

  a=(zgr-z0)/(cgr-c0); // параметр a линейно-аппроксимированного закона распределения скорости звука с глубиной от z0 до zgr
  b=(cgr*z0-c0*zgr)/(cgr-c0); // параметр b линейно-аппроксимированного закона распределения скорости звука с глубиной от z0 до zgr

  //!если существует, то рисуется только приповерхностный звуковой канал
  //определяем нижнюю границу для луча приповерхностного звукового канала
  if(MnMath::isEqual(ur_min_c, itb_.key())) {

    rayScale = rayScale*0.1f;
    r0=CtoT(min_c) * rayScale; //начальное значение в точке источника звукового луча
    r1=r0; //начальное значение в промежуточной точке источника звукового луча

    if(MnMath::isEqual(ur_gr_down,(ite_-1).key())) {
      for(it_cur = it_min_c+1; it_cur != ite_ ; ++it_cur) {
        if(it_cur.value() < (it_cur-1).value()) {
          ur_gr_down = (it_cur-1).key();
          break;
        }
      }
      if(MnMath::isEqual(ur_gr_down, (ite_-1).key())) {
        ur_gr_down = 1000.;
      }
    }
    //!переопределяем параметры распределения скорости звука от z0 до нижней zgr
    z=99999.;
    z0=ur_min_c; //горизонт оси ПЗК
    zgr=ur_gr_down; // горизонт нижней границы ПЗК
    z1 = z0; // промежуточное начальное значение глубины

    c0=min_c; //скорость звука на оси ПЗК
    cgr=c_gr; //скорость звука на границе ПЗК
    c1 = c0; // промежуточное начальное значение скорости звука
    xe0=sqrt(2 * (cgr - c0) / c0); //начальный угол выхода звукового луча из источника по отношению к оси (приравнивается к значению угла выхода граничного луча)
    xe=xe0; // промежуточное начальное значение угла выхода звукового луча

    a=(zgr-z0)/(cgr-c0); // параметр a линейно-аппроксимированного закона распределения скорости звука с глубиной от z0 до zgr
    b=(cgr*z0-c0*zgr)/(cgr-c0); // параметр b линейно-аппроксимированного закона распределения скорости звука с глубиной от z0 до zgr

    //    debug_log << r/rayScale << MAX_T;

    for(;r/rayScale<MAX_T;) {
      //!расчет 1 куска траектории зукового луча приповерхностного звукового канала
      for(z=z0, z1=z0, c1=c0;( z < zgr)&&( r/rayScale < MAX_T);z = z + 1.f) {
        riSumm = 0.;
        c=(z-b)/a;
        xe=sqrt(2 * (cgr - c) / c);

        //считаем интеграл численным методом (сумма площадей трапеций) (формула 10.4 "Физические основы распространения звука в океане")
        for(float i=0.;i<=count;i=i+1.) { //! @param count - число равных частей, на которые разбивается отрезок интегрирования
          z_ = z1 + i * (z - z1) / count + 0.5 * (z - z1) / count;
          c_ = (z_-b)/a;
          ri = cos(xe) / sqrt(fabs( c1 / c_ - pow(cos(xe),2) ));
          if(MnMath::isZero(i) ||MnMath::isEqual(i,count)){
              ri = ri *0.5f;
            }
          riSumm = riSumm + ri;
        }
        r = (fabs( z - z1 ) / count) * riSumm + r1;//полученное значение интеграла + r1

        raySkelet.append(GeoPoint(layerT_->xAxis()->scale2coord(r/rayScale),layerT_->yAxis()->scale2coord(z),0,LA_GENERAL));
       // debug_log << "r = " << r << " z = " << z << r/rayScale;
        c1 = c;
        r1 = r;
        z1 = z;
      }

      //!расчет 2 куска траектории зукового луча приповерхностного звукового канала
      for(z=z1,z1=z1,c1=c;(z>z0)&&(r/rayScale<MAX_T);z=z-1.) {
        riSumm = 0.;
        c=(z-b)/a;
        xe=sqrt(2 * (cgr - c) / c);

        //считаем интеграл численным методом (сумма площадей трапеций) (формула 10.4 "Физические основы распространения звука в океане")
        for(float i=0.;i<=count;i=i+1.) { //! @param count - число равных частей, на которые разбивается отрезок интегрирования
          z_ = z1 + i * (z - z1) / count + 0.5 * (z - z1) / count;
          c_ = (z_-b)/a;
          ri = cos(xe) / sqrt(fabs( c1 / c_ - pow(cos(xe),2) ));
          if(MnMath::isZero(i) || MnMath::isEqual(i,count)) ri = ri / 2.;
          riSumm = riSumm + ri;
        }
        r = (fabs( z - z1 ) / count) * riSumm + r1;//полученное значение интеграла + r1

        raySkelet.append(GeoPoint(layerT_->xAxis()->scale2coord(r/rayScale),layerT_->yAxis()->scale2coord(z),0,LA_GENERAL));
        //debug_log << "r = " << r << " z = " << z << r/rayScale;
        c1 = c;
        r1 = r;
        z1 = z;
      }

    }

    //    var(raySkelet.size());
    //рисуем траекторию звукового луча
    map::GeoPolygon* channelRay = new map::GeoPolygon(layerPZK_);
    channelRay->setPenWidth(2);
    channelRay->setPenColor(Qt::magenta);
    channelRay->setSkelet(raySkelet);
    raySkelet.clear();

    emit PZK_Exist(xe0);

    return true;
  }

  //  debug_log << MAX_T << zgr << ur_gr_down << ur_gr_up;
  //!расчет 1 куска траектории зукового луча (интеграл считается кусочно)
  for(z=z0,z1=z0,c1=c0;(z>zgr)&&(r/rayScale<MAX_T);z=z-1.) {
    riSumm = 0.;
    c=(z-b)/a;
    xe=sqrt(2 * (cgr - c) / c);

    //считаем интеграл численным методом (сумма площадей трапеций) (формула 10.4 "Физические основы распространения звука в океане")
    for(float i=0.;i<=count;i=i+1.) { //! @param count - число равных частей, на которые разбивается отрезок интегрирования
      z_ = z1 + i * (z - z1) / count + 0.5 * (z - z1) / count;
      c_ = (z_-b)/a;
      //debug_log << "abs = " << abs( c1 / c_ - pow(cos(xe),2) );
      //debug_log << "sqrt = " << sqrt(abs( c1 / c_ - pow(cos(xe),2) ));
      ri = cos(xe) / sqrt(fabs( c1 / c_ - pow(cos(xe),2) ));
      if(MnMath::isZero(i) ||MnMath::isEqual(i,count)){
          ri = ri *0.5f;
        }
      riSumm = riSumm + ri;
    }
    r = (fabs( z - z1 ) / count) * riSumm + r1;//полученное значение интеграла + r1

    raySkelet.append(GeoPoint(layerT_->xAxis()->scale2coord(r/rayScale),layerT_->yAxis()->scale2coord(z),0,LA_GENERAL));
    //debug_log << "r = " << r << " z = " << z << r/rayScale;
    c1 = c;
    r1 = r;
    z1 = z;
  }

  /*
  //!расчет 1 куска траектории зукового луча (интеграл считается каждый раз до рассчитываемой точки)
  for(z=z0,z1=z0,c1=c0;(z>zgr)&&(r/rayScale<MAX_T);z=z-1.) {
    riSumm = 0.;
    //считаем интеграл численным методом (формула 10.4 "Физические основы распространения звука в океане")
    for(float i=0.;i<=count;i=i+1.) { //! @param count - число равных частей, на которые разбивается отрезок интегрирования
      z_ = z0 + i * (z - z0) / count;
      c_ = (z_-b)/a;
      ri = cos(xe0) / sqrt( c0 / c_ - pow(cos(xe0),2) );
      if(i==0. || i==count) ri = ri / 2.;
      riSumm = riSumm + ri;
    }
    r = (abs( z - z0 ) / count) * riSumm + r0;//полученное значение интеграла + r0

    raySkelet.append(GeoPoint(r/rayScale,z,0,LA_GENERAL));
  }
  */
  /*
  //!расчет точки касания звуковым лучом горизонта верхней границы ПЗК (интеграл считается каждый раз до рассчитываемой точки)
  for(float i=0.;i<=1000000.;i=i+1.) { //! @param count - число равных частей, на которые разбивается отрезок интегрирования
    z_ = z0 + i * (zgr - z0) / 1000000.;
    c_ = (z_-b)/a;
    ri = cos(xe0) / sqrt( c0 / c_ - pow(cos(xe0),2) );
    if(i==0. || i==1000000.) ri = ri / 2;
    riSumm = riSumm + ri;
  }
  r = (abs( zgr - z0 ) / 1000000.) * riSumm + r0;//полученное значение интеграла + r0
  raySkelet.append(GeoPoint(r/rayScale,zgr,0,LA_GENERAL));
  */
  /*
  //!расчет точки касания звуковым лучом горизонта верхней границы ПЗК (интеграл считается кусочно) TODO есть маленькая ошибка менее шага сетки на диаграмме относительно расчета 1 куска
  r1=r0;
  for(z=z0,z1=z0,c1=c0;(z>=zgr)&&(r/rayScale<MAX_T);z=z+(zgr-z0)/count) {
    riSumm = 0.;
    c=(z-b)/a;
    xe=sqrt(2 * (cgr - c) / c);

    //считаем интеграл численным методом (формула 10.4 "Физические основы распространения звука в океане")
    for(float i=0.;i<=count;i=i+1.) { //! @param count - число равных частей, на которые разбивается отрезок интегрирования
      z_ = z1 + i * (z - z1) / count + 0.5 * (z - z1) / count;
      c_ = (z_-b)/a;
      ri = cos(xe) / sqrt( c1 / c_ - pow(cos(xe),2) );
      if(i==0 || i==count) ri = ri / 2.;
      riSumm = riSumm + ri;
    }
    r = (abs( z - z1 ) / count) * riSumm + r1;//полученное значение интеграла + r1

    c1 = c;
    r1 = r;
    z1 = z;
  }
  raySkelet.append(GeoPoint(r/rayScale,z,0,LA_GENERAL));
  */

  //!расчет 2 куска траектории зукового луча
  for(z=z1,z1=z1,c1=c;(z<z0)&&(r/rayScale<MAX_T);z=z+1.) {
    riSumm = 0.;
    c=(z-b)/a;
    xe=sqrt(2 * (cgr - c) / c);

    //считаем интеграл численным методом (сумма площадей трапеций) (формула 10.4 "Физические основы распространения звука в океане")
    for(float i=0.;i<=count;i=i+1.) { //! @param count - число равных частей, на которые разбивается отрезок интегрирования
      z_ = z1 + i * (z - z1) / count + 0.5 * (z - z1) / count;
      c_ = (z_-b)/a;
      ri = cos(xe) / sqrt(fabs( c1 / c_ - pow(cos(xe),2) ));
      if(MnMath::isZero(i) ||MnMath::isEqual(i,count)){
          ri = ri *0.5f;
        }
      riSumm = riSumm + ri;
    }
    r = (fabs( z - z1 ) / count) * riSumm + r1;//полученное значение интеграла + r1

    raySkelet.append(GeoPoint(layerT_->xAxis()->scale2coord(r/rayScale),layerT_->yAxis()->scale2coord(z),0,LA_GENERAL));
    //    debug_log << "r = " << r << " z = " << z << r/rayScale;
    c1 = c;
    r1 = r;
    z1 = z;
  }

  //TODO сделать расчет точки пересечения звукового луча с осью ПЗК

  //!переопределяем параметры распределения скорости звука от z0 до нижней zgr
  z=99999.;
  z0=ur_min_c; //горизонт оси ПЗК
  zgr=ur_gr_down; // горизонт нижней границы ПЗК
  z1 = z0; // промежуточное начальное значение глубины

  c0=min_c; //скорость звука на оси ПЗК
  cgr=c_gr; //скорость звука на границе ПЗК
  c1 = c0; // промежуточное начальное значение скорости звука
  xe0=sqrt(2 * (cgr - c0) / c0); //начальный угол выхода звукового луча из источника по отношению к оси (приравнивается к значению угла выхода граничного луча)
  xe=xe0; // промежуточное начальное значение угла выхода звукового луча

  a=(zgr-z0)/(cgr-c0); // параметр a линейно-аппроксимированного закона распределения скорости звука с глубиной от z0 до zgr
  b=(cgr*z0-c0*zgr)/(cgr-c0); // параметр b линейно-аппроксимированного закона распределения скорости звука с глубиной от z0 до zgr

  //  debug_log << z << zgr << r/rayScale << MAX_T;

  //!расчет 3 куска траектории зукового луча
  for(z=z0,z1=z0,c1=c0;(z<zgr)&&(r/rayScale<MAX_T);z=z+1.) {
    riSumm = 0.;
    c=(z-b)/a;
    xe=sqrt(2 * (cgr - c) / c);

    //считаем интеграл численным методом (сумма площадей трапеций) (формула 10.4 "Физические основы распространения звука в океане")
    for(float i=0.;i<=count;i=i+1.) { //! @param count - число равных частей, на которые разбивается отрезок интегрирования
      z_ = z1 + i * (z - z1) / count + 0.5 * (z - z1) / count;
      c_ = (z_-b)/a;
      ri = cos(xe) / sqrt(fabs( c1 / c_ - pow(cos(xe),2) ));
      if(MnMath::isZero(i) ||MnMath::isEqual(i,count)){
          ri = ri *0.5f;
        }
      riSumm = riSumm + ri;
    }
    r = (fabs( z - z1 ) / count) * riSumm + r1;//полученное значение интеграла + r1

    raySkelet.append(GeoPoint(layerT_->xAxis()->scale2coord(r/rayScale),layerT_->yAxis()->scale2coord(z),0,LA_GENERAL));
    //    debug_log << "r = " << r << " z = " << z << r/rayScale;
    c1 = c;
    r1 = r;
    z1 = z;
  }

  //!расчет 4 куска траектории зукового луча
  for(z=z1,z1=z1,c1=c;(z>z0)&&(r/rayScale<MAX_T);z=z-1.) {
    riSumm = 0.;
    c=(z-b)/a;
    xe=sqrt(2 * (cgr - c) / c);

    //считаем интеграл численным методом (сумма площадей трапеций)
    //(формула 10.4 "Физические основы распространения звука в океане")
    for(float i=0.;i<=count;i=i+1.) { //! @param count - число равных частей, на которые разбивается отрезок интегрирования
      z_ = z1 + i * (z - z1) / count + 0.5 * (z - z1) / count;
      c_ = (z_-b)/a;
      ri = cos(xe) / sqrt(fabs( c1 / c_ - pow(cos(xe),2) ));
      if(MnMath::isZero(i) ||MnMath::isEqual(i,count)){
          ri = ri *0.5f;
        }
      riSumm = riSumm + ri;
    }
    r = (fabs( z - z1 ) / count) * riSumm + r1;//полученное значение интеграла + r1

    raySkelet.append(GeoPoint(layerT_->xAxis()->scale2coord(r/rayScale),layerT_->yAxis()->scale2coord(z),0,LA_GENERAL));
    //    debug_log << "r = " << r << " z = " << z << r/rayScale;
    c1 = c;
    r1 = r;
    z1 = z;
  }


  //  var(raySkelet.size());
  //рисуем траекторию звукового луча
  map::GeoPolygon* channelRay = new map::GeoPolygon(layerPZK_);
  channelRay->setPenWidth(2);
  channelRay->setPenColor(Qt::magenta);
  channelRay->setSkelet(raySkelet);
  raySkelet.clear();

  //считаем рефракционную дальность обнаружения ГАС
  r = 0.;
  for(it_cur = itb_+1; it_cur != ite_ ; ++it_cur) {
    if(it_cur.key() >= ur_gr_up && (it_cur+1).key() < ur_gr_down) {
      h = (it_cur+1).key()-it_cur.key();
      ri = h * ( 1. / ( tan( (acos( it_cur.value() / c_gr ) + acos( (it_cur+1).value() / c_gr )) / 2. ) ) );
      r = r + ri;
      continue;
    }
    if(it_cur.key() >= ur_gr_down) {
      break;
    }
  }
  float R = r;

  emit PZK_Exist(xe0);

  //делаем поясняющие подписи к элементам ПЗК
  ::meteo::map::GeoText* labelChAxis = new ::meteo::map::GeoText(layerPZK_);
  labelChAxis->setPenColor(Qt::magenta);
  labelChAxis->setFontPointSize(10);
  labelChAxis->setDrawAlways(true);
  labelChAxis->setPos( kLeftCenter);
  labelChAxis->setText(tr("Ось ПЗК H = %1 м Рефракционная дальность обнаружения R = %2 м\nУгол выхода луча a = %3\xC2\xB0").arg(ur_min_c,0,'f',0).arg(R,0,'f',0).arg(xe0/M_PI*180.,0,'f',2));
  labelChAxis->setSkelet(GeoPoint(MAX_T,ur_min_c,0,LA_GENERAL));

  ::meteo::map::GeoText* labelChUp = new ::meteo::map::GeoText(layerPZK_);
  labelChUp->setPenColor(Qt::darkMagenta);
  labelChUp->setFontPointSize(10);
  labelChUp->setPos( kTopLeft);
  labelChUp->setText(tr("Верхняя граница ПЗК H = %1 м").arg(ur_gr_up,0,'f',0));
  labelChUp->setSkelet(GeoPoint(MAX_T,ur_gr_up,0,LA_GENERAL));

  ::meteo::map::GeoText* labelChDown = new ::meteo::map::GeoText(layerPZK_);
  labelChDown->setPenColor(Qt::darkMagenta);
  labelChDown->setFontPointSize(10);
  labelChDown->setPos( kTopLeft);
  labelChDown->setText(tr("Нижняя граница ПЗК H = %1 м").arg(ur_gr_down,0,'f',0));
  labelChDown->setSkelet(GeoPoint(MAX_T,ur_gr_down,0,LA_GENERAL));

  return true;
}

void OceanDiagDoc::setData(const ocean::PlaceData* aprofileData)
{
  pd_ = aprofileData;

  if ( pd_->ocean().urovenList().size() == 0 ) {
    showNoData();
  } else {
    hideNoData();
  }
}

bool OceanDiagDoc::create(const ocean::PlaceData* aprofileData)
{
  pd_ = aprofileData;

  if ( pd_->ocean().urovenList().size() == 0 ) {
    showNoData();
    return false;
  }

  hideNoData();

  createEmptyBlank();

  addChannel();

//  foreach ( map::Object* obj, layerT_->objects() ) {
//    delete obj;
//  }
//  foreach ( map::Object* obj, layerS_->objects() ) {
//    delete obj;
//  }
//  foreach ( map::Object* obj, layerC_->objects() ) {
//    delete obj;
//  }

  //QList<TMeteoData> mdList;
  mdList_.clear();
  QMap<float, ocean::Uroven>::const_iterator itb = pd_->ocean().urovenList().begin();
  QMap<float, ocean::Uroven>::const_iterator ite = pd_->ocean().urovenList().end();
  for ( ; itb != ite; ++itb ) {
    float h = itb.key();
    float t = itb.value().value(ocean::UR_T);
    float s = itb.value().value(ocean::UR_S);

    TMeteoData md;
    md.set("Tw", "", t, static_cast<control::QualityControl>(itb.value().quality(ocean::UR_T)));
    md.set("Sn", "", s, static_cast<control::QualityControl>(itb.value().quality(ocean::UR_S)));
    md.set("zn", "", h, control::RIGHT);

    if ( itb.value().isGood(ocean::UR_S) && itb.value().isGood(ocean::UR_T) ) {
      float c = 1492.9 + 3 * ( t - 10. ) - 0.006 * pow((t-10.),2) - 0.04 * pow((t-18.),2) + 1.2 * ( s - 35. ) - 0.01 * ( t - 18. ) * ( s - 35. ) + h/61.;
      md.set("ff", "", c, control::RIGHT);
    }
    mdList_ << md;
  }
  if(0 < mdList_.size()){
      has_data_ = true;
    } else {
      has_data_ = false;
    }
  layerT_->setData(mdList_, "Tw", "zn", false);
  layerS_->setData(mdList_, "Sn", "zn", false);
  layerC_->setData(mdList_, "ff", "zn", false);

  GeoVector graph = layerT_->graphCoords();
  for(int i= 0; i< graph.count();++i){
    GeoPoint point = graph.at(i);
    float h = layerT_->yAxis()->coord2scale(point.lon());

    ::meteo::map::GeoText* label = new ::meteo::map::GeoText(layerT_);
    label->setPenColor(Qt::black);
    label->setFontPointSize(10);
    label->setPos( kRightCenter);
    label->setText(QString::number(h/1000.,'f',1));
    point += GeoPoint(0.1,0);
    label->setSkelet(point);
  }

  return true;
}
bool OceanDiagDoc::hasData(){
  return  has_data_;
}


void OceanDiagDoc::repaintChannel(float xe0)
{
  addChannel(xe0);
}

void OceanDiagDoc::layerChanged(map::Layer* layer, int event)
{
  if ( map::LayerEvent::Deleted == event ) {
    if ( layerT_ == layer ) { layerT_ = nullptr; }
    if ( layerS_ == layer ) { layerS_ = nullptr; }
    if ( layerC_ == layer ) { layerC_ = nullptr; }
    if ( layerGrid_ == layer ) { layerGrid_ = nullptr; }
    if ( layerPZK_  == layer ) { layerPZK_  = nullptr; }
  }
}

const QList<TMeteoData>& OceanDiagDoc::getMdList(const ocean::PlaceData* aprofileData)
{
  create(aprofileData);
  return mdList_;
}

} // odiag
} // meteo
