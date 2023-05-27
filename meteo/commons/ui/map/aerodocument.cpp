#include "aerodocument.h"

#include <cross-commons/funcs/mn_funcs.h>

#include <cross-commons/debug/tmap.h>
#include <commons/geobasis/generalproj.h>

#include <meteo/commons/ui/map/axis/ramkaaxis.h>
#include <meteo/commons/ui/map/axis/axissimple.h>
#include <meteo/commons/ui/map/axis/axispuanson.h>
#include <meteo/commons/ui/map/graph/funcs.h>
#include <meteo/commons/global/global.h>

#include "geopolygon.h"
#include "geotext.h"
#include "geopixmap.h"
#include "puanson.h"
#include "ramka.h"
#include "weather.h"
#include <meteo/commons/global/weatherloader.h>
#include "incut.h"

namespace meteo {
namespace map {

static const auto kIsoLevels =
  QVector<float>() << 50 << 100 << 150
                    << 200 << 250 << 300
                    << 400 << 500 << 600
                    << 700 << 850 << 925 << 1000 << 1050;

static const QVector<float> kMeterH = QVector<float>()
    <<  103.528f
    <<  121.118f
    <<  141.707f
    <<  165.796f
    <<  193.994f
    <<  226.999f
    <<  264.999f
    <<  308.007f
    <<  356.516f
    <<  411.053f
    <<  472.176f
    <<  540.483f
    <<  616.604f
    <<  701.212f
    <<  795.014f
    <<  898.763f
    << 1013.250f
;

static const QList<QPair<float, float> >
  standartAtm = QList< QPair<float, float> >() << QPair<float, float>(1013.25, 15.000)
  << QPair<float, float>(898.763f, 8.501f)
  << QPair<float, float>(795.014f, 2.004f)
  << QPair<float, float>(701.212f, -4.491f)
  << QPair<float, float>(616.604f, -10.984f)
  << QPair<float, float>(540.483f, -17.474f)
  << QPair<float, float>(472.176f, -23.963f)
  << QPair<float, float>(411.053f, -30.450f)
  << QPair<float, float>(356.516f, -36.935f)
  << QPair<float, float>(308.007f, -43.417f)
  << QPair<float, float>(264.999f, -49.898f)
  << QPair<float, float>(226.999f, -56.376f)
  << QPair<float, float>(193.994f, -56.500)
  << QPair<float, float>(165.796f, -56.500)
  << QPair<float, float>(141.707f, -56.500)
  << QPair<float, float>(121.118f, -56.500)
  << QPair<float, float>(103.528f, -56.500);



static const QString kTitle = QObject::tr("Аэрологическая диаграмма");
static const int kLabelOffset5  = 5;
static const int kLabelOffset10 = 10;

static const float begFi = -80.0;
static const float endFi = 40.0;
static const float begLa = 1050.0;
static const float endLa = 50.0;
static float maxH = 31000.0;

static const QMap< LayerType, QString > kLayerNames = TMap< LayerType, QString >()
  << qMakePair( kAeroBlank1,     QObject::tr("Сетка бланка (крупная)") )
  << qMakePair( kAeroBlank2,     QObject::tr("Сетка бланка (средняя)") )
  << qMakePair( kAeroBlank3,     QObject::tr("Сетка бланка (мелкая)") )
  << qMakePair( kAeroT,         QObject::tr("Температура") )
  << qMakePair( kAeroTd,        QObject::tr("Температура точки росы ") )
  << qMakePair( kAeroSost,      QObject::tr("Кривая состояния") )
  << qMakePair( kAeroWindIso,   QObject::tr("Ветер (основной)") )
  << qMakePair( kAeroWindAdd,   QObject::tr("Ветер (дополнительный)") )
  << qMakePair( kAeroKondens,   QObject::tr("Уровень конденсации") )
  << qMakePair( kAeroKonvek,    QObject::tr("Уровень конвекции") )
  << qMakePair( kAeroKNS,       QObject::tr("Конвективно-неустойчивый слой") )
  << qMakePair( kAeroInvers,    QObject::tr("Слои инверсии и изотермии") )
  << qMakePair( kAeroTropo,     QObject::tr("Тропопауза") )
  << qMakePair( kAeroOblak,     QObject::tr("Облачность") )
  << qMakePair( kAeroObled,     QObject::tr("Обледенение") )
  << qMakePair( kAeroBolt,      QObject::tr("Болтанка") )
  << qMakePair( kAeroTrace,     QObject::tr("Конденсационные следы") )
  << qMakePair( kAeroAxis,      QObject::tr("Коордиантные оси") )
  << qMakePair( kAeroLines,     QObject::tr("Сухие и влажные адиабаты и изограммы") )
  << qMakePair( kAeroSaturIce,  QObject::tr("Температура насыщения относительно льда") );

static QVector<float> mkSequence(float beg, float end, float step)
{
  QVector<float> vec;
  for ( float v = beg; v <= end; v += step ) {
    vec << v;
  }
  return vec;
}

static QVector<QString> mkSequence(float beg, float end, float step, int prec)
{
  QVector<QString> vec;
  for ( float v = beg; v <= end; v += step ) {
    vec << QString::number(v,'f',prec);
  }
  return vec;
}

static QVector<float> mkSequenceSa81ByH(float beg, float end, float step)
{
  QVector<float> vec;

  for ( float v = beg; v <= end; v += step ) {
    float p,t;
    zond::SA81_P(v*1000,&t,&p);
    vec << p;
  }

  return vec;
}


AeroDocument::AeroDocument( const GeoPoint& mapcenter,
          const GeoPoint& doc_center,
          proto::DocumentType type,
          ProjectionType proj_type )
  : Document( mapcenter, doc_center, type, proj_type )
{
}

AeroDocument::AeroDocument( const proto::Document& doc )
  : Document( doc )
{
}

AeroDocument::AeroDocument()
  : Document()
{

}

AeroDocument::~AeroDocument()
{
}

AeroDocument::AeroDocument( Document* doc, const GeoPoint& c, const QSize& docsize )
  : Document( doc, c, docsize )
{
}

bool AeroDocument::init()
{
  clearDocument();
  ProjType proj_type = GENERAL;
//  GeoPoint mc( -23, 220, 0.0, LA_GENERAL );
  GeoPoint mc( 0, 0, 0.0, LA_GENERAL );
  projection_ = Projection::createProjection( proj_type, mc );

  if ( nullptr == projection_ ) {
    error_log << QObject::tr("Не удалось создать проекцию с типом = %1")
      .arg(proj_type);
    return false;
  }

  backgroundColor_ = Qt::white;

  GeoPoint sc = mc;
  if ( true == property_.has_doc_center() ) {
    sc = pbgeopoint2geopoint( property_.doc_center() );
  }
  setScreenCenter(sc);
  if ( false == property_.has_mapsize() ) {
    property_.mutable_mapsize()->CopyFrom( qsize2size( QSize( 3072, 3072 ) ) );
  }
  if ( false == property_.has_docsize() ) {
    property_.mutable_docsize()->CopyFrom( property_.mapsize() );
  }
  float Yfactor = 0.4f;
//  GeoPoint dc = GeoPoint( -23, 220, 0.0, LA_GENERAL );
  GeoPoint dc = GeoPoint( 0, 0, 0.0, LA_GENERAL );
  GeneralProj* proj = projection_cast<meteo::GeneralProj*>( Document::projection() );
  if ( nullptr == proj ) {
    return false;
  }
  property_.set_geoloader("");
  proj->setMapCenter(dc);
  GeoPoint start = GeoPoint( begFi, begLa, 0, LA_GENERAL );
  GeoPoint end = GeoPoint( endFi, endLa, 0, LA_GENERAL );
 // GeoPoint center = GeoPoint( ( start.lat() + end.lat() )*0.5f, ( start.lon() + end.lon() )*0.5f*(Yfactor), 0, LA_GENERAL );

  float h;
  float t;
  float p = end.la();
  zond::SA81_H_ext( p, &t, &h );
  maxH = h;
  proj->setStart(start);
  proj->setEnd(end);
  proj->setYfactor(Yfactor);

  proj->setFuncTransform( logF2X_one, logX2F_one);
  proj->setFuncTransform( logF2X_onef, logX2F_onef);
  proj->setRamka( start.fi(), start.la(), end.fi(), end.la() );


  Document::setScale(17.5);
  Document::ramka()->setHalfWidth(3);

  if ( true == WeatherLoader::instance()->punchlibraryspecial().contains("ddff") ) {
    ddff_.CopyFrom(meteo::map::WeatherLoader::instance()->punchlibraryspecial()["ddff"]);
  }
  else {
    warning_log << QObject::tr("Не найден знак ветра. Ветер не будет отображен в диаграмме");
  }

  RamkaAxis* ramka = new RamkaAxis(this);
  Document::setRamka(ramka);
  Document::setName(kTitle);
  return true;
}

Document* AeroDocument::stub( const GeoPoint& cntr, const QSize& docsize )
{
  Document* doc = new AeroDocument( this, cntr, docsize );
  return doc;
}

void AeroDocument::setData( const zond::PlaceData& profileData )
{
  data_ = profileData;
  data_.zond().UV2DF(true);
}


bool AeroDocument::create( const zond::PlaceData& data )
{
  clearDataLayers();
 /* for (int i =0; i< incuts().size();++i) {
      incuts().at(i)->setVisible(false);
    }*/
  createEmptyBlank();

  data_ = data;
  data_.zond().UV2DF(true);
  int cnt = data_.zond().urovenList().size();
  if ( cnt < 2 ) {  //может один приземный быть
    error_log << QObject::tr("Отсутсвуют уровни в зонде");
    return false;
  }

  addMain();
  addSost();
  addInvers();
  float Pkond;
  addKondens(&Pkond);
  addKonvek(Pkond);
  addKNS();
  addObled();
  addOblak();
  addBoltan();
  float Ptropo;
  addTropo(&Ptropo);
  addTrace(Ptropo);
  return true;
}

bool AeroDocument::addMain()
{
  if ( false == prepLayer(kAeroWindIso)
    || false == prepLayer(kAeroWindAdd)
       )
  {
    return false;
  }

  if ( false == prepLayer(kAeroTd)
    || false == prepLayer(kAeroT)
    || false == prepLayer(kAeroSaturIce) ) {
    return false;
  }

  GeoVector t_skelet;
  GeoVector td_skelet;
  GeoVector tice_skelet;


  auto itb = data_.zond().urovenList().begin();
  auto ite = data_.zond().urovenList().end();
  for ( ; itb != ite; ++itb ) {
    const zond::Uroven& ur_vs = itb.value();
    float p = itb.key();
    if( p < kIsoLevels.first() || p > kIsoLevels.last() ) {
      continue;
    }

    addWind(ur_vs);

    if ( true == ur_vs.isGood(zond::UR_Td) ) {
      float t = ur_vs.value(zond::UR_Td);
      td_skelet.append( GeoPoint(t, p, 0, LA_GENERAL) );
    }
    if ( true == ur_vs.isGood(zond::UR_T) ) {
      float t = ur_vs.value(zond::UR_T);
      t_skelet.append( GeoPoint(t, p, 0, LA_GENERAL) );
    }
    if ( true == ur_vs.isGood(zond::UR_D) ) {
      float p = itb.key();
      float tice = -8.0*ur_vs.value(zond::UR_D);
      if (tice < -(273.15 - 200.0) ) { //предел по фазовой диаграмме воды
        tice = -73.15;
      }
      tice_skelet.append(GeoPoint(tice, p, 0, LA_GENERAL));
    }
  }

  addTemperatureR(td_skelet);
  addTemperature(t_skelet);
  addTempSaturIce(tice_skelet);
  auto l = layerByCustomType(kAeroWindAdd);
  if ( nullptr != l ) {
    l->setVisisble(false);
  }
  return true;
}

bool AeroDocument::addTemperatureR( const GeoVector& skelet )
{
  auto l = layerByCustomType(kAeroTd);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg(kLayerNames[kAeroTd]);
    return false;
  }
  map::GeoPolygon* line = new GeoPolygon(l);
  line->setPenWidth(2);
  line->setPenColor(Qt::darkGreen);
  line->setSkelet(skelet);

