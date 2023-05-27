#include "tmeteosettings.h"

#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <commons/geobasis/geopoint.h>
#include <commons/textproto/tprototext.h>

// #include <sql/nspgbase/ns_pgbase.h>
// #include <sql/nspgbase/tsqlquery.h>
// #include <sql/nspgbase/tpgsettings.h>

// #include <meteo/commons/proto/dbsettings.pb.h>
#include <meteo/commons/proto/msgsettings.pb.h>
// #include <meteo/commons/rpc/taddress.h>
#include <meteo/commons/xmlproto/txmlproto.h>

template<> meteo::mappi::internal::TMeteoSettings* meteo::mappi::TMeteoSettings::_instance = 0;

namespace meteo {
namespace mappi {
namespace internal {

class MeteoSettingsPrivate
{
private:
  //! Настройки, соответствующие отдельным файлам настроек /etc/meteo/*.settings.conf
  // meteo::settings::DbSettings dbconf_;
  meteo::settings::proto::ServiceMachines servicesConf_; //!< параметры подключения к сервисам

  //!< Настройки журналирования
  settings::MsgSettings msgconf_;

  //! Настройки очистки и архивации
  //  ::mappi::cleaner::CleanerSettings cleanerConf_;

  ::mappi::conf::Reception      reception_;    //!< Конфигурация приймника
  ::mappi::conf::Pretreatment   satellites_;   //!< Параметры спутников
  ::mappi::conf::InstrCollect   instruments_;  //!< Параметры инструментов
  ::mappi::proto::ThematicProcs thematics_;    //!< Предустановленные тематические обработки

  ::mappi::proto::ThematicPalettes palettes_;


  //! Сериализованное при загрузке содержимое настроек
  QByteArray state_;

  //! Контейнер  хранения отдельных настроек и соответствующих им файлов
  QMap<QString, google::protobuf::Message*> msg_;

  QString _curLogin;

  QMap<int,QString> _ranks;

public:
  MeteoSettingsPrivate()
  {}

  bool isChanged() const
  {
    if (state_.isEmpty()) {
      return true;
    }
    std::string str;
    QByteArray data;
    google::protobuf::Message* msg;
    foreach( msg, msg_ ){
        msg->SerializeToString(&str);
        data += QByteArray(str.c_str(), str.length());
    }
    if( data !=  state_ ){
        return true;
    }
    return false;
  }

  const QString getCurrentUserLogin() const
  {
    struct passwd* pw;
    uid_t uid;
    uid = getuid();
    pw = getpwuid (uid);
    if (pw != NULL) {
       char* name = pw->pw_name;
       return QString::fromUtf8(name);
    }
    return QString();
  }


  const ::mappi::conf::Reception      reception()   const { return reception_;   }
  const ::mappi::conf::Pretreatment   satellites()  const { return satellites_;  }
  const ::mappi::conf::InstrCollect   instruments() const { return instruments_; }
  const ::mappi::proto::ThematicProcs thematics()   const { return thematics_;   }

  TColorGradList palette(const QString& thematicName) const
  {
    std::string name = thematicName.toStdString();

    TColorGradList grad;

    for ( int i = 0, isz = palettes_.palette_size(); i < isz; ++i ) {
      const ::mappi::proto::ThematicPalette& p = palettes_.palette(i);
      if ( p.name() != name ) {
        continue;
      }

      for ( int j = 0, jsz = p.color_size(); j < jsz; ++j ) {
        const ::meteo::map::proto::ColorGrad& c = p.color(j);
        double min = c.min_value();
        double max = min;
        if ( c.has_max_value() ) {
          max = c.max_value();
        }
        QColor ncolor = QColor::fromRgba( c.min_color() );
        QColor xcolor = ncolor;
        if ( c.has_max_color() ) {
          xcolor = QColor::fromRgba( c.max_color() );
        }

        grad.append(TColorGrad(min, max, ncolor, xcolor ));
      }

      break;
    }

    return grad;
  }

