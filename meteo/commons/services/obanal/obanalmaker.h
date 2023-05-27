#ifndef METEO_C_SERVICES_OBANAL_AUTOOBANALMAKER_H
#define METEO_C_SERVICES_OBANAL_AUTOOBANALMAKER_H


#include <qobject.h>
#include <qthread.h>
#include <cross-commons/app/options.h>
#include <meteo/commons/services/obanal/tobanalservice.h>
#include <meteo/commons/proto/obanal.pb.h>

namespace meteo {
namespace obanal {


  class ObanalMaker : public QObject
  {
    Q_OBJECT
  public:
    ObanalMaker();
    ~ObanalMaker();

  public:
    bool makeWindObanal( const surf::DataRequest& request );
    bool makeMinMaxTempr( const surf::DataRequest& request );
    bool makeObanal( const surf::DataRequest& request );
    bool obanalPoDescrs(const surf::DataRequest &rdt);
    bool obanalPoGeoData(meteo::GeoData *,const meteo::map::proto::WeatherLayer &);

  private:
    meteo::surf::DataReply* getMeteoData( const meteo::surf::DataRequest& request);
    meteo::TObanalService* service_ = nullptr;
   
 };

  class ObanalRuner : public QObject
   {
       Q_OBJECT

   public slots:
    void analyse(const meteo::surf::DataRequest &rdt) {
        QString result;
        meteo::obanal::ObanalMaker* maker = new meteo::obanal::ObanalMaker();
        result = QString("%1").arg(maker->obanalPoDescrs(rdt));
        emit resultReady(result);
        delete maker;
    }
    void analyseData(meteo::GeoData* data,const meteo::map::proto::WeatherLayer &info) {
        QString result;
        meteo::obanal::ObanalMaker* maker = new meteo::obanal::ObanalMaker();
        result = QString("%1").arg(maker->obanalPoGeoData(data,info));
        emit resultReady(result);
        delete maker;
    }

   signals:
       void resultReady(const QString &);
   };
}
}
Q_DECLARE_METATYPE(meteo::surf::DataRequest)
Q_DECLARE_METATYPE(meteo::map::proto::WeatherLayer)

#endif // METEO_PROGNOZ_SERVICES_OBANAL_AUTOOBANALMAKER_H
