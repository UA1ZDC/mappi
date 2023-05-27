#ifndef METEO_COMMONS_ZOND_ZONDFUNC_H
#define METEO_COMMONS_ZOND_ZONDFUNC_H

#include <qstring.h>
#include <qmap.h>

#include <meteo/commons/proto/surface.pb.h>

namespace zond {

class Zond;

bool calculateZondLayers( Zond* zond, meteo::surf::OneTZondValueOnStation* data );
void fillZondLayersFromList(QList<float> &layers, google::protobuf::RepeatedPtrField<meteo::surf::ZondLayer> *res, zond::Zond *znd);


}

#endif
