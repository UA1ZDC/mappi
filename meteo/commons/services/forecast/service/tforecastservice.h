#ifndef TforecastService_H
#define TforecastService_H

#include <meteo/commons/proto/state.pb.h>
#include <meteo/commons/proto/services.pb.h>
#include <meteo/commons/proto/forecast.pb.h>
#include <meteo/commons/rpc/rpc.h>

#include "tforecastdb.h"
#include "methodbag.h"
#include "fizmethodbag.h"
#include <commons/geobasis/geodata.h>

#include <qstring.h>


namespace forecast {

  //! Состояние
  enum StatType {
    kReqUnknown = -1,   //!< Для задания описания
    kParamErr = 0,	//!< Некорректные параметры
    kForecastOk = 1,      //!< Объективный анализ выполнен
    kForecastErr  = 2,    //!< Ошибка выполнения объективного анализа
  };
}
namespace meteo {
  namespace forecast {



//! сервис объективного анализа
class TForecastService : public ForecastData {
public:
  TForecastService();
  ~TForecastService();

  bool calcAccuracyForecast(const meteo::field::OnePointData* req);

  void SaveForecast( google::protobuf::RpcController* c,
                     const meteo::forecast::ForecastPointData* req,
                     meteo::field::SimpleDataReply* res,
                     google::protobuf::Closure* d );


  void SaveManyForecasts( google::protobuf::RpcController* c,
                     const meteo::forecast::ForecastManyPointsData* req,
                     meteo::field::SimpleDataReply* res,
                     google::protobuf::Closure* d );

  void CalcAccuracyForecast( google::protobuf::RpcController* c,
                     const meteo::field::OnePointData* req,
                     meteo::field::SimpleDataReply* res,
                     google::protobuf::Closure* d );

  void GetAccuracy(google::protobuf::RpcController* controller,
                   const AccuracyRequest* request,
                   AccuracyReply* response,
                   google::protobuf::Closure* done);

  // запрашиваем данные оправдываемости по всем методам по одной станции
  void GetManyAccuracy(google::protobuf::RpcController* controller,
                   const ManyAccuracyRequest* request,
                   ManyAccuracyReply* response,
                   google::protobuf::Closure* done);


  //
  // Считаем оправдавшиеся прогнозы по станции
  // на вход координаты станции или индекс
  // на выходе - запись в базе forecast_opr с количеством оправдавшихся и неоправд
  // расчетных методов
  //
  void CalcForecastOprStation(google::protobuf::RpcController* controller,
                   const meteo::forecast::AccuracyRequest* req,
                   meteo::field::SimpleDataReply* response,
                   google::protobuf::Closure* done);



  /**
   * получаем все прогнозы по станции для подсчета
   * @param controller [description]
   * @param request    [description]
   * @param response   [description]
   * @param done       [description]
   */
  void GetForecastResult(google::protobuf::RpcController* controller,
                         const ForecastResultRequest* request,
                         ForecastResultReply* response,
                         google::protobuf::Closure* done);


  void GetForecastPunkts(google::protobuf::RpcController* controller,
                         const PunktRequest *request,
                         PunktResponce *response,
                         google::protobuf::Closure* done);

  void GetMeteoData(google::protobuf::RpcController *controller,
                    const surf::DataRequest *request,
                    surf::DataReply *response,
                    google::protobuf::Closure *done);


  void GetOneForecastToCalculate(google::protobuf::RpcController *controller,
                     const Dummy *request,
                     ForecastResultReply *response,
                     google::protobuf::Closure *done);

  void FillGradacii(google::protobuf::RpcController *controller,
                    const Gradacii *request,
                    Gradacii *response,
                    google::protobuf::Closure *done);


  void SaveForecastOpr(google::protobuf::RpcController *controller,
                       const ForecastRequest *request,
                       ForecastResultReply *response,
                       google::protobuf::Closure *done);

  void SaveForecastError(google::protobuf::RpcController *controller,
                         const ForecastRequest *request,
                         ForecastResultReply *response,
                         google::protobuf::Closure *done);

  void CalAccuracyForecastMany( google::protobuf::RpcController* c,
                     const MsgCalAccuracyForecastMany* req,
                     meteo::field::SimpleDataReply* res,
                     google::protobuf::Closure* d );

  // метод сохранения введенных вручную прогнозов
  void SaveManualForecast( google::protobuf::RpcController* c,
                     const ForecastManualData* request,
                     ForecastManualReply* response,
                     google::protobuf::Closure* done );
                     
  // метод получения введенных вручную прогнозов
  void GetManualForecast( google::protobuf::RpcController* c,
                     const ForecastManualRequest* request,
                     ForecastManualReply* response,
                     google::protobuf::Closure* done );
                     

  void getForecastAV12( google::protobuf::RpcController* c,
                     const ForecastAv12Request* req,
                     ForecastAv12Responce* res,
                     google::protobuf::Closure* d );


   //получить список доступных методов
  void GetForecastMethodsList( google::protobuf::RpcController* c,
                     const Dummy* request,
                     ForecastMethodList* response,
                     google::protobuf::Closure* );

  void GetForecastMethodsResultRun(google::protobuf::RpcController* c,
                                   const ForecastResultRequest*,
                                   ForecastFizResult* response,
                                   google::protobuf::Closure* done);

  bool prognFromObanal();
private:
  void checkFizMethodsBag();

  meteo::rpc::Channel* ctrl_field_;
  TForecastDb *fdb_;

  MethodBag* methodbag_ = nullptr;
  FizMethodBag* fizmethodbag_ = nullptr;

};

} // namespace forecast
} // namespace meteo
#endif
