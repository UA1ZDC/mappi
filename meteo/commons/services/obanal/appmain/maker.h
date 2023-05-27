#ifndef METEO_PROGNOZ_SERVICES_OBANAL_AUTOOBANALMAKER_H
#define METEO_PROGNOZ_SERVICES_OBANAL_AUTOOBANALMAKER_H

#include <qobject.h>
#include <cross-commons/app/options.h>
#include <meteo/commons/services/obanal/tobanalservice.h>
#include <meteo/commons/services/obanal/obanalmaker.h>
#include <meteo/commons/proto/obanal.pb.h>

namespace meteo {
namespace obanal {


const commons::Arg kAeroArg     =    commons::Arg::makeLong(    "--aero"             );
const commons::Arg kSurfaceArg  =    commons::Arg::makeLong(    "--surface"          );
const commons::Arg kOceanArg    =    commons::Arg::makeLong(    "--ocean"            );
const commons::Arg kGribAnalyze =    commons::Arg::make( "-g",  "--grib"             );
const commons::Arg kBeginDate  =     commons::Arg::makeLong(    "--begin",      true );
const commons::Arg kFinishDate =     commons::Arg::makeLong(    "--end",        true );
const commons::Arg kDescr      =     commons::Arg::make( "-d",  "--descr",      true );
const commons::Arg kStation    =     commons::Arg::make( "-s",  "--station",    true );
const commons::Arg kLevelP     =     commons::Arg::make( "-p",  "--levelP",     true );
const commons::Arg kLevelH     =     commons::Arg::make( "-H",  "--levelH",     true );
const commons::Arg kHelp       =     commons::Arg::make( "-h",  "--help"             );
const commons::Arg kCenter     =     commons::Arg::make( "-c",  "--center",     true );
const commons::Arg kModel      =     commons::Arg::make( "-m",  "--model",      true );
const commons::Arg kLevelType  =     commons::Arg::make( "-l",  "--ltype",      true );
const commons::Arg kHour       =     commons::Arg::make( "-R",  "--hour",       true );
const commons::Arg kFile       =     commons::Arg::make( "-f",  "--file",       true );
const commons::Arg kTasksUid   =     commons::Arg::makeLong(    "--tasks-uid",  true );
const commons::Arg kToJSON     =     commons::Arg::makeLong(    "--to-json" );


enum OBANAL_TYPE
{
  kAllDataType =  meteo::surf::kAllDataType,  //!< анализ всех данных
  kSurface     =  meteo::surf::kSynopType,    //!< анализ приземных данных
  kAero        =  meteo::surf::kAeroType,     //!< анализ аэрологических данных
  kRadar       =  meteo::surf::kRadarMapType, //!< анализ радиолокационных данных
  kOcean       =  meteo::surf::kOceanType,    //!< анализ морских данных
  kGrib        = 400,                         //!< анализ в Grib
};

class Maker : public QObject
{
  Q_OBJECT
public:
  Maker(commons::ArgParser* options);
  ~Maker();

signals:
  void finished(int exitcode);

public slots:
  void slotMakeAutoObanal();

private:
  void autoObanal();
  void obanal();
  void gribObanal();
  bool customObanal();
  void customObanalAero( const surf::DataRequest& rdt );
  bool customObanalSurf( const surf::DataRequest& rdt );
  void customObanalOcean( const surf::DataRequest& rdt );
  bool customGribObanal();
  bool makeAutoObanalSurf( const surf::DataRequest& rdt );
  bool makeAutoObanalOcean( const surf::DataRequest& rdt );
  bool makeAutoObanalAero( const surf::DataRequest& rdt );
  bool makeCustomDateAutoObanalAero( const surf::DataRequest& rdt );
  bool saveJSONFile();
  QJsonDocument loadJson(QString fileName);
  void saveJson(QJsonDocument document, QString fileName);
  // отправляем запрос к сервису и получаем поля, транслируем в json объект
  QJsonObject getObjectFromFiled( meteo::field::DataRequest& request, QString& param, QString& level );

  bool makeGribObanal( const surf::GribDataRequest& request );
  bool makeAutoGribObanal();
  void fillRequestsFromArg(QList<surf::DataRequest>& requests, QList< surf::GribDataRequest>& gribRequests);
  static bool taskGribHandler(const obanalsettings::Task& task, surf::GribDataRequest* grib_request,
                              QList<surf::GribDataRequest> *gribRequests);
  static void run_fromFile( commons::ArgParser* options, surf::GribDataRequest* grib_request,
                            QList<surf::DataRequest> *requests, QList<surf::GribDataRequest> *gribRequests,
                            const std::string &beginDate, const std::string &endDate );
  static bool taskHandler(const  obanalsettings::Task& task, QList< surf::DataRequest> *requests,
                    const std::string &beginDate, const std::string &endDate);
  static void fillDtsRequests(QList< surf::DataRequest>* requests,  surf::DataRequest& request,
                              QDateTime& startDateTime, QDateTime& endDateTime );
  static void fillLevelsRequests(QList< surf::DataRequest> *requests,QList<int>& levels,
                                 QDateTime& startDateTime, QDateTime& endDateTime,
                                 int datatype,  surf::DataRequest request);
  static void fillDescrRequests(QList< surf::DataRequest> *requests,QList<int>& levels,
                                QDateTime& startDateTime, QDateTime& endDateTime,
                                int datatype, QList<int> descrs,  surf::DataRequest& request);

private:
  void getDt( const surf::DataRequest& rdt,QDateTime *dts, QDateTime *dte, int);
  bool obanalPoDescrs(const surf::DataRequest &rdt);
  bool checkParams( const surf::DataRequest& rdt);


  static const int hoursToOcean = -1;
  static const int hoursToSurf = -1; //на сколько часов назад
  static const int hoursToAero = -24; //на сколько часов назад

  static const int stepHoursToOcean = 1;
  static const int stepHoursToAero = 12;
  static const int stepHoursToSurf = 1; //через сколько


  bool json_saved_ = false;
  meteo::TObanalService* service_ = nullptr;
  meteo::obanal::ObanalMaker* omaker_ = nullptr;
  QList<surf::DataRequest> requests_;
  QList<surf::GribDataRequest> gribRequests_;
  commons::ArgParser* options_ = nullptr;

};

}
}

#endif