  void setPalette(const QString& thematicName, const TColorGradList& grad)
  {
    std::string name = thematicName.toStdString();

    int idx = -1;
    for ( int i = 0, isz = palettes_.palette_size(); i < isz; ++i ) {
      const ::mappi::proto::ThematicPalette& p = palettes_.palette(i);
      if ( p.name() == name ) {
        idx = i;
        break;
      }
    }

    ::mappi::proto::ThematicPalette* p = nullptr;
    if ( -1 == idx ) {
      p = palettes_.add_palette();
      p->set_name(name);
    }
    else {
      p = palettes_.mutable_palette(idx);
      p->clear_color();
    }

    for ( int i = 0, isz = grad.size(); i < isz; ++i ) {
      const TColorGrad& cg = grad.at(i);
      ::meteo::map::proto::ColorGrad* c = p->add_color();
      c->set_min_value(cg.begval());
      if ( !cg.oneval() ) {
        c->set_max_value(cg.endval());
      }
      c->set_min_color(cg.begcolor().rgba());
      if ( !cg.onecolor() ) {
        c->set_max_color(cg.endcolor().rgba());
      }
    }
  }

  // ::mappi::cleaner::DbIntervalOption dbClearConf(const QString &dbName) const
  // {
  //   std::string db = dbName.toUtf8().constData();
  //   for ( int i=0,isz=cleanerConf_.clear_interval_size(); i<isz; ++i ) {
  //     if ( cleanerConf_.clear_interval(i).connection_name() == db ) {
  //       return cleanerConf_.clear_interval(i);
  //     }
  //   }

  //   debug_log << QObject::tr("Параметры очистки для '%1' не найдены").arg(dbName);

  //   return ::mappi::cleaner::DbIntervalOption();
  // }

  // ::mappi::cleaner::DbIntervalOption dbArchiveConf(const QString& dbName) const
  // {
  //   std::string db = dbName.toUtf8().constData();
  //   for ( int i=0,isz=cleanerConf_.archive_interval_size(); i<isz; ++i ) {
  //     if ( cleanerConf_.archive_interval(i).connection_name() == db ) {
  //       return cleanerConf_.archive_interval(i);
  //     }
  //   }

  //   debug_log << QObject::tr("Параметры архивации для '%1' не найдены").arg(dbName);

  //   return ::mappi::cleaner::DbIntervalOption();
  // }


  bool satellite(const QString& satelliteName, ::mappi::conf::PretrSat& satellite)
  {
    bool result = false;
    for(int i = 0; i < satellites_.satellite_size(); ++i)
    {
      if(   satellites_.satellite(i).has_name()
         && QString::fromStdString(satellites_.satellite(i).name()) == satelliteName)
      {
        satellite.CopyFrom(satellites_.satellite(i));
        result = true;
        break;
      }
    }
    return result;
  }

  bool instrument(const QString& satelliteName, ::mappi::conf::Instrument& instrument)
  {
    ::mappi::conf::PretrSat sat;
    if(false == satellite(satelliteName, sat))
      return false;

    bool result = false;

    for(int i = 0; i < sat.instr_size(); ++i) {
      if(sat.instr(i).has_realtime() && true == sat.instr(i).realtime()) {
        if(true == sat.instr(i).has_type()) {
          for(int j = 0; j < instruments_.instrs_size(); ++j) {
            if(true == instruments_.instrs(j).has_type() && sat.instr(i).type() == instruments_.instrs(j).type()) {
              instrument.CopyFrom(instruments_.instrs(j));
              result = true;
              break;
            } //instrument.has_type && type == type
          } //for(instrument)
        } //sat.instr.has_type
      } //sat.instr.has_realtime and realtime == true
    } //for(sat.instr)
    return result;
  }

