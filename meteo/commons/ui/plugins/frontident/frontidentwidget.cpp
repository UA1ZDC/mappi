
#include "frontidentwidget.h"
#include "ui_frontidentwdgt.h"

#include <cross-commons/debug/tlog.h>
#include <commons/obanal/tfield.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/view/actions/selectaction.h>
#include <meteo/commons/ui/map/layeriso.h>
#include <meteo/commons/global/gradientparams.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <commons/textproto/pbtools.h>

namespace meteo {
namespace map {
  
  FrontIdentWidget::FrontIdentWidget(MapView* view) :
  QDialog(view),
  ui_(new Ui::FrontIdentWidget),
  view_(view),
  document_(nullptr),
  center_(-1),
  field_ctrl_ (nullptr)
{
  dfi_1 = 1.;
  dla_1 = 1.;
  
  ui_->setupUi(this);
  //ui_->timeBox->setCurrentIndex(8);
  reconnectFieldService();
}

FrontIdentWidget::~FrontIdentWidget()
{
  QApplication::restoreOverrideCursor();
  if(nullptr != field_ctrl_ ){
      delete field_ctrl_;
      field_ctrl_ = nullptr;
  }
}

void FrontIdentWidget::slotConnect(){
  connect(ui_->closeBtn, SIGNAL(clicked()), SLOT(slotClose()));
  connect(ui_->addBtn, SIGNAL(clicked()), SLOT(slotAdd()));
  connect(ui_->dateEdit, SIGNAL(userDateChanged(const QDate &)), SLOT(slotChanged()));
  connect(ui_->timeEdit, SIGNAL(userTimeChanged(const QTime &)), SLOT(slotChanged()));
  connect(ui_->hourCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotChanged()));

}

void FrontIdentWidget::slotChanged(){
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor)) ;
  fillFieldCombos();
  QApplication::restoreOverrideCursor();
}


bool FrontIdentWidget::reconnectFieldService()
{
  if(nullptr != field_ctrl_ ){ delete field_ctrl_ ;field_ctrl_ = nullptr; }
  field_ctrl_ = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if(nullptr == field_ctrl_) {
      error_log << meteo::msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kField));
      return false;
    }
  return true;
}

bool FrontIdentWidget::getFieldList(const meteo::field::DataRequest &request, QList< meteo::field::DataDesc> *afields)
{

  if(nullptr == field_ctrl_ ){
      error_log << meteo::msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kField));
      reconnectFieldService();
      return false;
    }
  //  QTime timer;
  //  timer.start();

  
  meteo::field::DataDescResponse * reply =
      field_ctrl_->remoteCall( &meteo::field::FieldService::GetAvailableData, request, 100000);
  // debug_log << "GetAvailableData 1 finished in: " << timer.elapsed();
  if ( nullptr == reply ) {
      error_log << QObject::tr("При попытке получить список доступных полей, ответ от сервиса данных не получен");
      reconnectFieldService();
      return 0;
    }
  for(int fc = 0; fc < reply->descr_size(); ++fc ){
      meteo::field::DataDesc * fdescr = reply->mutable_descr(fc);
      if(!fdescr->has_id()) continue;
      afields->insert(fdescr->priority(),*fdescr);
    }

  // debug_log<<reply->DebugString();
  
  delete reply; reply = nullptr;
  return afields->count();
}

void FrontIdentWidget::fillFieldCombos(){
  QDateTime acur_dt = QDateTime(ui_->dateEdit->date(),QTime(ui_->timeEdit->time().hour(),0));
  //debug_log << acur_dt;
  int hour = ui_->hourCombo->currentText().toInt()*3600;

  fillFieldList(ui_->fieldH850Combo,acur_dt,850,100,hour,10009);
  fillFieldList(ui_->fieldT850Combo,acur_dt,850,100,hour,12101);
  fillFieldList(ui_->fieldWindU850Combo,acur_dt,850,100,hour,11003);
  fillFieldList(ui_->fieldWindV850Combo,acur_dt,850,100,hour,11004);
  fillFieldList(ui_->fieldOT1500Combo,acur_dt,15000,15000,hour,10009);

}

