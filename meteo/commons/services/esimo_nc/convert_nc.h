#ifndef METEO_COMMONS_SERVICES_ESIMO_NC_CONVERT_NC_H
#define METEO_COMMONS_SERVICES_ESIMO_NC_CONVERT_NC_H

#include <commons/obanal/tfield.h>
#include <commons/obanal/obanal_struct.h>
#include <meteo/commons/proto/field.pb.h>

#include <commons/meteo_data/meteo_data.h>
#include <meteo/commons/proto/surface.pb.h>
#include <meteo/commons/proto/esimo.pb.h>

#include <netcdf.h>

class QDir;

namespace meteo {
  class EsimoParam;
  class EsimoConf;
  
  
  //ЕСИМО netCDF в TMeteoData или поля
  class ConvertNc {
  public:
    
    ConvertNc();
    ~ConvertNc();

    //усвоение папки, содержащей ответ (nc файлы, xml)
    bool assimilateDir(const QDir& esimodir, meteo::surf::DataType datatype, meteo::EsimoType esimonc, NetType net_type);
    
    //преобразование и сохранение в виде метеоданных
    bool assimilateFile(const QString& ncFile, int datatype);
    //преобразование и сохранение в виде сетки
    bool assimilateGridFile(const QString& ncFile, NetType net_type);
    
    //преобразование
    bool convert(const QString& ncFile, TMeteoData* md);

    
    bool esimoid2datatype(const QString esimoId, meteo::surf::DataType *datatype, meteo::EsimoType *esimonc, NetType* nettype);
    descr_t getDescriptor(const QString& esimoName);
    
    
  private:
    
    bool readParamDimension(int ncid, int paramId, size_t* size, QList<size_t>* dims);
    bool readParamData(int ncid, int paramId, int size, const QString& name, QVector<float>* data);
    bool readParamCodeData(int ncid, int paramId, const QString& name, size_t rows, size_t length, QStringList* data);
    bool readParamCodeData(int ncid, int paramId, const QString& name, size_t rows, size_t length, QVector<float>* data);

    //для метеодата
    bool fillTime(int ncid, TMeteoData* data);
    bool fillFragmentedTime(int ncid, TMeteoData* data);
    bool getParamAttrs(int ncid, int id, QString* sname, descr_t* descr, size_t* size, QList<size_t>* dims, nc_type* type);
    void fillStrMeteoData(int ncid, int id, const QString& sname, descr_t descr, const QList<size_t>& dims, TMeteoData* md);
    void fillNumMeteoData(int ncid, int id, const QString& sname, descr_t descr, size_t size, TMeteoData* md);
    
    //для сетки
    bool readOneTime(int ncid, QString* dt, QString* dtstart);
    bool readGridCoords(int ncid, QVector<float>* lat, QVector<float>* lon);
    bool addCorners(QVector<RegionParam>& corners, const QVector<float>& lat, const QVector<float>& lon);

    bool assimilateFields(int ncid, int nvars_in, NetType net_type,
			  const QVector<float>& lat, const QVector<float>& lon,
			  meteo::field::DataDesc* fdescr);
    bool saveField(meteo::field::DataDesc* fdescr, const QVector<float>& lat, const QVector<float>& lon, const QVector<float>& data);
    bool saveInterpolField(meteo::field::DataDesc* fdescr, const QVector<float>& lat, const QVector<float>& lon, const QVector<float>& data);
    
  private:
    QTextCodec* _codec = nullptr;
    EsimoParam* _ep = nullptr;

    QMap<QString, EsimoObjectConf> _conf;
  };
  
}


#endif