  bool instrument (::mappi::conf::InstrumentType type, ::mappi::conf::Instrument& instrument)
  {
    for ( int i = 0, sz = instruments_.instrs_size(); i < sz; ++i ) {
      if (true == instruments_.instrs(i).has_type() && instruments_.instrs(i).type() == type) {
        instrument.CopyFrom(instruments_.instrs(i));
        return true;
      }
    }
    return false;
  }


  const settings::MsgSettings msgSettings() const
  {
    return msgconf_;
  }

  void setMsgSettings( const settings::MsgSettings& opt )
  {
    msgconf_.CopyFrom(opt);
  }

  // ConnectProp dbConfJournal() const
  // {
  //   return dbConf(kJournal);
  // }
  
  // const settings::DbSettings& dbConfs() const
  // {
  //   return dbconf_;
  // }

  // ConnectProp dbConf(const QString &connName) const
  // {
  //   for (int i = 0, sz = dbconf_.db_connection_size(); i < sz; i++ ){
  //     if ( dbconf_.db_connection(i).conn_name() == connName.toStdString() ){
  //       // if (connName == ADMIN)
  //       // {
  //       //   return ConnectProp( QObject::tr(dbconf_.db_connection(i).host().c_str()),
  //       //                       QObject::tr(dbconf_.db_connection(i).name().c_str()),
  //       //                       getAdminLogin(),
  //       //                       QString::null,
  //       //                       QString::number(dbconf_.db_connection(i).port()));
  //       // }
  //       return ConnectProp( QObject::tr(dbconf_.db_connection(i).host().c_str()),
  //                           QObject::tr(dbconf_.db_connection(i).name().c_str()),
  //                           QString("postgres"),
  //                           QString::null,
  //                           QString::number(dbconf_.db_connection(i).port()));
  //     }
  //   }
  //   return ConnectProp();
  // }

  // settings::DbConnection dbConn(const QString &connName) const
  // {
  //   settings::DbConnection conn;
  //   for (int i = 0, sz = dbconf_.db_connection_size(); i < sz; i++ ){
  //       if ( dbconf_.db_connection(i).conn_name() == connName.toStdString() ){
  //         conn.CopyFrom(dbconf_.db_connection(i));
  //         return conn;
  //       }
  //   }
  //   return conn;
  // }

  // bool setDbConf(const QString &name, const settings::DbConnection& conf)
  // {
  //   for (int i = 0, sz = dbconf_.db_connection_size(); i < sz; i++ ){
  //     if ( dbconf_.db_connection(i).conn_name() == name.toStdString() ){
  //       dbconf_.mutable_db_connection(i)->CopyFrom(conf);
  //     }
  //   }
  //   return true;
  // }

  // bool setDbConf(const QString &name, const ConnectProp& conf)
  // {
  //   bool status = false;
  //   for (int i = 0, sz = dbconf_.db_connection_size(); i < sz; i++ ){
  //     if ( dbconf_.db_connection(i).conn_name() == name.toStdString() ){
  //       dbconf_.mutable_db_connection(i)->set_name(conf.name().toStdString());
  //       dbconf_.mutable_db_connection(i)->set_host(conf.host().toStdString());
  //       dbconf_.mutable_db_connection(i)->set_port(conf.port().toInt());
  //       status = true;
  //     }
  //   }
  //   return status;
  // }

  // bool setDbConfJournal( const settings::DbConnection &conf )
  // {
  //   return setDbConf(kJournal,conf);
  // }

  // bool setDbConfJournal( const ConnectProp &conf )
  // {
  //   return setDbConf(kJournal,conf);
  // }
  
  const ::meteo::settings::proto::ServiceMachines& services() const
  {
    return servicesConf_;
  }

  meteo::settings::proto::Service service( meteo::settings::proto::ServiceCode code, bool* ok ) const
  {
    QString serviceName = QString::fromStdString(settings::proto::ServiceCode_Name(code)).remove(0,1).toLower();

    if ( 0 != ok ) {
      *ok = false;
    }
    settings::proto::Service serv;
    for ( int i = 0, isz = servicesConf_.machine_size(); i < isz; ++i ) {
      const settings::proto::ServiceMachine& machine  = servicesConf_.machine(i);
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
        return serv;
      }
    }
    error_log << QObject::tr("Параметры подключения к сервису %1 не найдены").arg(serviceName);

