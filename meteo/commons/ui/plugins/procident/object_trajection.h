#ifndef OBJECT_TRAJECTION_H
#define OBJECT_TRAJECTION_H

#include <QObject>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/view/actions/selectaction.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/field.pb.h>
#include <commons/obanal/obanal_struct.h>
#include <commons/obanal/tfield.h>

namespace meteo {
namespace map {

class Puanson;

  class ObjectTrajection  : public QObject
{
  Q_OBJECT
public:
  ObjectTrajection (  );
  ~ObjectTrajection (  );
  
  void createObjTrajectory();
  
  void hasFields(::meteo::field::ManyDataReply* reply);
  void hasExtremums(meteo::field::ExtremumTrajReply*);
  void setLayer(Layer *layer){layer_ = layer;}
  void setCurDt(const QDateTime &dt){cur_dt_ = dt;}
  Layer *layer()const {return layer_;}

private:
  void drawTraj(const QMap <int,QMap <QDateTime, fieldExtremum > > &traj);
    
  
  Puanson* createIco(meteo::map::Layer *layer, const QDateTime& dt, const fieldExtremum &);
  //Puanson* createIco(meteo::map::Layer *layer,const meteo::field::Extremum &extr);
  void  drawTrajLine(meteo::map::Layer *layer, const GeoVector& line, float, int type, int shift);
  Layer *layer_; 
  QDateTime cur_dt_;
};

}
}

#endif