  return true;
}

bool AeroDocument::addTemperature( const GeoVector& skelet )
{
  auto l = layerByCustomType(kAeroT);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[kAeroT] );
    return false;
  }
  zond::Uroven urZ;
  if ( true == data_.zond().getUrz(&urZ) ) {
    float pZ = urZ.value(zond::UR_P);
    float tZ = urZ.value(zond::UR_T);
    GeoVector zskelet;
    zskelet.append( GeoPoint( tZ - 30.0f, pZ, 0, LA_GENERAL ) );
    zskelet.append( GeoPoint( tZ + 15.0f, pZ, 0, LA_GENERAL ) );
    auto line = new GeoPolygon(l);
    line->setPenWidth(2);
    line->setPenColor(Qt::black);
    line->setSkelet(zskelet);
  }

  auto line = new GeoPolygon(l);
  line->setPenWidth(2);
  line->setPenColor(Qt::red);
  line->setSkelet(skelet);

  addLinePodpis( skelet, l );
  addTempLayers();
  addTempMax();

  return true;
}

bool AeroDocument::addTempLayers()
{
  auto l = layerByCustomType(kAeroT);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[kAeroT] );
    return false;
  }
  QList<float> P, H;

  for ( float t = -20.0; t <= 0.0; t += 10.0 ) {
    data_.zond().oprPPoParam(zond::UR_T,t,&P);
    data_.zond().oprHPoParam(zond::UR_T,t,&H);
    if ( ( 0 == P.size() ) || ( 0 == H.size() ) || ( P.size() != H.size() ) ) {
      continue;
    }
    for ( int i = 0; i < P.size(); ++i ) {
      GeoVector skelet;
      GeoPoint p1 = GeoPoint( t, P.at(i), 0, LA_GENERAL);
      GeoPoint p2 = GeoPoint( t + 15.0, P.at(i), 0, LA_GENERAL );
      skelet.append(p1);
      skelet.append(p2);
      auto line = new GeoPolygon(l);
      line->setPenWidth(2);
      line->setPenColor(Qt::red);
      line->setSkelet(skelet);

      GeoVector lblSkelet;
      lblSkelet.append(p2);
      auto label = new GeoText(l);
      label->setPenColor(Qt::red);
      label->setFontPointSize(20);
      label->setPos(kTopLeft);
      label->setText(QObject::tr("%1").arg(QString::number(t,'f',0) + QObject::tr("\xC2\xB0""C")));
      label->setSkelet(lblSkelet);

      auto labelH = new GeoText(l);
      labelH->setPenColor(Qt::black);
      labelH->setFontPointSize(10);
      labelH->setPos(kRightCenter);
      labelH->setText(QString::number(H.at(i),'f',0) + QObject::tr(" м"));
      labelH->setSkelet(p2);
      labelH->setTranslate(kLabelOffset5, 0);
    }
    P.clear();
    H.clear();
  }
  return true;
}

bool AeroDocument::addTempMax()
{
  auto l = layerByCustomType(kAeroT);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[kAeroT] );
    return false;
  }
  auto axis = layerByCustomType(kAeroT);
  if ( nullptr == axis ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[kAeroT] );
    return false;
  }
  float temp;
  bool ok = data_.zond().getTmaxTitov(&temp);

  if ( false == ok ) {
    return false;
  }

  float maxP = kIsoLevels.last();
  GeoVector skelet;
  skelet << GeoPoint(temp, maxP - 10, 0, LA_GENERAL);
  skelet << GeoPoint(temp, maxP, 0, LA_GENERAL);
  map::GeoPolygon* line = addLine( l, skelet, 1, Qt::red);
  line->setTranslate(0, kLabelOffset10);

  auto text = addText( axis,
                       QString::number(temp, 'f', 1)+QObject::tr("\xC2\xB0"),
                       GeoPoint(temp, maxP, 0, LA_GENERAL), 10, Qt::black );
  text->setPos( kBottomCenter );
  text->setTranslate( 0, kLabelOffset5 );

  return true;
}

bool AeroDocument::addWind( const zond::Uroven& ur_vs )
{
  if( false == ur_vs.isGood(zond::UR_dd) || false == ur_vs.isGood(zond::UR_ff) ) {
    return false;
  }

  float t = -100;
  float p = ur_vs.value(zond::UR_P);
  int dd = ur_vs.value(zond::UR_dd);
  int ff = ur_vs.value(zond::UR_ff);
  LayerType lt;
  if( true == kIsoLevels.contains(p) ) {
    lt = kAeroWindIso;
  }
  else {
    lt = kAeroWindAdd;
  }
  auto l = layerByCustomType(lt);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.").arg( kLayerNames[lt] );
    return false;
  }

  TMeteoData md;
  md.set("dd", TMeteoParam("", dd, static_cast<control::QualityControl>(ur_vs.quality(zond::UR_dd))));
  md.set("ff", TMeteoParam("", ff, static_cast<control::QualityControl>(ur_vs.quality(zond::UR_ff))));
  // fx исп. для скорости ветра в км/ч
  md.set("fx", TMeteoParam("", ff*3.6f, static_cast<control::QualityControl>(ur_vs.quality(zond::UR_ff))));

  Puanson* pu = new Puanson(l);
  pu->setPunch(ddff_);
  pu->setSkelet( GeoPoint( t, p, 0, LA_GENERAL));
  pu->setDrawAlways(true);
  pu->setMeteodata(md);
//  pu->setVisible(false);

  return true;
}

bool AeroDocument::addEnergy()
{
  float p = 1100,
      t=0,
      tSost=0,
      pPrev=0,
      tPrev=0,
      tSostPrev= 0,
      dt=0;

  float P = 0, T=0;
  float pMax=400.;
  int sign=0,signPrev=0;
  bool signChanged=false;
  float aSt=0.,bSt=0.,aSo=0.,bSo=0.;//параметры a и b прямых пересекающихся участков кр. стратиф. и кр. сост.
  float pCross=-999.,tCross=-999.;
  GeoPoint firstPoint, crossPoint;
  QColor red;
  red.setRgbF(1.,0.,0.,0.3);
  QColor blue;
  blue.setRgbF(0.,0.,1.,0.3);
  QColor glass;
  glass.setRgbF(0.,0.,0.,0.);
  QColor color;

  auto l = layerByCustomType(kAeroSost);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[kAeroSost] );
    return false;
  }

  //определяем минимальное значение давления, до уровня которого рисовать энергию неустойчивости
  auto begin = data_.zond().urovenList().begin();
  auto end = data_.zond().urovenList().end();
  for ( auto it = --end; it != begin ; --it ) {
    if ( false == data_.zond().oprTsost( it.key(), &tSost ) ) {
      continue;
    }
    pMax = it.key();
    if( tSost > -70.f ) {
      continue;
    }
    else {
      break;
    }
  }

  begin = data_.zond().urovenList().begin();
  end = data_.zond().urovenList().end();
  GeoVector skelet;
  for ( auto it = --end; it != begin && p > pMax; --it ) {
    //берем значения давления и температуры на уровне, начиная от нижнего уровня
    if( false == it.value().isGood(zond::UR_T) ) {
      continue;
    }
    p = it.key();
    t = it.value().value(zond::UR_T);
    //определяем температуру кр. сост. на уровне
    if ( false == data_.zond().oprTsost( p, &tSost ) ) {
      continue;
    }
     //определяем знак энергии неустойчивости
    if ( t > tSost ) {
      sign = -1;
    }
    if ( t < tSost ) {
      sign = 1;
    }
    dt = t - tSost;
    if ( dt*dt < 0.00001f ) {
      sign = 0;
    }
    //определяем изменился ли знак энергии неустойчивости
    if ( 2 == abs(sign - signPrev) ) {
      signChanged = true;
    }
    if ( ( false == signChanged ) && ( ( it - 1 ) != begin ) && ( p > pMax ) ) {
      skelet << GeoPoint( t, p, 0, LA_GENERAL );
      if ( 1 == skelet.size() ) {
  firstPoint = skelet.first();
      }
    }
    else { //завершаем подъем по кр. стратиф., опускаемся по кр. сост. и закрашиваем полигон
      if ( ( ( ( it - 1 ) == begin ) || ( p <= pMax ) ) && false == signChanged ) {
  //ограничиваем полигон линией последнего уровня, либо линией тропопаузы
  skelet << GeoPoint(t, p, 0, LA_GENERAL);
  pCross = p;
  sign = -sign;
      }
      else {
        //находим точку пересечения кр. стратиф. и кр. сост., добавляем ее в скелет полигона
        if (MnMath::isEqual( t,tPrev) ) {
    aSo = ( p - pPrev )/( tSost - tSostPrev );
          bSo = ( tSost*pPrev - tSostPrev*p )/( tSost - tSostPrev );
    tCross = t;
    pCross = aSo*t + bSo;
  }
  else {
          aSt = ( p - pPrev )/( t - tPrev );
          bSt = ( t*pPrev - tPrev*p )/( t - tPrev );
          aSo = ( p - pPrev )/( tSost - tSostPrev );
          bSo = ( tSost*pPrev - tSostPrev*p )/( tSost - tSostPrev );
          tCross = ( bSo - bSt )/( aSt - aSo );
          pCross = aSt*( bSo - bSt )/( aSt - aSo ) + bSt;
  }
        crossPoint = GeoPoint( tCross, pCross, 0, LA_GENERAL);
        skelet.append(crossPoint);
      }
      //добавляем все точки кр. сост. от точки пересечения с кр. стратиф. до начальной в скелет полигона
      for( P = pCross; P < firstPoint.lon() ; P = P + 0.1f ) {
        if( false == data_.zond().oprTsost( P, &T ) ) {
          continue;
        }
  skelet.append( GeoPoint( T, P, 0, LA_GENERAL ) );
      }
      skelet.append( GeoPoint( firstPoint.lat(), firstPoint.lon(), 0, LA_GENERAL ) );
      //определяем цвет полигона
      if ( 1 == sign ) {
        color = blue;
      }
      else {
        color = red;
      }
      //закрашиваем полигон
      auto poly = new GeoPolygon(l);
      poly->setPenWidth(2);
      poly->setPenColor(glass);
      poly->setBrushColor(color);
      poly->setBrushStyle(Qt::SolidPattern);
      poly->setClosed(true);
      poly->setSkelet(skelet);
      skelet.clear();
      firstPoint=crossPoint;
      skelet.append( firstPoint );
      skelet.append( GeoPoint( t, p, 0, LA_GENERAL ) );
      signChanged = false;
    }
    pPrev = p;
    tPrev = t;
    tSostPrev = tSost;
    signPrev = sign;
  }

  return true;
}

