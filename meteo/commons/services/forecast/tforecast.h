#ifndef METEO_COMMONS_FORECAST_TFORECAST_H
#define METEO_COMMONS_FORECAST_TFORECAST_H

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geopoint.h>

#include "tdataprovider.h"
#include "tdataprocessor.h"

#include <qscriptengine.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qlayout.h>
// #include <QtScriptTools/qscriptenginedebugger.h>

#include <cross-commons/app/paths.h>

class TForecastList;
class TFItemDelegate;
class TFModel;
class TFItem;
struct StationData;

const QString scripts_path = MnCommon::varPath("meteo") + "/forecast/scripts/";
const QString ui_path =  MnCommon::varPath("meteo") + "/forecast/ui/";

Q_DECLARE_METATYPE(QString)
Q_DECLARE_METATYPE(StationData)
Q_DECLARE_METATYPE(meteo::GeoPoint)
Q_DECLARE_METATYPE(FMetaData)


//для регистрации структур в QScript:
QScriptValue toScriptStationData(QScriptEngine *engine, const StationData &s);
void fromScriptStationData(const QScriptValue &obj, StationData &s);
QScriptValue toScriptGeoPoint(QScriptEngine *engine, const meteo::GeoPoint &s);
void fromScriptGeoPoint(const QScriptValue &obj, meteo::GeoPoint &s);
enum{
  ZIMA,
  VESNA,
  LETO,
  OSEN
};

/**
 *  Один расчетный метод из списка методов
 *  здесь вызываются расчеты и загружается файл метода
 *
 *
 */
class TForecast: public QObject{
  Q_OBJECT
public:

  TForecast(TDataProvider* adataProvider_, TDataProcessor* adataProcessor, TForecastList* ,const StationData &);
  virtual ~TForecast();

  TFModel* getSrcModel() const {return srcModel_;} ;
  TFModel* getResultModel() const {return resModel_;} ;

  static QString stringSynSit(int);
  static QString stringSeason(int);


  void createSrcModel();
  void initScript(bool need_ui);
  void runForecast(bool need_ui);
  // запускаем прогнозы с предзаполненной моделью данных
  // используется в вебе для расчета во ввреденным пользователем значениям
  void runForecastWithFilledModelOnly(bool need_ui);
  void tryRunForecast(bool need_ui);//
  void runForecastOnly(bool need_ui);
  void setSynSit(int ss){cur_synSit_ = ss;}
  void setFrontType(int ss){front_type_ = ss;}
  void setSeason(int as){cur_season_ = as; }

  bool setMethodTime();

  //**                     Методы для работы со скриптами
  void loadScript(const QString&);
  QScriptValue evaluateFile(QScriptEngine *engine, const QString &fileName);

  TFItem *getMainItem(const QString &);

  void setStation(const StationData &ast);
  static int oprSeason(const QDate&);
  //**                     Разные Методы )))
  Q_INVOKABLE int getFrontType(){  return front_type_;}
  Q_INVOKABLE int getSynSit(){return cur_synSit_;}
  Q_INVOKABLE int getSeason(){return cur_season_;}


  Q_INVOKABLE meteo::GeoPoint toGeoPoint(const QVariant &v )const;//{return v.value<meteo::GeoPoint>();}
  Q_INVOKABLE StationData getStationData()const;
  Q_INVOKABLE QDateTime getDate()const {
    return dataProvider_->getDate();
  };
  Q_INVOKABLE QString getStationName() const { return station_.name;}
  Q_INVOKABLE QString getStationIndex() const { return station_.index;}
  Q_INVOKABLE meteo::GeoPoint getStationCoord() const { return station_.coord;}
  Q_INVOKABLE meteo::GeoPoint getCurrentCoord() const ;
  Q_INVOKABLE void setDate(const QDateTime &adt);

  Q_INVOKABLE void reset(){resetModels();}

