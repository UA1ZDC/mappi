#ifndef METEO_COMMONS_SERVICES_CLIMATDATA_VKOCALC_H
#define METEO_COMMONS_SERVICES_CLIMATDATA_VKOCALC_H


#include <qstring.h>
#include <qdatetime.h>
#include <qmap.h>
#include <boost/python.hpp>

#include <meteo/commons/proto/climat.pb.h>

class QApplication;

namespace meteo {
  namespace rpc {
    class Channel;
  }
}

namespace meteo {

  struct ClimAccum {
    float sum;
    float cnt;
    ClimAccum():sum(0), cnt(0) {}
  };

  struct ClimParam {
    float avg;
    float max;
    int maxYear;
    float min;
    int minYear;
    int count;
    ClimParam(): avg(0), max(0), maxYear(0), min(0), minYear(0),
		 count(0) {}
  };

  class VkoCalc
  {
  public:
    VkoCalc();
    ~VkoCalc();

    void setPath(const std::string& path) { _path = QString::fromStdString(path); }
    void setStation(const std::string& station, int diffutc) { _station = station; _diffUtc = diffutc; }
    //month - за один месяц в указанном периоде. если <= 0, то весь период
    void setDt(const std::string& start, const std::string& end, int month = 0) {
      _start = start; _end = end; _month = month; }
    
    bool isInit() { return _isOk && !_station.empty(); }

    bool getClimatData(int type);
    
    // QScriptValue calcAlexandersonTest(int month);
    // QScriptValue calcBuishandTest(int month);
    // QScriptValue calcPetitTest(int month);
    // QScriptValue calcFonNoimanTest(int month);

    // QScriptValue calcGraphValues(int month);
    boost::python::list calcTableValues();
    boost::python::list calcTableParamValues(int val1, int val2);
  
  private:
    void clearData();
    bool readSettings();

    bool requestData(meteo::climat::DataRequest* req, meteo::rpc::Channel* channel);

    QList<std::string> typeToDescr(climat::VkoType type, int* levtype = 0);
    int dayToDecade(int day);
    
    boost::python::list calcTableHum();
    boost::python::list calcTableSurfWithLimit(int month);
    boost::python::list calcVngoGdv(int month, int hour);
    boost::python::list calcTableClouds();
    boost::python::list calcTablePrecip();
    boost::python::list calcTablePrecipDays();
    boost::python::list calcTableWindDir();
    boost::python::list calcTableWindSpeed();
    boost::python::list calcTableTemp();
    boost::python::list calcTableWDays();
    boost::python::list calcTableSpeedDays();
    boost::python::list calcTableSnowHeight();
    boost::python::list calcTableCloudDays();
    boost::python::list calcTableDailyRateTemp(int month, int hour);
    boost::python::list calcTableFrostDate();
    boost::python::list calcTableSnowDate();
    boost::python::list calcTableAeroWind(int month, int layer);
    boost::python::list calcAeroTemp();
    boost::python::list calcAeroIsoterm();
    boost::python::list calcAeroTropo();

    boost::python::list calcVkoddY();
    boost::python::list calcVkoffxY();
    boost::python::list calcVkoNY();
    boost::python::list calcVkowRY();
    boost::python::list calcVkoTY();
    
    boost::python::list calcVkoRY();
    boost::python::list calcVkowY();
    
    boost::python::list calcVkoNm();
    boost::python::list calcVkohm();
    boost::python::list calcVkoVm();
    boost::python::list calcVkohVm();
    boost::python::list calcVkoRm();
    boost::python::list calcVkoYfxm();
    boost::python::list calcVkoP0m();
    boost::python::list calcVkowm();
    boost::python::list calcVkoTm();
    boost::python::list calcVkoUm();
    boost::python::list calcVkoff1m();
    boost::python::list calcVkoTropom();
    boost::python::list calcVkoTgm();
    boost::python::list calcVkossm();
    boost::python::list calcVkoYssm();
    
    boost::python::list fillTablePrecip(const QMap<int, ClimParam>& table);
    void createGraphPrecip(const QMap<int, ClimParam>& table);
    void createGraphTemp(const QMap<int, QMap<int, float> >& table);
    void createGraphDailyRateTemp(const QMap<int, QMap<int, float> >& table,
				  const QMap<int, QMap<int, int> >& count,
				  int month);
    void createGraphWind(const QMap<int, float>& table, int month);

    void calcCloudsDay(QMap<int, QMap<int, float> >* table, const climat::SurfMonthAllReply& data);
    void calcTableDailyRateTemp(const climat::SurfMonthAllValues& atemp,
    				const climat::SurfMonthAllValues& cloud,
    				int hour,
    				QMap<int, QMap<int, float> >* table,
    				QMap<int, QMap<int, int> >* count);
    boost::python::list fillMonthScriptObject(const QMap<int, QMap<int, float> >& table, const QList<int>& lim);
    void calcRepeatability(const climat::SurfAvgValues& vals, const QList<int>& lim, QMap<int, float>* monthval);
    void convertToPercent(QMap<int, float>* monthval, int count);
    bool findAeroValues(int season, int layer, const std::string& descrname, QMap<QDateTime, float>* avg);

  private:
    QApplication* _app = nullptr;

    meteo::climat::SurfDayReply*      _dayData = nullptr; //!< Данные за месяц по срокам
    meteo::climat::SurfDecadeReply*   _decadeData = nullptr; //!< Данные за месяц по декадам
    meteo::climat::SurfMonthReply*    _monthData = nullptr; //!< Данные за месяц по срокам
    meteo::climat::SurfMonthAvgReply* _monthAvgData = nullptr; //!< Средние данные за месяц
    meteo::climat::SurfYearReply*     _yearData = nullptr; //!< Данные за год по месяцам
    meteo::climat::SurfAllListReply*  _allDataList = nullptr; //!< Все данные по срокам, несколько дескрипторов
    meteo::climat::SurfDateReply*     _dateData = nullptr; //!< Дата начала/окончания явления
    meteo::climat::AeroLayerListReply* _aeroMonthData = nullptr; //!< Среднемесячные аэрологические
    meteo::climat::VkoDescription _etc;

    climat::VkoType _type = meteo::climat::kVkoCnt;
    std::string _start;
    std::string _end;
    int _month = 0;
    std::string _station;
    int _diffUtc = 0;

    bool _isOk = false;

    QString _path = QString("/tmp");
  };
  
  
} // meteo

#endif 