bool AeroDocument::addTempSaturIce( const GeoVector& skelet )
{
  auto l = layerByCustomType(kAeroSaturIce);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[kAeroSaturIce] );
    return false;
  }
  map::GeoPolygon* line = new GeoPolygon(l);
  line->setPenWidth(2);
  line->setPenColor(qRgb(80, 80, 170));
  line->setPenStyle(Qt::DashLine);
  line->setSkelet(skelet);

  return true;
}

bool AeroDocument::addSost()
{
  if ( false == prepLayer(kAeroSost) ) {
    error_log << QObject::tr("Не удалось создать слой '%1'")
      .arg( kLayerNames[kAeroSost] );
    return false;
  }
  auto l = layerByCustomType(kAeroSost);
  GeoVector skelet;
  //float Pkond = 9999;
  //float Tkond = 9999;
  //float Pprev = 9999;
  //if( false == data_.zond().oprPkondens(&Pkond,&Tkond)){
   // return false;
  //}
  float P = 1100.;
  float T = 9999;


  float p_kondens, tet, tpot, t_kondens;
  zond::Uroven ur;
  if(!data_.zond().getPprizInvHi(&ur)){
    if(!data_.zond().getUrz( &ur)){
      warning_log << QObject::tr("Нет данных о давлениии у поверхности за %1")
                            .arg(data_.zond().dateTime().toString("yyyy-MM-ddThh:mm:ss"));

      return false;
    }

  }
  if(!ur.isGood(zond::UR_T) ||!ur.isGood(zond::UR_Td) ||!ur.isGood(zond::UR_P) ){
    return false;
  }
  if(!zond::oprPkondens(ur.value(zond::UR_T) , ur.value(zond::UR_Td) ,ur.value(zond::UR_P),  &p_kondens, &t_kondens)) return false;
  if(!zond::oprTet(ur.value(zond::UR_P), ur.value(zond::UR_T), &tet)) return false;
  if(!zond::oprTPot(p_kondens, t_kondens, &tpot)) return false;
  P = ur.value(zond::UR_P);
  for( ; (T >-80.0f &&P > 0) ; P -= 1.0f ){
    if ( p_kondens < P ? zond::oprTPoTet(P, tet, &T) : zond::oprTPoTPot(P, tpot, &T)) {
      skelet.append( GeoPoint( T, P, 0, LA_GENERAL ) );
    }
    //Pprev = P;
  }
  GeoPolygon* line = new GeoPolygon(l);
  line->setPenWidth(2);
  line->setPenColor(Qt::black);
  line->setSkelet(skelet);

  addEnergy();

  return true;
}

bool AeroDocument::addInvers()
{
  if( 0 == data_.zond().urovenList().size() ) {
    return false;
  }
  if ( false == prepLayer( kAeroInvers ) ) {
    return false;
  }
  QVector<zond::InvProp> layers;

  if( false == data_.zond().getSloiInver(&layers) ) {
    return false;
  }
  auto l = layerByCustomType(kAeroInvers);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[kAeroInvers] );
    return false;
  }
  QColor yellow;
  yellow.setRgbF( 1.0, 1.0 , 0.0, 0.5 );
  QColor glass;
  glass.setRgbF( 0.0 , 0.0, 0.0, 0.0);
  for ( int i = 0; i < layers.size(); ++i ) {
    if ( 6000.0 < layers.at(i).h_lo ) {
      continue;
    }

    zond::Uroven ur1, ur2;
    if ( false == data_.zond().getUrPoH( layers.at(i).h_lo, &ur1 )
      || false == data_.zond().getUrPoH( layers.at(i).h_hi, &ur2 ) ) {
      continue;
    }

    float leftT1 = layers.at(i).T_lo;
    float leftT2 = layers.at(i).T_hi;
    float rightT1 = leftT2 + 15.0;
    float rightT2 = leftT2 + 15.0;
    float p1 = ur1.value( zond::UR_P );
    float p2 = ur2.value( zond::UR_P );

    GeoVector skelet;
    skelet.append(GeoPoint( rightT2, p2, 0, LA_GENERAL ) );
    skelet.append(GeoPoint( leftT2, p2, 0, LA_GENERAL ) );
    auto it = data_.zond().urovenList().begin();
    auto end = data_.zond().urovenList().end();
    for ( ; it.key() < p1 && it != end ; ++it ) {
      if ( it.key() < p2 ) {
        continue;
      }
      float p = it.key();
      float t = it.value().value( zond::UR_T );
      if (it.value().isGood( zond::UR_T )) {
  skelet.append( GeoPoint( t, p, 0, LA_GENERAL ) );
      }
    }
    skelet.append( GeoPoint( leftT1, p1, 0, LA_GENERAL ) );
    skelet.append( GeoPoint( rightT1, p1, 0, LA_GENERAL ) );
    skelet.append( GeoPoint( rightT2, p2, 0, LA_GENERAL ) );

    auto poly = new GeoPolygon(l);
    poly->setPenWidth(2);
    poly->setPenColor(glass);
    poly->setBrushColor(yellow);
    poly->setBrushStyle(Qt::SolidPattern);
    poly->setClosed(true);
    poly->setSkelet(skelet);

    skelet.clear();
    skelet.append( GeoPoint( leftT1, p1, 0, LA_GENERAL) );
    skelet.append( GeoPoint( rightT1, p1, 0, LA_GENERAL) );
    poly = new GeoPolygon(l);
    poly->setPenWidth(2);
    poly->setPenColor( qRgb(230,200,0) );
    poly->setSkelet(skelet);
    float h1 = layers.at(i).h_lo;
    auto lbl = new GeoText(l);
    lbl->setPenColor(Qt::black);
    lbl->setFontPointSize(10);
    lbl->setPos(kRightCenter);
    lbl->setText( QString::number(h1,'f',0) + QObject::tr(" м") );
    lbl->setSkelet( GeoPoint( rightT1, p1, 0, LA_GENERAL) );
    lbl->setTranslate( kLabelOffset10, 0 );

    skelet.clear();
    skelet.append( GeoPoint( leftT2, p2, 0, LA_GENERAL ) );
    skelet.append( GeoPoint( rightT2, p2, 0, LA_GENERAL ) );
    poly = new GeoPolygon(l);
    poly->setPenWidth(2);
    poly->setPenColor( qRgb( 230, 200, 0 ) );
    poly->setSkelet(skelet);
    float h2 = layers.at(i).h_hi;
    lbl = new GeoText(l);
    lbl->setPenColor(Qt::black);
    lbl->setFontPointSize(10);
    lbl->setPos(kRightCenter);
    lbl->setText( QString::number( h2, 'f', 0) + QObject::tr(" м") );
    lbl->setSkelet( GeoPoint( rightT2, p2, 0, LA_GENERAL ) );
    lbl->setTranslate( kLabelOffset10, 0);

    lbl = new GeoText(l);
    lbl->setPenColor(Qt::black);
    lbl->setFontPointSize(10);
    lbl->setPos(kCenter);
    lbl->setText(
        QObject::tr("\xCE\x94""H=%1 м, ""\xCE\x94""T=%2\xC2\xB0""C")
        .arg(QString::number(h2-h1,'f',0))
        .arg(QString::number(layers.at(i).dT,'f',0)));
    lbl->setSkelet(GeoPoint( ( leftT2 + rightT2 )/2.0,( p1 + p2 )/2.0) );
  }

  return true;
}

bool AeroDocument::addKondens( float* Pkondens )
{
  if ( false == prepLayer( kAeroKondens ) ) {
    return false;
  }
  auto l = layerByCustomType(kAeroKondens);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[kAeroKondens] );
    return false;
  }
  float Pkond = BAD_METEO_ELEMENT_VAL, Tkond = BAD_METEO_ELEMENT_VAL, Hkond =BAD_METEO_ELEMENT_VAL;;
  if ( false == data_.zond().oprPkondens( &Pkond, &Tkond ) ) {
    return false;
  }

  //debug_log << "P,T kond" << Pkond << Tkond;

  *Pkondens = Pkond;
  zond::Uroven ur_kond;
  if ( false == data_.zond().getUrPoP( Pkond, &ur_kond ) ) {
    return false;
  }
  Hkond=ur_kond.value(zond::UR_H);

  GeoVector skelet;
  skelet.append( GeoPoint( Tkond, Pkond, 0, LA_GENERAL ) );
  skelet.append( GeoPoint( Tkond + 10, Pkond, 0, LA_GENERAL ) );

  auto line = new GeoPolygon(l);
  line->setPenWidth(2);
  line->setPenColor(Qt::blue);
  line->setOrnament("wavyline");
  line->setDrawAlways(true);
  line->setSkelet(skelet);
  auto lbl = new GeoText(l);
  lbl->setPenColor(Qt::black);
  lbl->setFontPointSize(20);
  lbl->setPos(kTopRight);
  lbl->setDrawAlways(true);
  lbl->setText(QObject::tr("УК"));
  lbl->setSkelet(GeoPoint( Tkond + 1.0, Pkond - 1.0, 0, LA_GENERAL ) );

  lbl = new GeoText(l);
  lbl->setPenColor(Qt::black);
  lbl->setFontPointSize(10);
  lbl->setPos(kTopRight);
  lbl->setText( QString::number( Hkond, 'f', 0 ) + QObject::tr(" м") );
  lbl->setSkelet( GeoPoint( Tkond + 10, Pkond, 0, LA_GENERAL ) );
  lbl->setTranslate( kLabelOffset10, 0 );
  return true;
}