int FrontIdentWidget::fillFieldList(QComboBox *combo, const QDateTime &acur_dt, int level, int level_type, int hour, int descr){
    meteo::field::DataRequest request;
    request.add_level(level);
    if(0< center_){
        request.add_center(center_);
      }
    request.add_type_level(level_type);
    request.set_only_last(false);
    request.set_only_best(false);
    request.add_meteo_descr(descr);
    request.add_hour(hour);
    request.set_date_start(acur_dt.toString(Qt::ISODate).toStdString());
    request.set_date_end(acur_dt.toString(Qt::ISODate).toStdString());
   // debug_log<< "try get field data" << request.ShortDebugString();
    QList< meteo::field::DataDesc> afields;
    int kol_fields = getFieldList(request,&afields);
    combo->clear();
    for (int i =0; i <afields.count();++i ){
      int ind = combo->count();
    //  if(fieldlist_.values().value(i).has_priority() && fieldlist_.values().value(i).priority() > -1){
   //     ind = fieldlist_.values().value(i).priority()-1;
   //   }
      combo->insertItem(ind ,getComboText(afields.at(i)),pbtools::toQString(afields.at(i).id()));
    }
    if(0 == kol_fields){
        combo->insertItem(0,QObject::tr("Нет данных"));
      }

    return kol_fields;
}

QString FrontIdentWidget::getComboText(const meteo::field::DataDesc &d){
  QString svs = pbtools::toQString(d.center_name());
  if(d.has_hour() && 0 < d.hour()){
    svs+=QString::fromUtf8(" (%1 ч)").arg(d.hour()/3600);
  }
  return svs;
}

bool FrontIdentWidget::getField(const std::string &id, obanal::TField * fd){
   meteo::field::SimpleDataRequest request;
  request.add_id(id);
  

  meteo::field::DataReply * reply = nullptr;
  if(nullptr == field_ctrl_) {
      error_log << meteo::msglog::kServiceConnectFailedSimple.arg("FieldService");
      return false;
  }
  reply = field_ctrl_->remoteCall( &meteo::field::FieldService::GetFieldDataPoID, request,  30000);
  if ( nullptr == reply ) {
      error_log << meteo::msglog::kServiceAnalyzeAnswerFailed.arg("GRIB").arg("FieldService");
      return false;
  }

  bool result = false;
  reply->fielddata();
  if(false ==  reply->has_fielddata()){
      error_log << QObject::tr("Нет данных")<<request.DebugString();
      result = false;
  }
  else {
      QByteArray arr( reply->fielddata().data(), reply->fielddata().size() );
      // QDataStream stream(arr);
      //stream >> (*fd);
      if( !fd->fromBuffer(&arr)){
          error_log << QObject::tr("Не удалось получить поле из сериализованного сообщения");
          result = false;
      }
      else {
          result = true;
      }
  }
  //debug_log << "G"<<reply->descr().ShortDebugString();

  delete reply;
  return result;
}


double FrontIdentWidget::getTadv(const meteo::GeoPoint& p){
  int i,j;
  if( !fdH5001000_.getNumPoFila(p,&i,&j))
    {
      return 0.; //FIXME нужно возвращать &OK
    }
  float dtx = fdH5001000_.dPx(i,j,dla_1);
  float dty = fdH5001000_.dPy(i,j,dfi_1);
  float u = 0.;
  float v = 0.;
  
  if(0 < fdU850_.kolData() && 0 < fdV850_.kolData() ){
    u = fdU850_.getData(i,j);
    v = fdV850_.getData(i,j);
  } else {
    fdH850_.geostrofWind(i,j,&u,&v);
    u=-u;
    v=-v;
  }
  
  return -1.*( u*dtx + v*dty )/63.71;
}

void FrontIdentWidget::setProp( obanal::TField *field){
  if(nullptr == field) {
      ui_->dateEdit->setDate(QDate::currentDate());
      ui_->timeEdit->setTime(QTime::currentTime());
      fillFieldCombos();
      return;
    }
  ui_->dateEdit->setDate(field->getDate().date());
  ui_->timeEdit->setTime(field->getDate().time());
  center_ = field->getCenter();
  int hour = field->getHour()/3600;
  int kol = ui_->hourCombo->count();
  if(0 == hour ){
      ui_->hourCombo->setCurrentText(QObject::tr("Фактический"));
    } else {
      for(int i =0; i<kol;++i){
          QString text = ui_->hourCombo->itemText(i);
          if(text == QString::number(hour)){
              ui_->hourCombo->setCurrentIndex(i);
              break;
            }
        }
  }
  fillFieldCombos();

}

