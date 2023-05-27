#ifndef METEO_COMMONS_FIELD_ANALYSE_WRAP_H
#define METEO_COMMONS_FIELD_ANALYSE_WRAP_H

#include <memory>

#include <meteo/commons/proto/field.pb.h>

#include "methodwrap.h"

class NS_PGBase;

namespace meteo {
namespace field {
  class hdelta {
    public:
    hdelta(int h,float d):hour(h),delta(d){}
    hdelta():hour(0),delta(0.){}

    int hour ;
    float delta ;
  };

  class forecastDelta{
  public:
    int level;
    int type_level;
    int descriptor;
    QList< hdelta > deltas;
  };

class Control;

class TFieldAnalyseWrap : public MethodWrap
{
  using RpcController = google::protobuf::RpcController;
  using Closure = google::protobuf::Closure;

public:
  TFieldAnalyseWrap(TFieldDataService* service);
  ~TFieldAnalyseWrap();

  //void GetAdvectFields( const AdvectDataRequest* req, AdvectDataReply* resp);
  //void GetAdvectPoints( const AdvectDataRequest* req, AdvectDataReply* resp);
  void GetAdvectObject( const AdvectObjectRequest* req, AdvectObjectReply* resp);
 // void GetAdvectObjects( const AdvectObjectsRequest* req, AdvectObjectsReply* resp);
  void getSynSit(const DataRequest* req, ValueDataReply* resp);
  void CalcForecastOpr( const DataAnalyseRequest* req, SimpleDataReply* res);//!< Провести анализ поля
  void GetExtremums( const DataRequest* req, meteo::field::ExtremumTrajReply* resp);
  void GetExtremumsTraj(const SimpleDataRequest* req, meteo::field::ExtremumTrajReply* resp );
  void calcNear( const QDateTime& cdt, const QVector<fieldExtremum>& next_extr,
                                     QMap<int, QMap<QDateTime, fieldExtremum>>* traj_extr);
private:


  void advect(const AdvectObjectRequest* ,
                   const QMap<QDateTime, QString>&,
                   QMap<QString, QVector<GeoVector> > *, QVector<DataDesc> *);

  bool analyseForecast(const QDateTime &dt, const forecastDelta &, int center);
  bool loadForecast(DataReply* mresp, const forecastDelta& deltas);
  bool getOnePointSynSit(const QString& oid, const DataRequest* req, ValueDataReply* resp);
  bool getFieldsIdForAdvect(const AdvectObjectRequest* req, QMap<QDateTime, QString>* field_for_advect);
  bool getForecastsDeltas(QList<forecastDelta> *resps);
  bool loadField(const DataRequest* req, DataReply* resp);
  bool loadFields(const DataRequest* req, ManyDataReply* resp);
  bool getFieldModels(const DataRequest& req, QList<int>* models);
  bool hasDates(const DataRequest& req,const::std::string& date);

};

}
}

#endif
