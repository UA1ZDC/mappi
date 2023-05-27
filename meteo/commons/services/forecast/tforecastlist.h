#ifndef METEO_COMMONS_FORECAST_TFORECASTLIST_H
#define METEO_COMMONS_FORECAST_TFORECASTLIST_H

#include <qobject.h>
#include <qdatetime.h>
#include <qmap.h>
#include <meteo/commons/global/global.h>

#include "tdataprovider.h"

class TDataProcessor;
class TDataProvider;
class TForecast;

/**
 * Список расчетных методов
 * методы загружаются из файлов и добавляются в список
 */
class TForecastList : public QObject
{
  Q_OBJECT
public:
  TForecastList(QObject *, bool is_service = false);

  explicit TForecastList(const QDateTime&, const StationData & , QObject *parent = 0);
  ~TForecastList();
  TForecast * addMethodNoRun(const QString& scriptName);
  void addMethod(const QString& scriptName, bool n=false);
  TForecast* getMethod(const QString& methodName);

  int loadMethodsNoRun(const QString &v = meteo::global::kForecastMethodsDir);
  int loadMethods(const QString &v = meteo::global::kForecastMethodsDir,bool need_ui=false);
  void setDateTime(const QDateTime&);
  void setSynSit(int ss);
  void setSeason(int as);
  void setFrontType(int ss);
  void setStation(const StationData &);

  int getSynSit();
  int getSeason();

  const QMap<QString,TForecast*> methodsList();
  const QStringList methodYavlTypes();
  //    void setStationIndex(int );
  //    void setPunktPos(int );
  QString fileListName()const;
  bool validateMethod(const QString & mn);
  bool setValidateMethod(const QString & mn, bool);

private:
  void checkProvider();


  QDateTime date_time_;
  StationData  station_;
  int cur_synSit_;
  int cur_season_;
  QStringList methodYavlTypes_;

  QMap<QString,TForecast*> methodsList_;
  TDataProvider* dataProvider_;
  TDataProcessor* dataProcessor_;
  bool is_service_ = false;
  //TODO TGis::GeoPoint punktPos;

};

#endif // METEO_COMMONS_FORECAST_TFORECASTLIST_H
