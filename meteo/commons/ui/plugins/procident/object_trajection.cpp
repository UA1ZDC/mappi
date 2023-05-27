#include "object_trajection.h"

#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/widgetitem.h>
#include <meteo/commons/ui/map/view/menu.h>
#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>
#include <commons/obanal/tfield.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/puanson.h>

#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/view/mapview.h>

#include <meteo/commons/global/weatherloader.h>

#include <meteo/commons/ui/map/view/mapscene.h>

#include <meteo/commons/ui/map/geotext.h>




namespace meteo {
namespace map {

ObjectTrajection ::ObjectTrajection (  )
  :layer_(nullptr)
{
  cur_dt_=QDateTime::currentDateTimeUtc();
}

ObjectTrajection ::~ObjectTrajection (  ){

}


void ObjectTrajection::hasExtremums(meteo::field::ExtremumTrajReply* reply){
  int ds = reply->traj_size();
  if (1 > ds) {
      warning_log.msgBox() << tr("Внимание!\n Расчет траекторий перемещения барических образований произвести не удалось");
      return;
    }

  //debug_log<<reply->DebugString();
  QMap <int,QMap <QDateTime, fieldExtremum > > extr_cur;
  for(int i=0; i< ds; ++i ){
      // GeoVector line;
    auto traj = reply->traj(i);
    QMap <QDateTime, fieldExtremum > extrs;
    for(int j=0; j< traj.extremums_size(); ++j ){
          QDateTime dt_0 = QDateTime::fromString(QString::fromStdString(traj.extremums(j).date()), Qt::ISODate);
          // line<<GeoPoint(traj.extremums(j).coord().fi(),traj.extremums(j).coord().la());
          fieldExtremum extr;
          auto coord = traj.extremums(j).pbcoord();
          extr.koord = GeoPoint( coord.lat_radian(), coord.lon_radian() );
//          extr.koord = GeoPoint(traj.extremums(j).coord().fi(),traj.extremums(j).coord().la());
          extr.tip = traj.extremums(j).tip();
          extr.znach = traj.extremums(j).value();
          extrs.insert(dt_0,extr);
        }
     if(2 > extrs.size() ) continue;
     extr_cur.insert(i,extrs);
    }

  drawTraj(extr_cur);
  // info_log.msgBox() << tr("Расчет траекторий перемещения барических образований завершен");
}


void ObjectTrajection::drawTraj(const QMap <int,QMap <QDateTime, fieldExtremum > > &traj){
  if (1 > traj.count() || nullptr == layer_) {
      return;
    }
  QList<int> tr_list= traj.uniqueKeys();
  //  debug_log<<reply->DebugString();
  for(int i=0; i< tr_list.size(); ++i ){
      GeoVector line;
      const QMap <QDateTime, fieldExtremum > &extrs = traj.value(tr_list.at(i));
      QList<QDateTime> dt_list = extrs.uniqueKeys();
      qSort(dt_list);
      for(int j=0; j< dt_list.size(); ++j ){
          const fieldExtremum &extr = extrs.value(dt_list.at(j));
          meteo::map::Puanson* puan = createIco(layer_, dt_list.at(j),extr);
          float r = 0.;
          if( nullptr !=puan ) r = puan->punch().radius();
          line<<  GeoPoint(extr.koord.fi(),extr.koord.la());
          //    debug_log<< GeoPoint(extr.koord.fi(),extr.koord.la());
          if(2 == line.size()) {
              QDateTime dt_1 = dt_list.at(j-1);
              QDateTime dt_0 = dt_list.at(j);
              float vel = line.first().calcDistance(line.last())/(dt_1.secsTo(dt_0)/3600.);
              drawTrajLine(layer_, line,vel,extr.tip,r);
              line.remove(0);
            }
        }
    }
}

void ObjectTrajection::hasFields(::meteo::field::ManyDataReply* reply){
  delete reply;
}

void  ObjectTrajection::drawTrajLine(meteo::map::Layer *layer, const GeoVector& line, float vel, int type, int shift){
  if(nullptr==layer) return;
  GeoPolygon* gline = new GeoPolygon(layer );
  gline->setDoubleLine(type == 1);
  gline->setPenStyle(Qt::SolidLine);
  gline->setPenWidth(2);
  gline->setPenColor(Qt::black);
  gline->setSplineFactor(1);
  gline->setSkelet( line );
  gline->setValue(vel);
  gline->setArrowPlace( kEndArrow );
  gline->setArrowType( kSimpleArrow);
  gline->setDrawOnOnEnds(false);
  gline->setPosOnParent( kCenter );
  gline->setGeolineEndShift(-shift);
  gline->setGeolineStartShift(shift);
  std::unordered_set<GeoText*> chl = gline->childsByType<GeoText*>();
  if(chl.size() > 0){
      (*chl.begin())->setPos(kCenter);
    }
}

meteo::map::Puanson* ObjectTrajection::createIco(meteo::map::Layer *layer, const QDateTime& dt, const fieldExtremum &extr){

  if(nullptr==layer) return nullptr;
  float dtval = dt.date().day()+dt.time().hour()/100.;
  QMap< QString, meteo::puanson::proto::Puanson > map = WeatherLoader::instance()->punchlibraryspecial();
  GeoPoint coord = GeoPoint(extr.koord.fi(),extr.koord.la());
  float value = extr.znach;
  int type = extr.tip;
  Puanson* puan = new Puanson(layer);
  puan->setPunch(map["obj_traj"]);
  //  debug_log<<dtval<<value<<type;
  puan->setParamValue("G1G1", TMeteoParam("", dtval, control::RIGHT));
  puan->setParamValue("U", TMeteoParam("", value, control::RIGHT));
  puan->setParamValue("zn", TMeteoParam("", type, control::RIGHT));
  puan->setDrawAlways(true);
  puan->setSkelet(coord);
  return puan;
}




}
}
