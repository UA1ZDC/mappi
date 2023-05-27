#ifndef METEO_COMMONS_SERVICE_FIELDATA_METHODWRAP
#define METEO_COMMONS_SERVICE_FIELDATA_METHODWRAP

#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/rpc/clienthandler.h>
#include <commons/obanal/tfieldmongo.h>

#include <meteo/commons/zond/turoven.h>
#include <meteo/commons/zond/indexes.h>
#include <meteo/commons/zond/zond.h>

#include <sql/nosql/nosqlquery.h>

class GDALDataset;
class GDALDriver;

namespace meteo {

class NoSql;
class NosqlQuery;
class GridFile;
namespace field {

static const QString kObanaldb;
static const QString kFieldPrefix      = "obanal";

class TFieldDataService;

class MethodWrap : public QObject
{
  Q_OBJECT
public:
  MethodWrap( TFieldDataService* service );
  ~MethodWrap();


  void GetFieldData( const DataRequest* req, DataReply* resp);
  void GetFieldsData( const DataRequest* req, ManyDataReply* res );
  void GetFieldsMData( const DataRequest* req, meteo::surf::DataReply* resp );
  void GetFieldsDataWithoutForecast( const DataRequest* req, ManyDataReply* resp );
  void GetFieldDataPoID( const SimpleDataRequest* req, DataReply* resp ,int num_f);
  void GetFieldDataPoID( const SimpleDataRequest* req, DataReply* resp );
  void GetFieldDataForMeteosummary( const DataRequest *req, ValueDataReply *resp );
  void GetAvailableCenters( const DataRequest* req, CentersResponse* resp );
  void GetAvailableCentersForecast( const DataRequest *req, CentersResponse *resp );
  void GetAvailableCentersAD( const DataRequest *req, CentersResponse *resp );
  void GetForecastValues( const DataRequest* req, ValueDataReply* resp );
  void GetAvailableData( const DataRequest* req, DataDescResponse* resp );
  void GetAvailableHours( const DataRequest* req, HourResponse* resp );

  void GetADZond( const DataRequest *req, surf::TZondValueReply *resp );
  void GetLastDate( const DataRequest* req, SimpleDataReply* resp );
  void GetFieldDates( const DataRequest *req, DateReply *resp );
  void GetNearDate( const DataRequest* req, DateReply* resp );
  void GetFieldsForDate( const DataRequest* req, ManyFieldsForDatesResponse* resp );

  /*!
     * \brief - получить набор изолиний из одного поля, в точности соответствующего параметрам запроса req
     * \param req - параметры запроса
     * \param resp - набор изолиний
     */
  void GetIsoLines( const DataRequest* req, meteo::IsoLines* resp );
  
  /*!
     * \brief - получить поле величины в виде proto-сообщения
     * \param req - параметры запроса
     * \param resp - набор полей, удовлетворяющих условиям запроса
     */
  void GetFieldProto( const DataRequest* req, meteo::field::FieldReply* resp );

  // заполняем уровни из зонда
  //void fillZondLayersFromList(QList<float> &layers, google::protobuf::RepeatedPtrField<meteo::surf::ZondLayer> *res, zond::Zond *znd);
  void GetGeoTiff( const DataRequest* req, DataReply* resp);
  void GetIsoLinesShp( const DataRequest* req, meteo::IsoLinesShp* resp );

protected:
  bool getFieldFromFile( const QString &file_id, QByteArray *barr );
  bool getAvailableForecastData( const DataRequest* req, DataDescResponse* resp );
  bool getAvailableDataForMeteosummary( const DataRequest *req, DataDescResponse *resp );
  bool loadFieldsForPuanson( const DataRequest* req, ManyDataReply* resp );
  bool GetFieldDescrPoID( const QString& oid, DataReply* resp );
  bool GetFieldPoID( const QString& oid, DataReply* resp );
  bool getOnePointData( const GridFile& file, const DataRequest* req, ValueDataReply* resp );
  bool getFieldsIdForProfile( const DataRequest *req, DataDescResponse *resp );
  bool getTzonds( const DataRequest* req, meteo::surf::TZondValueReply* resp );
  bool getFieldsForAD( const DataRequest* req, ManyFieldsForDatesResponse* resp );
  bool createGeotiff(::obanal::TField* field, const QString& tiffname, const QString& layername);
  bool createEPSG3857(GDALDriver* driver, char **papszOptions, GDALDataset*gds, const QString& tiffname, const QString& layername);
  bool fromFileToByteAndRm(const QString & file_name, QByteArray * barr);

  /*!
     * \brief - получить описание поля по запросу
     * \param req - параметры запроса
     * \param desc - параметры поля
     * \param file_id - id файла с полем
     * \param errorstr - описание ошибки
     * \return - true - все ОК, false - не удалось прочитать описание поля
     */
  bool loadFieldInfo( const DataRequest* req, DataDesc* desc, QString* file_id, QString* errorstr );

protected:
  TFieldDataService* service_ = nullptr;
  QString dbname_;

};

}
}


#endif