bool AeroDocument::addKonvek( float Pkondens )
{
  if ( false == prepLayer( kAeroKonvek ) ) {
    return false;
  }
  auto l = layerByCustomType(kAeroKonvek);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[kAeroKonvek] );
    return false;
  }
 // float Tmax = BAD_METEO_ELEMENT_VAL;
  float Pkond = -1.0;
 /* if ( false == data_.zond().getTmaxTitov(&Tmax) ) {
    return false;
  }
  if ( false == data_.zond().averConvecLevel( &Pkond, Tmax ) ) {
    return false;
  }

  if ( Pkond > Pkondens ) {
    return false;
  }*/// упростил задачу

  zond::Uroven ur;
  if(!data_.zond().getPprizInvHi(&ur)){
    if(!data_.zond().getUrz( &ur)){
        warning_log << QObject::tr("Нет данных о давлениии у поверхности за %1")
                              .arg(data_.zond().dateTime().toString("yyyy-MM-ddThh:mm:ss"));
      return false;
    }

  }
  if(!ur.isGood(zond::UR_T) ||!ur.isGood(zond::UR_Td) ||!ur.isGood(zond::UR_P) ){
    return false;
  }
  for( float p = 10.0f; p < 1050.0f; ++p ) {
    if ( 0 < data_.zond().oprZnakEnergy(p) ) {
      Pkond = p;
      break;
    }
  }

  if ( 100 > Pkond ) {
    return false;
  }
  if ( Pkond > Pkondens ) {
    return false;
  }

  //немного точнее
  for( float p = Pkond - 1; p <= Pkond; p += 0.001 ) {
    if ( 0 < data_.zond().oprZnakEnergy(p) ) {
      Pkond = p;
      break;
    }
  }

  zond::Uroven ur_kond;
  if( 0 == data_.zond().getUrPoP( Pkond, &ur_kond ) ) {
    return false;
  }
  float Hkond = ur_kond.value(zond::UR_H);
  float Tkond = ur_kond.value(zond::UR_T);

  GeoVector skelet;
  skelet.append( GeoPoint( Tkond, Pkond, 0, LA_GENERAL ) );
  skelet.append( GeoPoint( Tkond + 10, Pkond, 0, LA_GENERAL ) );
  auto line = new GeoPolygon(l);
  line->setOrnament("wavyline");
  line->setDrawAlways(true);
  line->setPenWidth(2);
  line->setPenColor(Qt::red);
  line->setSkelet(skelet);
  auto lbl = new GeoText(l);
  lbl->setPenColor(Qt::black);
  lbl->setFontPointSize(20);
  lbl->setPos(kTopRight);
  lbl->setDrawAlways(true);
  lbl->setText(QObject::tr("Уровень конвекции"));
  lbl->setSkelet( GeoPoint( Tkond + 1.0, Pkond - 1.0, 0, LA_GENERAL ) );

  lbl = new GeoText(l);
  lbl->setPenColor(Qt::black);
  lbl->setFontPointSize(10);
  lbl->setPos(kTopRight);
  lbl->setText( QString::number( Hkond, 'f', 0 ) + QObject::tr(" м") );
  lbl->setSkelet( GeoPoint( Tkond + 10, Pkond, 0, LA_GENERAL ) );
  lbl->setTranslate( kLabelOffset10, 0 );
  return true;
}

bool AeroDocument::addKNS(){
  if ( false == prepLayer( kAeroKNS ) ) {
    return false;
  }
  auto l = layerByCustomType(kAeroKNS);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[kAeroKNS] );
    return false;
  }

  QColor blue;
  blue.setRgbF( 0.0, 0.0, 1.0, 0.2 );
  float Tmax = BAD_METEO_ELEMENT_VAL, p = BAD_METEO_ELEMENT_VAL;
  if( false == data_.zond().getTmaxTitov(&Tmax) ) {
    return false;
  }

  if ( false == data_.zond().oprKNS( &p, Tmax) ) {
    return false;
  }
  zond::Uroven ur;
  if ( false == data_.zond().getUrPoP( p, &ur ) ) {
    return false;
  }
  float t = ur.value(zond::UR_T);

  zond::Uroven urZ;
  if ( !data_.zond().getUrz(&urZ) ) {
    return false;
  }
  float pZ = urZ.value(zond::UR_P);
  float tZ = urZ.value(zond::UR_T);

  if (MnMath::isEqual(p, pZ) ) {
    return false;
  }

  GeoVector skelet;
  skelet.append( GeoPoint( t, p, 0, LA_GENERAL ) );
  skelet.append( GeoPoint( t - 20.0, p, 0, LA_GENERAL ) );
  auto line = new GeoPolygon(l);
  line->setPenWidth(2);
  line->setPenColor(Qt::blue);
  line->setSkelet(skelet);

  skelet.append( GeoPoint( t - 20.0, pZ, 0, LA_GENERAL ) );
  skelet.append( GeoPoint( tZ, pZ, 0, LA_GENERAL ) );
  for( float P = pZ, TSost = BAD_METEO_ELEMENT_VAL; P > p; P -= 0.1f ) {
    if ( false == data_.zond().getUrPoP( P, &ur ) ) {
      continue;
    }
    float T = ur.value(zond::UR_T);
    if ( true == data_.zond().oprTsost( P, &TSost ) &&  T > TSost ) {
      skelet.append( GeoPoint( TSost, P, 0, LA_GENERAL ) );
    }
    else {
      skelet.append( GeoPoint( T, P, 0, LA_GENERAL ) );
    }
  }
  line = new GeoPolygon(l);
  line->setPenWidth(2);
  line->setPenColor(blue);
  line->setBrushColor(blue);
  line->setBrushStyle(Qt::SolidPattern);
  line->setClosed(true);
  line->setSkelet(skelet);

  auto lbl = new GeoText(l);
  lbl->setPenColor(Qt::blue);
  lbl->setFontPointSize(15);
  lbl->setPos(kCenter);
  lbl->setDrawAlways(true);
  lbl->setText(QObject::tr("КНС"));
  lbl->setSkelet( GeoPoint( t - 10, ( p + pZ )/2.0, 0, LA_GENERAL ) );

  return true;
}

bool AeroDocument::addObled()
{
  if( 0 == data_.zond().urovenList().size() ) {
    return false;
  }
  if ( false == prepLayer( kAeroObled ) ) {
    return false;
  }
  auto l = layerByCustomType(kAeroObled);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[kAeroObled] );
    return false;
  }
  QList<float> gran_obled;
  data_.zond().oprGranObled(&gran_obled);
  if( 1 > gran_obled.size() ) {
    return false;
  }
  if( 1 == gran_obled.size()%2 ) {
    float Pz = 0.0;
    if( true == data_.zond().getPz(&Pz) ) {
      gran_obled.prepend(Pz);
    }
    else {
      gran_obled.prepend(1050.);
    }
  }

  float prev_p1 = 0.0, prev_p2 = 0.0;
  for ( int i = 0, sz = gran_obled.size(); i < sz; i += 2 ) {
    float p1 = gran_obled.at(i);
    float p2 = gran_obled.at(i+1);
    if ( 0 == i ) {
      prev_p1 = p1;
      prev_p2 = p2;
      if( 2 == gran_obled.size() ) {
        p1 = 10.0;
        p2 = 5.0;
      }
      else {
        continue;
      }
    }
    if ( 1100.0 > ( data_.zond().oprHpoP(p1) - data_.zond().oprHpoP(prev_p2) ) ) {
      prev_p2 = p2;
      if ( i != sz - 2 ) {
        continue;
      }
    }

    if ( 100.0 > ( data_.zond().oprHpoP(prev_p2) - data_.zond().oprHpoP(prev_p1) ) ) {
      prev_p1 = p1;
      prev_p2 = p2;
      continue;
    }

    drawSectionPhenomen( prev_p1, prev_p2, 27, Qt::red, ":/meteo/icons/geopixmap/obledenenie.png", l );

    prev_p1 = p1;
    prev_p2 = p2;
  }
  return true;
}

bool AeroDocument::addOblak()
{
  if( 0 == data_.zond().urovenList().size() ) {
    return false;
  }
  if ( false == prepLayer( kAeroOblak ) ) {
    return false;
  }
  auto l = layerByCustomType(kAeroOblak);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[kAeroOblak] );
    return false;
  }
  QList<float> gran_oblak;
  data_.zond().oprGranOblak(&gran_oblak);
  if( 0 == gran_oblak.size() ) {
    return false;
  }
  if ( 1 == gran_oblak.size()%2 ) {
    float Pz=0;
    if ( true == data_.zond().getPz(&Pz) ) {
      gran_oblak.prepend(Pz);
    }
    else {
      gran_oblak.prepend(1050.0);
    }
  }

  float prev_p1 = 0.0;
  float prev_p2 = 0.0;
  float p = 0.0;
  QColor blue;
  blue.setRgbF( 0.0, 0.0, 1.0, 0.2 );
  QString str;
  int kol_obl = 0;
  for ( int i = 0, sz = gran_oblak.size(); i <= sz; i += 2 ) {
    float p1 = 10.0;
    float p2 = 5.0;
    if ( i < sz ) {
      p1 = gran_oblak.at(i);
      p2 = gran_oblak.at(i+1);
    }

    if ( 0 == i ) {
      prev_p1 = p1;
      prev_p2 = p2;
      if ( 2 == gran_oblak.size() ) {
        p1 = 10.0;
        p2 = 5.0;
        continue;
      }
    }

    if ( 1100.0 > ( data_.zond().oprHpoP(p1) - data_.zond().oprHpoP(prev_p2) ) ) {
      prev_p2 = p2;
      continue;
    }

    if ( 100.0 > ( data_.zond().oprHpoP(prev_p2) - data_.zond().oprHpoP(prev_p1) ) ) {
      prev_p1 = p1;
      prev_p2 = p2;
      continue;
    }

    zond::Uroven ur1;
    if ( false == data_.zond().getUrPoP( prev_p1, &ur1 ) ) {
      return false;
    }
    float h1 = ur1.value(zond::UR_H);

    float T = ur1.value(zond::UR_T);
    float leftT = T + 10.0;
    float rightT = leftT + 12.0;

    GeoVector skelet1;
    skelet1.append( GeoPoint( leftT, prev_p1, 0, LA_GENERAL ) );
    skelet1.append( GeoPoint( rightT, prev_p1, 0, LA_GENERAL ) );

    auto line = new GeoPolygon(l);
    line->setOrnament("wavyline");
    line->setPenWidth(1);
    line->setPenColor(Qt::blue);
    line->setSkelet(skelet1);
    auto lbl = new GeoText(l);
    lbl->setPenColor(Qt::black);
    lbl->setFontPointSize(10);
    lbl->setPos(kRightCenter);
    lbl->setText( QString::number( h1, 'f', 0 ) + QObject::tr(" м") );
    lbl->setSkelet( GeoPoint( rightT, p, 0, LA_GENERAL ) );
    lbl->setTranslate(kLabelOffset5, 0);

    zond::Uroven ur2;
    if ( false == data_.zond().getUrPoP( prev_p2, &ur2 ) ) {
      return false;
    }
    float h2 = ur2.value(zond::UR_H);

    GeoVector skelet2;
    skelet2.append( GeoPoint( leftT, prev_p2, 0, LA_GENERAL) );
    skelet2.append( GeoPoint( rightT, prev_p2, 0, LA_GENERAL) );
    line = new GeoPolygon(l);
    line->setOrnament("wavyline");
    line->setPenWidth(1);
    line->setPenColor(Qt::blue);
    line->setSkelet(skelet2);
    lbl = new GeoText(l);
    lbl->setPenColor(Qt::black);
    lbl->setFontPointSize(10);
    lbl->setPos(kRightCenter);
    lbl->setText( QString::number( h2,'f',0) + QObject::tr(" м") );
    lbl->setSkelet(GeoPoint( rightT, p, 0, LA_GENERAL ) );
    lbl->setTranslate( kLabelOffset5, 0 );

    for ( int i = 0; i < skelet2.size(); ++i ) {
      skelet1.prepend( skelet2.at(i) );
    }
    line = new GeoPolygon(l);
    line->setPenWidth(2);
    line->setPenColor(blue);
    line->setBrushColor(blue);
    line->setBrushStyle(Qt::SolidPattern);
    line->setClosed(true);
    line->setSkelet(skelet1);

    lbl = new GeoText(l);
    lbl->setPenColor(Qt::black);
    lbl->setFontPointSize(20);
    lbl->setPos(kCenter);
    if ( 2000.0 > h1 ) {
      if (MnMath::isEqual( 37, data_.zond().cloud().value(zond::CL))
          && MnMath::isEqual(22, data_.zond().cloud().value(zond::CM)) ) {
        str.append("St fr. Ns ");
      }
      else {
        if ( 300.0 > h1 ) {
          str.append("St ");
        }
        else {
          str.append("Sc ");
        }
      }
      if ( 2000.0 < h2 ) {
        str.append("As ");
        if ( 6000.0 < h2 ) {
          str.append("Cs ");
        }
      }
    }
    if ( 2000.0 <= h1 && 6000.0 > h1 ) {
      str.append("As ");
      if ( 6000.0 < h2 ) {
        str.append("Cs ");
      }
    }
    if ( 6000.0 <= h1 ) {
      str.append("Cs ");
    }
    if ( MnMath::isZero( data_.zond().cloud().value(zond::Nh)) ) {
      kol_obl = 1;
    }
    else {
      kol_obl = data_.zond().cloud().value(zond::Nh);
    }

    str.append( QObject::tr("%1 б.").arg(kol_obl) );
    lbl->setText(str);
    str.clear();
    lbl->setSkelet( GeoPoint( ( leftT + rightT )/2.0, ( prev_p1 + prev_p2 )/2.0, 0, LA_GENERAL ) );

    prev_p1 = p1;
    prev_p2 = p2;
  }

  return true;
}