    return serv;
  }

  // rpc::Address serviceAddress( meteo::settings::proto::ServiceCode code, bool* ok ) const
  // {
  //   meteo::settings::proto::Service serv = service( code, ok );
  //   if ( 0 != ok && false == *ok ) {
  //     return rpc::Address();
  //   }

  //   if ( servicesConf_.machine_size() == 0 ) { return rpc::Address(); }

  //   if ( !serv.has_host() ) {
  //     return rpc::Address(QString::fromStdString(servicesConf_.machine(0).host()), serv.port());
  //   }

  //   return rpc::Address(QString::fromStdString(serv.host()), serv.port());
  // }

  // rpc::Channel* serviceChannel( meteo::settings::proto::ServiceCode code ) const
  // {
  //   bool ok = false;
  //   rpc::Address addr = serviceAddress( code, &ok );
  //   if ( false == ok ) {
  //     error_log << QObject::tr("Не удалось получить адрес сервиса %1")
  //       .arg(code);
  //     return 0;
  //   }
  //   rpc::Channel* ch = rpc::Channel::connect(addr);
  //   if ( 0 == ch ) {
  //     error_log << QObject::tr("Не удалось установить соединение со службой %1 по адресу %2")
  //       .arg(code)
  //       .arg( addr.asString() );
  //   }
  //   return ch;
  // }

  // rpc::TController* serviceController( meteo::settings::proto::ServiceCode code ) const
  // {
  //   QString serviceName = QString::fromUtf8(meteo::settings::proto::ServiceCode_Name(code).c_str()).remove(0,1).toLower();

  //   bool ok = false;
  //   rpc::Address addr = serviceAddress( code, &ok );
  //   if ( false == ok ) {
  //     error_log << QObject::tr("Не удалось получить адрес сервиса %1")
  //       .arg(serviceName);
  //     return 0;
  //   }
  //   rpc::TController* ctrl = new rpc::TController;
  //   if ( false == ctrl->initChannel(addr) ) {
  //     error_log << QObject::tr("Не удалось установить соединение с сервисом %1 по адресу %2")
  //       .arg( serviceName )
  //       .arg( addr.asString() );
  //     delete ctrl;
  //     return 0;
  //   }
  //   return ctrl;
  // }

  int serviceTimeout( meteo::settings::proto::ServiceCode code ) const
  {
    bool ok;
    meteo::settings::proto::Service serv = service( code, &ok );
    if ( false == ok ) {
      return -1;
    }
    return serv.default_timeout();
  }

  bool load()
  {
    init();

    bool status = true;
    for(auto it = msg_.cbegin(); it != msg_.cend(); ++it) {
      if(false == load(it.key(), it.value()))
        status = false;
    }

    if (false == status) { return false; }

    updateSaveState();

    _curLogin = getCurrentUserLogin();

    return true;
  }

  bool save()
  {
    bool status = true;
    for(auto it = msg_.cbegin(); it != msg_.cend(); ++it) {
      if(false == save(it.key(), it.value()))
        status = false;
    }
    return status;
  }

  bool save( const QString& name, const google::protobuf::Message* message )
  {
    msg_[name]->CopyFrom(*message);

    bool status = false;
    QFile file( name );
    if ( false == file.exists() ) {
      error_log << QString("Нет файла настроек %1").arg( name );
      return false;
    }
    if ( false == file.open( QIODevice::WriteOnly) ) {
      error_log << QString("Не удалось открыть файл настроек %1").arg( name );
      return false;
    }
    QString data = TProtoText::toText(*message);
    if(false == data.isEmpty() ){
      file.write( data.toUtf8() );
      status = true;
    }
    else
    {
      error_log << QString("Получены пустые значения  настройки %1").arg( name );
    }
    file.close();
    updateSaveState();
    return status;
  }

