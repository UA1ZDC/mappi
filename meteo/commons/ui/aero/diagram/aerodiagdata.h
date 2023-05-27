#ifndef METEO_COMMONS_UI_AeroDiagData_H
#define METEO_COMMONS_UI_AeroDiagData_H

#include <qstandarditemmodel.h>

#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>
#include <commons/meteo_data/meteo_data.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/services.pb.h>
#include <meteo/commons/zond/placedata.h>


class TMeteoData;

namespace zond {
  class PlaceData;
}
namespace meteo {
  class InputForm;
  namespace sprinf {
    class Stations;
  } // sprinf
  namespace surf {
    class DataRequest;
    class DataReply;
    class ZondValueReply;
    class OneZondValueOnStation;
  } // surf
  namespace field {
    class DataRequest;
  } // surf
} // meteo

namespace meteo {
  namespace adiag {
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
    enum ServiceType {
      kSrcData = 0,
      kFieldData = 1,
      kSatData = 2};
  
  // enum Type {
  //   kSpaceType,     //!< Пространственный.
  //   kTimeType,      //!< Временной.
  //   kSpaceTimeType,  //!< Пространственно-временной.
  //   kSensType,     //!< данные зондирования
  //   kForecType      //!< прогностическая
  // };
  //! Данный тип используется для обозначения типа аэрологической диаграммы

  class AeroDiagData 
{
  public:
    
  AeroDiagData();
    ~AeroDiagData();
    
    bool getData(zond::PlaceData* pdata,QList <zond::PlaceData*> *rez_pdata, bool is_last);
    //! Запрашивает у сервиса sprinf ближайшую к координате coord станцию в радиусе radius.
    bool loadCenters( QMap< int , QString > *list );
    field::DataRequest makeDataFieldRequest(const GeoPoint& );
    
    
    void setTime(const QDateTime& dt ){dt_ = dt; }
    void setCenter(int c ){center_ = c; }
    void setServiceType(ServiceType c ){stype_ = c; }
  // void setType(Type c ){type_ = c; }
    void setDataRestore(bool _isDataRestore) {isDataRestore = _isDataRestore; }
    void setForecHour(int _forecHour) {forecHour = _forecHour;}
    
  private:
    //! Формирует запрос, по входным данным из формы для в.р.а. типа type, на получение данных от сервиса field.service.
    
    bool zondToPdata(zond::PlaceData *pdata, const surf::OneZondValueOnStation &one_zond ) ;
    //!
    surf::TZondValueReply* loadSrcData(   const QString& ,bool) ;
    surf::TZondValueReply* loadFieldData( zond::PlaceData *,bool) ;
  
    QDateTime dt_;
    int forecHour;
    ServiceType stype_;
  //    Type type_;
    int center_;
    bool isDataRestore;
};

}
}

#endif
