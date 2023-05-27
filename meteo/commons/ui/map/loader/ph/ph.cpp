#include "ph.h"

#include <qfile.h>
#include <qprocess.h>
#include <qdir.h>
#include <qvariant.h>
#include <qsettings.h>
#include <qfileinfo.h>
#include <qtextcodec.h>
#include <qpair.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>
#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/generalproj.h>
#include <commons/geobasis/geovector.h>
#include <commons/geobasis/projection.h>
#include <commons/obanal/tisolinedata.h>

#include <meteo/commons/rpc/rpc.h>

#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/map.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/loader/loader.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/fonts/weatherfont.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/proto/meteo.pb.h>

#include <meteo/commons/ui/verticalcut/verticalcut.h>
#include <meteo/commons/global/global.h>

namespace {
  const QString kLoaderType("ph.load");

  static bool res = meteo::map::Loader::instance()->registerDataHandler(kLoaderType, &meteo::map::ph::loadGeoData );
}

namespace meteo {
namespace map {
namespace ph {
  bool createRazrezAxis(Map* map,int min_x, int min_y, int max_x, int max_y ){

    GeoVector xaxis;
    for ( int i = min_x, sz = max_x; i <= sz; ++i ) {
      GeoPoint gp( i, 0.0, 0.0, LA_GENERAL );
      xaxis.append(gp);
    }
    GeoVector yaxis;
    for ( int i = min_y, sz = max_y; i <= sz; ++i ) {
      GeoPoint gp( 0.0, i, 0.0, LA_GENERAL );
      yaxis.append(gp);
    }
    meteo::map::Layer* l = map->createBasisLayer( QObject::tr("Координатная метка") );
    meteo::map::GeoPolygon* geopol = new GeoPolygon(l);
    geopol->setSkelet(xaxis);
    geopol = new GeoPolygon(l);
    geopol->property().pen.setWidth(2);
    geopol->property().pen.setColor(Qt::blue);
    geopol->setSkelet(yaxis);
    return true;
  }


  bool createRazrezLayer(Map* map, obanal::TField *razr_field, const QString& rname,
                         int min, int max, int step, const QColor& col){
    if(0 == razr_field|| 0 == map) return false;

    meteo::map::Layer* l_t = map->createBasisLayer( rname );
    razr_field->smootchField(10);
    l_t->setField(razr_field);

    int i = MnMath::ftoi_norm(min);
    int imax = MnMath::ftoi_norm(max);
    QVector<meteo::GeoVector> isolines;
    TIsoLineData iso_t(razr_field);

    for (; i < imax; i+=step ){
      double val = i;
      isolines.clear();
      iso_t.makeOne( val, &isolines);
      //  debug_log<<i << isolines ;
      for (int nl =0; nl < isolines.size(); ++nl ){
        meteo::map::GeoPolygon* geopol = new GeoPolygon(l_t);
        geopol->property().pen.setWidth(2);
        geopol->property().pen.setColor(col);
        geopol->property().posOnParent = kTopCenter;
        geopol->setValue(val);
        geopol->setSkelet(isolines.at(nl));
      }
    }
    return true;

  }

