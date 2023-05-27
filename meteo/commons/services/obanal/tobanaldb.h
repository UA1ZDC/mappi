#ifndef METEO_COMMONS_SERVICES_OBANAL_OBANALDB_H
#define METEO_COMMONS_SERVICES_OBANAL_OBANALDB_H

//#include <sql/nspgbase/ns_pgbase.h>
#include <sql/dbi/dbiquery.h>

#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/proto/process.pb.h>

#include <commons/obanal/tfield.h>

#include <meteo/commons/proto/surface_service.pb.h>
#include <commons/meteo_data/meteo_data.h>

#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>

#include <sql/dbi/gridfs.h>

class TFieldKey;
class TFieldData;
class QDomNode;

namespace meteo {
  class GridFile;
  
  namespace prop {
    class MeteoRequest;
  } // prop
} // meteo

namespace obanal {
 //! общее описание таблицы, состоящей из колонок-дескрипторов
  struct CommonTable {
    QString table; //!< название таблицы
    QString idCol; //!< название колонки первичного ключа
  };
  //! описание таблиц
  struct TObanalTables {
    CommonTable obanal;
  };
} // obanal

class TObanalDb {
  QString basePath_;
  QString dbname_;
  meteo::GridFs gridfs_;

public:
  TObanalDb();
  ~TObanalDb();
  
  void setPath(const QString& path) { basePath_ = path + "/"; }

  bool loadForecastPunkts(meteo::field::DataRequest& arp);
  bool loadRegions(QMap<int, RegionParam>& arp);

  bool saveAccuracy(int reg_id, const QDateTime& an_dt, const QDateTime& acur_dt, int level,
                    int hour_0, int hour, int descr, int center, double kk);

  bool saveField(const obanal::TField& data, const meteo::field::DataDesc&,const QString collection = "obanal");
  // сохраняем модель машинного обучения
  bool saveMLModel(const QByteArray& data, const meteo::field::DataDesc&,const QString collection = "mlmodel",const QString mlmodel = "krls", const meteo::GeoPoint geopoint=meteo::GeoPoint());
  // загружаем модель машинного обучения из файла в базе
  bool loadlMLModel(QByteArray& data, const meteo::field::DataDesc&,const QString collection = "mlmodel",const QString mlmodel = "krls",  const meteo::GeoPoint geopoint=meteo::GeoPoint());
  bool loadAllMLModels(QList<meteo::GeoPoint> *data, QList<QString> *file_ids, const meteo::field::DataDesc&, const QString &mlmodel );

  //  сохраняем приоритеты центров
  bool saveCentersPriority( const int center, const int priority, const double priority_val, 
                                     const int forecast_count, const int forecast_true_count );
  // загружаем приоритеты из базы
  // передаем номер центра, получаем приоритеты
  bool loadCenterPriority( const int center, int& priority, double& priority_val, 
                                     int& forecast_count, int& forecast_true_count );
  // вычитаем регион из базы по координатам
  bool substractRegionFromSrc( const meteo::GeoVector& gv, const meteo::field::DataDesc descr );

  // загружаем текущие процессы / задания
  bool loadProcess( meteo::forecast::ProcessParams& processParam );
  // сохраняем рещультат выполнения процесса
  bool saveProcess( meteo::forecast::ProcessParams& processParam );
  bool readFileById( QByteArray& data, QString fileId, QString collection );
  // подключаемся к чтению файлов
  void connectToGridFs(QString collection);

  bool readFileByFileId( QByteArray& data, QString fileId );
  bool readFileByName( QByteArray& data, QString fileName, QString collection );

  bool updateQuality(const QDateTime &dt, const QStringList& dataId, const QString& descrname, control::QualityControl qual);

  bool saveBinaryToFile(const QString& basePath, const QString& fileName, const QByteArray& ba, ::meteo::GridFile* gridfile = nullptr);

private:

  /*!
   * \brief saveToFile Сохранение бинарных данных в файл
   * \param basePath - Путь к файлу
   * \param fileName - Имя файла
   * \param data     - Бинарные данные
   * \return успех сохранения в файл
   */
  bool saveToFile(const QString& basePath, const QString& fileName, const obanal::TField& data, ::meteo::GridFile* gridfile);
  
  
};

#endif // METEO_COMMONS_SERVICES_OBANAL_OBANALDB_H