bool AeroDocument::addBoltan()
{
  if( 0 == data_.zond().urovenList().size() ) {
    return false;
  }

  if ( false == prepLayer(kAeroBolt) ) {
    return false;
  }
  auto l = layerByCustomType(kAeroBolt);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[kAeroBolt] );
    return false;
  }

  QList<float> gran_bolt;
  data_.zond().oprGranBoltan(&gran_bolt);
  if( 0 == gran_bolt.size() ) {
    return false;
  }
  if( 1 == gran_bolt.size()%2 ) {
    float Pz=0;
    if ( data_.zond().getPz(&Pz) ) {
      gran_bolt.prepend(Pz);
    }
    else {
      gran_bolt.prepend(1050.0);
    }
  }

  float p1 = 0.0, p2 = 0.0, prev_p1 = 0.0, prev_p2 =0.0;
  for ( int i = 0; i < gran_bolt.size() + 2; i += 2 ) {
    if ( i == gran_bolt.size()  ) {
      p1 = 10.0;
      p2 = 5.0;
    }
    else {
      p1 = gran_bolt.at(i);
      p2 = gran_bolt.at(i+1);
    }

    if ( 0 == i ) {
      prev_p1 = p1;
      prev_p2 = p2;
      if ( 2 == gran_bolt.size() ) {
  p1 = 10.0;
  p2 = 5.0;
  continue;
      }
    }

    if ( 1100.0 > ( data_.zond().oprHpoP(p1) - data_.zond().oprHpoP(prev_p2) ) ) {
      prev_p2 = p2;
      continue;
    }

    if( 100.0 > ( data_.zond().oprHpoP(prev_p2) - data_.zond().oprHpoP(prev_p1) ) ) {
      prev_p1 = p1;
      prev_p2 = p2;
      continue;
    }

    drawSectionPhenomen( prev_p1, prev_p2, 34, Qt::red, ":/meteo/icons/geopixmap/boltanka.png", l );

    prev_p1 = p1;
    prev_p2 = p2;
  }

  return true;
}

bool AeroDocument::addTropo( float* Ptropo )
{
  if ( false == prepLayer( kAeroTropo ) ) {
    return false;
  }
  auto l = layerByCustomType(kAeroTropo);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[kAeroTropo] );
    return false;
  }
  QList<zond::Uroven> ur_tropo;
  if( false == data_.zond().getTropo(&ur_tropo) || 0 == ur_tropo.size() ) {
    return false;
  }
  *Ptropo = ur_tropo.first().value(zond::UR_P);

  for (int idx = 0; idx < ur_tropo.size(); idx++) {
    addTropo( ur_tropo.at(idx), l );
  }
  return true;
}

void AeroDocument::addTropo( const zond::Uroven& ur_tropo, Layer* l )
{
  float t = ur_tropo.value(zond::UR_T);
  float p = ur_tropo.value(zond::UR_P);
  float h = ur_tropo.value(zond::UR_H);
  GeoVector skelet;
  GeoPoint p1 = GeoPoint( t, p, 0, LA_GENERAL);
  GeoPoint p2 = GeoPoint( t + 30.0, p, 0, LA_GENERAL);
  skelet.append(p1);
  skelet.append(p2);
  auto line = new GeoPolygon(l);
  line->setPenWidth(2);
  line->setPenColor(qRgb(162, 37, 0));
  line->setSkelet(skelet);

  auto lbl = new GeoText(l);
  lbl->setPenColor(Qt::black);
  lbl->setFontPointSize(20);
  lbl->setPos(kTopLeft);
  lbl->setDrawAlways(true);
  lbl->setText(QObject::tr("%1\xC2\xB0""C").arg(QString::number(t,'f',1)));
  lbl->setSkelet(p2);

  lbl = new GeoText(l);
  lbl->setPenColor(Qt::black);
  lbl->setFontPointSize(10);
  lbl->setPos(kRightCenter);
  lbl->setDrawAlways(true);
  lbl->setText(QString::number(h,'f',0) + QObject::tr(" м"));
  lbl->setSkelet(p2);
  lbl->setTranslate( kLabelOffset10, 0);
}

bool AeroDocument::addTrace(float Ptropo)
{
  if( 0 == data_.zond().urovenList().size() ) {
    return false;
  }

  if ( false == prepLayer( kAeroTrace ) ) {
    return false;
  }
  auto l = layerByCustomType(kAeroTrace);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[kAeroTrace] );
    return false;
  }

  QList<float> gran_trace;
  data_.zond().oprGranTrace(&gran_trace);
  if ( 0 == gran_trace.size() ) {
    return false;
  }
  MnCommon::reverseContainer( &gran_trace );
  if ( 1 == gran_trace.size()%2 ) {
    float Pz = 0.0;
    if ( true == data_.zond().getPz(&Pz) ) {
      gran_trace.prepend(Pz);
    }
    else {
      gran_trace.prepend(1050.0);
    }
  }

  float p1 = 0.0, p2 = 0.0, prev_p1 = 0.0, prev_p2 = 0.0;
  float T = BAD_METEO_ELEMENT_VAL;
  for ( int i=0; i < gran_trace.size() + 2; i += 2 ) {
    if ( i == gran_trace.size() ) {
      p1 = 10.0;
      p2 = 5.0;
    }
    else {
      p1 = gran_trace.at(i);
      p2 = gran_trace.at( i + 1 );
    }
    if ( 0 == i ) {
      prev_p1 = p1;
      prev_p2 = p2;
      if( 2 ==  gran_trace.size() ) {
  p1 = 10.0;
  p2 = 5.0;
  continue;
      }
    }

    if ( 1100.0 >  ( data_.zond().oprHpoP(p1) - data_.zond().oprHpoP(prev_p2) ) ) {
      prev_p2 = p2;
      continue;
    }

    if ( 100.0 > ( data_.zond().oprHpoP(prev_p2) - data_.zond().oprHpoP(prev_p1) ) ) {
      prev_p1 = p1;
      prev_p2 = p2;
      continue;
    }

    zond::Uroven ur;
    data_.zond().getUrPoP( prev_p1, &ur );
    T = ur.value(zond::UR_T);
    Qt::PenStyle lineStyle = Qt::SolidLine;
    if ( prev_p1 < Ptropo ) {
      lineStyle = Qt::DashLine;
    }

    drawSectionPhenomen( prev_p1, prev_p2, T + 3, Qt::blue, ":/meteo/icons/geopixmap/kond.sledi.png", l, lineStyle);
    prev_p1=p1;
    prev_p2=p2;
  }

  return true;
}

bool AeroDocument::addBlankLines()
{
  if ( false == prepLayer( kAeroLines ) ) {
    return false;
  }
  auto l = layerByCustomType(kAeroLines);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[kAeroLines] );
    return false;
  }
  l->setVisisble(false);
  l->setBase(true);
  addSuhAdiab();
  addVlazhAdiab();
  addUdelVlazh();
  return true;
}

bool AeroDocument::addBlank()
{
  if ( false == prepLayer( kAeroBlank1 )
       || false == prepLayer( kAeroBlank2 )
       || false == prepLayer( kAeroBlank3 )
       ) {
    return false;
  }

  auto l1 = layerByCustomType(kAeroBlank1);
  auto l2 = layerByCustomType(kAeroBlank2);
  auto l3 = layerByCustomType(kAeroBlank3);

  l1->setBase(true);
  l2->setBase(true);
  l3->setBase(true);

  addSetka();
  addSA81();

  return true;
}

bool AeroDocument::addSetka()
{

  float minP = kIsoLevels.first();
  float maxP = kIsoLevels.last();
  float minT = -80.0;
  float maxT = 40.0;
  meteo::Property prop;
  meteo::Property prop1;
  QPen pen;
  pen.setColor(qRgb(80, 255, 80));
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(2);
  prop1.mutable_pen()->CopyFrom( qpen2pen(pen) );
  meteo::Property prop2;
  pen.setColor(qRgb(150, 255, 150));
  pen.setStyle(Qt::SolidLine);
  pen.setWidthF(1.5);
  prop2.mutable_pen()->CopyFrom( qpen2pen(pen) );
  meteo::Property prop3;
  pen.setColor(qRgb(150, 255, 150));
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(1);
  prop3.mutable_pen()->CopyFrom(qpen2pen(pen));

  GeoVector skelet;
  // горизонтальные
  for( float p = 30.0; p <= maxP; p +=10.0 ) {
    skelet.append(GeoPoint( minT - 7, p, 0, LA_GENERAL ) );
    skelet.append(GeoPoint( maxT, p, 0, LA_GENERAL ) );
    if ( true == kIsoLevels.contains(p) ) {
      prop.CopyFrom(prop1);
      addLine( skelet, prop, kAeroBlank1 );
    }
    else if((static_cast<int>(p)%50)==0) {
      prop.CopyFrom(prop2);
      addLine( skelet, prop, kAeroBlank2 );
    }
    // else {
    //   prop.CopyFrom(prop3);
    //   addLine( skelet, prop, kAeroBlank3 );
    // }
    skelet.clear();
  }
  for( float t = minT; t <= maxT; t += 1.0 ) {
    skelet.append(GeoPoint(t,minP,0,LA_GENERAL));
    skelet.append(GeoPoint(t,maxP,0,LA_GENERAL));
    if ( 0 == (static_cast<int>(t)%10) ) {
      prop.CopyFrom(prop1);
      addLine( skelet, prop, kAeroBlank1 );
    }
    else if( 0 == (static_cast<int>(t)%5) ) {
      prop.CopyFrom(prop2);
      addLine( skelet, prop, kAeroBlank2 );
    }
    // else {
    //   prop.CopyFrom(prop3);
    //   addLine( skelet, prop, kAeroBlank3 );
    // }
    skelet.clear();
  }
//  skelet.append( GeoPoint( minT - 7.0, minP, 0, LA_GENERAL ) );
//  skelet.append( GeoPoint( minT - 7.0, maxP, 0, LA_GENERAL ) );
//  addLine( skelet, prop1, kAeroBlank1 );
  return true;
}

