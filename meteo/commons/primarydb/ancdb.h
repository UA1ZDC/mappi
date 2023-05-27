#ifndef METEO_COMMONS_PRIMARYDB_ANCDB_H
#define METEO_COMMONS_PRIMARYDB_ANCDB_H

#include <meteo/commons/primarydb/dbmeta.h>
#include <meteo/commons/services/decoders/tservicestat.h>
#include <meteo/commons/services/decoders/meteo_convert.h>
#include <meteo/commons/proto/alphanum.pb.h>
#include <sql/psql/tpgsettings.h>
#include <sql/dbi/dbientry.h>
#include <sql/dbi/dbiquery.h>

#include <qstring.h>
class ComplexMeteo;

namespace meteo {
  class GridFs;
  class GridFile;
  class AncDb {
  public:
    AncDb();
    AncDb( const ConnectProp& params );
    ~AncDb();

    //! Источник сохраняемой информации
    enum SourceType {
      kANCSource  = 0, //!< Буквенно-цифровые
      kBufrSource = 1, //!< BUFR
      kGphSource  = 2, //!< Геофизические карты
    };

    //! Состояние
    enum StatType {
      kSaveOk   = 0, //!< Успешное сохранение
      kSaveErr  = 1, //!< Ошибка сохранения
      kRepeated = 2, //!< Повторная сводка
      kUpdated  = 3, //!< Обновленная сводка
      kStatCnt  = 4
    };


    void init( const ConnectProp& params );

    void setSaveDubl(bool fl) { _prop.set_dubl(fl); }

    bool fillStationInfo(TMeteoData* data, int category, StationInfo* info);

    bool loadEsimo() {  return meta_->loadEsimo(); }
    descr_t esimoDescr(const QString& name) { return meta_->esimoDescr(name); }

    /*!
     * \brief saveReport - сохранение TMeteoData (один параметр = одна запись)
     * \param md         - корневая TMeteoData
     * \param category   - категория данных
     * \param dt         - время из телеграммы
     * \param stationType- тип станции
     * \param collection - название коллекции
     * \return           - результат работы
     */
    bool saveReport(const TMeteoData &md, int category, const QDateTime& dt,
                    const StationInfo& info, const QString &collection);
    bool saveRadarMap(const TMeteoData* data, int category, const QDateTime& dt,
                      const QString& collection);
    bool saveSigWx(const meteo::sigwx::Header& header, const QByteArray& ba, int category,
                   const QDateTime& dt, const QString& collection);
    bool updateMeteoData(const TMeteoData &data, int category, const QDateTime& dt,
                         const StationInfo& info, const QString &collection);

    void setConnectionStatus(meteo::app::OperationStatus* status);

    const QString& ptkppId() { return _idPtkpp; }
    void setPtkppId(const QString& id) { _idPtkpp = id; }
    void setTlgii(int ii) { _iiTlg = ii; }
    void setCorNumber( int cor_num ) { _cor_number = cor_num; }
    void setMeteoSource(const QString& path, SourceType type) { _source = path; _sourceType = type; }
    anc::DbMeta* dbmeta() { return meta_; }

    const service::Status<SourceType, StatType>& status() const { return _stat; }
    bool saveClimatIsd(const QStringList& cm, const QString& collection, int* maxsize);

  protected:
    virtual bool saveReportData( const GeoPoint& point,const QMap<QString, QString>& header, const QString& jinfo,
                          const ComplexMeteo& md, const QString& collection, bool* notEmptyFlag);
    virtual bool saveGeophysData(const GeoPoint& point,const QMap<QString, QString>& header, const QString& jinfo,
         const ComplexMeteo& md, const QString& collection, bool* notEmptyFlag);