  bool loadGeoData( Map* map )
  {
    Q_UNUSED( map );
//    rpc::Address address;

//    if ( false == rpc::ServiceLocation::instance()->findLocalService( ::meteo::global::kServiceNames[::meteo::global::  kFieldService], 500, &address ) ) {
//      if ( false == rpc::ServiceLocation::instance()->findService( ::meteo::global::kServiceNames[::meteo::global::kFieldService], 500, &address ) ) {
//        error_log << QObject::tr("Сервис %1 не найден")
//                     .arg(::meteo::global::kServiceNames[::meteo::global::kFieldService]);
//        address = rpc::Address("192.168.1.40", 38767);
//      }
//    }
//    rpc::Channel* ch = rpc::ServiceLocation::instance()->serviceClient(address);
//    if ( 0 == ch ) {
//      error_log << QObject::tr("Не удалось установить соединение с сервисом данных по адресу %1:%2")
//                   .arg( address.host() )
//                   .arg( address.port() );
//      return false;
//    }

//    rpc::TController ctrl;
//    ctrl.setChannel(ch);

//    meteo::GeoVector gv;
//    for(int la = 0; la< 90; la+=10 ){
//      gv.append(meteo::GeoPoint(60. /180.*M_PI, la/180.*M_PI));
//    }

//    int center = 34;
//    QString date = "2016-01-13 00:00:00";
//    int hour =0;

//    meteo::field::DataRequest request;
//    for(int i =0; i< gv.size(); ++i){
//      ::meteo::surf::Point* p =  request.add_coords();
//      p->set_fi( gv.at(i).fi());
//      p->set_la(gv.at(i).la());
//    }
//    request.add_center(center);
//    request.add_hour(hour);

//    request.set_need_field_descr(true);
//    request.set_date_start(date.toStdString());

//    meteo::field::ProfileDataReply * reply = ctrl.remoteCall( &meteo::field::FieldService::GetProfile, request,  30000, true);
//    if ( 0 == reply ) {
//      error_log << QObject::tr("При попытке получить  данные в коде GRIB ответ от сервиса данных не получен");
//      return false;
//    }
//    ch->disconnect();
//    delete ch;

//    Q_UNUSED(map);
//     meteo::VerticalCut* vcut = new meteo::VerticalCut();

//   // ((meteo::GeneralProj*)map->projection())->setXfactor(0.2);
//     debug_log << "calc start";
//    if ( !vcut->processData(reply) ) {
//      delete reply;
//      return false;
//    }
//    debug_log << "calc end";

//    delete reply;

//    obanal::TField *razr_field = 0;


//    QVector<DecartData> tropo;
//    if(vcut->getTropo(&tropo)){
//      GeoVector tropo_h;
//      for(int i =0; i< tropo.size(); ++i  ){
//        tropo_h.append(GeoPoint(tropo.at(i).x,tropo.at(i).y,0,LA_GENERAL));
//      }
//      meteo::map::Layer* l_t = map->createBasisLayer( QObject::tr("Тропопауза") );
//      meteo::map::GeoPolygon* geopol = new GeoPolygon(l_t);
//      geopol->property().pen.setWidth(8);
//      geopol->property().pen.setColor(Qt::darkYellow);
//      geopol->setValue(tropo.at(0).fun);
//      geopol->property().spline_koef = 4;
//      geopol->setSkelet(tropo_h);

//    }
//    razr_field = vcut->fillField(TemprRazr);
//    if(0 != razr_field){
//      createRazrezLayer(map, razr_field, QObject::tr("Температура"),- 80,50, 10, Qt::red);
//    }
//    razr_field = vcut->fillField(TemprDRazr);
//    if(0 != razr_field){
//      createRazrezLayer(map, razr_field, QObject::tr("Температура точки росы"),- 80,50, 10, Qt::green);
//    }
//    razr_field = vcut->fillField(ddRazr);
//    if(0 != razr_field){
//      createRazrezLayer(map, razr_field, QObject::tr("Направление ветра"),0, 360, 10, Qt::blue);
//    }
//    razr_field = vcut->fillField(ffRazr);
//    if(0 != razr_field){
//      createRazrezLayer(map, razr_field, QObject::tr("Скорость ветра"),0, 60, 10, Qt::magenta);
//    }
//    razr_field = vcut->fillField(HeightRazr);
//    if(0 != razr_field){
//      createRazrezLayer(map, razr_field, QObject::tr("Геопотенциальная высота"),- 100, 15000, 100, Qt::black);
//    }

//    debug_log<<"end";
//    createRazrezAxis(map, razr_field->netFi(0), razr_field->netLa(0),
//                     razr_field->netFi(razr_field->kolFi()-1), razr_field->netLa(razr_field->kolLa()-1));

    return true;
}


}
}
}
