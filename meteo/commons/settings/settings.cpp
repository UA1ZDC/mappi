#include "settings.h"
#include "tusersettings.h"

#include <qdir.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>
#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <sql/psql/psql.h>
#include <sql/psql/tpgsettings.h>
#include <sql/psql/psqlquery.h>
#include <commons/funcs/fs.h>
#include <meteo/commons/global/log.h>
#include <meteo/commons/global/global.h>


namespace meteo {

Settings::Settings()
{
  appendConf("*.conf", MnCommon::etcPath() + "/services.conf.d", &services_, QObject::tr("параметры подключения к сервисам"));
  appendConf(dbConfMask(), dbConfPath(), &dbs_, QObject::tr("параметры подключения к БД"));
  appendConf("msgstream.conf", MnCommon::etcPath(), &streams_, QObject::tr("параметры приёма (передачи) телеграмм"));
  appendConf("location.conf", MnCommon::etcPath(), &location_, QObject::tr("параметры пункта"));
  appendConf(stormSettingsFileName(), MnCommon::etcPath(), &storm_, QObject::tr("параметры штормового оповещения"));  
  appendConf("climat.conf", MnCommon::etcPath(), &climat_, QObject::tr("параметры климатического сервиса"));
  appendConf(displayMessageConfigName(), MnCommon::etcPath(), &msgDisplaySettings_, QObject::tr("параметры отображения системных сообщений"));
//  appendConf("nearCenters.conf", MnCommon::etcPath(), &connectionSettings_, QObject::tr("Список центров для подключения"));
  appendConf("docservice.conf", MnCommon::etcPath(), &docserviceparams_, QObject::tr("Параметры службы формирования документов"));
  appendConf("*.conf", MnCommon::etcPath()+"/route.conf.d", &centerRoutes_, QObject::tr("параметры маршрутизации телеграмм"));
}

Settings::~Settings()
{}

const QString Settings::stormSettingsFileName() const
{
  return QObject::tr("storm.conf");
}

const QString& Settings::displayMessageConfigName()
{
  static const QString& configName = QObject::tr("msg.display.conf");
  return configName;
}

const settings::MsgSettings& Settings::displayMessageConfig()
{
  return this->msgDisplaySettings_;
}

const QString& Settings::dbConfPath()
{
  static const QString& confPath = QObject::tr("%1/db.conf.d").arg(MnCommon::etcPath());
  return confPath;
}

const QString& Settings::dbConfMask()
{
  static const QString& confMask = QObject::tr("*.conf");
  return confMask;
}

bool Settings::setIndex(const QString& index)
{
  if ( index.isEmpty() ) {
    return false;
  }
  location_.set_index(index.toStdString());
  return saveLocation(location_);
}


bool Settings::setHmsId(const QString& hmsId)
{
  if ( hmsId.isEmpty() ) {
    return false;
  }
  location_.set_hms_id(hmsId.toStdString());
  return saveLocation(location_);
}


bool Settings::saveLocation(const settings::Location &location)
{
  QFile ofile(MnCommon::etcPath()+"/location.conf");
  if ( false == ofile.open(QIODevice::WriteOnly)){
    error_log << meteo::msglog::kFileWriteFailed
                              .arg(MnCommon::etcPath()+"/nearCenters.conf")
                              .arg(ofile.errorString());
    return false;
  }

  auto data = TProtoText::toText(location);
  auto written = ofile.write(data);
  if ( written != data.size() ){
    error_log << QObject::tr("Ошибка при записи файла. Ожидалось, что будет записано %1 символов, записано %2 символов")
                 .arg(data.size())
                 .arg(written);
    return false;
  }
  location_.CopyFrom(location);
  return true;
}

msgstream::Options Settings::msgstreamConf(const QString& id) const
{
  const std::string sid = id.toStdString();
  for ( int i = 0, isz = streams_.msgstream_settings_size(); i < isz; ++i ) {
    if ( sid == streams_.msgstream_settings(i).id() ) {
      return streams_.msgstream_settings(i);
    }
  }
  error_log << QObject::tr("Параметры для потока с идентификаторм %1 не найдены.").arg(id);
  return msgstream::Options();
}

void Settings::setMsgstreamConf(const msgstream::Options& conf)
{
  for ( int i = 0, isz = streams_.msgstream_settings_size(); i < isz; ++i ) {
    if ( streams_.msgstream_settings(i).id() == conf.id() ) {
      streams_.mutable_msgstream_settings(i)->CopyFrom(conf);
      break;
    }
  }
}

settings::DbConnection Settings::dbConn(const QString& connName) const
{
  settings::DbConnection conn;
  for ( int i = 0, sz = dbs_.db_connection_size(); i < sz; i++ ) {
    QString currentConnectionName = QString::fromStdString(dbs_.db_connection(i).conn_name());

    if ( 0 == currentConnectionName.compare(connName) ) {
      conn.CopyFrom(dbs_.db_connection(i));
      if ( false == dbs_.db_connection(i).has_dbdriver() ) {
        conn.set_dbdriver( dbs_.dbdriver() );
      }
      return conn;
    }

  }
  return conn;
}

settings::proto::Service Settings::service(settings::proto::ServiceCode code, bool* ok) const
{
  QString serviceName = pbtools::toQString(settings::proto::ServiceCode_Name(code)).remove(0,1).toLower();

  if ( 0 != ok ) {
    *ok = false;
  }
  settings::proto::Service serv;
  for ( int i = 0, isz = services_.machine_size(); i < isz; ++i ) {
    const settings::proto::ServiceMachine& machine  = services_.machine(i);
    if ( false == machine.has_host() ) {
      warning_log << QObject::tr("В настройках подключения к сервисам есть параметры ЭВМ без адреса");
      continue;
    }
    for ( int j = 0, jsz = machine.serv_size(); j < jsz; ++j ) {
      const settings::proto::Service& srv = machine.serv(j);
      if ( srv.code() != code ) {
        continue;
      }
      if ( 0 != ok ) {
        *ok = true;
      }
      serv.CopyFrom(srv);
      if ( false == serv.has_host() ) {
        serv.set_host( machine.host() );
      }
      return serv;
    }
  }
  error_log << QObject::tr("Параметры подключения к сервису %1 не найдены").arg(serviceName);

  return serv;
}

QString Settings::serviceAddress(settings::proto::ServiceCode code, bool* ok) const
{
  meteo::settings::proto::Service serv = service( code, ok );
  if ( 0 != ok && false == *ok ) {
    return QString();
  }

  if ( services_.machine_size() == 0 ) { return QString(); }

  if ( !serv.has_host() ) {
    return QString("%1:%2").arg(QString::fromStdString(services_.machine(0).host())).arg(serv.port());
  }

  return QString("%1:%2").arg(QString::fromStdString(serv.host())).arg(serv.port());
}

int Settings::serviceTimeout(settings::proto::ServiceCode code) const
{
  bool ok;
  meteo::settings::proto::Service serv = service( code, &ok );
  if ( false == ok ) {
    return -1;
  }

  if ( services_.machine_size() == 0 ) { return -1; }

  return services_.machine(0).default_timeout();
}

void Settings::appendConf(const QString& fileMask, const QString& path, google::protobuf::Message* msg, const QString& descr)
{
  confList_.append( ConfigData(fileMask,path,msg,descr) );
}

void Settings::appendConf(const ConfigData& conf)
{
  confList_.append(conf);
}

bool Settings::load()
{
  foreach ( const ConfigData& conf, confList_ ) {
    if (nullptr != conf.proto()) {
      conf.proto()->Clear();
    }
  }
  foreach ( const ConfigData& conf, confList_ ) {
    if ( !load(conf) ) {
      continue;
    }
  }

  return true;
}

bool Settings::save()
{
  bool status = save(MnCommon::etcPath() + "/msgstream.conf", &streams_);
  return status;
}

bool Settings::save(const QString& fileName, google::protobuf::Message* message)
{
  QString data = TProtoText::toText(*message);

  bool status = false;
  if ( false == data.isEmpty() ) {
    SettingsFile sf(fileName);
    status = sf.write( data.toUtf8() );
    if(true == status){
        info_log<<QObject::tr("Настройки сохранены успешно (%1)").arg(fileName);
      }
  }
  else {
    error_log << QString("Получены пустые значения настройки %1").arg( fileName );
  }

  return status;
}

bool Settings::load(const ConfigData& config)
{
  google::protobuf::Message* message = config.proto();
  if ( 0 == message ) { return false; }

  QString text;

  google::protobuf::Message* tmp = message->New();

  QStringList paths = dirList({ config.path() });
  paths << config.path();
  for ( const QString& path : paths ) {
    for ( const QString& f : QDir(path).entryList({ config.mask() }, QDir::Files) ) {
      QFile file( QDir(path).absoluteFilePath(f));
      if ( false == file.exists() ) {
        continue;
      }
      if ( true == file.open(QIODevice::ReadOnly) ) {
        text = QString::fromUtf8(file.readAll());
      }
      else {
        warning_log_if( file.exists() ) << QString::fromUtf8("Не удалось открыть файл настроек %1").arg(file.fileName());
      }

      if ( TProtoText::fillProto(text,tmp) ) {
        message->MergeFrom(*tmp);
      }
    }
  }

  delete tmp;

  return true;
}

bool Settings::loadPogodaConfig( const QString& login, PogodaInputSts* conf ) const
{
  auto ch = std::unique_ptr<meteo::rpc::Channel>(global::serviceChannel(meteo::settings::proto::ServiceCode::kDiagnostic));
  if ( nullptr == ch ){
    error_log.msgBox() << meteo::msglog::kServiceConnectFailed;
    return false;
  }

  meteo::app::UserPogodaInputStationsLoadRequest req;
  req.add_login( login.toStdString() );

  auto resp = std::unique_ptr<meteo::app::PogodaInputStationsLoadResponce>(ch->remoteCall(&app::ControlService::UserPogodaInputStsLoad, req, 10000));
  if ( nullptr == resp ){
    error_log.msgBox() << meteo::msglog::kServiceRequestFailed;
    return false;
  }

  if ( false == resp->result() ){
    error_log.msgBox() << QObject::tr("Запрос к сервису выполнен успешно, на стороне сервиса произошла ошибка.");
    return false;
  }

  if ( resp->data_size()>0 ){
    for ( int i = 0, isz = resp->data_size(); i < isz; ++i ) {
      if ( resp->data(i).has_param() ){
        error_log<<"param has!";
        error_log<<"addres size "<<resp->data(i).param().enabled_inputs().address_size();
        
        for ( int j = 0, iszz = resp->data(i).param().enabled_inputs().address_size(); j < iszz; ++j ) {
            error_log << resp->data(i).param().enabled_inputs().address(j);
            *conf = resp->data(i).param();
            return true;
        }
      }
    }
  }
  return false;
}

bool Settings::pogodainput( const QString& login, PogodaInputSts* userdroms ) const
{
  if ( nullptr == userdroms ) {
    error_log << QObject::tr("Нулевой указатель 'userdroms'");
    return false;
  }
  return loadPogodaConfig( login, userdroms);
}

QStringList Settings::ttaa() const
{
  QStringList list;
  for ( int i = 0, isz = location().ttaa_size(); i < isz; ++i ) {
    list << QString::fromStdString(location().ttaa(i));
  }
  return list;
}

Settings *gSettings(Settings *settings)
{
  static Settings* obj = nullptr;

  if ( nullptr == settings ) {
    return obj;
  }
  obj = settings;
  return obj;
}

template<> ::meteo::Settings* meteo::global::Settings::_instance = 0;


}