    QString createParentUpsertedParams(const ComplexMeteo& cm, const QMap<QString, QString>& header, const QString& uuid);
    QString createUpsertedParams(const ComplexMeteo& cm, const QMap<QString, QString>& header, const QString& uuid);
    QString createParamJson(const ComplexMeteo& cm, const QMap<QString, QString>& header, const QString& name,
                            descr_t descr, const TMeteoParam& param, const QString& uuid );
    bool fillPrevParams(const DbiEntry &doc, QMap<QString, Document>* paramPrev,
                        QString* oid, bool* upserted);
    bool updateMeteoData(const GeoPoint& point, const QMap<QString, QString>& header, const QString& jinfo,
                         const ComplexMeteo& cm, const QString& collection);
    QString createUpdFindAndModifyJson(const ComplexMeteo& cm, const QMap<QString, QString>& header,
                                       const GeoPoint& point, const QString& uuid, const QString& jinfo,
                                       const QString& collection);
    QString createParamsJson(const ComplexMeteo& cm,
                        const QMap<QString, QString>& header,
                        const QString& uuid, QMap<QString, Document>& paramPrev,
                        const QString& oid, const QString& collection);
    QString createDublParamJson(const ComplexMeteo& /*cm*/, const QMap<QString, QString>& header,
                                     const QString& name, descr_t descr,
                                     const TMeteoParam& param, const QString& uuid, const QString& oid);
    QString updateParamJson(const ComplexMeteo& /*cm*/, const QMap<QString, QString>& header,
                                 const QString& name, descr_t descr, const TMeteoParam& param,
                                 const QString& uuid, Document& paramPrev, const QString& oid);

    QString createGeophysReportQuery(const ComplexMeteo& cm,
             const QMap<QString, QString>& header,
             const GeoPoint& point, const QString& jinfo,
             const QString& collection);
    QString createGeophysChildUpsertedParams(const ComplexMeteo& cm, const QMap<QString, QString>& header, const QString& uuid);
    QString createGeophysUpsertedParams(const ComplexMeteo& cm, const QMap<QString, QString>& header, const QString& uuid);
    QString createGeophysSimpleUpsertedParams(const ComplexMeteo& cm, const QMap<QString, QString>& header, const QString& uuid);
  private:
    bool saveToDb(const QString& collection, const QString& fileName,
                  const QByteArray& msg, GridFile* gridfile);

    void updateStationHistory(const QString&, int, int, const QDateTime & );
   // virtual Document getExecResult();

    bool checkDb();
    bool execQuery(DbiQuery *query);

    bool paramsEqual(const Document& prev,
                     const TMeteoParam& cur, const ComplexMeteo& cm);

    QString updateParamsJson(const QString& updparam,
                                     const QString& collection);

    QString createStationInfo(const StationInfo& info);
    QString createMeteoReportQuery(const ComplexMeteo& cm,
                                           const QMap<QString, QString>& header,
                                    const GeoPoint& point, const QString& jinfo,
                                           const QString& collection);
    QString createUpdateParamsJson(const ComplexMeteo& cm,
                                           const QMap<QString, QString>& header,
                                   QMap<QString, Document>& paramPrev,
                                   const QString& oid, const QString& collection);
    QString setParamJson(const ComplexMeteo& cm, const QMap<QString, QString>& header,
                     const QString& name, const TMeteoParam& param,
                     const QString& uuid, const QString& oid);

    bool saveClimatIsd(const QStringList& reports, int first, int size, const QString& collection);
    QString createIsdReports(const QMap<QString, QString>& header,
                             const ComplexMeteo& cm);
    QString createParamIsd(const QMap<QString, QString>& header, const ComplexMeteo& cm, const QString& name,
                           descr_t descr, const TMeteoParam& param, const QString& uuid);

    void fillCreateUpdateParamsJson( const ComplexMeteo& cmeteo, const QMap<QString,
                                     QString>& header,const QMap<QString, Document>& paramPrev,
                                     const QString& uuid, QString* addparam, QStringList* pullparam );
    QString createReportParams(const QMap<QString, QString>& header,
                               const ComplexMeteo& cm);
    protected:
    ConnectProp params_;
    bool _defaultDb = true;     //!< false - заданные параметры подключения, а не из global

    int           _iiTlg;       //!< Уникальный номер телеграммы для одной станции, одной сводки (ii в заголовке телеграммы)
    QString       _idPtkpp;     //!< id в БД ПТКПП сообщения (для отладки)
    int           _cor_number;  //!< порядковый номер телеграммы-исправления (bbb=CCA - исправление №1, bbb=CCD - исправление №4 и т.д.)
    //    QString       _basePath = QString(); //!< Базовый путь для сохранения файлов
    QString       _source;      //!< bufr или alphanum (для отладки)
    SourceType _sourceType; //!< bufr или alphanum (для состояния)

    service::Status<SourceType, StatType> _stat;

    ::meteo::anc::AncSaveProp _prop;

    anc::DbMeta * meta_ = nullptr;
    Dbi* db_ = nullptr;
    GridFs* gridfs_ = nullptr;

    //uint _cnt = 0; //!< счетчик для создания имени файла
  };

  AncDb* ancdb();

}

#endif

