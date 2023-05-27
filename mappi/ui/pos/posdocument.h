#ifndef MAPPI_UI_POS_POSDOCUMENT_H
#define MAPPI_UI_POS_POSDOCUMENT_H

#include <meteo/commons/proto/map_document.pb.h>
#include <meteo/commons/ui/map/document.h>

class SatelliteBase;

namespace meteo {
namespace map {

class PosDocument : public Document
{
  public:
    PosDocument( const proto::Document& doc );

    /*! 
     * \brief Инициализация параметров документа в соответствии с параметрами сеанса
     * \param ts        - время начала сеанса
     * \param te        - время окончания сеанса
     * \param params    - параметры орбиты
     * \param sa        - угол сканирования
     * \param w         - количество зачений в строке сканирования
     * \params lps      - скорость приема (строк/сек)
     * \return          - флаг успешной инициализации 
     */
    bool init( const QDateTime& ts, const QDateTime& te, SatelliteBase* params, double sa, double w, double lps );
};

} // map
} // meteo

#endif
