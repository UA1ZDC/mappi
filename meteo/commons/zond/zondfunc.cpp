#include "zondfunc.h"

#include <qlist.h>

#include "zond.h"
#include "indexes.h"

namespace zond {

bool calculateZondLayers( Zond* znd, meteo::surf::OneTZondValueOnStation* data )
{
  QList <float> layers ;
  // болтанка
  if ( znd->oprGranBoltan(&layers) ){
    fillZondLayersFromList(layers, data->mutable_boltanka(), znd );
  }
  layers.clear();
  // облачность
  if ( znd->oprGranOblak(&layers) ){
    fillZondLayersFromList(layers, data->mutable_cloudlayers(), znd );
  }
  layers.clear();
  // обледенение
  if ( znd->oprGranObled(&layers) ){
    fillZondLayersFromList(layers, data->mutable_obledenenie(), znd );
  }
  layers.clear();
  // слои конденсационный следов
  if ( znd->oprGranTrace(&layers) ){
    fillZondLayersFromList(layers, data->mutable_kondensate_trace(), znd );
  }
  layers.clear();

  // 
  // Ищем границы КНС
  if ( znd->oprGranKNS(&layers) ){
    if ( layers.size()==2 ){
      meteo::surf::ZondLayer* zondlayer = data->mutable_kns();
      zondlayer->set_bottom( layers.at(0)   );
      zondlayer->set_top( layers.at(1) );
    }
  }

  // забираем тропопаузу
  QList<zond::Uroven> ur_tropo;
  if( true == znd->getTropo(&ur_tropo) && ur_tropo.size()>0 ) {
    for (int idx = 0; idx < ur_tropo.size(); idx++) {
      meteo::surf::ZondLayer *zondlayer = data->add_tropo();
      zondlayer->set_bottom( ur_tropo.at(idx).value(zond::UR_H) );
      zondlayer->set_top( ur_tropo.at(idx).value(zond::UR_H) );
      zondlayer->set_t( ur_tropo.at(idx).value(zond::UR_T) );
    }
  }

  // слои инверсии и изотремии
  QVector<zond::InvProp> vectorlayers;
  if ( znd->getSloiInver(&vectorlayers) ){
    for (int i =0; i< vectorlayers.size(); ++i ){
      meteo::surf::ZondLayer *zondlayer;
      if ( vectorlayers.at(i).invType == zond::InvProp::InvType::INVERSIA ) {
        zondlayer = data->add_inverse();
      }else{
        zondlayer = data->add_isoterm();
      }
      zondlayer->set_bottom( vectorlayers.at(i).h_lo   );
      zondlayer->set_top(    vectorlayers.at(i).h_hi   );
    }
  }

  // энергия неустойчивости
  int energy = 0;
  energy = znd->oprZnakEnergy( 850.0 );
  data->set_energy( energy );

  // значение индекса
  float index_value;
  
  // индексы неустойчивости
  if ( zond::kiIndex(*znd, &index_value) ) {
    // ссылка на протоструктуру
    meteo::surf::MeteoParamProto* cur_index = data->add_indexes();
    cur_index->set_value(index_value);
    cur_index->set_descrname("K_index");
  }
  if ( zond::verticalTotalsIndex(*znd, &index_value) ) {
    // ссылка на протоструктуру
    meteo::surf::MeteoParamProto* cur_index = data->add_indexes();
    cur_index->set_value(index_value);
    cur_index->set_descrname("VT_index");
  }
  if ( zond::crossTotalsIndex(*znd, &index_value) ) {
    // ссылка на протоструктуру
    meteo::surf::MeteoParamProto* cur_index = data->add_indexes();
    cur_index->set_value(index_value);
    cur_index->set_descrname("CT_index");
  }
  if ( zond::totalTotalsIndex(*znd, &index_value) ) {
    // ссылка на протоструктуру
    meteo::surf::MeteoParamProto* cur_index = data->add_indexes();
    cur_index->set_value(index_value);
    cur_index->set_descrname("T_index");
  }
  if ( zond::sweatIndex(*znd, &index_value) ) {
    // ссылка на протоструктуру
    meteo::surf::MeteoParamProto* cur_index = data->add_indexes();
    cur_index->set_value(index_value);
    cur_index->set_descrname("SWEAT_index");
  }
  return true;
}

void fillZondLayersFromList( QList<float> &layers,
                             google::protobuf::RepeatedPtrField<meteo::surf::ZondLayer> *res,
                             Zond* znd )
{
  // если слоев четное количество
  if ( layers.size()%2 == 0 ){
    for (int i =0; i< layers.size(); i+=2 ){
      meteo::surf::ZondLayer *zondlayer = res->Add();
      zondlayer->set_bottom( znd->oprHpoP(layers.at(i)) );
      zondlayer->set_top( znd->oprHpoP(layers.at(i+1)) );
    }
  }
  return;
}

}
