#ifndef METEO_COMMONS_SERVICES_ESIMO_NC_ESIMOPARAM_H
#define METEO_COMMONS_SERVICES_ESIMO_NC_ESIMOPARAM_H

#include <commons/meteo_data/meteo_data.h>

namespace meteo {

  typedef bool (*FuncConvertEsimo)(const QString& esimoname, descr_t descr, float val, TMeteoData* data);

  //! для преобразования параметров есимо к метеодате
  class EsimoParam {
  public:
    EsimoParam();
    ~EsimoParam();

    bool fillMeteo(const QString& esimoname, descr_t descr, float val, TMeteoData* data);
    
  private:
    void init();
    
  private:
    QHash<QString, FuncConvertEsimo> _funcs; //!< Функции для преобразования есимо значения в метеодату
    
  };

}





#endif
