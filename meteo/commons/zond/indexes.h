#ifndef METEO_COMMONS_ZOND_INDEXES_H
#define METEO_COMMONS_ZOND_INDEXES_H

namespace zond {
  class Zond;
  
  //Индексы неустойчивости
  bool kiIndex(const zond::Zond& zond, float* result);
  bool verticalTotalsIndex(const zond::Zond& zond, float* result);
  bool crossTotalsIndex(const zond::Zond& zond, float* result);
  bool totalTotalsIndex(const zond::Zond& zond, float* result);
  bool sweatIndex(const zond::Zond& zond, float* result);
}


#endif