  Q_INVOKABLE void setStationName(const QString& );
  //**              Методы для работы с ГУИ                                            **
  Q_INVOKABLE void addFunction(const  QScriptValue& );
  Q_INVOKABLE void addFieldValuetem( const QString &itemText, const QString & aparam,
                                     const QString &asrok,const QString &alevel,const QString &type_level,const QString& aName);
  Q_INVOKABLE void addSimpleItem(const QString& itemText,const QString& descrName,const QString& val);
  Q_INVOKABLE void addFieldAdditem( const QString &itemText, const QString & aparam, const QString & ,
                                    const QString &asrok,const QString &alevel,const QString &atypelevel,const QString& aName);
  Q_INVOKABLE void addComboFunction(const  QScriptValue& );


  Q_INVOKABLE QScriptValue  loadUi(const QString& );
  Q_INVOKABLE void addDataItem(const QString& itemName, const QString& itemText, const QString &alevel, const QString &type_level, const QString &, const QString &aname);
  Q_INVOKABLE void addForecastDataItem(const QString& itemText,const QString& itemDescr, const QString & alvl, const QString & atypelevel, const QString &srok );
  Q_INVOKABLE void addCalcItem(const QString& itemName, const QString& descrName, const QString& funcName/*,const QString& itemText,const QString &alevel*/);
  Q_INVOKABLE void addComboItem(const QString& text, const QString& descr, const QString& items);
  Q_INVOKABLE void addDTItem(const QScriptValue &list);
  Q_INVOKABLE void setParrent(const QString& parentName,const QString& separatorName);
  Q_INVOKABLE void addSeparator(const QString& separatorName);
  Q_INVOKABLE void setFieldList(const QString &itemText, const QString & aparam,
                                const QString & asrok,const QString &alevel,const QString &type_level,const QString &atime,const QString& itemName );

  Q_INVOKABLE void addResultFunction(const  QScriptValue& );
  Q_INVOKABLE void addResultItem(const QString& itemText,const QString&);

  QString methodFullName(){ return methodFullName_;}
  QString methodName(){ return methodName_;}
  QString methodYavlType(){return methodYavlType_;}
  QString methodHelpFile(){return methodHelpFile_;}
  QString methodSeason(){return methodSeason_;}
  QString methodSynSit(){return methodSynSit_;}

  QString methodHours(){return methodHours_;}
  QString methodTimes(){return methodTimes_;}


  const QString iconName() {return methodIconName_;}
  QWidget *getWidget(){return ui_;}
  bool checkSeason();
  bool checkSynSit();
  void resetModels();
  QScriptEngine *engine(){return engine_;}

private:
  void loadModels();
  void setUiTitle();
  QWidget *ui_; // главный виджет метода

  QString GBStyle = "QGroupBox[success='true'] \
  {\
                    border-color:     #68d720!important;    \
                    border-width:     1px;      \
                    border-style:     solid;    \
                    border-radius:    3px;      \
                    padding:          4px;      \
                    padding-top:      20px; \
} \
                    QGroupBox { \
                    border-color:     #f33f1d;    \
                    border-width:     1px;      \
                    border-style:     solid;    \
                    border-radius:    3px;      \
                    padding:          4px;      \
                    padding-top:      20px; \
} ";

                    protected:
                    QString methodIconName_;
  QScriptEngine *engine_;

  // дебажим код
  // QScriptEngineDebugger debugger_;

  TFModel *srcModel_;
  TFModel *resModel_;

  TFItemDelegate* delegat_;

  QString currentParent;
  TDataProvider* dataProvider_;
  TDataProcessor* dataProcessor_;

  QString methodFullName_;
  QString methodName_;
  QString methodHelpFile_;


  int cur_synSit_;
  int cur_season_;
  int front_type_; //0 - внутримассовый 1 - теплый 2 - холодный

  QString methodHours_;//заблаговременность прогноза
  QString methodTimes_;//заблаговременность прогноза  QString methodSeason_;
  QString methodSynSit_;
  QString methodYavlType_;
  QString methodSeason_;
  TForecastList* methodsList_;
  StationData station_;
private slots:
  void destroyedW(QObject *obj);

  // проверяем стили  в соответствии с введенными данными
  void checkGBStyles();

};
#endif