  bool load( const QString& name, google::protobuf::Message* message )
  {
    QFile file( name );
    if ( false == file.exists() ) {
      error_log << QString("Нет файла настроек %1").arg( name );
      if ( false == file.open( QIODevice::WriteOnly) ) {
        error_log << QString("Не удалось создать файл настроек %1").arg( name );
        return false;
      }
      else
      {
        file.close();
      }
    }
    if ( false == file.open( QIODevice::ReadOnly) ) {
      error_log << QString("Не удалось открыть файл настроек %1").arg( name );
      return false;
    }
    QString strconf( file.readAll() );
    file.close();
    if (strconf.isEmpty() == true)
    {
      // if (name == MnCommon::etcPath() + "/db.settings.conf") {
      //   warning_log << QObject::tr("Будут использованы настройки подключения к базам данных по умолчанию");
      //   loadDefaultDb();
      //   return save(MnCommon::etcPath() + "/db.settings.conf", message);
      // }
      // else
	if (name == MnCommon::etcPath() + "/msg.settings.conf") {
        warning_log << QObject::tr("Будут использованы настройки журналирования по умолчанию");
        loadDefaultMsg();
        return save(MnCommon::etcPath() + "/msg.settings.conf", message);
      }
      else if (name == MnCommon::etcPath() + "/services.conf") {
        warning_log << QObject::tr("Будут использованы настройки подключения к сервисам по умолчанию");
        loadDefaultServices();
        return save(MnCommon::etcPath() + "/services.conf", message);
      }
      else if (name == MnCommon::etcPath() + "/thematics.conf") {
        warning_log << QObject::tr("Нет предустановленых тематических обработок");
        return true;
      }
      else if ( name == MnCommon::sharePath() + "/palette.conf") {
        return true;
      }
      else {
        return false;
      }
    }
    bool res = TProtoText::fillProto( strconf, message );
    return res;
  }

  void updateSaveState()
  {
    state_.clear();
    std::string str;
    google::protobuf::Message* msg;
    foreach( msg, msg_ ){
      if ( false == msg->IsInitialized() ) {
        /*error_log << QObject::tr("Протоструктура настройки не инициализирована %1")
                     .arg( msg->InitializationErrorString().c_str() );*/
        continue;
      }
      msg->SerializeToString(&str);
      state_ += QByteArray(str.c_str(), str.length());
    }
  }

  bool saveDbSettings()
  {
    return save(MnCommon::etcPath() + "/db.settings.conf", msg_[MnCommon::etcPath() + "/db.settings.conf"]);
  }

  bool loadDbSettings()
  {
    return load(MnCommon::etcPath() + "/db.settings.conf", msg_[MnCommon::etcPath() + "/db.settings.conf"]);
  }

private:
  void init()
  {
    // msg_[MnCommon::etcPath() + "/db.settings.conf"] = &dbconf_;
    // msg_[MnCommon::etcPath() + "/loc.settings.conf"] = &locconf_;
    // msg_[MnCommon::etcPath() + "/log.settings.conf"] = &logconf_;
    // msg_[MnCommon::etcPath() + "/mappi.settings.conf"] = &mappiconf_;
    // msg_[MnCommon::etcPath() + "/msg.settings.conf"] = &msgconf_;
    // msg_[MnCommon::etcPath() + "/reception.conf"] = &reception_;
    //msg_[MnCommon::etcPath() + "/satframe.conf"] = &satframe_;
    // msg_[MnCommon::etcPath() + "/satinstr.conf"] = &instruments_;
    // msg_[MnCommon::etcPath() + "/thematics.conf"] = &thematics_;
    // msg_[MnCommon::etcPath() + "/satpretr.conf"] = &satellites_;
    // msg_[MnCommon::etcPath() + "/services.conf"] = &servicesConf_;
    // msg_[MnCommon::etcPath() + "/cleaner.conf"] = &cleanerConf_;
    // msg_[MnCommon::etcPath() + "/palette.conf"] = &palettes_;
  }

