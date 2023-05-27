#ifndef METEO_COMMONS_UI_OCEANDIAGRAM_OCEANCORE_OCEANDIAGDATA_H
#define METEO_COMMONS_UI_OCEANDIAGRAM_OCEANCORE_OCEANDIAGDATA_H

#include <qstandarditemmodel.h>

#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>
#include <commons/meteo_data/meteo_data.h>

#include <meteo/commons/ocean/placedata.h>
#include <meteo/commons/proto/services.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/rpc/rpc.h>



class TMeteoData;

namespace ocean {
class PlaceData;
}
namespace meteo {
class InputForm;
namespace sprinf {
class Stations;
} // sprinf
namespace surf {
class DataRequest;
class ZondValueReply;
class OneZondValueOnStation;
class OceanOnPointReply;
class OneOceanValueOnPoint;
} // surf
namespace field {
class DataRequest;
} // surf
} // meteo

namespace meteo {
namespace odiag {
//! Данный тип используется для обозначения номера секции, в которой хранится значение входного параметра.
enum Section {
  kStartDt,
  kEndDt,
  kMeteoDescr,
  kHour,
  kModel,
  kLevel,
  kCenter,
  kNetType,
  kTypeLevel,
  kOnlyEqually,
  kNeedFieldDescr,
  kFieldId,
  kIsDf,
  kCoords
};
//! Данный тип используется для обозначения типа сервиса, к которому должен осуществляться запрос.
enum ServiceType { kSrcData, kFieldData };

//! Данный тип используется для обозначения типа диаграммы
enum Type {
  kSensType,     //!< данные зондирования
  kForecType      //!< прогностическая
};


class OceanDiagData 
{
public:

  OceanDiagData();
  ~OceanDiagData();

  bool loadAvailableData(QMap<QString, ocean::PlaceData>* pdataList);
  bool fillProfile(QString* id, ocean::PlaceData* data);

  void setStartTime(const QDateTime& dt ){sdt_ = dt; }
  void setEndTime(const QDateTime& dt ){edt_ = dt; }
  void setCenter(int c ){center_ = c; }
  void setServiceType(ServiceType c ){stype_ = c; }
  void setType(Type c ){type_ = c; }
  
  
private:

  // список полей, заполняемых при формирование запроса на получение данных (field.service), для каждого типа в.р.а
  QMap<Type,QList<Section> > dataFieldReqMap_;

  QDateTime sdt_;
  QDateTime edt_;
  ServiceType stype_;
  Type type_;
  int center_;
};

}
}

#endif // METEO_COMMONS_UI_OCEANDIAGRAM_OCEANCORE_OCEANDIAGDATA_H
