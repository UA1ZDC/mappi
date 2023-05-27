#ifndef func_fillData_h
#define func_fillData_h

#include <string.h>
#include "data_structures.h"
//#include "data_types.h"
#include "func_common.h"
//v#include "tdatatext.h"

void fillWeatherDataAll(TWeatherDataAll *);
void fillWeatherDataAllBlank(TWeatherDataAll *);
void fillUroven(Uroven *);
void fillGRWUroven(GRWUroven *);
void fillSeaDataAll(TSeaDataAll *);
void fillSea_gr_3(TSea_gr_3 *);
void fillSea_gr_6(TSea_gr_6 *);
void fillSea_gr_7(TSea_gr_7 *);
void fillTesacDataAll(TTesacDataAll *);
void konstrWeather(TWeatherDataAll *);
void konstrShar(TSharDataAll *);
void konstrSharBasa(TSharDataBasa *);
void konstrAero(TAeroDataAll *);
void konstrMeteo11(TMeteo11DataAll *);
void konstrLayer(TLayerDataAll *);
TAeroDataAll Preobr(TAeroDataBasa);
void Preobr(TAeroDataBasa *, TAeroDataAll *);
TSharDataAll PreobrShar(TSharDataBasa);
bool PreobrShar1(TSharDataAll, TSharDataBasa *);
bool Preobr1(TAeroDataAll, TAeroDataBasa *);
void Zagolovok(TAeroDataAll *, TAeroDataBasa);
void ZagolovokShar(TSharDataAll *, TSharDataBasa);
int ZamenaKN01(TWeatherDataAll *, TWeatherDataAll);
int ZamenaShar(TSharDataBasa *, TSharDataBasa);
int ZamenaKN04(TAeroDataBasa *, TAeroDataBasa);
void konstrAeroBasa(TAeroDataBasa *);

#endif