  // bool loadDefaultDb()
  // {
  //   dbconf_.Clear();
  //   meteo::settings::DbConnection* params_telegram = dbconf_.add_db_connection();
  //   params_telegram->set_name("db_ptkpp");
  //   params_telegram->set_human_name("База данных телеграмм");
  //   params_telegram->set_host("localhost");
  //   params_telegram->set_conn_name("telegram");
  //   params_telegram->set_port(params_telegram->port());

  //   meteo::settings::DbConnection* params_meteo = dbconf_.add_db_connection();
  //   params_meteo->set_name("db_meteo");
  //   params_meteo->set_human_name("База данных метеоданных");
  //   params_meteo->set_host("localhost");
  //   params_meteo->set_conn_name("meteo");
  //   params_meteo->set_port(params_meteo->port());

  //   meteo::settings::DbConnection* params_obanal = dbconf_.add_db_connection();
  //   params_obanal->set_name("db_obanal");
  //   params_obanal->set_human_name("База данных прогнозирования");
  //   params_obanal->set_host("localhost");
  //   params_obanal->set_conn_name("obanal");
  //   params_obanal->set_port(params_obanal->port());

  //   meteo::settings::DbConnection* params_journal = dbconf_.add_db_connection();
  //   params_journal->set_name("db_journal");
  //   params_journal->set_human_name("База данных журналирования");
  //   params_journal->set_host("localhost");
  //   params_journal->set_conn_name("journal");
  //   params_journal->set_port(params_journal->port());

  //   meteo::settings::DbConnection* params_admin = dbconf_.add_db_connection();
  //   params_admin->set_name("db_journal");
  //   params_admin->set_human_name("База данных администрирования");
  //   params_admin->set_host("localhost");
  //   params_admin->set_conn_name("admin");
  //   params_admin->set_port(params_admin->port());
  //   params_admin->set_login(params_admin->login());

  //   return true;
  // }


  bool loadDefaultMsg()
  {
    meteo::settings::MsgSettings opt;
    opt.set_debug(meteo::settings::kNone);
    opt.set_info(meteo::settings::kReg);
    opt.set_warning(meteo::settings::kShow);
    opt.set_error(meteo::settings::kShow);
    opt.set_critical(meteo::settings::kShow);
    msgconf_.CopyFrom(opt);
    return true;
  }

