#include "fizmethodbag.h"
#include "tforecastservice.h"
#include "tgradaciidb.h"

#include <meteo/commons/global/global.h>
#include <meteo/commons/global/dbnames.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/textproto/pbtools.h>
#include <meteo/commons/global/common.h>

#include "meteo/commons/services/forecast/tffielditem.h"

namespace meteo {
namespace forecast {

FizMethodBag::FizMethodBag( TForecastService* s )
  : QObject(),
    service_(s)
{
}

FizMethodBag::~FizMethodBag()
{

}
/*
void FizMethodBag::loadSettings()
{

}*/

int FizMethodBag::oprSynSit(const QDateTime& dt, const GeoPoint& station_coord)
{

  meteo::rpc::Channel * ctrl_field=0;
  ctrl_field = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if(0 == ctrl_field ) {
    error_log << meteo::msglog::kNoConnect.arg(meteo::settings::proto::kField);
  }
  meteo::field::DataRequest request;
  request.set_date_start(dt.addSecs(-12*3600).toString(Qt::ISODate).toStdString());
  request.set_date_end(dt.toString(Qt::ISODate).toStdString());
  // request.add_center(34);
  request.add_level(1000);
  request.add_type_level(100);
  request.add_hour(0);
  request.add_meteo_descr(10009);
  request.set_need_field_descr(true);
  ::meteo::surf::Point* p =  request.add_coords();
  p->set_fi(station_coord.fi());
  p->set_la(station_coord.la());
  int cur_syn_sit =-1;
  if(ctrl_field){
    meteo::field::ValueDataReply * reply =
        ctrl_field->remoteCall( &meteo::field::FieldService::getSynSit, request,  30000);
    delete ctrl_field; ctrl_field = nullptr;
    if ( 0 == reply ) {
      error_log << meteo::msglog::kServiceAnswerFailed;
    }
    int i =reply->data_size()-1;
    QString tp = QObject::tr("Ситуация автоматически не определена");
    for ( ; i > 0; --i ){
      const meteo::field::OnePointData& d = reply->data(i);
      if(d.fdesc().count_point() < 2000) continue;
      cur_syn_sit = (int)d.value();
      tp = QObject::tr("Ситуация определена по полю давления за %1 центр %2")
           .arg(QString::fromStdString(d.fdesc().date()))
           .arg(QString::fromStdString(d.fdesc().center_name()));
      break;
    }
    info_log<< tp;
    delete reply; reply = 0;
  }
  return cur_syn_sit;
}


void FizMethodBag::GetForecastMethodsList( const Dummy*,  ForecastMethodList* response)
{
  Q_UNUSED(response);
  QString methods_dir = global::kForecastMethodsDir;
  TForecastList * methods_list = new TForecastList(nullptr,true);

  if(0 == methods_list->loadMethodsNoRun(methods_dir)){
    error_log<< msglog::kForecastNoMethods.arg(methods_dir);
  }

  foreach( TForecast* f, methods_list->methodsList() ){
    if( nullptr != f ){
      ::meteo::forecast::ForecastMethod* method = response->add_method();
      *method->mutable_name() = f->methodFullName().toStdString();
      *method->mutable_short_name() = f->methodName().toStdString();
      *method->mutable_hours() = f->methodHours().toStdString();
      *method->mutable_yavl_type() = f->methodYavlType().toStdString();
      *method->mutable_season() = f->methodSeason().toStdString();
      *method->mutable_synsit() = f->methodSynSit().toStdString();
      //TODO methodHelpFile()
      //TODO methodTimes()
    }
  }
  delete methods_list;
}

void fillProtoFromModelItem(TFItem* item, ForecastFizData *response){
  if(nullptr == item ) return;
  response->set_descr(item->getDescr().toStdString());
  response->set_text(item->getText().toStdString());
  response->set_value(item->getItemData().toString().toStdString());
  // заполняем тип поля, роли и достпные для выбора варианты

  response->set_item_role( item->role() );
  QStringList items_dropboxes;
  items_dropboxes = item->getItems();
  if ( items_dropboxes.size()>0 ){
    for (int i = 0; i < items_dropboxes.size(); ++i){
      response->add_data_options( items_dropboxes.at(i).toStdString() ); 
    }
  }
  
  QList<meteo::field::DataDesc> field_list; //<meteo::field::DataDesc>
  TFFieldItem *casted_item = dynamic_cast<TFFieldItem *>( item );
  if ( casted_item != nullptr ){
    field_list = casted_item->getFieldlist();
    if ( field_list.size()>0 ){
      for (int i = 0; i < field_list.size(); ++i){
        // debug_log<<field_list.at(i).center_name();
        meteo::field::DataDesc *available_center = response->add_available_centers();//->CopyFrom(field_list.at(i));
        available_center->set_id( field_list.at(i).id() );
        available_center->set_level( field_list.at(i).level() );
        available_center->set_date( field_list.at(i).date() );
        available_center->set_hour( field_list.at(i).hour() );
        available_center->set_center( field_list.at(i).center() );
        available_center->set_center_name( field_list.at(i).center_name() );
        available_center->set_meteodescr( field_list.at(i).meteodescr() );
      }
    }
  }
  // устанавливаем выбранный прогностический центр
  response->set_selected_center( item->currentIndex() );

  // debug_log << item->getDescr()<<item->getText()<<item->getItemData().toString();
  for(int i =0; i< item->childCount();++i){
    fillProtoFromModelItem(item->getChilds().at(i),response->add_childs());
  }
}



/**
 * @brief Заполняем модель данных пришедшими параметрами из прото (если они заданы)
 * 
 * @param request 
 * @param item 
 */
void fillModelItemFromProto(const ForecastFizData *request, TFItem* item, QString *parent){
  if(nullptr == item ) return;
  QString name;
  if ( parent->isEmpty() ){
    name = QString("%1").arg( QString::fromStdString(request->descr()) );
  }else{
    name = QString("%1.%2").arg( *parent ).arg( QString::fromStdString(request->descr()) );
  }

  TFItem* needed_item = item->getItem( name );

  if (  needed_item != nullptr ){
    // debug_log<<"try to cast "<<request->item_role() << needed_item->role()<<" selected center: "<<request->selected_center();
    needed_item->disableAutoReset();

    if ( needed_item->role()==meteo::forecast::ForecastMethodItemRole::kFieldData ){
      // преобразуем айтем в прогностическое поле, чтобы вызвать уникальный метод
      TFFieldItem *casted_item = dynamic_cast<TFFieldItem *>( needed_item );
      
      if ( casted_item != nullptr ){
        casted_item->setCurrentIndex( request->selected_center() );
        casted_item->setData( QVariant( request->selected_center() ) );
      }
      
    }else{
      if (!request->value().empty()){
        needed_item->setItemData( QString::fromStdString(request->value()) );
      }
    }
    
    needed_item->enableAutoReset();
  }

  for(int i =0; i< request->childs_size();++i){
    fillModelItemFromProto( &request->childs(i), item, &name );
  }
}

//запустить петод прогнозироания и получить результат
void FizMethodBag::GetForecastMethodsResultRun( const ForecastResultRequest *request,  ForecastFizResult *response)
{

  QDateTime dt = pbtools::toQDateTime(request->time_start());
  if(false == dt.isValid()) {
    debug_log<< "time_start() некорректен";
    response->set_result(false);
    return;
  }

  GeoPoint station_coord =  meteo::surfGeopoint2geopoint(request->coord());
  int syn_sit = oprSynSit(dt, station_coord);
  StationData astation;
  astation.coord = station_coord;
  astation.index = QString::fromStdString(request->coord().index());
  astation.name = QString::fromStdString(request->coord().name());
  //astation.station_type = request->coord().type();
  QString methods_dir = global::kForecastMethodsDir;
  TForecastList * methods_list = new TForecastList(nullptr,true);


  if(methods_list->methodsList().empty() && 0 == methods_list->loadMethodsNoRun(methods_dir)){
    error_log<< msglog::kForecastNoMethods.arg(methods_dir);
  }
  methods_list->setStation(astation);
  methods_list->setDateTime(dt);
  methods_list->setSynSit(syn_sit);
  methods_list->setSeason(TForecast::oprSeason(dt.date()));

  QString method_name = QString::fromStdString(request->method_name());
  TForecast * cf = methods_list->getMethod(method_name);
  QString notice;
  if(nullptr == cf) {
    notice = "Метод не найден: "+method_name;
    debug_log << notice;
    response->set_notice(notice.toStdString());
    response->set_result(false);
    delete methods_list;
    return;
  }
  response->set_date_time(pbtools::toString(cf->getDate()));
  response->set_syn_sit(TForecast::stringSynSit(cf->getSynSit()).toStdString());
  response->set_name(cf->methodFullName().toStdString());
  response->set_season(TForecast::stringSeason(cf->getSeason()).toStdString());
  response->set_result(false);
   

  if(false == cf->checkSeason()){
    notice = QObject::tr("Метод не работает в это время года");
    response->set_notice(notice.toStdString());
    delete methods_list;
    return;
  }

  if(false == cf->checkSynSit()){
    notice = QObject::tr("Метод не работает в текущей синоптической ситуации");
    response->set_notice(notice.toStdString());
    delete methods_list;
    return;
  }

  // заполняем данными пришедшими из сервиса
  // cf->runForecastWithFilledModelOnly(false);
  cf->runForecastOnly(false);
  if(nullptr != cf->getSrcModel()){
    QString parent;
    parent = "";
    fillModelItemFromProto( &request->manual_data(), cf->getSrcModel()->rootItem(), &parent );

    cf->getSrcModel()->rootItem()->resetFirstChildItemForScript( );

    fillProtoFromModelItem(cf->getSrcModel()->rootItem(),response->mutable_source_data());
  }
  if(nullptr != cf->getResultModel()){
    fillProtoFromModelItem(cf->getResultModel()->rootItem(),response->mutable_rezult_data());
  }
  response->set_result(true);
  delete methods_list;
}

// endnamespace
}
}
