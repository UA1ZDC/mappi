#ifndef MAPPI_GLOBAL_GLOBAL_H
#define MAPPI_GLOBAL_GLOBAL_H


#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/services.pb.h>


#include <mappi/settings/mappisettings.h>



namespace meteo {
  namespace global {
    Dbi* dbMappi();       //!< БД для спутниковой информации
    ConnectProp dbConfMappi();

    
    static const QString kMainWindowPluginPath( MnCommon::pluginPath("") + "/mappi.mainwindow/" );
    // proto::Location ukazpunkt();
    QString findWeatherFile(const QString& tleDir, const QDateTime& timeCompare );
    
  }
}


namespace meteo {
  namespace internal {
    //  void logoutToDb(tlog::Priority priority, const QString& facility, const QString& fileName, int line, const QString &m);
    
    
    //!
    class MappiGlobal : public Global
    {
    public:
      MappiGlobal(){}
      virtual ~MappiGlobal(){}
      
      ConnectProp dbConfMappi() const;
      Dbi* dbMappi();

      friend class TSingleton<Global>;
      
      // virtual ConnectProp dbConfJournal();
      // virtual ConnectProp dbConfTelegram();
      // virtual ConnectProp dbConfMeteo();
      // virtual ConnectProp dbConfMappi();
      // virtual ConnectProp dbConfObanal();
      // virtual ConnectProp dbConfForecast();
      
      // virtual NS_PGBase* dbTelegram();
      // virtual NS_PGBase* dbMeteo();
      // virtual NS_PGBase* dbMappi();
      // virtual NS_PGBase* dbObanal();
      // virtual NS_PGBase* dbJournal();
      // virtual NS_PGBase* dbForecast();
      
      // virtual bool connectDb();
      // virtual bool init();
      // virtual QString currentUserName();
      // virtual QString currentUserShortName();
      // virtual QString currentRank();
      
      // virtual QString rank(int code);
      
      // QStringList geoLoaders();
      
      // virtual ::rpc::Address serviceAddress(meteo::settings::proto::ServiceCode code, bool* ok);
      // virtual ::rpc::Channel* serviceChannel(meteo::settings::proto::ServiceCode code);
      // virtual ::rpc::TController* serviceController(meteo::settings::proto::ServiceCode code);
      // virtual int serviceTimeout(meteo::settings::proto::ServiceCode code);
      // virtual meteo::settings::proto::Service service(meteo::settings::proto::ServiceCode code, bool* ok);
      
      //ukaz::proto::Location ukazpunkt();
      
      //private:
      //ConnectionDict dbConn_;
    };
      
  }

  using MappiGlobal = TSingleton<internal::MappiGlobal>;

}


#endif // MAPPI_GLOBAL_GLOBAL_H