  void loadDefaultServices()
  {
    meteo::settings::proto::ServiceMachines opt;
    meteo::settings::proto::ServiceMachine* machine = opt.add_machine();
    machine->set_host("armvgm");
    machine->set_default_timeout(5000);
    meteo::settings::proto::Service* serv = machine->add_serv();
    serv->set_name("mappi.receive.manager");
    serv->set_port(64569);
    serv->set_title("Управление приёмом спутниковых данных");
    serv->set_code( meteo::settings::proto::kRecvStream );
    // serv = machine->add_serv();
    // serv->set_port(51002);
    // serv = machine->add_serv();
    // serv->set_port(54569);
    // serv->set_name("bronenosec.send.service");
    // serv->set_title("Служба сортировки ГМИ");
    // serv->set_code( meteo::settings::proto::kMsgNotify );
    // serv = machine->add_serv();
    // serv->set_port(54571);
    // serv->set_name("bronenosec.alphanum");
    // serv->set_title("Служба раскодирования текстовых сообщений");
    // serv->set_code( meteo::settings::proto::kAlphanum );
    // serv = machine->add_serv();
    // serv->set_port(54572);
    // serv->set_name("bronenosec.bufr");
    // serv->set_title("Служба раскодирования сообщений BUFR");
    // serv->set_code( meteo::settings::proto::kBufr );
    // serv = machine->add_serv();
    // serv->set_port(54573);
    // serv->set_name("bronenosec.grib");
    // serv->set_title("Служба раскодирования GRIB");
    // serv->set_code( meteo::settings::proto::kGrib );
    // serv = machine->add_serv();
    // serv->set_port(58765);
    // serv->set_name("bronenosec.srcdata");
    // serv->set_title("Служба доступа к результатам наблюдений");
    // serv->set_code( meteo::settings::proto::kSrcData );
    // serv = machine->add_serv();
    // serv->set_port(58767);
    // serv->set_name("bronenosec.field");
    // serv->set_title("Служба доступа к результатам объективного анализа");
    // serv->set_code( meteo::settings::proto::kField );
    // serv = machine->add_serv();
    // serv->set_port(54574);
    // serv->set_name("bronenosec.sprinf");
    // serv->set_title("Служба доступа к справочной информации");
    // serv->set_code( meteo::settings::proto::kSprinf );
    servicesConf_.CopyFrom(opt);
  }
};

TMeteoSettings::TMeteoSettings(QObject* parent) :
  QObject(parent),
  pimpl_(new MeteoSettingsPrivate())
{
  _settingswatcher.addPath(MnCommon::etcPath() + "/db.settings.conf");

  connect( &_settingswatcher, SIGNAL( fileChanged( const QString& ) ), this, SLOT( slotFileChanged( const QString& ) ) );
}

TMeteoSettings::~TMeteoSettings()
{
  delete pimpl_;
  pimpl_= 0;
}

void TMeteoSettings::slotFileChanged(const QString &filepath)
{
  if (filepath == MnCommon::etcPath() + "/db.settings.conf") {
    emit dbSettingsChanged();
  }
}

bool TMeteoSettings::isChanged() const
{
  return pimpl_->isChanged();
}

const QString TMeteoSettings::getCurrentUserLogin() const
{
  return pimpl_->getCurrentUserLogin();
}


const ::mappi::conf::Reception     TMeteoSettings::reception()   const { return pimpl_->reception();   }
const ::mappi::conf::Pretreatment  TMeteoSettings::satellites()  const { return pimpl_->satellites();  }
const ::mappi::conf::InstrCollect  TMeteoSettings::instruments() const { return pimpl_->instruments(); }
const ::mappi::proto::ThematicProcs TMeteoSettings::thematics()  const { return pimpl_->thematics();   }

TColorGradList TMeteoSettings::palette(const QString& thematicName) const
{
  return pimpl_->palette(thematicName);
}

void TMeteoSettings::setPalette(const QString& thematicName, const TColorGradList& grad)
{
  pimpl_->setPalette(thematicName, grad);
}

bool TMeteoSettings::satellite(const QString& satelliteName, ::mappi::conf::PretrSat& satellite)
{
  return pimpl_->satellite(satelliteName, satellite);
}
bool TMeteoSettings::instrument(const QString& satelliteName, ::mappi::conf::Instrument& instrument)
{
  return pimpl_->instrument(satelliteName, instrument);
}

bool TMeteoSettings::instrument(::mappi::conf::InstrumentType type, ::mappi::conf::Instrument &instrument)
{
  return pimpl_->instrument(type, instrument);
}


const settings::MsgSettings TMeteoSettings::msgSettings() const
{
  return pimpl_->msgSettings();
}

void TMeteoSettings::setMsgSettings( const settings::MsgSettings& opt )
{
  pimpl_->setMsgSettings(opt);
}

 // msgcenter::RouteOptions TMeteoSettings::route()
 // {
 //   msgcenter::RouteOptions conf;
 //   load(MnCommon::etcPath() + "/msgcenter.route.conf", &conf);
 //   return conf;
 // }

// ConnectProp TMeteoSettings::dbConfTelegram() const
// {
//   return pimpl_->dbConfTelegram();
// }

 // ConnectProp TMeteoSettings::dbConfMeteo() const
 // {
 //   return dbConf(kMeteo);
 // }

