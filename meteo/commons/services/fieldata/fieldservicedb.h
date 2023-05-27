#ifndef METEO_COMMONS_SERVICE_FIELDATA_FIELDSERVICE_DB_H
#define METEO_COMMONS_SERVICE_FIELDATA_FIELDSERVICE_DB_H

#include <meteo/commons/proto/field.pb.h>
#include <commons/obanal/tfieldmongo.h>
#include <sql/dbi/gridfs.h>
#include <sql/nosql/nosqlquery.h>

class NS_PGBase;

namespace meteo {
namespace field {

  static const QString kObanaldb;
  static const QString kFieldPrefix      = "obanal";
  static const QString kProcesscol       = "process_fields";

  class Control;
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

class TFieldDataDb
{
  QStringList createMatch(const DataRequest* req, const QString& dt_column);

public:
  TFieldDataDb();
  ~TFieldDataDb();

  //bool GetFieldPoID (const QString& oid, DataReply* resp);
  //bool getAvailableCenters(const DataRequest* req, QMap<int, int>* centers, CentersResponse* resp = nullptr);
  //bool getAvailableForecastData(const DataRequest* req, DataDescResponse* resp);
  //bool getAvailableDataForMeteosummary(const DataRequest* req, DataDescResponse* resp);
  //bool getAvailableHours(const DataRequest* req, HourResponse* resp);
  //bool getLastDate(const DataRequest* req, SimpleDataReply* resp);
  //bool getFieldDates(const DataRequest* req, DateReply* resp);
  //bool loadField(const DataRequest* req, DataReply* resp);
  //bool loadFields(const DataRequest* req, ManyDataReply* resp);
  //bool loadFieldsForPuanson(const DataRequest* req, ManyDataReply* resp);
  //bool getCentersForProfile(const DataRequest* req, CentersResponse* resp);
  //bool getFieldsIdForProfile(const DataRequest* req, DataDescResponse* resp);
  //bool getFieldsForDate(const DataRequest *req, ManyFieldsForDatesResponse *resp);
  //bool GetFieldsIdForValue  (const DataRequest* req, DataDescResponse* resp);
  //void deleteProcessField(const SimpleDataRequest* req, SimpleDataReply* resp);
  //bool EditProcessField(const ProcessFieldData* req, SimpleDataReply* resp);
  //bool AddProcessField(const ProcessFieldData* req, SimpleDataReply* resp);
  //bool GetAvailableCentersForecast(const DataRequest* req, QMap<int, int>* centers, CentersResponse* resp = nullptr);

  //void toProtoUroven(int descr, double aval, ::meteo::surf::Uroven* pur);

  //bool getOnePointData(const GridFile & , const DataRequest* req, ValueDataReply* resp);
  //bool getOnePointSynSit(const QString& oid, const DataRequest* req, ValueDataReply* resp);
 // bool getExtremums(const QString& oit, const DataRequest* req, ValuesDataReply* resp);
  //bool getFieldFromFile(const QString &fn, QByteArray *ba);

 // bool GetFieldDescrPoID(const QString& oid, DataReply* resp);

 // bool getForecastsDeltas(QList<forecastDelta> *resp);
 // bool getFieldModels(const DataRequest& req, QList<int>* models);
  //bool hasDates(const DataRequest& req,const::std::string& date);


private:


  QHash<QString, DataReply>         fieldHash;
  QHash<int, DataReply>             fieldHash_old;

  QHash<QString, DataDescResponse>  aviableFieldHash;
  QHash<QString, QByteArray>        fileHash;

  QString dbname_;

  /*! \brief Сохранение бинарных данных в файл
  *  \param msg    Бинарные данные
  *  \param path   Путь к файлу
  *  \param fileName  Имя файла
  *  \return false - в случае ошибки записи
  */
  bool saveToFile(const QString& basePath,const QString& fileName, const std::string &data);

  bool prepareQuery(const QString& templ_name, NoSql** client,QString* queryStr, QString* errstr = nullptr);
  bool prepareQueryExec(const QString& templ_name, NoSql** client, QString* errstr = nullptr);
  void checkQueryMatch(const DataRequest* req, const QString& querys, NosqlQuery* query);

};

} // field
} // meteo

#endif
