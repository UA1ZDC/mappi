#ifndef METEO_COMMONS_SERVICES_OBANAL_TFIELD_CALC_H
#define METEO_COMMONS_SERVICES_OBANAL_TFIELD_CALC_H

#include <memory>

#include <qstring.h>

#include <commons/obanal/tfield.h>

#include <sql/psql/psql.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/services/sprinf/sprinfservice.h>

class TObanalDb;

namespace obanal {

class TFieldCalc
{
  using ObanalDbPtr      = std::shared_ptr<TObanalDb>;

  // QMap< int, RegionParam > rpars;
   ObanalDbPtr db_;

public:
  TFieldCalc(ObanalDbPtr db);
  ~TFieldCalc();
  
  bool calcOT500_1000( const obanal::TField& f, const meteo::field::DataDesc* fdescr );
  bool calcWind( const obanal::TField& f, const meteo::field::DataDesc* _fdescr );
  bool calcGeopotentialHeight( const obanal::TField& f, const meteo::field::DataDesc* _fdescr );
  
private:
  bool getField(const QDateTime&, int level, int hour, int descr, int, obanal::TField*);
  bool getField(const meteo::field::DataDesc* req, TField* fd);
};

} // obanal

#endif // METEO_COMMONS_SERVICES_OBANAL_TFIELD_CALC_H