bool AeroDocument::addSA81()
{
  GeoVector fl;
  QString new_text;

  if ( false == addSA81Line() ) {
    return false;
  }

  if ( false == addSA81BlankText() ) {
    return false;
  }


  meteo::Property prop2;
  prop2.mutable_pen()->set_color( QColor(Qt::green).rgba() );
  prop2.mutable_pen()->set_style(static_cast<PenStyle>( Qt::SolidLine ) );
  prop2.mutable_pen()->set_width(2);
  fl.append( GeoPoint( -93.0, standartAtm.at(0).first, 0, LA_GENERAL ) );
  fl.append( GeoPoint( -93.0, standartAtm.last().first, 0, LA_GENERAL ));
  addLine( fl, prop2, kAeroBlank1 );
  fl.clear();

  meteo::Property textProp3;
  textProp3.mutable_pen()->set_color( QColor(Qt::darkGreen).rgba() );
  textProp3.set_pos(kLeftCenter);
  textProp3.mutable_pen()->set_style( static_cast<PenStyle>( Qt::SolidLine));
  textProp3.mutable_font()->set_family("Helvetica");
  textProp3.mutable_font()->set_pointsize(12);

  for ( int j = 0; j < standartAtm.size(); ++j ) {
    fl.append( GeoPoint( -96.0, standartAtm.at(j).first, 0, LA_GENERAL ) );
    fl.append( GeoPoint( -93.0, standartAtm.at(j).first, 0, LA_GENERAL ) );
    addLine( fl, prop2, kAeroBlank1 );
    fl.remove(1);
    new_text = QString("%1").arg(j);
    addText( fl, new_text, textProp3, kAeroBlank1 );
    fl.clear();
    if ( j + 1 < standartAtm.size() ) {
      fl.append( GeoPoint( -94.5, (standartAtm.at(j).first + standartAtm.at(j+1).first)/2.0, 0, LA_GENERAL ) );
      fl.append( GeoPoint( -93.0, (standartAtm.at(j).first + standartAtm.at(j+1).first)/2.0, 0, LA_GENERAL ) );
      addLine( fl, prop2, kAeroBlank1 );
      fl.clear();
    }
  }

  fl.append( GeoPoint( 45.0, standartAtm.at(0).first, 0, LA_GENERAL ) );
  fl.append( GeoPoint( 45.0, standartAtm.last().first, 0, LA_GENERAL ) );
  addLine( fl, prop2, kAeroBlank1 );
  fl.clear();

  meteo::Property textProp4(textProp3);
  textProp4.set_pos(::meteo::kRightCenter);
  for ( int j = 0;j < 17; ++j ) {
    fl.append(GeoPoint(48., standartAtm.at(j).first, 0, LA_GENERAL));
    fl.append(GeoPoint(45., standartAtm.at(j).first, 0, LA_GENERAL));
    addLine( fl, prop2, kAeroBlank1 );
    fl.remove(1);
    new_text = QString("%1").arg(j);
    addText( fl, new_text, textProp4, kAeroBlank1 );
    fl.clear();
    if ( j + 1 < standartAtm.size()) {
      fl.append(GeoPoint(46.5,(standartAtm.at(j).first + standartAtm.at(j+1).first)/2.,0,LA_GENERAL));
      fl.append(GeoPoint(45., (standartAtm.at(j).first + standartAtm.at(j+1).first)/2.,0,LA_GENERAL));
      addLine( fl, prop2, kAeroBlank1 );
      fl.clear();
    }
  }
  return true;
}

bool AeroDocument::addSA81Line()
{
  GeoVector fl;

  meteo::Property prop1;
  QPen pen;
  pen.setColor(Qt::red);
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(2);
  prop1.mutable_pen()->CopyFrom(qpen2pen(pen) );

  fl.append(GeoPoint(0.,1050.,0,LA_GENERAL));
  fl.append(GeoPoint(0.,300.,0,LA_GENERAL));
  addLine( fl, prop1, kAeroBlank1 );
  fl.clear();

  fl.append(GeoPoint(-60.,1050.,0,LA_GENERAL));
  fl.append(GeoPoint(-60.,300.,0,LA_GENERAL));
  addLine( fl, prop1, kAeroBlank1 );
  fl.clear();

  for( int i = 0; i < standartAtm.size(); ++i ) {
    fl.append(GeoPoint(standartAtm.at(i).second, standartAtm.at(i).first, 0, LA_GENERAL));
  }
  addLine( fl, prop1, kAeroBlank1 );
  fl.clear();

  meteo::Property textProp1;
  textProp1.mutable_pen()->set_color(QColor(Qt::red).rgba());
  textProp1.set_pos(::meteo::kTopCenter);
  textProp1.mutable_pen()->set_style(static_cast<PenStyle>(Qt::SolidLine));
  textProp1.mutable_font()->set_family("Helvetica");
  textProp1.mutable_font()->set_pointsize(14);

  fl.append(GeoPoint(-56.5,100.,0,LA_GENERAL));
  addText( fl, QObject::tr("CA-95"), textProp1, kAeroBlank1 );
  fl.clear();

  return true;
}

void AeroDocument::addSA81BlankText( int* value, int size, float Tstart, float Tstep, float P, const meteo::Property& prop )
{
  GeoVector fl;
  float temp = Tstart;
  for ( int idx = 0; idx < size; ++idx, temp += Tstep ) {
    fl.append(GeoPoint(temp, P,0,LA_GENERAL));
    QString text = QString("%1").arg(value[idx]);
    addText( fl, text, prop, kAeroBlank1 );
    fl.clear();
  }
}

bool AeroDocument::addSA81BlankText()
{
  float Tpop[14] = { 4.6, 4.4, 4.2, 4.0, 3.8, 3.6, 3.3, 3.0, 2.7, 2.3, 1.9, 1.5, 1.0, 0.5 };
  int H1[9] = { 105,110,115,120,125,130,135,140,145 };
  int H2[11]={125,130,135,140,145,150,155,160,165,170,175 };
  int H3[10]={210,220,230,240,250,260,270,280,290,300 };
  int H4[12]={310,320,330,340,350,360,370,380,390,400,410,420 };
  int H5[8]={240,250,260,270,280,290,300,310 };
  int H6[10]={400,410,420,430,440,450,460,470,480,490};

  meteo::Property textProp2;
  textProp2.mutable_pen()->set_color(QColor(Qt::darkGreen).rgba());
  textProp2.set_pos(kTopCenter);
  textProp2.mutable_pen()->set_style( static_cast<PenStyle>(Qt::SolidLine));
  textProp2.mutable_font()->set_family("Helvetica");
  textProp2.mutable_font()->set_pointsize(10);

  GeoVector fl;
  for (int idx = 0, temp = -70; idx < 15; idx++, temp += 5) {
    fl.append(GeoPoint(temp, 525. , 0, LA_GENERAL));
    QString new_text = QString("+") + QString::number(Tpop[idx],'f',1);
    addText( fl, new_text, textProp2, kAeroBlank1 );
    fl.clear();
  }
  addSA81BlankText(H1, sizeof(H1)/sizeof(H1[0]), -52, 10.5, 950, textProp2);
  addSA81BlankText(H2, sizeof(H2)/sizeof(H2[0]), -53, 8.8, 800, textProp2);
  addSA81BlankText(H3, sizeof(H3)/sizeof(H3[0]), -60, 10.16, 600, textProp2);
  addSA81BlankText(H4, sizeof(H4)/sizeof(H4[0]), -65.5, 6.68, 400, textProp2);
  addSA81BlankText(H5, sizeof(H5)/sizeof(H5[0]), -71, 8.43, 250, textProp2);
  addSA81BlankText(H6, sizeof(H6)/sizeof(H6[0]), -76, 4.94, 150, textProp2);

  return true;
}

bool AeroDocument::addSuhAdiab()
{
  auto layer = layerByCustomType(kAeroLines);
  if ( nullptr == layer ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[kAeroLines] );
    return false;
  }

  meteo::Property prop1;
  QPen pen;
  pen.setColor(Qt::darkGreen);
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(1);
  prop1.mutable_pen()->CopyFrom( qpen2pen(pen) );

  // meteo::Property textProp;
  // textProp.mutable_pen()->set_color(QColor(Qt::darkGreen).rgba());
  // textProp.set_pos(proto::kTopCenter);
  // textProp.mutable_pen()->set_style( static_cast<PenStyle>( Qt::SolidLine));
  // textProp.mutable_font()->set_family("Helvetica");
  // textProp.mutable_font()->set_pointsize(10);

  float minT[49]={-80.,-80.,-80.,-80.,-80.,-80.,-80.,-80.,-80.,-80.,-80.,-80.,-80.,-80.,
       -80.,-80.,-80.,-80.,-80.,-80.,-80.,-80.,-80.,-80.,-80.,-80.,-80.,-80.,
       -80.,-80.,-80.,-80.,-80.,-80.,-80.,-77.307,-74.718, -72.128,-69.539,
       -66.949,-64.36,-61.771,-59.181,-56.592,-54.002,-48.823,-43.644,-38.465,-33.286};
  float maxT[49]={-67.148,-62.078,-57.007,-51.937,-46.867,-41.797,-36.727,-31.656,-26.586,
       -21.516,-16.446,-11.376,-6.305,-1.235,3.835,8.905,13.975,19.046,24.116,29.186,
       34.256,39.326,40.,40.,40.,40.,40.,40.,40.,40.,40.,40.,40.,40.,40.,40.,
       40.,40.,40.,40.,40.,40.,40.,40.,40.,-30.,-30.,-30.,-30.};
  float potT[49]={-70.,-65.,-60.,-55.,-50.,-45.,-40.,-35.,-30.,-25.,-20.,-15.,-10.,-5.,
       0.,5.,10.,15.,20.,25.,30.,35.,40.,45.,50.,55.,60.,65.,70.,75.,80.,85.,
       90.,95.,100.,105.,110.,115.,120.,125.,130.,135.,140.,145.,150.,160.,170.,180.,190.};


   for(int i = 0; i < 49; ++i ) {
    GeoVector fl;

    float curTemp = minT[i]+0.5;
    for (; curTemp < maxT[i]; curTemp += 5) {
      //float p = 1000./powf(((potT[i]+273.15)/(curTemp+273.15)), 1004.7/287.05);
      float p = 1000./powf(((potT[i]+273.15)/(curTemp+273.15)), 1/zond::kKap);
      fl.append(GeoPoint(curTemp, p, 0, LA_GENERAL));
    }
    curTemp = maxT[i] - 0.5;
    float p = 1000./powf(((potT[i]+273.15)/(curTemp+273.15)), 1/zond::kKap);
    fl.append(GeoPoint(curTemp, p, 0, LA_GENERAL));


    map::GeoPolygon* line = new GeoPolygon(layer);
    line->setProperty(prop1);
    line->setSkelet(fl);

    if (0 == (int(potT[i]) % 2)) {
      //QString new_text = QString::number(potT[i],'f',0);
      // //подписи справа
      // if(potT[i]>=40.)  {
      // 	GeoVector fl_vs = GeoPoint(fl.at(1).fi()-2.,fl.at(1).la(),0,LA_GENERAL);
      // 	addText(fl_vs, new_text, textProp, kAeroLines);
      // }
      // //подписи слева
      // if ( ( potT[i]<=100.) && (!(((int) potT[i])%2) ) ) {
      // 	GeoVector fl_vs = GeoPoint(fl.at(0).fi()+2.,fl.at(0).la()-10.,0,LA_GENERAL);
      // 	addText(fl_vs, new_text, textProp, kAeroLines );
      // }

      line->setValue(potT[i], "3' '.0'0'");
    }
  }

  return true;
}

