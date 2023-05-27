#ifndef TZondDataService_H
#define TZondDataService_H

#include <meteo/commons/proto/surface_service.pb.h>

#include <meteo/commons/zond/turoven.h>
#include <meteo/commons/zond/indexes.h>
#include <meteo/commons/zond/zond.h>
#include "base_data_service.h"
#include <qrunnable.h>


namespace meteo {
  namespace surf {

    class Control;

enum kZondReqType{
  rGetValue,
  rGetValueWithCalc, // получаем  уровни с рассчитанными показателями
  //rGetValues,
  rGetOneAeroData,
  rGetManyAeroData,
  rGetTZondValue,
  rGetDates,
  rGetZondAvailable,
  rGetNo,
  rGetTZondSatValue
};

class TZondDataService  : public TBaseDataService {
  Q_OBJECT
public:
    TZondDataService(const meteo::surf::DataRequest*, google::protobuf::Closure* done  );

    ~TZondDataService();



    void setDataRequest(const meteo::surf::DataRequest* r);
    void setZondValueReply( meteo::surf::ZondValueReply* r);
    void setDataReply( meteo::surf::DataReply* r);
    void setDataReply( meteo::surf::TZondValueReply* r);
    void setReplyType( kZondReqType r);
    void setDateReply( meteo::surf::DateReply* r);
    void setZondStationsReply(StationsDataReply *r);


     //только одно значение у земли - по индексу, времени и дескриптору
    bool getValue(const meteo::surf::DataRequest* req,
                  meteo::surf::ZondValueReply* res);
   /* bool getValues(const meteo::surf::DataRequest* req,
                   meteo::surf::ManyZondValueReply* res);*/
    //данные зондирования по индексу и времени
    bool getOneAeroData(const meteo::surf::DataRequest* req, meteo::surf::ZondValueReply* res);
    //много данных зондирования по индексу и интервалу времени - поле
    bool getManyAeroData(const meteo::surf::DataRequest* req, meteo::surf::ZondValueReply* res);

    bool getTZondValue(const meteo::surf::DataRequest* req, meteo::surf::TZondValueReply* res);
    // получаем АД с расчетами слоев, высот и индексов в прото формате
    bool getValueWithCalc(const meteo::surf::DataRequest* req, meteo::surf::TZondValueReply* res);
    bool getTZondSatValue(const meteo::surf::DataRequest* req, meteo::surf::TZondValueReply* res);
    bool getDates(const meteo::surf::DataRequest* req, meteo::surf::DateReply* res);
    bool getAvailable(const meteo::surf::DataRequest* req, meteo::surf::StationsDataReply* res);

    //несколько значений - по индексу, дескриптору и интервалу времени
    //! формирование запроса для таблицы индетификации по региону
     //! формирование запроса для таблицы индетификации по имеющимся парам (дескриптор, значение)
     //! создание правила для выборки данных по дате/времени начала, конца, категрии, запроса для таблицы идентификации
private :
  //значение уровня  по дескриптору, индексу и времени
  bool getOneAeroLevel(zond::ValueType ur_type, const meteo::surf::DataRequest* req, meteo::surf::ZondValueReply* res);
  //данные зондирования по индексу и времени в  TMeteoData
  bool getOneAeroData(const QString &  station_id_, const QString &dts, zond::Zond *znd);
  bool getOneZondData(const QString &  station_number_, const QString &dts, zond::Zond *znd);
  bool getOneZondSatData(const QString &  station_number, int datattype, const QString &dts, zond::Zond *znd);

  bool getNearDate(const QString &  station_id_,int, int, QString *dts);
  bool getAeroDataList(const QString & station_id_,  const QString &dts,  const QString &dte, int type, QList <QString> * dlist);

  void toProtoUroven(const zond::Uroven &ur_, meteo::surf::Uroven*  res);

  // заполняем уровни из зонда
  //void fillZondLayersFromList(QList<float> &layers, google::protobuf::RepeatedPtrField<meteo::surf::ZondLayer> *res, zond::Zond *znd);

  const meteo::surf::DataRequest* data_request_;
  meteo::surf::ZondValueReply* zond_value_repl_ = nullptr;
  meteo::surf::DataReply* data_reply_ = nullptr;
  meteo::surf::TZondValueReply *tzond_data_reply_ = nullptr;
  meteo::surf::DateReply* date_reply_ = nullptr;
  meteo::surf::StationsDataReply* stations_reply_ = nullptr;

  kZondReqType req_type_;

  QList<int> aero_types_;

public slots:
  void run();

};

}
}

#endif
