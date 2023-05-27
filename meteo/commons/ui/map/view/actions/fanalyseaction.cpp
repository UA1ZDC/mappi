#include "fanalyseaction.h"

#include <qevent.h>
#include <qtoolbutton.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsproxywidget.h>
#include <qgraphicswidget.h>
#include <qobject.h>
#include <qdebug.h>
#include <qicon.h>
#include <qlabel.h>

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geopoint.h>
#include <commons/obanal/tfield.h>
#include <commons/obanal/tisolinedata.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/incut.h>
#include <meteo/commons/ui/map/isoline.h>

#include <meteo/commons/global/gradientparams.h>

#include <meteo/commons/ui/map/layeriso.h>

#include "../mapview.h"
#include "../mapscene.h"
#include "../menu.h"
#include "../widgetitem.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>



namespace meteo {
namespace map {

FieldsAnalyseAction::FieldsAnalyseAction( MapScene* scene )
: Action(scene, "fieldsanalyseaction" )
{
  createServices();
  scene_->document()->turnEvents();
  if (scene_->document()->eventHandler() != 0) {
    scene_->document()->eventHandler()->installEventFilter(this);
  }
}

FieldsAnalyseAction::~FieldsAnalyseAction()
{
}


void FieldsAnalyseAction::createServices(){
  rpc::Channel* ch_field = meteo::global::serviceChannel( meteo::settings::proto::kField );
  if ( 0 != ch_field ) {
    ctrl_field = ch_field;
  }
  else {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных. Проверьте настройки подключения");
  }
}


void FieldsAnalyseAction::addActionsToMenu( Menu* menu ) const
{

debug_log<<"FieldsAnalyseAction";

if(0 == menu ) return;
if( !menu->isEmpty()){  menu->addSeparator();  }
Layer* l = scene_->document()->activeLayer();
if(0 == l) {
return ;
}
if ( true == l->hasField() ||  0 != l->field()) {
menu->addAction("Траектории движения циклонов",this, SLOT (calcTraject()) );
menu->addAction("Градиент",this, SLOT (calcGrad()) );
}

//point_ = view()->mapToScene(e->pos()).toPoint();
}


void FieldsAnalyseAction::calcGrad(){
  if(0 == scene_ || 0 == scene_->document() ) return;

  Layer* l = scene_->document()->activeLayer();

  if (0 == l || true != l->hasField() ||  0 == l->field()) {
    debug_log <<"No field in layer!";
    return;
  }
  QTime tt;
  tt.start();

  obanal::TField* af = l->field();
  if(0 == af) return;
  obanal::TField* agrf = af->getCleanCopy();
  if(0 == agrf) return;
  double min = 1e10;
  double max = -1e10;

  for(int i = 0; i < af->kolFi(); ++i){
    for(int j = 0; j < af->kolLa(); ++j){
      float grad_fi,grad_la;
      if(fabs (af->getFi(i) *180/3.14) > 88.) continue;

      if(af->gradient( i,  j, &grad_fi, &grad_la)){
        double gg = sqrt (grad_fi*grad_fi + grad_la*grad_la)*1e5;
        agrf->setData(i,j,gg,true);
        if(min > gg) min = gg;
        if(max < gg) max = gg;
      }
    }
  }
  debug_log<<"min"<<min<<"max"<<max<<"gradient calc "  <<tt.elapsed();
  tt.start();
  if(0 != agrf->smootchField(1)){
    debug_log <<" error in smootchField";
  }

  QPen pen;
  pen.setWidth(2);
  pen.setColor(Qt::green);
  pen.setStyle(Qt::SolidLine);
  LayerIso* li = 0;
  li = new LayerIso(  scene_->document(), "gradient " );
  li->setField(agrf);
  li->setPen(pen);
  int iso_count = li->addIsoLines(min,max,1);
  li->addExtremums();
  debug_log<<"gradient calc "  <<tt.elapsed()<<"iso_count"<< iso_count;

}

void FieldsAnalyseAction::calcTraject(){

  Layer* l = scene_->document()->activeLayer();

  if (0 == l || true != l->hasField() ||  0 == l->field()) {
    debug_log <<"No field in layer!";
    return;
  }
  obanal::TField* f_0 = l->field();
  if(0 == f_0) return;

  meteo::Projection* proj = meteo::Projection::createProjection(meteo::STEREO); //может быть здесь проекцию надо брать у документа?

  TIsoLineData iso(f_0);
  QVector<meteo::GeoVector> isolines;

  int level = f_0->getLevel();
  int leveltype = f_0->getLevelType();
  int descr = f_0->getDescr();
  QTime tt;
  tt.start();


  GradientParams gradient = GradientParams( meteo::global::kIsoParamPath() );
  proto::FieldColor lvl_settings = gradient.protoParams(descr);

  double level_min = gradient.isoMin( level, leveltype, lvl_settings );
  double level_max = gradient.isoMax( level, leveltype, lvl_settings );
  double step_iso = gradient.isoStep( level, leveltype, lvl_settings );

  for (double i = level_min; i <= level_max; i += step_iso ){
    isolines.clear();
    iso.makeOne(i, &isolines );

    for( int j = 0; j < isolines.count(); j++ ) {
      QPen pen = pen2qpen( lvl_settings.pen() );
      pen.setWidth(2);
      pen.setColor(Qt::black);
      lvl_settings.mutable_pen()->CopyFrom( qpen2pen(pen) );
      IsoLine* gp = new IsoLine( l, lvl_settings, level, leveltype );

      meteo::GeoVector geoPoints = isolines.at(j);

      QVector<QPolygon> scrPoints;
      proj->F2X(geoPoints,  &scrPoints, false);
      //   debug_log<< "scrPoints.size()"<< scrPoints.size();
      if(1 <= scrPoints.size() ){
        QPolygon points = scrPoints[0];
        QRect rect = points.boundingRect();
        meteo::GeoPoint tlgp;
        proj->X2F_one(rect.topLeft(),  &tlgp);
        meteo::GeoPoint trgp;
        proj->X2F_one(rect.topRight(),  &trgp);
        meteo::GeoPoint blgp;
        proj->X2F_one(rect.bottomLeft(),  &blgp);
        meteo::GeoPoint brgp;
        proj->X2F_one(rect.bottomRight(),  &brgp);
        double distw =  tlgp.calcDistance(trgp);
        double disth =  tlgp.calcDistance(blgp);
        if(distw < 2500. && disth <2500.){
          pen = gp->qpen();
          pen.setColor(Qt::red);
          gp->setPen(pen);
          gp->setSkelet(isolines.at(j));
          gp->setValue(i);
        }
      }
    }
  }
  delete proj;
  //debug_log<<"1 iso calc "  <<tt.elapsed();

}

}
}