bool AeroDocument::addVlazhAdiab()
{
  GeoVector fl;
  int i,kol;
  float p0,p1;
  GeoVector fl_vs;
  QString new_text;
  float t3, Tet0, p = 0.0, Tet1, Tet, ss;

  meteo::Property prop1;
  QPen pen;
  pen.setColor(Qt::red);
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(1);
  prop1.mutable_pen()->CopyFrom( qpen2pen(pen) );

  meteo::Property textProp;
  textProp.mutable_pen()->set_color(QColor(Qt::red).rgba());
  textProp.set_pos(::meteo::kTopCenter);
  textProp.mutable_pen()->set_style( static_cast<PenStyle>( Qt::SolidLine));
  textProp.mutable_font()->set_family("Helvetica");
  textProp.mutable_font()->set_pointsize(10);

  float s[39] = {150.,145.,140.,135.,130.,125.,120.,115.,110.,105.,100.,95.,90.,
         85.,80.,75.,70.,65.,60.,55.,50.,45.,40.,35.,30.,25.,20.,15.,10.,
         5.,0.,-5.,-10.,-15.,-20.,-25.,-30.,-35.,-40.};
  for(i=0;i<39;i++) {
    ss=s[i];
    kol=0;
    for( t3 = 40.; t3 > -60.; t3-=1.) {
      p0=1050.;
      p1=100.;
      if ( false == zond::oprTPot(p0, t3, &Tet0)) {
        continue;
      }
      if ( false == zond::oprTPot(p1, t3, &Tet)) {
  continue;
      }

      Tet1 = Tet;


      if((Tet0-ss)*(Tet1-ss)>0.) {
        continue;
      }
      float delta = 0.0001f;
      while (!MnMath::isEqual(Tet,ss,delta)) {
        p=(p0+p1)*0.5;
        if ( false == zond::oprTPot(p, t3, &Tet)) { //!TODO
  }
        if ( ( Tet1 - ss )*( Tet - ss ) > 0.0 ) {
          p1 = p;
          Tet1 = Tet;
        } else {
          p0 = p;
          Tet0 = Tet;
        }
      }
      fl.append(GeoPoint(t3,p,0,LA_GENERAL));
      ++kol;
    }
    if ( 0 != fl.size() ) {
      fl_vs.append(GeoPoint(fl.last().fi(),fl.last().la(),0,LA_GENERAL));
      addLine(fl,prop1, kAeroLines );
    }
    fl.clear();
    new_text = QString::number(s[i],'f',1);
    if ( ( s[i] >= -40.0 ) && ( s[i] <= 140.0 ) && ( false == (((int)s[i])%2) ) ) {
      addText(fl_vs,new_text,textProp, kAeroLines);
    }
    fl_vs.clear();
  }

  return true;
}

bool AeroDocument::addUdelVlazh(){

  GeoVector fl;
  int i, j;
  float logE, E, T,p;
  GeoVector fl_vs;
  QString new_text;
  float dT;


  meteo::Property prop1;
  QPen pen;
  pen.setColor(Qt::red);
  pen.setStyle(Qt::DashLine);
  pen.setWidth(1);
  prop1.mutable_pen()->CopyFrom( qpen2pen(pen) );

  meteo::Property textProp;
  textProp.mutable_pen()->set_color(QColor(Qt::red).rgba());
  textProp.set_pos(::meteo::kTopCenter);
  textProp.mutable_pen()->set_style( static_cast<PenStyle>( Qt::SolidLine));
  textProp.mutable_font()->set_family("Helvetica");
  textProp.mutable_font()->set_pointsize(10);

  int numP=20;
  float minT[39]={-61., -55.45, -47.6, -41.2, -34.36, -30.1, -27.05, -24.6, -22.55,
       -20.78, -19.25, -17.85, -16.6, -14.4,  -12.5, -10.82, -9.34, -8.,
       -5.08, -2.65, -0.55, 1.3, 2.95, 4.45, 7.1, 9.3, 11.3, 13.1, 14.75,
       17.6, 20.1, 22.2, 24.2, 26., 29.8, 33.05, 35.8, 38.2, 40.};
  float maxT[39]={-70., -70., -66., -61., -55.8, -52.4, -49.89, -47.95, -46.3, -44.9,
       -43.7, -42.5, -41.5, -39.7, -38.3, -37., -35.8, -34.8, -28.5, -26.5,
       -24.8, -23.2, -21.9, -20.6, -18.5, -13.5, -11.5, -10., -8.5, -6.5,
       1., 3., 4.5, 6., 17.5, 20., 22.5, 30.5, 32.5};
  float s[39]={0.01, 0.02, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1., 1.2,
         1.4, 1.6, 1.8, 2., 2.5, 3., 3.5, 4., 4.5, 5., 6., 7., 8., 9., 10., 12.,
         14., 16., 18., 20., 25., 30., 35., 40., 45.};

  for ( i = 0; i < 39; ++i ) {
    dT=(maxT[i]-minT[i])/(numP-1);
    for(j=0;j<numP;j++) {
      T = minT[i]+j*dT;
      logE = 10.79574*(1.-273.16/ (T+273.15) )-5.02800*::log10( (T+273.15)/273.16)
        + 1.50475e-4*(1.-pow(10., -8.2969*((T+273.15)/273.16-1.) ))
        + 0.42873e-3*(pow (10., 4.76955*(1.-273.16/(T+273.15)))-1.)
        + 0.78614;
      E = pow(10.,logE);
      p = 622.*E/s[i];
      fl.append(GeoPoint(T,p,0,LA_GENERAL));
    }
    fl_vs.append(GeoPoint(fl.last().fi(),fl.last().la(),0,LA_GENERAL));
    addLine(fl, prop1, kAeroLines );
    fl.clear();
    new_text = QString::number(s[i],'f',2);
    addText( fl_vs, new_text, textProp, kAeroLines );
    fl_vs.clear();
  }

  return true;
}

bool AeroDocument::createEmptyBlank()
{
  addBlank();
  addBlankLines();
  createAxis();
  return true;
}


void AeroDocument::createAxis()
{
  auto l = layerByCustomType(kAeroAxis);
  if ( nullptr != l ) {
    delete l;
    l=nullptr;
  }

  const float minT = -80.0;
  const float maxT = 40.0;

  LayerAxis* layer = new LayerAxis( this, kLayerNames[kAeroAxis], kAeroAxis );
  layer->setBase(true);

  layer->xAxis2()->setVisible(true);
  layer->yAxis2()->setVisible(true);

  for ( map::AxisSimple* x : { layer->xAxis(), layer->xAxis2() } ) {
    x->setSubTickCount(0);
    x->setRange(minT,maxT);
    x->setTickLabelType(map::AxisSimple::kltNumber);
    x->setTickLabelFormat("f,0,°", 20);

    x->setTickVector(mkSequence(minT, maxT, 10), 20);
  }

  for ( map::AxisSimple* y : { layer->yAxis(), layer->yAxis2() } ) {
    y->setSubTickCount(0);
    y->setRange(kIsoLevels.first(),kIsoLevels.last());
    y->setTickLabelType(map::AxisSimple::kltNumber);
    y->setTickLabelFormat("f,0,", 20);
    y->setTickLabelFormat("f,0,", 18);
    y->setTickLabelFormat("f,0,", 16);
    y->setTickLabelFormat("f,0,", 14);
    y->setTickLabelFormat("f,0,", 12);

    y->setTickVector(kIsoLevels, 20);
    y->setTickVector(mkSequence(kIsoLevels.first(),kIsoLevels.last(),50), 18);
    y->setTickVector(mkSequence(kIsoLevels.first(),kIsoLevels.last(),25), 16);
    y->setTickVector(mkSequence(kIsoLevels.first(),kIsoLevels.last(),10), 14);
    y->setTickVector(mkSequence(kIsoLevels.first(),kIsoLevels.last(), 5), 12);
    y->setLabel(QObject::tr("гПа"));
  }

  l = layerByCustomType(kAeroAltAxis);
  if ( nullptr != l ) {
    delete l;
  }

  layer = new LayerAxis( this, QObject::tr("Шкала высоты"), kAeroAltAxis );
  layer->xAxis()->setVisible(false);
  layer->yAxis2()->setVisible(true);
  for ( map::AxisSimple* y : { layer->yAxis(), layer->yAxis2() } ) {
    y->setSubTickCount(1);
    y->setRange(kIsoLevels.first(),kIsoLevels.last());

    y->setTickVector(mkSequenceSa81ByH(0,16,1.0), 20);
    y->setTickVector(mkSequenceSa81ByH(0,16,0.5), 16);
    y->setTickVector(mkSequenceSa81ByH(0,16,0.1), 14);

    y->setTickLabelVector(mkSequence(0,16,1.0,0), 20);
    y->setTickLabelVector(mkSequence(0,16,0.5,1), 16);
    y->setTickLabelVector(mkSequence(0,16,0.1,1), 14);

    y->setLabel(QObject::tr("км"));
  }
}

bool AeroDocument::addLinePodpis( const GeoVector& line, Layer* layer )
{
  if ( nullptr == layer ) {
    return false;
  }
  float h = -1.0;

  for ( int i = 0, sz = line.size(); i < sz; ++i ) {
    const auto& tc = line[i];
    h = data_.zond().oprHpoP( tc.la() );
    if (MnMath::isEqual(BAD_METEO_ELEMENT_VAL, h)) {
      continue;
    }
    auto label = new GeoText(layer);
    label->setPenColor(Qt::black);
    label->setFontPointSize(10);
    label->setPos( kRightCenter );
    label->setText( QString::number( h/1000.0, 'f' , 1 ) );
    label->setSkelet(tc);
    label->setTranslate( kLabelOffset10, 0 );
  }
  return true;
}

