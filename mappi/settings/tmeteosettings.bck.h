#ifndef MAPPI_SETTINGS_TMETEOSETTINGS_H
#define MAPPI_SETTINGS_TMETEOSETTINGS_H

#include <qobject.h>
#include <qfilesystemwatcher.h>

#include <cross-commons/singleton/tsingleton.h>

#include <commons/funcs/tcolorgrad.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/services.pb.h>
#include <meteo/commons/rpc/rpc.h>

#include <mappi/proto/reception.pb.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/proto/thematic.pb.h>
//#include <mappi/proto/cleaner.pb.h>


template <typename K, typename V> class QMap;

namespace google {
namespace protobuf {

class Message;

} // protobuf
} // google

namespace mappi {
namespace cleaner {

class DbIntervalOption;

} // cleaner
} // meteo

class ConnectProp;

const QString kJournal = "journal";
const QString kMeteo   = "meteo";
const QString kMappi   = "mappi";

namespace meteo {

namespace settings {
class DbConnection;
class DbSettings;
class ServerAddress;
class MsgSettings;

namespace proto {
class ServiceMachines;
} //proto
} //settings

namespace mappi {

enum DbType { kDbMappi, kDbMeteo };

namespace internal {

class MeteoSettingsPrivate;

class TMeteoSettings : public QObject
{
  Q_OBJECT

public:
  bool isChanged() const;

  const QString getCurrentUserLogin() const;

  const ::mappi::conf::Reception      reception()   const;  //! Конфигурации приемника
  const ::mappi::conf::Pretreatment   satellites()  const;  //! Конфигурации спутников
  const ::mappi::conf::InstrCollect   instruments() const;  //! Конфигурации инструментов
  const ::mappi::proto::ThematicProcs thematics()   const;  //! Предустановленные тематические обработки
  //! Возвращает палитру для тематической обработки с именем thematicName.
  TColorGradList palette(const QString& thematicName) const;
  void setPalette(const QString& thematicName, const TColorGradList& grad);

  //! Конфигурация одного спутника(по имени)
  bool satellite(const QString& satelliteName, ::mappi::conf::PretrSat& satellite);
  //! Конфигурация активного инструмента на спутнике(по имени)
  bool instrument(const QString& satelliteName, ::mappi::conf::Instrument& instrument);
  //! Конфигурация инструмента (по типу)
  bool instrument(::mappi::conf::InstrumentType type, ::mappi::conf::Instrument& instrument);

  // // документ
  // //! Получение настроек для нового документа
  // const meteo::map::proto::Document docSettings() const;
  // //! Установка новых настроек для документа
  // void setDocSettings( const meteo::map::proto::Document& doc );

  // // пункт
  // //! Получение настроек для пункта
  // const Location locSettings() const;
  // //! Установка новых настроек для пункта
  // void setLocSettings( const Location& loc );

  // настройки журналирования
  //! Получение настроек для журналирования
  const settings::MsgSettings msgSettings() const;
  //! Установка новых настроек для пункта
  void setMsgSettings( const settings::MsgSettings& opt );

  // // карта
  // const meteo::map::proto::MapSettings mapSettings() const;
  // meteo::map::proto::IsolineSettings isoline( const int32_t index ) const;
  // void setIsoline( const meteo::map::proto::IsolineSettings& isoline );
  // bool removeIsoline(const int32_t index);

  // ConnectProp dbConfMeteo() const;
  // ConnectProp dbConfMappi() const;
  // ConnectProp dbConfJournal() const;
  
  // const settings::DbSettings& dbConfs() const;

  // ConnectProp dbConf(const QString &connName) const;
  // settings::DbConnection dbConn(const QString &connName) const;

  // bool setDbConf(const QString &name, const settings::DbConnection& conf);
  // bool setDbConf(const QString &name, const ConnectProp& conf);

  // bool setDbConfJournal( const settings::DbConnection &conf );
  // bool setDbConfJournal( const ConnectProp &conf );

  // параметры хранения информации в БД
  // ::mappi::cleaner::DbIntervalOption dbClearConf(const QString& dbName) const;
  // ::mappi::cleaner::DbIntervalOption dbArchiveConf(const QString& dbName) const;

  const meteo::settings::proto::ServiceMachines& services() const ;
  meteo::settings::proto::Service                service( meteo::settings::proto::ServiceCode code, bool* ok = 0 ) const ;
  // rpc::Address      serviceAddress( meteo::settings::proto::ServiceCode code, bool* ok = 0 ) const ;
  // rpc::Channel*     serviceChannel( meteo::settings::proto::ServiceCode code ) const ;
  // rpc::TController* serviceController( meteo::settings::proto::ServiceCode code ) const ;
  int               serviceTimeout( meteo::settings::proto::ServiceCode code ) const ;

  bool load();
  bool save();

  //! Сохранение настроек из памяти в файлы /etc/meteo/*.settings.conf
  bool save(const QString& name, const google::protobuf::Message* message );
  //! Загрузка настроек в память из файлов /etc/meteo/*.settings.conf
  bool load( const QString& name, google::protobuf::Message* message );
  //! Обновление сериализованного содержимого настроек, при загрузке и сохранении
  void updateSaveState();

  bool saveDbSettings();
  bool loadDbSettings();

signals:
  void dbSettingsChanged();

private slots:
  void slotFileChanged( const QString& filepath);

private:
  explicit TMeteoSettings(QObject* parent = 0);
  virtual ~TMeteoSettings();

private:
  MeteoSettingsPrivate* pimpl_;
  QFileSystemWatcher _settingswatcher;

  friend class TSingleton<TMeteoSettings>;
};

} //internal

typedef TSingleton<internal::TMeteoSettings> TMeteoSettings;

} // mappi
} // meteo

#endif