bool FrontIdentWidget::loadFields(){
  //QDateTime acur_dt = QDateTime(ui_->dateEdit->date(),QTime(ui_->timeEdit->time().hour(),0));
  //int hour = ui_->hourCombo->currentText().toInt()*3600;
  fdT850_.clear();
  fdH5001000_.clear();
  fdH850_.clear();
  fdU850_.clear();
  fdV850_.clear();

  std::string id = ui_->fieldT850Combo->currentData().toString().toStdString();
  if(true != getField( id, &fdT850_ )){return false;}
  id = ui_->fieldOT1500Combo->currentData().toString().toStdString();
  getField( id, &fdH5001000_ );
  id = ui_->fieldH850Combo->currentData().toString().toStdString();
  getField( id, &fdH850_ );
  id = ui_->fieldWindU850Combo->currentData().toString().toStdString();
  getField( id, &fdU850_ );
  id = ui_->fieldWindV850Combo->currentData().toString().toStdString();
  getField( id, &fdV850_ );

  setInfo(&fdT850_);
  float dfi_ =1.f;
  fdT850_.stepFi(&dfi_);
  float dla_=1.f;
  fdT850_.stepLa(&dla_);
  dfi_1 = 1.f/dfi_;
  dla_1 = 1.f/dla_;
  fdH5001000_.smootchField(50.);
  fdU850_.smootchField(50.);
  fdV850_.smootchField(50.);
  fdT850_.smootchField(50.);
  fdH850_.smootchField(50.);
  return true;
  
}


void FrontIdentWidget::setInfo(obanal::TField * field){
  if(nullptr == field) return ;
  info_.set_mode(proto::kIsoline);
  info_.set_source(proto::kField);
  info_.set_center(field->getCenter());
  info_.set_center_name(field->getCenterName().toStdString());
  info_.set_model(field->getModel());
  QString ads = field->getDate().toString("dd.MM.yy hh:mm");
  info_.set_datetime(ads.toStdString());
  info_.set_hour(field->getHour()/3600);
}


proto::WeatherLayer FrontIdentWidget::getInfo(const QString& an){
  proto::WeatherLayer info = info_; 
  QString tmpl = QString("%1 за %2 Центр: [center]")
  .arg(an)
  .arg(QString::fromStdString(info_.datetime())); 
  info.set_layer_name_pattern(tmpl.toStdString());
  return info;
}

void FrontIdentWidget::addFrontLine(double val, const QString & aname)
{
  
  if(!document_) return;
  Layer *flayer = new Layer( document_);
 // "Линии атмосферных фронтов ("+aname+") за "
 // +fdT850_.getDate().toString("dd-MM hh:mm")
 //+" по данным "+field->getCenterName()
 flayer->setInfo(getInfo(aname));
 QVector<meteo::GeoVector> isolines;
 fdT850_.getIsolines(&isolines, val);
 int iso_cnt =  isolines.count();
 if(iso_cnt < 1 ) {
   delete flayer;
   flayer = nullptr;
   return;
 }
 meteo::GeoVector cur_t_line;
 meteo::GeoVector cur_h_line;
 for(int i = 0; i< iso_cnt; ++i){
   const meteo::GeoVector &line = isolines.at(i);
   int kol_point =  line.size();
   if(kol_point < 300) continue;
   cur_h_line.clear();
   cur_t_line.clear();
   frontType ftype;
   for(int j = 0; j < kol_point-4; j+=4){
     const meteo::GeoPoint &p = line.at(j);
     ftype = addFront(flayer,p,&cur_t_line, &cur_h_line);
   }
   
  // debug_log << val<<" summ grad "<< gval<<gval/kol_point;
  addFront(flayer,line.first(),&cur_t_line, &cur_h_line );
  flushFront(flayer,ftype,&cur_t_line, &cur_h_line);
 }
}


