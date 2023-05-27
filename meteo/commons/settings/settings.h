#ifndef METEO_COMMONS_SETTINGS_SETTINGS_H
#define METEO_COMMONS_SETTINGS_SETTINGS_H

#include <qmap.h>
#include <qdir.h>
#include <qlist.h>

#include <meteo/commons/proto/services.pb.h>
#include <sql/proto/dbsettings.pb.h>
#include <meteo/commons/proto/locsettings.pb.h>
#include <meteo/commons/proto/msgstream.pb.h>
#include <meteo/commons/proto/storm.pb.h>
#include <meteo/commons/proto/pogodainput.pb.h>
#include <meteo/commons/proto/climat.pb.h>
#include <meteo/commons/proto/msgsettings.pb.h>
#include <meteo/commons/proto/document_service.pb.h>
#include <meteo/commons/rpc/rpc.h>
#include <cross-commons/singleton/tsingleton.h>
#include <meteo/commons/proto/appconf.pb.h>
#include <cross-commons/debug/tlog.h>

namespace meteo {

//! Класс ConfigData содержит описание конфигурацианных файлов и указатель на прото-структуру, в которую они
//! будут загружены.
class ConfigData
{
public:
  //! mask - шаблон имени файла
  //! path - путь к файлу
  //! storage - указатель на прото-структуру
  //! descr - описание
  ConfigData(const QString& mask = QString(),
             const QString& path = QString(),
             google::protobuf::Message* storage = 0,
             const QString& descr = QString::fromUtf8("< без имени >"))
    : mask_(mask), path_(path), description_(descr), conf_(storage) {}

  QString filePath() const { return path_ + "/" + mask_; }

  const QString& mask() const { return mask_; }
  const QString& path() const { return path_; }
  const QString& description() const { return description_; }
  google::protobuf::Message* proto() const { return conf_; }

public:
  QString mask_;
  QString path_;
  QString description_;
  google::protobuf::Message* conf_ = nullptr;
};


//!
class SettingsFile
{
public:
  SettingsFile(const QString& fileName)
  {
    init(fileName);
  }

  inline const QString& fileName() const { return name_; }
  inline const QString& filePath() const { return path_; }

  bool write(const QByteArray& data) const
  {
    QFile file(path_ + QDir::separator() + name_);
    if ( !file.exists() ) {
      QDir dir;
      if ( !dir.mkpath(path_) ) {
        error_msg << QObject::tr("Ошибка при создании пути для сохранения файла настроек %1").arg(file.fileName());
        return false;
      }
    }

    if ( !file.open(QFile::WriteOnly) ) {
      error_msg << QObject::tr("Не удалось открыть файл настроек %1").arg(file.fileName());      
      return false;
    }

    if ( file.write(data) != data.size() ) {
      error_msg << QObject::tr("Ошибка при записи настроек на диск (%1)").arg(file.fileName());      
      return false;
    }
    file.flush();

    return true;
  }

private:
  QString init(const QString& fileName)
  {
    QString p = QDir::cleanPath(fileName);
    p = QDir::toNativeSeparators(p);
    QStringList list = p.split(QDir::separator());
    if ( list.size() > 1 ) {
      name_ = list.takeLast();
      path_ = list.join(QDir::separator());
    }
    else {
      name_ = p;
    }

    return fileName;
  }

private:
  QString name_;
  QString path_;
};


class Settings;


Settings* gSettings(Settings* settings = nullptr);


class Settings
{
  public:
    explicit Settings();
    virtual ~Settings();



    bool load();
    bool save();

    // параметры пункта
    const settings::Location& location() const { return location_; }
    QString wmoId() const                      { return QString::fromStdString(location_.wmo_id()); }
    QString hmsId() const                      { return QString::fromStdString(location_.hms_id()); }
    QString name()  const                      { return QString::fromStdString(location_.name()); }
    QString index()  const                     { return QString::fromStdString(location_.index()); }
    settings::NodeType stationType()  const    { return location_.station_type(); }
    bool setHmsId(const QString& hmsId );
    bool setIndex(const QString& index );
    bool saveLocation( const settings::Location& location );

    // параметры штормового оповещения
    const QString stormSettingsFileName() const;
    const StormSettings& storm() const { return storm_; }

    // параметры аэродромов пользователя, разрешенных для ввода погоды
    bool pogodainput( const QString& login, PogodaInputSts* userdroms ) const ;

    //параметры климатического сервиса
    const climat::ClimatSaveProp& climat() const { return climat_; }

    // msgstream
    msgstream::Options msgstreamConf(const QString& id) const;
    void setMsgstreamConf(const msgstream::Options& conf);

    // базы данных
    static const QString& dbConfPath();
    static const QString& dbConfMask();
    const settings::DbSettings& dbConfs() const { return dbs_; }
    settings::DbConnection dbConn(const QString &connName) const;

    // сервисы
    const meteo::settings::proto::ServiceMachines& services() const { return services_; }
    meteo::settings::proto::Service service( meteo::settings::proto::ServiceCode code, bool* ok = 0 ) const;
    QString serviceAddress( meteo::settings::proto::ServiceCode code, bool* ok = 0 ) const;
    int serviceTimeout( meteo::settings::proto::ServiceCode code ) const;

    meteo::map::proto::DocServiceParam docserviceparams() const { return docserviceparams_; }

    // параметры отображения системных сообщений
    const QString& displayMessageConfigName();
    const settings::MsgSettings& displayMessageConfig();

    const msgcenter::RuleTable& route() const { return centerRoutes_; }
    QStringList ttaa() const;

  protected:
    //! Добавляет в список параметры для загрузки настроек.
    //! \param fileMask шаблон имени файла
    //! \param path     путь к директории с файлами настроек
    //! \param msg      указатеть на структуру, в которую будут загружены настройки
    //! \param descr    описание (используется для журналирования и при выводе сообщений пользователю)
    void appendConf(const QString& fileMask, const QString& path, google::protobuf::Message* msg, const QString& descr);
    void appendConf(const ConfigData& conf);

  private:
    bool load(const ConfigData& config);
    bool save(const QString& fileName, google::protobuf::Message* message);

    bool loadPogodaConfig( const QString& login, PogodaInputSts* conf ) const ;

  private:
    msgstream::MsgStreamSettings streams_;
    StormSettings storm_;
    climat::ClimatSaveProp climat_;

    settings::Location location_;
    settings::DbSettings dbs_;
    settings::proto::ServiceMachines services_;
    meteo::settings::MsgSettings msgDisplaySettings_;

    meteo::map::proto::DocServiceParam docserviceparams_;
    msgcenter::RuleTable centerRoutes_;

    QList<ConfigData> confList_;

  friend class TSingleton<Settings>;
};

namespace global {
  using Settings = TSingleton<meteo::Settings>;
}

}




#endif
