#ifndef METEO_COMMONS_SERVICES_CLIMATDATA_CLIMATSURF_H
#define METEO_COMMONS_SERVICES_CLIMATDATA_CLIMATSURF_H

#include <qobject.h>

#include <meteo/commons/proto/climat.pb.h>

namespace meteo {
  class DbiQuery;
  namespace climat {

    class Control;
    
    class ClimatSurfService : public QObject
    {
      Q_OBJECT
      public:
      ClimatSurfService();
      
      bool getDayValue(const meteo::climat::DataRequest* req,
		       meteo::climat::SurfDayReply* res);

      bool getDecadeValue(const meteo::climat::DataRequest* req,
			  meteo::climat::SurfDecadeReply* res);

      bool getMonthValue(const meteo::climat::DataRequest* req,
      			 meteo::climat::SurfMonthReply* res);

      bool getAvgMonthValue(const meteo::climat::DataRequest* req,
			    meteo::climat::SurfMonthAvgReply* res);
      
      bool getYearValue(const meteo::climat::DataRequest* req,
			meteo::climat::SurfYearReply* res);

      // bool getAllValue(const meteo::climat::DataRequest* req,
      // 		       meteo::climat::SurfMonthAllReply* res);

      bool getAllValueList(const meteo::climat::DataRequest* req,
			   meteo::climat::SurfAllListReply* res);

      bool getDateValue(const meteo::climat::DataRequest* req,
			meteo::climat::SurfDateReply* res);

    private:
      bool checkRequest(const meteo::climat::DataRequest* req, QDateTime* dtStart = 0, QDateTime* dtEnd = 0, QString* err = 0);

      bool fillMatch(DbiQuery* query, const meteo::climat::DataRequest* req, QString* err);
      //QString createMatchJson(const meteo::climat::DataRequest* req,  QString* err);
      // NoSql* execAggregate(const QString& collection,
      // 				  const QString& match,
      // 				  const QString& strq,
      // 				  QString* err);
    };
    
  }
}

#endif