bool AeroDocument::addLine(const GeoVector& skelet, const meteo::Property& prop, LayerType type)
{
  auto l = layerByCustomType(type);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[type] );
    return false;
  }
  map::GeoPolygon* line = new GeoPolygon(l);
  line->setProperty(prop);
  line->setSkelet(skelet);
  return true;
}

GeoPolygon* AeroDocument::addLine( Layer* layer, const GeoVector& skelet, int width, const QColor& color )
{
  auto line = new GeoPolygon(layer);
  line->setPenWidth(width);
  line->setPenColor(color);
  line->setSkelet(skelet);

  return line;
}

bool AeroDocument::addText(const GeoVector& skelet, const QString & text, const meteo::Property& prop, LayerType type )
{
  auto l = layerByCustomType(type);
  if ( nullptr == l ) {
    error_log << QObject::tr("Не найден слой '%1'. Непредвиденная ситуация.")
      .arg( kLayerNames[type] );
    return false;
  }

  auto lbl = new GeoText(l);
  lbl->setProperty(prop);
  lbl->setText(text);
  lbl->setSkelet(skelet);
  return true;
}

meteo::map::GeoText* AeroDocument::addText( Layer* layer, const QString& text, const GeoPoint& pnt, int fntsz, const QColor& clr )
{
  auto lbl = new GeoText(layer);
  lbl->setPenColor(clr);
  lbl->setFontPointSize(fntsz);
  lbl->setText(text);
  lbl->setSkelet(pnt);

  return lbl;
}
void AeroDocument::drawSectionPhenomen( float p_hi,
                                        float p_lo,
                                        float temp,
                                        const QColor& lineColor,
                                        const QString& pixmap,
                                        Layer* layer,
                                        Qt::PenStyle lineStyle )
{
  if ( nullptr == layer ) {
    return;
  }
  float leftT = temp - 1.0, rightT = temp + 1.0;

  GeoVector skelet;
  skelet.append( GeoPoint( leftT, p_hi, 0, LA_GENERAL ) );
  skelet.append( GeoPoint( rightT, p_hi, 0, LA_GENERAL ) );
  addLine( layer, skelet, 2, lineColor );

  float h1 = data_.zond().oprHpoP(p_hi);
  auto lbl = new GeoText(layer);
  lbl->setPenColor(Qt::black);
  lbl->setFontPointSize(10);
  lbl->setPos(kRightCenter);
  lbl->setText( QString::number( h1, 'f', 0 ) + QObject::tr(" м") );
  lbl->setSkelet(GeoPoint( rightT, p_hi, 0, LA_GENERAL ) );
  lbl->setTranslate(kLabelOffset5, 0);

  skelet.clear();
  skelet.append( GeoPoint( leftT, p_lo, 0, LA_GENERAL ) );
  skelet.append( GeoPoint( rightT, p_lo, 0, LA_GENERAL ) );
  addLine( layer, skelet, 2, lineColor );
  float h2 = data_.zond().oprHpoP(p_lo);
  lbl = new GeoText(layer);
  lbl->setPenColor(Qt::black);
  lbl->setFontPointSize(10);
  lbl->setPos(kRightCenter);
  lbl->setText( QString::number( h2, 'f', 0 ) + QObject::tr(" м") );
  lbl->setSkelet( GeoPoint( rightT, p_lo, 0, LA_GENERAL ) );
  lbl->setTranslate(kLabelOffset5, 0);

  skelet.clear();
  skelet.append( GeoPoint( (leftT+rightT)/2.0, p_hi, 0, LA_GENERAL ) );
  skelet.append( GeoPoint( (leftT+rightT)/2.0, p_lo, 0, LA_GENERAL ) );
  auto line = addLine( layer, skelet, 2, lineColor );
  line->setArrowPlace(kBothArrow);
  line->setPenStyle(lineStyle);

  int y1 = Document::coord2screen( skelet.at(0) ).y();
  int y2 = Document::coord2screen( skelet.at(1) ).y();
  int x =  Document::coord2screen( GeoPoint( rightT, p_lo, 0, LA_GENERAL ) ).x();
  GeoPoint gp = Document::screen2coord( QPoint( x, ( y1 + y2 )/2.0 ) );
  auto pix = new GeoPixmap(layer);
  pix->setImage( QImage(pixmap) );
  pix->setPenColor(lineColor);
  pix->setPos(kRightCenter);
  pix->setScaleXy( QPointF( 0.15, 0.15 ) );
  pix->setDrawAlways(true);
  pix->setSkelet(gp);
  pix->setTranslate(kLabelOffset5, 0);
}

bool AeroDocument::prepLayer( LayerType type )
{
  auto l = layerByCustomType(type);
  if ( nullptr != l ) {
    delete l;
  }

  if ( kAeroAxis == type ) {
    new LayerAxis( this, kLayerNames[type], type );
  }
  else if ( kAeroWindIso == type || kAeroWindAdd == type ) {
    QMap< QString, meteo::puanson::proto::Puanson > map = WeatherLoader::instance()->punchlibraryspecial();

    LayerAxis* layer = new LayerAxis( this, kLayerNames[type], type );
    layer->xAxis()->setVisible(false);

    AxisPuanson* y = new AxisPuanson(layer);
    y->setAxisType(map::AxisPuanson::kLeft);
    y->setPuanson(map["aero_wind"]);
    y->setRange(kIsoLevels.first(),kIsoLevels.last());
    y->setLabel(QObject::tr("км/ч"));
    layer->setAxisY(y);

    y = new AxisPuanson(layer);
    y->setAxisType(map::AxisPuanson::kRight);
    y->setPuanson(map["aero_wind"]);
    y->setRange(kIsoLevels.first(),kIsoLevels.last());
    y->setLabel(QObject::tr("км/ч"));
    layer->setAxisY2(y);
  }
  else {
    new Layer( this, kLayerNames[type], type );
  }

  return true;
}

bool AeroDocument::logF2X_one( const Projection& proj, const GeoPoint& ageoCoord, QPoint* meterCoord )
{
//  GeoPoint start = GeoPoint(begFi,begLa,0,LA_GENERAL);
//  GeoPoint end = GeoPoint(endFi,endLa,0,LA_GENERAL);

  GeoPoint geoCoord(ageoCoord);

  if(geoCoord.fi() > proj.endFi()) {
    geoCoord.setFi(proj.endFi());
  }
  else if (geoCoord.fi()< proj.startFi()) {
    geoCoord.setFi(proj.startFi());
  }

  if (geoCoord.la() > proj.startLa()) {
    geoCoord.setLa(proj.startLa());
  }
  else if (geoCoord.la() < proj.endLa()) {
    geoCoord.setLa(proj.endLa());
  }

  float lon = geoCoord.lon() - proj.getMapCenter().lon();
  float lat = geoCoord.lat() - proj.getMapCenter().lat();

  float p = lon;
  float h;
  float t;
  zond::SA81_H_ext(p,&t,&h);


  h=maxH-h;
  lon = h/80.;

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

bool AeroDocument::logX2F_one( const Projection& proj, const QPoint& meterCoord, GeoPoint* geoCoord )
{
  if ( nullptr == geoCoord ) {
    error_log << QObject::tr("Нулевой указатель GeoPoint*");
    return false;
  }
  *geoCoord = GeoPoint( 0,0,0, LA_GENERAL );

  float x = meterCoord.x();
  float y = meterCoord.y();
  y = y/(kRVSG*proj.yfactor());
  float h = y*80.;
  h=maxH-h;
  float p;
  float t;
  zond::SA81_P_ext(h,&t,&p);
  float lat = proj.getMapCenter().lat() + x/( kRVSG*proj.xfactor() );
  float lon = proj.getMapCenter().lon() + p ;

  if(lat > proj.endFi()) {
    lat = proj.endFi();
  }
  else if (lat < proj.startFi()) {
   lat = proj.startFi();
  }

  if (lon > proj.startLa()) {
    lon = proj.startLa();
  }
  else if (lon < proj.endLa()) {
    lon = proj.endLa();
  }

  geoCoord->setLat(lat);
  geoCoord->setLon(lon);

  return true;
}

bool AeroDocument::logF2X_onef( const Projection& proj, const GeoPoint& ageoCoord, QPointF* meterCoord )
{
//  GeoPoint start = GeoPoint(-100,1200,0,LA_GENERAL);
//  GeoPoint end = GeoPoint(50,5,0,LA_GENERAL);

  GeoPoint geoCoord(ageoCoord);

  if(geoCoord.fi() > proj.endFi()) {
    geoCoord.setFi(proj.endFi());
  }
  else if (geoCoord.fi()< proj.startFi()) {
    geoCoord.setFi(proj.startFi());
  }

  if (geoCoord.la() > proj.startLa()) {
    geoCoord.setLa(proj.startLa());
  }
  else if (geoCoord.la() < proj.endLa()) {
    geoCoord.setLa(proj.endLa());
  }

  float lon = geoCoord.lon() - proj.getMapCenter().lon();
  float lat = geoCoord.lat() - proj.getMapCenter().lat();

  float p = lon;
  float h;
  float t;
  zond::SA81_H_ext(p,&t,&h);

  h=maxH-h;
  lon = h/80.;

  float x = kRVSG*lat*proj.xfactor();
  float y = kRVSG*(lon*proj.yfactor());

  meterCoord->setX(x);
  meterCoord->setY(y);

//    if ( 0 > x && ::abs(x) == x ) {
//      return false;
//    }
//    if ( 0 > y && ::abs(y) == y ) {
//      return false;
//    }
  return true;
}

bool AeroDocument::logX2F_onef( const Projection& proj, const QPointF& meterCoord, GeoPoint* geoCoord )
{
  if ( nullptr == geoCoord ) {
    error_log << QObject::tr("Нулевой указатель GeoPoint*");
    return false;
  }
  *geoCoord = GeoPoint( 0,0,0, LA_GENERAL );

  float x = meterCoord.x();
  float y = meterCoord.y();
  y = y/(kRVSG*proj.yfactor());
  float h = y*80.;
  h=maxH-h;
  float p;
  float t;
  zond::SA81_P_ext(h,&t,&p);
  float lat = proj.getMapCenter().lat() + x/( kRVSG*proj.xfactor() );
  float lon = proj.getMapCenter().lon() + p ;

  if(lat > proj.endFi()) {
    lat = proj.endFi();
  }
  else if (lat < proj.startFi()) {
   lat = proj.startFi();
  }

  if (lon > proj.startLa()) {
    lon = proj.startLa();
  }
  else if (lon < proj.endLa()) {
    lon = proj.endLa();
  }

  geoCoord->setLat(lat);
  geoCoord->setLon(lon);

  return true;
}

}
}