bool FrontIdentWidget::getGradT( const meteo::GeoPoint &p, float *resval){
  float rk = 0.;
  int ii,jj;
  if (!fdT850_.getNumPoFila(p,&ii,&jj)){
      return false;
    }
  if(ii <1 || jj< 1 || ii >= fdT850_.kolFi()-1 || jj >= fdT850_.kolLa()-1 ) return false;
  float dtx = fdT850_.dPx(ii,jj,dla_1)/63.71;
  float dty = fdT850_.dPy(ii,jj,dfi_1)/63.71;
  rk = 5.*qSqrt(dtx*dtx+dty*dty);
  *resval = rk;
  return true;
}

frontType FrontIdentWidget::identFrontType( const meteo::GeoPoint &p){
  float rk = 0.;
  if(!getGradT(p, &rk)) return fNoType;
  frontType aft = fWarm;
  double val = getTadv( p );
  if(3. > rk) aft = fWarmRazm;
  if(0. > val){
    aft = fCold;
    if(3. > rk) aft = fColdRazm;
  }
  return aft;
}

void FrontIdentWidget::flushFront(Layer * layer,frontType ftype, meteo::GeoVector *tl, meteo::GeoVector *hl ){
  if(hl->size() > 1){
    addColdFront(layer,*hl,ftype);
    hl->clear();
  }
  if(tl->size() > 1){
    addWarmFront(layer,*tl,ftype);
    tl->clear();
  }
}

frontType FrontIdentWidget::addFront(Layer * layer, const meteo::GeoPoint &p,meteo::GeoVector *tl, meteo::GeoVector *hl ){
  frontType ftype = identFrontType(p);
  
  switch(ftype){
    case fWarm:
    case fWarmRazm:
      if(hl->size() > 1){
        addColdFront(layer,*hl,ftype);
        tl->append(hl->last());
        hl->clear();
      }
      tl->append(p);
      break;      
    case fCold:
    case fColdRazm:
      if(tl->size() > 1){
        addWarmFront(layer,*tl,ftype);
        hl->append(tl->last());
        tl->clear();
      }
      hl->append(p);
      break;
    default:
      break;      
  }
  return ftype;
}

void FrontIdentWidget::addColdFront(Layer * layer,const meteo::GeoVector &line, frontType l){
  if(nullptr == layer) return;
  GeoPolygon *pol = new GeoPolygon(layer);
  pol->setSplineFactor(10.);
  pol->setSkelet(line);
  QPen pen;
  pen.setWidth(1);
  pen.setColor(Qt::blue);
  if(10 <= l ){
    pen.setStyle(Qt::DashLine);
  }
  pol->setPen(pen);
  pol->setOrnament("0001_coldfront");
}

void FrontIdentWidget::addWarmFront(Layer * layer,const meteo::GeoVector &line,frontType l){
  if(nullptr == layer) return;
  GeoPolygon *pol = new GeoPolygon(layer);
  pol->setSplineFactor(10.);
  pol->setSkelet(line);
  QPen pen;
  pen.setWidth(1);
  pen.setColor(Qt::red);
  if(10 <= l ){
    pen.setStyle(Qt::DashLine);
  }
  pol->setPen(pen);
  pol->setOrnament("0000_warmfront");
}
  
void FrontIdentWidget::slotAdd(){
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor)) ;

  loadFields();
  if(ui_->AFcheckBox->isChecked()) addFrontLine(ui_->AFrSpin->value(),  "Арктический фронт");
  if(ui_->SAcheckBox->isChecked()) addFrontLine(ui_->SAFrSpin->value(), "Старый арктический фронт");
  if(ui_->PcheckBox->isChecked())  addFrontLine(ui_->PFrSpin->value(),  "Полярный фронт");
  QApplication::restoreOverrideCursor();
  //calcTFT();
}

void FrontIdentWidget::slotClose(){
  
  close();
}


void FrontIdentWidget::closeEvent(QCloseEvent *event)
{ 
  QApplication::restoreOverrideCursor();
  event->accept();
 //   event->ignore();
}


}
}