 // ConnectProp TMeteoSettings::dbConfMappi() const
 // {
 //   return dbConf(kMappi);
 // }

// ConnectProp TMeteoSettings::dbConfObanal() const
// {
//   return pimpl_->dbConfObanal();
// }

// ConnectProp TMeteoSettings::dbConfJournal() const
// {
//   return pimpl_->dbConfJournal();
// }

// ConnectProp TMeteoSettings::dbConfForecast() const
// {
//   return pimpl_->dbConfForecast();
// }

// ConnectProp TMeteoSettings::dbConfAdmin() const
// {
//   return pimpl_->dbConfAdmin();
// }

// ConnectProp TMeteoSettings::dbConfClimat() const
// {
//   return pimpl_->dbConfClimat();
// }

// const settings::DbSettings& TMeteoSettings::dbConfs() const
// {
//   return pimpl_->dbConfs();
// }

// ConnectProp TMeteoSettings::dbConf(const QString &connName) const
// {
//   return pimpl_->dbConf(connName);
// }

// settings::DbConnection TMeteoSettings::dbConn(const QString &connName) const
// {
//   return pimpl_->dbConn(connName);
// }

// bool TMeteoSettings::setDbConf(const QString &name, const settings::DbConnection& conf)
// {
//   return pimpl_->setDbConf(name, conf);
// }

// bool TMeteoSettings::setDbConf(const QString &name, const ConnectProp& conf)
// {
//   return pimpl_->setDbConf(name, conf);
// }

// bool TMeteoSettings::setDbConfJournal( const settings::DbConnection &conf )
// {
//   return pimpl_->setDbConfJournal(conf);
// }

// bool TMeteoSettings::setDbConfJournal( const ConnectProp &conf )
// {
//   return pimpl_->setDbConfJournal(conf);
// }

// ::mappi::cleaner::DbIntervalOption TMeteoSettings::dbClearConf(const QString &dbName) const
// {
//   return pimpl_->dbClearConf(dbName);
// }

// ::mappi::cleaner::DbIntervalOption TMeteoSettings::dbArchiveConf(const QString &dbName) const
// {
//   return pimpl_->dbArchiveConf(dbName);
// }

// const meteo::settings::proto::ServiceMachines& TMeteoSettings::services() const
// {
//   return pimpl_->services();
// }

// meteo::settings::proto::Service TMeteoSettings::service( meteo::settings::proto::ServiceCode code, bool* ok ) const
// {
//   return pimpl_->service( code, ok );
// }

// rpc::Address TMeteoSettings::serviceAddress( meteo::settings::proto::ServiceCode code, bool* ok ) const
// {
//   return pimpl_->serviceAddress(code, ok);
// }

// rpc::Channel* TMeteoSettings::serviceChannel( meteo::settings::proto::ServiceCode code ) const
// {
//   return pimpl_->serviceChannel(code);
// }

// rpc::TController* TMeteoSettings::serviceController( meteo::settings::proto::ServiceCode code ) const
// {
//   return pimpl_->serviceController(code);
// }

int TMeteoSettings::serviceTimeout( meteo::settings::proto::ServiceCode code ) const
{
  return pimpl_->serviceTimeout(code);
}

bool TMeteoSettings::load()
{
  return pimpl_->load();
}

bool TMeteoSettings::save()
{
  return pimpl_->save();
}

bool TMeteoSettings::save( const QString& name, const google::protobuf::Message* message )
{
  return pimpl_->save(name, message);
}

bool TMeteoSettings::load( const QString& name, google::protobuf::Message* message )
{
  return pimpl_->load(name, message);
}

void TMeteoSettings::updateSaveState()
{
  pimpl_->updateSaveState();
}

bool TMeteoSettings::saveDbSettings()
{
  return pimpl_->saveDbSettings();
}

bool TMeteoSettings::loadDbSettings()
{
  return pimpl_->loadDbSettings();
}

} //internal

} // mappi
} // meteo
