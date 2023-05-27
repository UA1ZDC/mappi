#ifndef METEO_COMMONS_PRIMARYDB_GRIBMONGO_H
#define METEO_COMMONS_PRIMARYDB_GRIBMONGO_H

#include <meteo/commons/services/decoders/tservicestat.h>
#include <meteo/commons/grib/tgribdefinition.h>
#include <meteo/commons/proto/tgribformat.pb.h>
#include <meteo/commons/proto/surface.pb.h>
#include <commons/geobasis/geodata.h>

#include <qstring.h>
namespace meteo {

  class Dbi;
  class GridFs;
  class GridFile;

  class GribMongoDb {
  public:
    GribMongoDb();
    ~GribMongoDb();

    bool save(const grib::TGribData* grib);

    void setConnectionStatus(meteo::app::OperationStatus* status);   
    const service::Status<grib::SourceType, grib::StatType>& status() const { return _stat; }
    const QString& ptkppId() { return _ptkppId; }
    void setPtkppId(const QString& id) { _ptkppId = id; }

    bool readFromDb( const QString& id, grib::TGribData* grib );
    bool parceGrib( const QString& obj, ::grib::TGribData* grib );
    bool getNotAnalysedGribs( QList<QPair<QString,QString>> *gribs, ::grib::GribInfo* info );
    bool getGribData(QList<QPair<QString,QString>> *gribs,
                     const surf::GribDataRequest *req, surf::GribDataReply* res);
    bool getGribFileId(QList<QPair<int, QString> >* gribs,
		       const surf::GribDataRequest* req, surf::GribDataReply* res);
    bool setAnalysed( const QString &id );

    int64_t descrForParam( int32_t paramv1 ) const ;
    int64_t descrForParam( int32_t discipline, int32_t category, int32_t paramv2 ) const ;
    QString unitByDescr(int64_t descr);
    
    bool fillFromGrib( meteo::GeoData* data, /*NetType *nt,*/
		       const meteo::surf::GribData& grib, bool convertUnit);
    bool fillGridWithoutBorder(meteo::GeoData* data,
			       meteo::GeoData* left,
			       meteo::GeoData* top,
			       meteo::GeoData* right,
			       meteo::GeoData* bottom,
			       const meteo::surf::GribData& grib);
  private:
    bool init();
    bool checkDb() const ;
    bool loadParameters();
    bool saveToDb(const QString& collection, const QString& fileName,
                  const std::string& msg, GridFile* gridfile);

  private:
    service::Status<grib::SourceType, grib::StatType> _stat;

    Dbi* db_ = nullptr;
    GridFs* gridfs_ = nullptr;

    grib::SourceType _edition = grib::kGribUnk; //!< номер издания GRIB

    QString _ptkppId; //!< id таблицы, содержащей исходную телеграмму
    //    uint _cnt = 0; //!< счетчик для создания имени файла
    grib::Parameters gribparams_;
    QMap< int, grib::Parameter* > paramv1_;
    QMap< QPair< int, QPair< int, int > >, grib::Parameter* > paramv2_;
  };

  namespace grib {
    GribMongoDb* dbmongo();
  }
}

#endif
