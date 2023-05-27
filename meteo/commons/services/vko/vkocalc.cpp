#include "vkocalc.h"

#include <qdebug.h>
#include <qdir.h>
#include <qfile.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/settings/settings.h>

#include <commons/ui/qcustomplot/qcustomplot.h>
#include <commons/ui/qcustomplot/axiswindrose.h>

#include <commons/textproto/tprototext.h>

#include <QApplication>

using namespace meteo;

const int kTimeout = 1000000;

//TODO сроки в графиках, таблицах к локальному
//vkodt проверить когда какие облака

VkoCalc::VkoCalc()
{
  _isOk = readSettings();

  int cnt = 1;
  char* name = new char(2);
  name[0] = 'v';
  name[1] = 0;

  ::meteo::gSettings(meteo::global::Settings::instance());
  meteo::global::Settings::instance()->load();

  _app = new QApplication(cnt, &name);

  Py_Initialize();
}

VkoCalc::~VkoCalc()
{
  clearData();
  _app->deleteLater();
  _app = nullptr;
}

void VkoCalc::clearData()
{
  delete _dayData;
  _dayData = nullptr;
  
  delete _decadeData;
  _decadeData = nullptr;
  
  delete _monthData;
  _monthData = nullptr;
  
  delete _monthAvgData;
  _monthAvgData = nullptr;
  
  delete _yearData;
  _yearData = nullptr;
  
  delete _allDataList;
  _allDataList = nullptr;
  
  delete _dateData;
  _dateData = nullptr;

  delete _aeroMonthData;
  _aeroMonthData = nullptr;
}

bool VkoCalc::readSettings()
{
  TAPPLICATION_NAME("meteo");

  const QString kConfFile = MnCommon::etcPath() + "vko.conf";
  
  QFile file(kConfFile);
  if ( !file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка загрузки настроек из '%1'").arg(kConfFile);
    return false;
  }
  
  QString text = QString::fromUtf8(file.readAll());
  file.close();
    
  if ( !TProtoText::fillProto(text, &_etc) || !_etc.IsInitialized() ) {
    error_log << QObject::tr("Ошибка в структуре файла настроек");
    return false;
  }

  //  var(_etc.DebugString());

  return true;
}


bool VkoCalc::getClimatData(int type)
{
  clearData();

  _type = meteo::climat::VkoType(type);
  // var(_type);
  
  if (!isInit()) {
    error_log << QObject::tr("Ошибка инициализации");
    return false;
  }

  ::meteo::rpc::Channel* channel = ::meteo::global::serviceChannel(meteo::settings::proto::kClimat);
  if (nullptr == channel) {
    error_log << meteo::msglog::kServiceConnectFailedSimple.arg(meteo::global::serviceTitle(meteo::settings::proto::kClimat));
    return false;
  }
  
  meteo::climat::DataRequest req;
  if (!_start.empty()) {
    req.set_date_start(_start);
  }
  if (!_end.empty()) {
    req.set_date_end(_end);
  }
  
  if (_month > 0) {
    req.set_month(_month);
  }
  
  req.set_station(_station);

  int levtype = 0;
  QList<std::string> dlist = typeToDescr(_type, &levtype);
  if (dlist.isEmpty()) {
    error_log << QObject::tr("Не заданы метеопараметры для типа ВКО %1").arg(type);
    delete channel;
    return false;
  }

  for (int i = 0 ; i < dlist.size(); i++) {
    req.add_descrname(dlist.at(i));
  }
  if (0 != levtype) {
    req.set_levtype(levtype);
  }

  bool ok = requestData(&req, channel);

  delete channel;
  
  return ok;
}


//! Запрос данных из БД климата
bool VkoCalc::requestData(meteo::climat::DataRequest* req, rpc::Channel* channel)
{
  bool ok = true;

  req->set_station_type(sprinf::kStationSynop);

  //var(req->DebugString());
  
  switch (_type) {
  case climat::kVkoh:
  case climat::kVkoV: {
    _monthData = channel->remoteCall(&::meteo::climat::Service::GetSurfMonth, *req, kTimeout);
    if ( nullptr == _monthData ) {
      ok = false;
    }
  }
    break;
  case climat::kVkoC: 
  case climat::kVkodd:
  case climat::kVkoddY:
    {
      _monthAvgData = channel->remoteCall(&::meteo::climat::Service::GetSurfMonthAvg, *req, kTimeout);
      if ( nullptr == _monthAvgData ) {
	ok = false;
      }    
    }    
    break;
  case climat::kVkoR:
  case climat::kVkoYR:
  case climat::kVkoU:
  case climat::kVkoP0m:
  case climat::kVkoUm:
    _yearData = channel->remoteCall(&::meteo::climat::Service::GetSurfYear, *req, kTimeout);
    if ( nullptr == _yearData ) {
      ok = false;
    }
    break;
  case climat::kVkoff:
  case climat::kVkoffxY:    
    _yearData = channel->remoteCall(&::meteo::climat::Service::GetSurfYear, *req, kTimeout);
    if ( nullptr == _yearData ) {
      ok = false;
    }

    _dayData = channel->remoteCall(&::meteo::climat::Service::GetSurfDay, *req, kTimeout);
    if ( nullptr == _dayData ) {
      ok = false;
    }
    break;
  case climat::kVkow:
  case climat::kVkoYfx:
  case climat::kVkoYfxm:
  case climat::kVkowRY:
  case climat::kVkowm:
  case climat::kVkoRm:
    _dayData = channel->remoteCall(&::meteo::climat::Service::GetSurfDay, *req, kTimeout);
    if ( nullptr == _dayData ) {
      ok = false;
    }
    break;
  case climat::kVkoss:
  case climat::kVkossm:
    _decadeData = channel->remoteCall(&::meteo::climat::Service::GetSurfDecade, *req, kTimeout);
    if ( nullptr == _decadeData ) {
      ok = false;
    }
    break;
  case climat::kVkoT:    
  case climat::kVkoN:
  case climat::kVkodT:
  case climat::kVkohV:
  case climat::kVkoTY:
  case climat::kVkoNY:
  case climat::kVkoNm:
  case climat::kVkohm:
  case climat::kVkoVm:
  case climat::kVkohVm:
  case climat::kVkoTm:
  case climat::kVkoTgm:
    _allDataList = channel->remoteCall(&::meteo::climat::Service::GetSurfAllList, *req, kTimeout);
    if ( nullptr == _allDataList ) {
      ok = false;
    }
    break;
  case climat::kVkoff1:
  case climat::kVkot1:
  case climat::kVkoTropo:
  case climat::kVkoIsoterm:
  case climat::kVkoff1m:
  case climat::kVkoTropom:
    req->set_station_type(sprinf::kStationAero);
    _aeroMonthData = channel->remoteCall(&::meteo::climat::Service::GetAeroMonthList, *req, kTimeout);
    if (nullptr == _aeroMonthData) {
      ok = false;
    }
    break;
  case climat::kVkoYFr:
    req->set_limit(0);
    req->set_less_then(true);
    req->set_lmonth_start(8);
    req->set_lmonth_end(6);
    req->set_collection("surf_hour");
    _dateData = channel->remoteCall(&::meteo::climat::Service::GetSurfDate, *req, kTimeout);
    // var(_dateData->DebugString());
    if ( nullptr == _dateData ) {
      ok = false;
    }
    break;
  case climat::kVkoYss:
  case climat::kVkoYssm:
    req->set_limit(11);
    req->set_less_then(false);
    req->set_lmonth_start(8);
    req->set_lmonth_end(6);
    req->set_collection("surf_day");
    req->clear_month();
    _dateData = channel->remoteCall(&::meteo::climat::Service::GetSurfDate, *req, kTimeout);
    if ( nullptr == _dateData ) {
      ok = false;
    }
    break;    
    
  default: {
    debug_log << "No rule for type" << _type;
  }
  }

  return ok;
}


// QScriptValue QsClimat::calcAlexandersonTest(int month)
// {
  
//   float err = 2;
//   float cnt = 100 + month;

//   QScriptValue testRes = engine()->newArray();
//   testRes.setProperty("cnt", err);
//   testRes.setProperty("err", cnt);
//   testRes.setProperty("string", QString::number(err/cnt, 'f', 2) + "% (" + 
// 		      QString::number(err) + "/" + QString::number(cnt) + ")");

//   return testRes;
// }

// QScriptValue QsClimat::calcBuishandTest(int month)
// {
//   float err = 3;
//   float cnt = 100 + month;

//   QScriptValue testRes = engine()->newArray();
//   testRes.setProperty("cnt", err);
//   testRes.setProperty("err", cnt);
//   testRes.setProperty("string", QString::number(err/cnt, 'f', 2) + "% (" + 
// 		      QString::number(err) + "/" + QString::number(cnt) + ")");

//   return testRes;
// }

// QScriptValue QsClimat::calcPetitTest(int month)
// {
//   float err = 4;
//   float cnt = 100 + month;

//   QScriptValue testRes = engine()->newArray();
//   testRes.setProperty("cnt", err);
//   testRes.setProperty("err", cnt);
//   testRes.setProperty("string", QString::number(err/cnt, 'f', 2) + "% (" + 
// 		      QString::number(err) + "/" + QString::number(cnt) + ")");

//   return testRes;
// }

// QScriptValue QsClimat::calcFonNoimanTest(int month)
// {
//   float err = 5;
//   float cnt = 100 + month;

//   QScriptValue testRes = engine()->newArray();
//   testRes.setProperty("cnt", err);
//   testRes.setProperty("err", cnt);
//   testRes.setProperty("string", QString::number(err/cnt, 'f', 2) + "% (" + 
// 		      QString::number(err) + "/" + QString::number(cnt) + ")");

//   return testRes;
// }


// QScriptValue QsClimat::calcGraphValues(int month)
// {
//   Q_UNUSED(month);
//   QScriptValue data = engine()->newObject();

//   QScriptValue x = engine()->newArray();
//   QScriptValue y = engine()->newArray();
//   for (int i = 0; i < 10; ++i) {
//     x.setProperty(i, i);
//     y.setProperty(i, i);
//   }
  
//   data.setProperty("x",  x);
//   data.setProperty("y", y);
  
//   return data;
// }


boost::python::list VkoCalc::calcTableValues()
{
  switch (_type) {
  case climat::kVkoN:
    return calcTableCloudDays();
    break;
  case climat::kVkoC:
    return calcTableClouds();
  case climat::kVkoU:
    return calcTableHum();
  case climat::kVkoR:
    return calcTablePrecip();
  case climat::kVkoYR:
    return calcTablePrecipDays();
  case climat::kVkodd:
    return calcTableWindDir();
  case climat::kVkoff:
    return calcTableWindSpeed();
  case climat::kVkoT:
    return calcTableTemp();
  case climat::kVkow:
    return calcTableWDays();
  case climat::kVkoYfx:
    return calcTableSpeedDays();
  case climat::kVkoss:
    return calcTableSnowHeight();
  case climat::kVkoYFr:
    return calcTableFrostDate();
  case climat::kVkoYss:
    return calcTableSnowDate();
  case climat::kVkot1:
    return calcAeroTemp();
  case climat::kVkoTropo:
    return calcAeroTropo();
  case climat::kVkoIsoterm:
    return calcAeroIsoterm();

    //для общей справки
  case climat::kVkoddY:
    return calcVkoddY();
  case climat::kVkoffxY:
    return calcVkoffxY();
  case climat::kVkoNY:
    return calcVkoNY();
  case climat::kVkowRY:
    return calcVkowY();
  case climat::kVkoTY:
    return calcVkoTY();
          
    //месячные
  case climat::kVkoNm:
    return calcVkoNm();
  case climat::kVkohm:
    return calcVkohm();
  case climat::kVkoVm:
    return calcVkoVm();
  case climat::kVkohVm:
    return calcVkohVm();
    
  case climat::kVkoRm:
    return calcVkoRm();
  case climat::kVkoYfxm:
    return calcVkoYfxm();
  case climat::kVkoP0m:
    return calcVkoP0m();
  case climat::kVkowm:
    return calcVkowm();
  case climat::kVkoTm:
    return calcVkoTm();
  case climat::kVkoUm:
    return calcVkoUm();   
  case climat::kVkoff1m:
    return calcVkoff1m();
  case climat::kVkoTropom:
    return calcVkoTropom();
  case climat::kVkoTgm:
    return calcVkoTgm();
  case climat::kVkossm:
    return calcVkossm();
  case climat::kVkoYssm:
    return calcVkoYssm();    
    
  default: {
    debug_log << QObject::tr("Не указан обрабочик");
  }
  }

  boost::python::list res;
  return res;
}

boost::python::list VkoCalc::calcTableParamValues(int val1, int val2)
{
  switch (_type) {
  case climat::kVkoff1:
    return calcTableAeroWind(val1, val2); //season, layer
  case climat::kVkoh:
  case climat::kVkoV:
    return calcTableSurfWithLimit(val1);//month
  case climat::kVkohV:
    return calcVngoGdv(val1, val2); //month, hour
  case climat::kVkodT:
    return calcTableDailyRateTemp(val1, 21-_diffUtc); //month, hour
  default: {
    debug_log << QObject::tr("Не указан обрабочик");
  }
  }

  boost::python::list res;
  return res;
}

boost::python::list VkoCalc::calcTableClouds()
{
  boost::python::list res;
  
  if (_monthAvgData == 0 || !_monthAvgData->IsInitialized()) {
    return res;
  }

  if (_monthAvgData->has_error()) {
    error_log << QString::fromStdString(_monthAvgData->error());
    return res;
  }

  QList<int> lim;
  lim << 7 << 6 << 5 << 4 << 3 << 2 << 1 << 0;

  QMap<int, QMap<int, float> > table; //месяц, (категория, количество)
  for (int idx = 0; idx < _monthAvgData->val_size(); idx++) {
    const climat::SurfAvgValues& vals = _monthAvgData->val(idx);
    QMap<int, float> monthval;
    
    calcRepeatability(vals, lim, &monthval);
    convertToPercent(&monthval, vals.avg_size());
    table.insert(vals.month(), monthval);
  }

  //var(table);
  
  return fillMonthScriptObject(table, lim);
}

boost::python::list VkoCalc::calcTableWindDir()
{
  boost::python::list res;
  
  if (_monthAvgData == 0 || !_monthAvgData->IsInitialized()) {
    return res;
  }
  
  if (_monthAvgData->has_error()) {
    error_log << QString::fromStdString(_monthAvgData->error());
    return res;
  }
  
  //var(_monthAvgData->DebugString());

  QList<int> lim;
  lim << 8 << 7 << 6 << 5 << 4 << 3 << 2 << 1 << 0;
  QList<int> fillOrder;
  fillOrder << 0 << 7 << 6 << 5 << 4 << 3 << 2 << 1 << 8; //обратный, т.к. в функции fillMonthScriptObject так
  
  QMap<int, QMap<int, float> > table; //месяц, (категория, количество)
  for (int idx = 0; idx < _monthAvgData->val_size(); idx++) {
    const climat::SurfAvgValues& vals = _monthAvgData->val(idx);
    QMap<int, float> monthval;

    calcRepeatability(vals, lim, &monthval);
    int count = vals.avg_size() - monthval.value(0); //без штиля
    //debug_log << monthval << count << vals.avg_size();

    QMap<int, float>::iterator it = monthval.begin();
    while (it != monthval.end()) {
      if (it.key() == 0) {
	if (vals.avg_size() == 0) {
	  it.value() = 0;
	} else {
	  it.value() = it.value() / vals.avg_size() * 100;
	} 
      } else {
	if (count <= 0) {
	  it.value() = 0;
	} else {
	  it.value() = it.value() / count * 100;
	}	       
      }
      ++it;
    }

    table.insert(vals.month(), monthval);
  }

  //var(table);
  
  QMapIterator<int, QMap<int, float>> it(table);
  while (it.hasNext()) {
    it.next();
    createGraphWind(it.value(), it.key());
  }

  return fillMonthScriptObject(table, fillOrder);
}

void VkoCalc::createGraphWind(const QMap<int, float>& table, int month)
{
  QCustomPlot* customPlot = new QCustomPlot;
  customPlot->resize(640, 640);
  
  AxisWindRose* axes = new AxisWindRose(customPlot);
  if (table.contains(0)) {
    axes->setCalm(table.value(0));
  } else {
    axes->setCalm(0);
  }	  
  axes->setCurve(table);
  
  customPlot->savePng(_path + QString("/vkodd_") + QString::number(month) + ".png");
  
  delete customPlot;  
}

boost::python::list VkoCalc::calcTableWindSpeed()
{
  boost::python::list res;
  
  if (_yearData == 0 || !_yearData->IsInitialized() ||
      _dayData == 0  || !_dayData->IsInitialized()) {
    return res;
  }
  
  if (_yearData->has_error() || _dayData->has_error()) {
    error_log << QString::fromStdString(_yearData->error());
    return res;
  }

  QMap<int, ClimParam> table; //<месяц, параметры>
  for (int idx = 0; idx < _yearData->val_size(); idx++) {
    const climat::SurfMonthValues& vals = _yearData->val(idx);
    ClimParam param;
    for (int hidx = 0; hidx < vals.avglist_size(); hidx++) {
      param.avg += vals.avglist(hidx).avg();
    }
    param.avg /= vals.avglist_size();
    //debug_log<< vals.month() << param.min  << param.minYear << param.avg << param.max << param.maxYear;

    table.insert(vals.month(), param);
  }

  for (int idx = 0; idx < _dayData->val_size(); idx++) {
    const climat::SurfDayValues& vals = _dayData->val(idx);
    float max = 0;
    for (int yIdx = 0; yIdx < vals.avglist_size(); yIdx++) {
      for (int hidx = 0; hidx < vals.avglist(yIdx).avg_size(); hidx++) {
	if (max < vals.avglist(yIdx).avg(hidx)) {
	  max = vals.avglist(yIdx).avg(hidx);
	}
      }
    }
    ClimParam& param = table[vals.month()];
    param.max = max;
  }

  boost::python::list trow1;
  //Object::tr("Средняя"));
  for (int mIdx = 1; mIdx <= 12; mIdx++) { //месяц
    if (table.contains(mIdx)) {
      float val = round(table.value(mIdx).avg * 10) / 10;
      trow1.append(QString::number(val).toStdString());
    } else {
      trow1.append(std::string(""));
    }
  }
  res.append(trow1);

  boost::python::list trow2;
  //QObject::tr("Максимальная"));
  for (int mIdx = 1; mIdx <= 12; mIdx++) { //месяц
    if (table.contains(mIdx)) {
      float val = round(table.value(mIdx).max * 10) / 10;
      trow2.append(QString::number(val).toStdString());
    } else {
      trow2.append(std::string(""));
    }
  }
  res.append(trow2);
  
  return res;
}


boost::python::list VkoCalc::calcTableSurfWithLimit(int month)
{
  boost::python::list table;
  if (_monthData == 0 || !_monthData->IsInitialized()) {
    error_log << QObject::tr("Нет данных");
    return table;
  }

  if (_monthData->has_error()) {
    error_log << QString::fromStdString(_monthData->error());
    return table;
  }
  // var(_monthData->DebugString());
  // var(_type);

  QList<int> lim;
  if (_type == climat::kVkoh) {
    lim << 2500 << 2000 << 1000 << 600 << 300 << 200 << 100 << 50 << 0;
  } else if (_type == climat::kVkoV) {
    lim << 10000 << 4000 << 2000 << 1000 << 500 << 200 << 50 << 0;
  } else {
    return table;
  }

  int idx = -1;
  for (int i = 0; i < _monthData->val_size(); i++) {
    if (_monthData->val(i).month() == month) {
      idx = i;
      break;
    }
  }

  if (idx == -1) {
    return table;
  }

  //  var(_monthData->DebugString());

  const climat::SurfHourValues& vals = _monthData->val(idx); //один месяц

  for (int row = 0; row < vals.avglist_size(); ++row) {
    int allCount = 0;
    QMap<int, float> res; //предел, сумма
    for (int avgIdx = 0; avgIdx < vals.avglist(row).avg_size(); ++avgIdx) {
      for (int limIdx = 0; limIdx < lim.count(); limIdx++) {
	if (vals.avglist(row).avg(avgIdx) >= lim.at(limIdx)) {
	  ++allCount;
	  if (!res.contains(lim.at(limIdx))) {
	    res.insert(lim.at(limIdx), 1);
  	  } else {
	    res[lim.at(limIdx)] += 1;
	  }
	  break;
	}
      }
    }

    convertToPercent(&res, allCount);
    //var(res);

    boost::python::list trow;
    for (int limIdx = lim.count() - 1; limIdx >=0; limIdx--) {
      if (!res.contains(lim.at(limIdx))) {
	trow.append("");
      } else {
	float val = round(res.value(lim.at(limIdx)) * 10 ) / 10;
	trow.append(QString::number(val).toStdString());
      }
    }
    table.append(trow);
  }

  return table;
}

boost::python::list VkoCalc::calcVngoGdv(int month, int hour)
{
  boost::python::list res;
  if (_allDataList == 0 || !_allDataList->IsInitialized() ||
      _allDataList->all_size() < 2) {
    error_log << QObject::tr("Нет данных");
    return res;
  }
  
  //var(_allDataList->DebugString());  
  
  if (_allDataList->has_error()) {
    error_log << QString::fromStdString(_allDataList->error());
    return res;
  }
  
  QList<int> limh;
  limh << 601 << 400 << 300 << 200 << 100 << 60 << 50 << 30 << 0;
  QList<int> limV;
  limV << 6000 << 4000 << 3000 << 2000 << 1000 << 800 << 500 << 400 << 0;

  QMap<QDateTime, int> h; //дата, предел в который попало значение (>= предел)
  QMap<QDateTime, int> V;
  
  for (int m = 0; m < _allDataList->all(1).val_size(); m++) {
    if (_allDataList->all(1).val(m).month() == month) {
      for (int idx = 0; idx < _allDataList->all(1).val(m).avglist_size(); idx++) {
	if (_allDataList->all(1).val(m).avglist(idx).hour() == hour) {
	  const climat::SurfAllValuesList& v= _allDataList->all(1).val(m).avglist(idx);
	  for (int limIdx = 0; limIdx < limh.count(); limIdx++) {
	    if (v.avg() >= limh.at(limIdx)) {
	      h.insert(QDateTime(QDate(v.year(), month, v.day()), QTime(v.hour(), 0)), limh.at(limIdx));
	      break;
	    }
	  }
	}
      }
    }
  }
  
  for (int m = 0; m < _allDataList->all(0).val_size(); m++) {
    if (_allDataList->all(0).val(m).month() == month) {
      for (int idx = 0; idx < _allDataList->all(0).val(m).avglist_size(); idx++) {
	if (_allDataList->all(0).val(m).avglist(idx).hour() == hour) {
	  const climat::SurfAllValuesList& v= _allDataList->all(0).val(m).avglist(idx);
	  for (int limIdx = 0; limIdx < limV.count(); limIdx++) {
	    if (v.avg() >= limV.at(limIdx)) {
	      V.insert(QDateTime(QDate(v.year(), month, v.day()), QTime(v.hour(), 0)), limV.at(limIdx));
	      break;
	    }
	  }
	}
      }
    }
  }
 
  QMap<int, QMap<int, float> > table; //предел ВНГО, (предел ГДВ, повторяемость)
  int cnt = 0;
  
  QMap<QDateTime, int>::iterator it = h.begin();
  while (it != h.end()) {
    if (V.contains(it.key())) {
      QMap<int, float>& Vtable = table[it.value()];
      if (Vtable.contains(V.value(it.key()))) {
	Vtable[V.value(it.key())] += 1;
      } else {
	Vtable.insert(V.value(it.key()), 1);
      }
      ++cnt;      
    }
    ++it;
  }

  //var(table);

  // QStringList htext;
  // htext << "<30" << "30-49" << "50-59" << "60-99" << "100-199" << "200-299"
  // 	<< "300-399" << "400-600" << ">600";

  for (int limhIdx = limh.count() - 1; limhIdx >= 0; limhIdx--) {
    boost::python::list trow;
    for (int limIdx = limV.count() - 1; limIdx >= 0; limIdx--) {
      if (cnt != 0) {
	float val = table.value(limh.at(limhIdx)).value(limV.at(limIdx));
	if (qFuzzyIsNull(val)) {
	  trow.append("");
	} else {
	  val = round((val * 100 / cnt) * 10) / 10;
	  trow.append(QString::number(val).toStdString());
	}
      } else {
	trow.append("");
      }
    }
    res.append(trow);
  }

  return res;
}

boost::python::list VkoCalc::calcTableHum()
{
  boost::python::list res;
  if (_yearData == 0 || !_yearData->IsInitialized()) {
    return res;
  }
  
  if (_yearData->has_error()) {
    error_log << QString::fromStdString(_yearData->error());
    return res;
  }

  QList<int> allHours;
  allHours << 0 << 3 << 6 << 9 << 12 << 15 << 18 << 21;

  //var(_yearData->DebugString());

  QMap<int, QMap<int, float> > table; //месяц, (срок, значение)
  bool errFlag = false;

  for (int idx = 0; idx < _yearData->val_size(); idx++) {
    const climat::SurfMonthValues& vals = _yearData->val(idx);
    QMap<int, float> monthval; //срок, среднее

    for (int hour = 0; hour < allHours.count(); hour++) {
      float avg = 0;
      float cnt = 0;
      for (int hidx = 0; hidx < vals.avglist_size(); hidx++) {
	if (!vals.avglist(hidx).has_hour()) {
	  errFlag = true;
	  continue;
	}
      
	if (vals.avglist(hidx).hour() == allHours.at(hour)) {
	  avg += vals.avglist(hidx).avg();
	  ++cnt;
	}
      }
      if (cnt != 0) {
	monthval.insert(allHours.at(hour), avg/cnt);
      } else {
	//monthval.insert(allHours.at(hour), 0);
      }
    }

    table.insert(_yearData->val(idx).month(), monthval);
  }
  
  if (errFlag) {
    error_log << QObject::tr("Часть данных не содержит информации о сроке");
  }

  //var(table);

  for (int row = 0; row < allHours.count(); ++row) {
    boost::python::list trow;
    for (int mIdx = 1; mIdx <= 12; mIdx++) { //месяц
      if (table.contains(mIdx) && table.value(mIdx).contains(allHours.at(row))) {
	float val = round(table.value(mIdx).value(allHours.at(row)) * 10 ) / 10;
	trow.append(QString::number(val).toStdString());
      } else {
	trow.append(std::string(""));
      }
    }
    res.append(trow);
  }
  
  return res;
}


//! Количество осадков по среднемесячным данным
boost::python::list VkoCalc::calcTablePrecip()
{
  boost::python::list res;
  
  if (_yearData == 0 || !_yearData->IsInitialized()) {
    return res;
  }

  if (_yearData->has_error()) {
    error_log << QString::fromStdString(_yearData->error());
    return res;
  }

  //var(_yearData->DebugString());
  
  QMap<int, ClimParam> table; //<месяц, параметры>
  for (int idx = 0; idx < _yearData->val_size(); idx++) {
    const climat::SurfMonthValues& vals = _yearData->val(idx);
    ClimParam param;
    for (int hidx = 0; hidx < vals.avglist_size(); hidx++) {
      param.avg += vals.avglist(hidx).avg();
      if (param.max < vals.avglist(hidx).avg() || param.maxYear == 0) {
	param.max = vals.avglist(hidx).avg();
	param.maxYear = vals.avglist(hidx).year();
      }
      if (param.min > vals.avglist(hidx).avg() || param.minYear == 0) {
	param.min = vals.avglist(hidx).avg();
	param.minYear = vals.avglist(hidx).year();
      }
    }
    param.avg /= vals.avglist_size();
    //debug_log<< vals.month() << param.min  << param.minYear << param.avg << param.max << param.maxYear;

    table.insert(vals.month(), param);
  }

  createGraphPrecip(table);
  return fillTablePrecip(table);
}

boost::python::list VkoCalc::fillTablePrecip(const QMap<int, ClimParam>& table)
{
  boost::python::list res;
  
  //var(table.keys());
  boost::python::list trow1;
  //"Среднее"
  for (int mIdx = 1; mIdx <= 12; mIdx++) { //месяц
    if (table.contains(mIdx)) {
      float val = round(table.value(mIdx).avg * 100) / 100;
      trow1.append(QString::number(val).toStdString());
    } else {
      trow1.append("");
    }
  }
  res.append(trow1);

  boost::python::list trow2Val, trow2Year;
  //"Наибольшее\nГод"
  for (int mIdx = 1; mIdx <= 12; mIdx++) { //месяц
    if (table.contains(mIdx)) {
      float val = round(table.value(mIdx).max * 10) / 10;
      trow2Val.append(QString::number(val).toStdString());
      trow2Year.append(QString::number(table.value(mIdx).maxYear).toStdString());
    } else {
      trow2Val.append("");
      trow2Year.append("");
    }
  }
  res.append(trow2Val);
  res.append(trow2Year);

  boost::python::list trow3Val, trow3Year;
  //Наименьшее\nГод
  for (int mIdx = 1; mIdx <= 12; mIdx++) { //месяц
    if (table.contains(mIdx)) {
      float val = round(table.value(mIdx).min * 10 ) / 10;
      trow3Val.append(QString::number(val).toStdString());
      trow3Year.append(QString::number(table.value(mIdx).minYear).toStdString());
    } else {
      trow3Val.append("");
      trow3Year.append("");
    }
  }
  res.append(trow3Val);
  res.append(trow3Year);

  return res;
}

void VkoCalc::createGraphPrecip(const QMap<int, ClimParam>& table)
{
  QCustomPlot* customPlot = new QCustomPlot;
  customPlot->resize(800, 600);
  
  QVector<double> x(12), yavg(12), ymax(12), ymin(12);
  float max = 0;
  
  for (int idx = 0; idx < 12; idx++) {
    x[idx] = idx + 1;
    if (table.contains(idx + 1)) {
      ymin[idx] = table.value(idx + 1).min;
      yavg[idx]  = table.value(idx + 1).avg - table.value(idx + 1).min;
      ymax[idx] = table.value(idx + 1).max - table.value(idx + 1).avg;   
      if (max < table.value(idx + 1).max) {
	max = table.value(idx + 1).max;
      }
    }
  }
 
  QCPBars *barsmax = new QCPBars(customPlot->xAxis, customPlot->yAxis);
  barsmax->setName(QObject::tr("Наибольшее"));
  barsmax->setData(x, ymax);
  barsmax->setBrush(QColor(0, 200, 200, 130));
  barsmax->rescaleAxes(false);
  
  QCPBars *barsavg = new QCPBars(customPlot->xAxis, customPlot->yAxis);
  barsavg->setName(QObject::tr("Среднее"));
  barsavg->setData(x, yavg);
  barsavg->setBrush(QColor(0, 100, 230, 200));
   
  QCPBars *barsmin = new QCPBars(customPlot->xAxis, customPlot->yAxis);
  barsmin->setName(QObject::tr("Наименьшее"));
  barsmin->setData(x, ymin);
  barsmin->setBrush(QColor(0, 50, 255, 220));

  barsavg->moveAbove(barsmin);
  barsmax->moveAbove(barsavg);

  barsavg->setStackingGap(-1);
  barsmin->setStackingGap(-1);
  barsmax->setStackingGap(-1);
  
  customPlot->legend->setVisible(true);
  customPlot->setAutoAddPlottableToLegend(true);

  customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignTop); 
  QCPLayoutGrid *subLayout = new QCPLayoutGrid;
  customPlot->plotLayout()->addElement(1, 0, subLayout);
  subLayout->setMargins(QMargins(5, 0, 5, 5));
  subLayout->addElement(0, 0, customPlot->legend);
  customPlot->legend->setFillOrder(QCPLegend::foColumnsFirst);
  customPlot->plotLayout()->setRowStretchFactor(1, 0.001);
   
  customPlot->xAxis->setLabel(QObject::tr("Месяц"));
  customPlot->yAxis->setLabel(QObject::tr("Количество осадков, мм"));

  customPlot->xAxis->setSubTicks(false);
  QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
  customPlot->xAxis->setTicker(fixedTicker);
  fixedTicker->setTickStep(1.0); 
  fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);
  
  customPlot->xAxis->setRange(0.5, 12.5);
  //  customPlot->xAxis->setTickVector(x);
  customPlot->yAxis->setRange(0, max + 1);
  
  customPlot->savePng(_path + "/vkoR.png");
  
  delete customPlot;
}

boost::python::list VkoCalc::calcTablePrecipDays()
{
  boost::python::list res;
  if (_yearData == 0 || !_yearData->IsInitialized()) {
    return res;
  }

  if (_yearData->has_error()) {
    error_log << QString::fromStdString(_yearData->error());
    return res;
  }

  QMap<int, float> table; //<месяц, среднее>
  for (int idx = 0; idx < _yearData->val_size(); idx++) {
    const climat::SurfMonthValues& vals = _yearData->val(idx);
    float avg = 0;
    for (int hidx = 0; hidx < vals.avglist_size(); hidx++) {
      avg += vals.avglist(hidx).avg();
    }
    avg /= vals.avglist_size();

    table.insert(vals.month(), avg);
  }

  //var(table);

  boost::python::list trow;
  for (int mIdx = 1; mIdx <= 12; mIdx++) { //месяц
    if (table.contains(mIdx)) {
      float val = round(table.value(mIdx) * 10) / 10;
      trow.append(QString::number(val).toStdString());
    } else {
      trow.append("");
    }
  }
  res.append(trow);

  return res;
}

boost::python::list VkoCalc::calcTableWDays()
{
  boost::python::list res;
  
  if (_dayData == 0  || !_dayData->IsInitialized()) {
    return res;
  }

  if (_dayData->has_error()) {
    error_log << QString::fromStdString(_dayData->error());
    return res;
  }
  //гроза (0x1), туман (0x2), метель (0x4), град (0x8), гололед (0x10), пыль (0x20)
  QList<int> lim;
  lim << 0 << 1 << 2 << 3 << 4 << 5;

  for (int lidx = 0; lidx < lim.count(); lidx++) {
    QMap<int, ClimParam> table; //месяц, параметры
    for (int idx = 0; idx < _dayData->val_size(); idx++) {
      ClimParam param;
      const climat::SurfDayValues& vals = _dayData->val(idx);

      for (int yIdx = 0; yIdx < vals.avglist_size(); yIdx++) {
	param.count = 0;//число дней в месяце с явлением
	for (int hidx = 0; hidx < vals.avglist(yIdx).avg_size(); hidx++) {
	  int mask = 1 <<  lim.at(lidx);
	  if (0 != ((int)vals.avglist(yIdx).avg(hidx) & mask)) {
	    param.count += 1;
	  }
	}
	if (param.max < param.count || param.maxYear == 0) {
	  param.max = param.count;
	  param.maxYear = vals.avglist(yIdx).year();
	}
	param.avg += param.count;
      }
      param.avg /= vals.avglist_size();
      
      //debug_log << vals.month() << param.avg << param.max;
      table.insert(vals.month(), param);
    }

    //var(table.keys());
    
    boost::python::list trow1;
    boost::python::list trow2;
    for (int mIdx = 1; mIdx <= 12; mIdx++) { //месяц
      if (table.contains(mIdx)) {
	float val = round(table.value(mIdx).avg * 10) / 10;
	if (qFuzzyIsNull(val)) {
	  trow1.append("");
	} else {
	  trow1.append(QString::number(val).toStdString());
	}
	if (qFuzzyIsNull(table.value(mIdx).max)) {
	  trow2.append("");	  
	} else {
	  trow2.append(QString::number(table.value(mIdx).max, 'f', 0).toStdString());
	}
      } else {
	trow1.append("");
	trow2.append("");
      }
    }
    res.append(trow1);
    res.append(trow2);
  }

  return res;
}

boost::python::list VkoCalc::calcTableSpeedDays()
{
  boost::python::list res;
  
  if (_dayData == 0 || !_dayData->IsInitialized()) {
    return res;
  }

  if (_dayData->has_error()) {
    error_log << QString::fromStdString(_dayData->error());
    return res;
  }

  QList<int> lim;
  lim << 10 << 15 << 20;

  int row = 1;

  for (int lidx = 0; lidx < lim.count(); lidx++) {
    QMap<int, ClimParam> table; //месяц, параметры
    for (int idx = 0; idx < _dayData->val_size(); idx++) {
      ClimParam param;
      const climat::SurfDayValues& vals = _dayData->val(idx);

      for (int yIdx = 0; yIdx < vals.avglist_size(); yIdx++) {
	param.count = 0; //число дней в месяце с явлением
	for (int hidx = 0; hidx < vals.avglist(yIdx).avg_size(); hidx++) {
	  if (vals.avglist(yIdx).avg(hidx) > lim.at(lidx)) {
	    param.count += 1;
	  }
	}
	if (param.max < param.count || param.maxYear == 0) {
	  param.max = param.count;
	  param.maxYear = vals.avglist(yIdx).year();
	}
	param.avg += param.count;
	// if (idx == 0) {
	//   debug_log <<  param.count << param.avg << param.max;
	// }
      }
      param.avg /= vals.avglist_size();
      // if (idx == 0) {
      // 	debug_log << param.avg;
      // }
      
      table.insert(vals.month(), param);
    }

    boost::python::list trow1;
    boost::python::list trow2;
    for (int mIdx = 1; mIdx <= 12; mIdx++) { //месяц
      if (table.contains(mIdx)) {
	float val = round(table.value(mIdx).avg * 10) / 10;
	if (qFuzzyIsNull(val)) {
	  trow1.append(std::string(""));
	} else {
	  trow1.append(QString::number(val).toStdString());
	}
	if (qFuzzyIsNull(table.value(mIdx).max)) {
	  trow2.append(std::string(""));
	} else {
	  trow2.append(QString::number(table.value(mIdx).max, 'f', 0).toStdString());
	}
      } else {
	trow1.append(std::string(""));
	trow2.append(std::string(""));
      }
    }
    res.append(trow1);
    res.append(trow2);
    row += 2;  
  }

  return res;
}

boost::python::list VkoCalc::calcTableTemp()
{
  boost::python::list res;
  
  if (_allDataList == 0 || !_allDataList->IsInitialized() ||
      _allDataList->all_size() == 0) {
    return res;
  }
  
  if (_allDataList->has_error()) {
    error_log << QString::fromStdString(_allDataList->error());
    return res;
  } 
  //var(_allDataList->DebugString());

  // QStringList categ;
  // categ << QObject::tr("Средняя месячная") << QObject::tr("Средняя максимальная") 
  // 	  << QObject::tr("Средняя минимальная") << QObject::tr("Абсолютный максимум") 
  // 	<< QObject::tr("Абсолютный минимум");
  int categCnt = 5;

  QMap<int, QMap<int, float> > table; //месяц, (категория, количество)
  for (int idx = 0; idx < _allDataList->all(0).val_size(); idx++) {
    const climat::SurfMonthAllValues& vals = _allDataList->all(0).val(idx);

    QMap<int, ClimParam> yearval;//год, параметры
    for (int hidx = 0; hidx < vals.avglist_size(); hidx++) {
      ClimParam& param = yearval[vals.avglist(hidx).year()];
      ++param.count;
      param.avg += vals.avglist(hidx).avg();
      if (param.max < vals.avglist(hidx).avg() || param.maxYear == 0) {
	param.max = vals.avglist(hidx).avg();
	param.maxYear = vals.avglist(hidx).year();
      }
      if (param.min > vals.avglist(hidx).avg() || param.minYear == 0) {
	param.min = vals.avglist(hidx).avg();
	param.minYear = vals.avglist(hidx).year();
      }
    }

    QMap<int, float> monthval;
    for (int c = 0; c < categCnt; c++) {
      monthval.insert(c, 0);
    }
    monthval[3] = monthval[4] = -999;
    
    QMap<int, ClimParam>::iterator it = yearval.begin();
    while (it != yearval.end()) {
      it.value().avg /= it.value().count;
      monthval[0] += it.value().avg;
      monthval[1] += it.value().max;
      monthval[2] += it.value().min;
      if (monthval[3] < it.value().max) {
	monthval[3] = it.value().max;
      }
      if (monthval[4] > it.value().min || monthval[4] == -999) {
	monthval[4] = it.value().min;
      }
      ++it;
    }
    if (yearval.count() != 0) {
      monthval[0] /= yearval.count();
      monthval[1] /= yearval.count();
      monthval[2] /= yearval.count();
      
      table.insert(vals.month(), monthval);
    }
  }

  // var(table);
  createGraphTemp(table);

  for (int row = 0; row < categCnt; row++) {
    boost::python::list trow;
    for (int mIdx = 1; mIdx <= 12; mIdx++) { //месяц
      if (table.contains(mIdx)) {
	float val = round(table.value(mIdx).value(row) * 100) / 100;
	trow.append(QString::number(val).toStdString());
      } else {
	trow.append(std::string(""));
      }
    }
    res.append(trow);
  }
 
  return res;
}

void VkoCalc::createGraphTemp(const QMap<int, QMap<int, float> >& table)
{
  QCustomPlot* customPlot = new QCustomPlot;
  customPlot->resize(800, 600);
  
  QStringList names;
  names << QObject::tr("Средняя") << QObject::tr("Средняя максимальная") 
	<< QObject::tr("Средняя минимальная");

  QVector<double> x(12);
  QList<QVector<double>> y;
  y << QVector<double>(12) << QVector<double>(12) << QVector<double>(12);

  for (int categ = 0; categ < 3; categ++) {
    for (int idx = 0; idx < 12; idx++) {
      x[idx] = idx + 1;
      if (table.contains(idx + 1)) {
	y[categ][idx] = table.value(idx + 1).value(categ);
      }
    }
  }

  for (int categ = 0; categ < 3; categ++) {
    customPlot->addGraph();
    customPlot->graph()->setName(names.at(categ));
    customPlot->graph()->setLineStyle(QCPGraph::lsLine);
    customPlot->graph()->setData(x, y.at(categ));
    customPlot->graph()->rescaleAxes(true);
  }

  customPlot->graph(0)->setPen(QColor(0, 180, 30));
  customPlot->graph(1)->setPen(QColor(200, 50, 50));
  customPlot->graph(2)->setPen(QColor(0, 50, 200));
  customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
  customPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDiamond, 6));
  customPlot->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross , 6));

  //легенда
  customPlot->legend->setVisible(true);
  customPlot->setAutoAddPlottableToLegend(true);
  customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignTop); 
  QCPLayoutGrid *subLayout = new QCPLayoutGrid;
  customPlot->plotLayout()->addElement(1, 0, subLayout);
  subLayout->setMargins(QMargins(5, 0, 5, 5));
  subLayout->addElement(0, 0, customPlot->legend);
  customPlot->legend->setFillOrder(QCPLegend::foColumnsFirst);
  customPlot->plotLayout()->setRowStretchFactor(1, 0.001);

  //оси
  customPlot->xAxis->setLabel(QObject::tr("Месяц"));
  customPlot->yAxis->setLabel(QObject::tr("Температура, °С"));  

  customPlot->xAxis->setSubTicks(false);
  QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
  fixedTicker->setTickStep(1.0); 
  fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);
  customPlot->xAxis->setTicker(fixedTicker);
  customPlot->xAxis->setRange(0, 12.1);
  
  customPlot->yAxis->scaleRange(1.1, customPlot->yAxis->range().center());
    
  customPlot->savePng(_path + "/vkoT.png");
  
  delete customPlot;
}


boost::python::list VkoCalc::calcTableSnowHeight()
{
  boost::python::list res;
  
  if (_decadeData == 0 || !_decadeData->IsInitialized()) {
    error_log << QObject::tr("Нет данных");
    return res;
  }

  if (_decadeData->has_error()) {
    error_log << QString::fromStdString(_decadeData->error());
    return res;
  }

  QList<int> month;
  month << 10 << 11 << 12 << 1 << 2 << 3 << 4 << 5;

  //var(_decadeData->DebugString());
  
  QMap<int, QList<float> > table;//месяц, высоты по декадам
  for (int idx = 0; idx < _decadeData->val_size(); idx++) {
    const climat::SurfDecadeValues& vals = _decadeData->val(idx);
    QList<float> decade; //сумма
    QList<int> count; //количество ненулевых
    decade << 0 << 0 << 0;
    count << 0 << 0 << 0;

    for (int hIdx = 0; hIdx < vals.avglist_size(); hIdx++) {
      if (vals.avglist(hIdx).decade() > 3) continue;
      if (vals.avglist(hIdx).avg() != 0) {
	count[vals.avglist(hIdx).decade() - 1] += 1;
	decade[vals.avglist(hIdx).decade() - 1] += vals.avglist(hIdx).avg();
      } 
    }
    
    for (int i = 0; i < count.size(); i++) {
      if (count.at(i) < vals.avglist_size() / 3 / 2 || count[i] == 0) { //1 одна декада, 50 %
	decade[i] = 0;
      } else {
	decade[i] = round (decade[i] / count[i]);
      }
    }
    
    table.insert(vals.month(), decade);
  }

  //var(table);

  boost::python::list trow;
  for (int mIdx = 0; mIdx < month.count(); mIdx++) { //месяц
    for (int d = 0 ; d < 3; d++) {
      if (!table.contains(month.at(mIdx)) || table.value(month.at(mIdx)).at(d) <= 0) {
	trow.append(std::string("."));
      } else {
	trow.append(QString::number(table.value(month.at(mIdx)).at(d), 'f', 0).toStdString());
      }
    }
  }
  res.append(trow);

  return res;
}

boost::python::list VkoCalc::calcTableCloudDays()
{
  boost::python::list table;
  
  if (_allDataList == 0 || !_allDataList->IsInitialized() ||
      _allDataList->all_size() < 2) {
    error_log << QObject::tr("Нет данных");
    return table;
  }

  //var(_allDataList->DebugString());  

  if (_allDataList->has_error()) {
    error_log << QString::fromStdString(_allDataList->error());
    return table;
  }

  QMap<int, QMap<int, float> > N; //месяц, (категория, число дней)
  QMap<int, QMap<int, float> > Nh;
  calcCloudsDay(&N,  _allDataList->all(0));
  calcCloudsDay(&Nh, _allDataList->all(1));

  // var(N);
  // var(Nh);
  
  for (int mIdx = 0; mIdx < 12; mIdx++) { //месяц
    boost::python::list trow;
    if (N.contains(mIdx)) {
      for (int limIdx = 0; limIdx < N.value(mIdx).count(); limIdx++) {
	float val = round(N.value(mIdx).value(limIdx) * 10) / 10;
	trow.append(QString::number(val).toStdString());
      }
    } else {
      for (int limIdx = 0; limIdx < 4; limIdx++) {
	trow.append("");
      }
    }
    if (Nh.contains(mIdx)) {
      for (int limIdx = 0; limIdx < Nh.value(mIdx).count(); limIdx++) {
	float val = round(Nh.value(mIdx).value(limIdx) * 10 ) / 10;
	trow.append(QString::number(val).toStdString());
      }
    } else {
      for (int limIdx = 0; limIdx < 4; limIdx++) {
	trow.append("");
      }
    }
    
    table.append(trow);
  }
  
  return table;
}

void VkoCalc::calcCloudsDay(QMap<int, QMap<int, float> >* table, const climat::SurfMonthAllReply& data)
{
  for (int mIdx = 0; mIdx < data.val_size(); mIdx++) {
    const climat::SurfMonthAllValues& vals = data.val(mIdx);
    // var(vals.DebugString());
   
    QMap<int, QMap<int, QPair<ClimAccum, ClimAccum> > > yearval; //год, день, (сумма баллов днем, сумма балов ночью)
    for (int hidx = 0; hidx < vals.avglist_size(); hidx++) {
      QMap<int, QPair<ClimAccum, ClimAccum> >& day = yearval[vals.avglist(hidx).year()];
      if (!day.contains(vals.avglist(hidx).day())) {
	day.insert(vals.avglist(hidx).day(), QPair<ClimAccum, ClimAccum>());
      }      
      int hour = vals.avglist(hidx).hour() + _diffUtc; //9, 12, 15, 18 по локальному времени - день
      if (hour >= 7.5 && hour < 19.5) {
	day[vals.avglist(hidx).day()].first.sum += vals.avglist(hidx).avg();
	day[vals.avglist(hidx).day()].first.cnt += 1;
      } else {
	day[vals.avglist(hidx).day()].second.sum += vals.avglist(hidx).avg();
	day[vals.avglist(hidx).day()].second.cnt += 1;
      }
    }

    QMap<int, float> monthval; //день ясно, день пасмурно, ночь ясно, ночь пасмурно
    for (int c = 0; c < 4; c++) {
      monthval.insert(c, 0);
    }
    QMap<int, QMap<int, QPair<ClimAccum, ClimAccum> > >::iterator it = yearval.begin();
    while (it != yearval.end()) {
      QMap<int, QPair<ClimAccum, ClimAccum> >::iterator dit = it.value().begin();
      while (dit != it.value().end()) {
	if (dit.value().first.cnt != 0) {
	  float avg = dit.value().first.sum / dit.value().first.cnt;
	  if (avg <= 3) { //12 за 4 срока
	    monthval[0] += 1;
	  } else if (avg >= 8) { //32 за 4 срока
	    monthval[1] += 1;
	  }
	}
	if (dit.value().second.cnt != 0) {
	  float avg = dit.value().second.sum / dit.value().second.cnt;
	  if (avg <= 3) { //12 за 4 срока
	    monthval[2] += 1;
	  } else if (avg >= 8) { //32 за 4 срока
	    monthval[3] += 1;
	  }
	}
	++dit;
      }
      ++it;
    }
    for (int c = 0; c < 4; c++) {
      monthval[c] /= yearval.count();
    }

    table->insert(vals.month() - 1, monthval);
  }
}


boost::python::list VkoCalc::calcTableDailyRateTemp(int month, int hour)
{
  boost::python::list res;
  
  if (_allDataList == 0 || !_allDataList->IsInitialized() ||
      _allDataList->all_size() < 2) {
    return res;
  }
  
  if (_allDataList->has_error()) {
    error_log << QString::fromStdString(_allDataList->error());
    return res;
  }

  int idx = -1;
  for (int i = 0; i < _allDataList->all(1).val_size(); i++) {
    if (_allDataList->all(1).val(i).month() == month) {
      idx = i;
      break;
    }
  }

  if (idx == -1) {
    return res;
  }

  const climat::SurfMonthAllValues& temp = _allDataList->all(1).val(idx); //один месяц

  idx = -1;
  for (int i = 0; i < _allDataList->all(0).val_size(); i++) {
    if (_allDataList->all(0).val(i).month() == month) {
      idx = i;
      break;
    }
  }

  if (idx == -1) {
    return res;
  }
  const climat::SurfMonthAllValues& cloud = _allDataList->all(0).val(idx); //один месяц
  //ясно, полуясно, пасмурно
  QMap<int, QMap<int, float> > table; //срок, (категория, сумма отклонений)
  QMap<int, QMap<int, int> > count; // количество случаев
  
  calcTableDailyRateTemp(temp, cloud, hour, &table, &count);

  //var(table);
  //var(count);

  createGraphDailyRateTemp(table, count, month);
  
  QList<int> allHours;
  allHours << 0 << 3 << 6 << 9 << 12 << 15 << 18 << 21;

  for (int row = 0; row < allHours.count(); ++row) {
    boost::python::list trow;
    for (int hIdx = 0; hIdx < 3; hIdx++) { 
      if (count.contains(allHours.at(row)) && 
	  count.value(allHours.at(row)).contains(hIdx) && 
	  count.value(allHours.at(row)).value(hIdx) != 0) {
	float val = round(10 * table.value(allHours.at(row)).value(hIdx) /
			  count.value(allHours.at(row)).value(hIdx)) / 10;
	trow.append(QString::number(val).toStdString());
      } else {
	trow.append(std::string(""));
      }
    }
    res.append(trow);
  }
 
  return res;
}

void VkoCalc::createGraphDailyRateTemp(const QMap<int, QMap<int, float> >& table,
				       const QMap<int, QMap<int, int> >& count,
				       int month)
{
  QCustomPlot* customPlot = new QCustomPlot;
  customPlot->resize(800, 600);
  
  QStringList names;
  names << QObject::tr("Ясно") << QObject::tr("Полуясно") 
	<< QObject::tr("Пасмурно");

  QVector<double> x(9);
  QList<QVector<double>> y;
  y << QVector<double>(9) << QVector<double>(9) << QVector<double>(9);

  for (int categ = 0; categ < 3; categ++) {
    for (int idx = 0; idx < 8; idx++) {
      x[idx] = idx * 3;
      if (table.contains(idx*3)) {
	y[categ][idx] = table.value(idx*3).value(categ) /
	  count.value(idx*3).value(categ);
      }
    }
    int idx = 8;
    x[idx] = idx * 3;
    if (table.contains(0)) {
      y[categ][idx] = table.value(0).value(categ) /
	count.value(0).value(categ);
    }
  }

  for (int categ = 0; categ < 3; categ++) {
    customPlot->addGraph();
    customPlot->graph()->setName(names.at(categ));
    customPlot->graph()->setLineStyle(QCPGraph::lsLine);
    customPlot->graph()->setData(x, y.at(categ));
    customPlot->graph()->rescaleAxes(true);
  }

  customPlot->graph(0)->setPen(QColor(0, 180, 30));
  customPlot->graph(1)->setPen(QColor(200, 50, 50));
  customPlot->graph(2)->setPen(QColor(0, 50, 200));
  customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
  customPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDiamond, 6));
  customPlot->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross , 6));

  //легенда
  customPlot->legend->setVisible(true);
  customPlot->setAutoAddPlottableToLegend(true);
  customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignTop); 
  QCPLayoutGrid *subLayout = new QCPLayoutGrid;
  customPlot->plotLayout()->addElement(1, 0, subLayout);
  subLayout->setMargins(QMargins(5, 0, 5, 5));
  subLayout->addElement(0, 0, customPlot->legend);
  customPlot->legend->setFillOrder(QCPLegend::foColumnsFirst);
  customPlot->plotLayout()->setRowStretchFactor(1, 0.001);

  //оси
  customPlot->xAxis->setLabel(QObject::tr("Срок (UTC), ч"));
  customPlot->yAxis->setLabel(QObject::tr("Температура, °С"));

  customPlot->xAxis->setSubTicks(false);
  QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
  customPlot->xAxis->setTicker(fixedTicker);
  fixedTicker->setTickStep(3.0);
  fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);
  customPlot->xAxis->setRange(0, 24);
  
  customPlot->graph(0)->rescaleAxes();
  customPlot->graph(1)->rescaleAxes(true);
  customPlot->graph(2)->rescaleAxes(true);
  customPlot->yAxis->scaleRange(2, customPlot->yAxis->range().center());

  customPlot->savePng(_path + QString("/vkodT_") + QString::number(month) + ".png");
  
  delete customPlot;
}


void VkoCalc::calcTableDailyRateTemp(const climat::SurfMonthAllValues& atemp,
				     const climat::SurfMonthAllValues& cloud,
				     int hour,
				     QMap<int, QMap<int, float> >* table,
				     QMap<int, QMap<int, int> >* count)
{
  QMap<QDate, int> maxcloud;
  for (int idx = 0; idx < cloud.avglist_size(); idx++) {
    const climat::SurfAllValuesList& v= cloud.avglist(idx);
    QDate date(v.year(), cloud.month(), v.day());
    if (!maxcloud.contains(date) || maxcloud.value(date) < v.avg()) {
      maxcloud.insert(date, v.avg());
      //qDebug() << date << v.avg();
    }
  }

  QMap<QDateTime, float> temp;
  for (int idx = 0; idx < atemp.avglist_size(); idx++) {
    const climat::SurfAllValuesList& v= atemp.avglist(idx);
    temp.insert(QDateTime(QDate(v.year(), atemp.month(), v.day()), QTime(v.hour(), 0)), v.avg());
  }

  //var(temp);

  QMap<QDateTime, float>::iterator it = temp.begin();
  while (it != temp.end()) {
    QMap<int, float>& h = (*table)[it.key().time().hour()];
    int categ = -1;
    // debug_log << it.key() << it.value();

    if (maxcloud.contains(it.key().date()) && temp.contains(QDateTime(it.key().date(), QTime(hour, 0)))) {
      if (maxcloud.value(it.key().date()) <= 3) {
	categ = 0;
      } else if (maxcloud.value(it.key().date()) >= 4 && maxcloud.value(it.key().date()) <= 7) {
	categ = 1;
      } else if (maxcloud.value(it.key().date()) >= 8) {
	categ = 2;
      }
      if (!h.contains(categ)) {
	h.insert(categ, 0);
	(*count)[it.key().time().hour()].insert(categ, 0);
      }
      h[categ] += it.value() - temp.value(QDateTime(it.key().date(), QTime(hour, 0)));
      (*count)[it.key().time().hour()][categ] += 1;

      // if (categ == 1 && it.key().time().hour() == 0) {
      // 	debug_log << categ << it.key().time().hour() << it.value() << temp.value(QDateTime(it.key().date(), QTime(21, 0))) 
      // 		  << h.value(categ) << count->value(it.key().time().hour()).value(categ) << 
      // 	  h.value(categ) / count->value(it.key().time().hour()).value(categ);
      // 	var(*table);
      // }
      
    }
    ++it;
  }
}

boost::python::list VkoCalc::calcTableFrostDate()
{
  boost::python::list res;
  if (_dateData == 0 || !_dateData->IsInitialized()) {
    return res;
  }

  if (_dateData->has_error()) {
    error_log << QString::fromStdString(_dateData->error());
    return res;
  }

  // последнего средняя, ранняя, поздняя; первого средняя, ранняя, поздняя
  QMap<int, QDate> dt;// категория, дата
  QList<int> dtLength;//продолжительность средняя, наименьшая, наибольшая
  dtLength << 0 << 400 << -1;
  float early = 0, late = 0;

  QDate gstart = QDate::fromString(QString::fromStdString(_start), "yyyy-MM-dd");
  QDate gend   = QDate::fromString(QString::fromStdString(_end), "yyyy-MM-dd");
  
  int startCnt = 0;
  int endCnt = 0;
  int lenCnt = 0;
  for (int idx = 0; idx < _dateData->val_size(); idx++) {
    QDate start = QDate::fromString(QString::fromStdString(_dateData->val(idx).start_date()), Qt::ISODate);
    QDate end = QDate::fromString(QString::fromStdString(_dateData->val(idx).end_date()), Qt::ISODate);
    if (start.isValid()) {
      ++startCnt;
      early += start.dayOfYear();
      if (!dt.contains(4) || dt.value(4).dayOfYear() > start.dayOfYear()) {
	dt.insert(4, start);
      }
      if (!dt.contains(5) || dt.value(5).dayOfYear() < start.dayOfYear()) {
	dt.insert(5, start);
      }
    }
    
    if (end.isValid()) {
      ++endCnt;
      late += end.dayOfYear();
      if (!dt.contains(1) || dt.value(1).dayOfYear() > end.dayOfYear() ) {
	dt.insert(1, end);
      }
      if (!dt.contains(2) || dt.value(2).dayOfYear() < end.dayOfYear() ) {
	dt.insert(2, end);
      }

      if (start.isValid()) {
	dtLength[0] += end.daysTo(start);
	++lenCnt;
	if (end.daysTo(start) < dtLength.at(1)) {
	  dtLength[1] = end.daysTo(start);
	}
	if (end.daysTo(start) > dtLength.at(2)) {
	  dtLength[2] = end.daysTo(start);
	}
      }
    }
  }


  if (gstart == dt[4]) {
    startCnt = 0;
    lenCnt = 0;
    dt.remove(4);
    dt.remove(5);
    dtLength[0] = 0;
    dtLength[1] = 400;
  }

  if (gend == dt[1]) {
    endCnt = 0;
    lenCnt = 0;
    dt.remove(1);
    dt.remove(2);
    dtLength[0] = 0;
    dtLength[1] = 400;
  }

  
  QDate cur(1, 1, 1);
  if (early != 0 && startCnt != 0) {
    early /= startCnt;        
    dt.insert(3, cur.addDays(early - 1));
  }
  if (late != 0 && endCnt != 0) {
    late /= endCnt;    
    dt.insert(0, cur.addDays(late - 1));
  }
  
  // debug_log;
  // qDebug() << "dt" << dt << dtLength;
  
  boost::python::list trow;
  for (int idx = 0; idx < 6; idx++) {
    if (!dt.contains(idx)) {
      trow.append(std::string(""));
    } else {
      if (idx == 0 || idx == 3) {
	trow.append(dt.value(idx).toString("dd.MM").toStdString());
      } else {
	trow.append(dt.value(idx).toString("dd.MM.yyyy").toStdString());
      }
    }
  }
  if (lenCnt != 0 && dtLength.at(1) < 400) {
    trow.append(QString::number(dtLength.at(0)/lenCnt, 'f', 0).toStdString());
  } else {
    trow.append(std::string(""));
  }
  if (dtLength.at(1) < 400) {
    trow.append(QString::number(dtLength.at(1)).toStdString());
  } else {
    trow.append(std::string(""));
  }
  if (dtLength.at(2) >= 0) {
    trow.append(QString::number(dtLength.at(2)).toStdString());
  }
  res.append(trow);
  
  return res;
}

//! Дата образования и разрушения устройчивого снежного покрова
boost::python::list VkoCalc::calcTableSnowDate()
{
  boost::python::list res;
  if (_dateData == 0 || !_dateData->IsInitialized()) {
    return res;
  }

  if (_dateData->has_error()) {
    error_log << QString::fromStdString(_dateData->error());
    return res;
  }
  // var(_dateData->DebugString());
  
  // образования средняя, ранняя, поздняя; разрушения средняя, ранняя, поздняя
  QMap<int, QDate> dt; //категория, дата
  float early = 0, late = 0;

  QDate gstart = QDate::fromString(QString::fromStdString(_start), "yyyy-MM-dd");
  QDate gend   = QDate::fromString(QString::fromStdString(_end), "yyyy-MM-dd");

  int startCnt = 0;
  int endCnt = 0;  
  for (int idx = 0; idx < _dateData->val_size(); idx++) {
    QDate start = QDate::fromString(QString::fromStdString(_dateData->val(idx).start_date()), Qt::ISODate);
    QDate end = QDate::fromString(QString::fromStdString(_dateData->val(idx).end_date()), Qt::ISODate);
    if (start.isValid()) {
      ++startCnt;
      early += start.dayOfYear();
      if (!dt.contains(1) || dt.value(1).dayOfYear() > start.dayOfYear()) {
	dt.insert(1, start);
      }
      if (!dt.contains(2) || dt.value(2).dayOfYear() < start.dayOfYear()) {
	dt.insert(2, start);
      }
    }
    
    if (end.isValid()) {
      ++endCnt;
      late += end.dayOfYear();
      if (!dt.contains(4) || dt.value(4).dayOfYear() > end.dayOfYear() ) {
	dt.insert(4, end);
      }
      if (!dt.contains(5) || dt.value(5).dayOfYear() < end.dayOfYear() ) {
	dt.insert(5, end);
      }
    }
  }

  if (gstart == dt[1]) {
    startCnt = 0;
    dt.remove(1);
    dt.remove(2);
  }

  if (gend == dt[5]) {
    endCnt = 0;
    dt.remove(4);
    dt.remove(5);
  }
  
  QDate cur(1, 1, 1);
  if (early != 0 && startCnt != 0) {
    early /= startCnt;    
    dt.insert(0, cur.addDays(early - 1));
  }
  if (late != 0 && endCnt != 0) {
    late /= endCnt;    
    dt.insert(3, cur.addDays(late - 1));
  }

  // debug_log;
  // qDebug() << "dt" << dt;
  
  boost::python::list trow;
  for (int idx = 0; idx < 6; idx++) {
    if (!dt.contains(idx)) {
      trow.append(std::string(""));
    } else {
      if (idx == 0 || idx == 3) {
	trow.append(dt.value(idx).toString("dd.MM").toStdString());
      } else {
	trow.append(dt.value(idx).toString("dd.MM.yyyy").toStdString());
      }
    }
  }
  res.append(trow);

  return res;
}

//layer : [1500, 3000, 6000, 12000]
boost::python::list VkoCalc::calcTableAeroWind(int season, int layer)
{
  boost::python::list res;

  if (_aeroMonthData == nullptr || !_aeroMonthData->IsInitialized() ||
      _aeroMonthData->all_size() == 0) {
    return res;
  }

  if (_aeroMonthData->has_error()) {
    error_log << QString::fromStdString(_aeroMonthData->error());
    return res;
  }

  //var(_aeroMonthData->DebugString());
  
  QMap<QDateTime, float> dd;
  QMap<QDateTime, float> ff;
  bool ok = findAeroValues(season, layer, "dd", &dd);
  if (!ok) {
    return res;
  }
  ok = findAeroValues(season, layer, "ff", &ff);
  if (!ok) {
    return res;
  }

  //расчет повторяемости в слоях
  QList<int> lim;
  lim << 8 << 7 << 6 << 5 << 4 << 3 << 2 << 1 << 0;
  QList<float> limSpeed;//м/с
  limSpeed << 55.56 << 41.67 << 27.78 << 19.44 << 13.89 << 11.11 << 8.333 << 5.56 << 2.78 << 0;

  // var(dd);
  // var(ff);

  QMap<int, QMap<int, float> > table;//скорость, (направление, повторяемость)
  int count = 0;
  QMapIterator<QDateTime, float> it(dd);
  while (it.hasNext()) {
    it.next();
    for (int limIdx = 0; limIdx < lim.count(); limIdx++) {
      if (it.value() >= lim.at(limIdx)) {
	if (ff.contains(it.key())) {
	  int ffIdx = -1;
	  for (int fIdx = 0; fIdx < limSpeed.count(); fIdx++) {
	    if (ff.value(it.key()) >= limSpeed.at(fIdx)) {
	      ffIdx = fIdx;
	      if (ff.value(it.key()) != 0) {
		count++;
	      }
	      break;
	    }
	  }
	  if (-1 != ffIdx) {
	    QMap<int, float>& v = table[ffIdx];
	    if (!v.contains(lim.at(limIdx))) {
	      v.insert(lim.at(limIdx), 1);
	    } else {
	      v[lim.at(limIdx)] += 1;
	    }
	  }
	}
	break;
      }
    }
  }

  //  var(table);

  // QList<std::string> strSpeed;
  // strSpeed << "<10" << "10-19" << "20-29" << "30-39" << "40-49" 
  // 	   << "50-69" << "70-99" << "100-149" << "150-199" << "≥200";
  QList<int> fillOrder;
  fillOrder << 8 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 0;

  for (int row = 0; row < limSpeed.count(); ++row) {
    boost::python::list rowVal;
    //    rowVal.append(strSpeed.at(row));
    if (!table.isEmpty()) {
      for (int col = 0; col < fillOrder.count(); col++) {
	float val;
	if (table.contains(limSpeed.count() - 1 - row) && 
	    table.value(limSpeed.count() - 1 - row).contains(fillOrder.at(col))) {
	  if (fillOrder.at(col) == 0) {
	    if (dd.count() != 0) {
	      val = round(table.value(limSpeed.count() - 1 - row).value(fillOrder.at(col)) / dd.count() * 100 * 10) / 10;
	      rowVal.append(QString::number(val).toStdString());
	    } else {
	      rowVal.append("");
	    }
	  } else {
	    if (count != 0) {
	      val = round(table.value(limSpeed.count() - 1 - row).value(fillOrder.at(col)) / count * 100 * 10) / 10;
	      rowVal.append(QString::number(val).toStdString());
	    } else {
	      rowVal.append("");
	    }
	  }
	} else {
	  rowVal.append("");
	}
      }
    }
    res.append(rowVal);
  }

  return res;
}

bool VkoCalc::findAeroValues(int season, int layer, const std::string& descrname, QMap<QDateTime, float>* avg)
{
  for (int idx = 0; idx < _aeroMonthData->all_size(); idx++) {
    if (_aeroMonthData->all(idx).descrname() == descrname) { //дескриптор
      for (int lidx = 0; lidx < _aeroMonthData->all(idx).val_size(); lidx++) {
	if (_aeroMonthData->all(idx).val(lidx).layer() == layer) { //уровень
	  for (int midx = 0; midx < _aeroMonthData->all(idx).val(lidx).avglist_size(); midx++) {
	    QDateTime dt = QDateTime::fromString(QString::fromStdString(_aeroMonthData->all(idx).val(lidx).avglist(midx).date()), Qt::ISODate);
	    if ((season == 1 && (dt.date().month() == 12 || dt.date().month() < 3)) ||
		(season == 2 && (dt.date().month() >= 3 && dt.date().month() < 6))  ||
		(season == 3 && (dt.date().month() >= 6 && dt.date().month() < 9))  ||
		(season == 4 && (dt.date().month() >= 9 && dt.date().month() < 12)) ) { //все значения сезона
	      avg->insert(dt, _aeroMonthData->all(idx).val(lidx).avglist(midx).avg());
	    }
	  }
	  return true;
	}
      }
    }
  }

  return false;
}

boost::python::list VkoCalc::calcAeroTemp()
{
  boost::python::list table;
  
  if (_aeroMonthData == 0 || !_aeroMonthData->IsInitialized() ||
      _aeroMonthData->all_size() == 0) {
    error_log << QObject::tr("Нет данных");
    return table;
  }

  if (_aeroMonthData->has_error()) {
    error_log << QString::fromStdString(_aeroMonthData->error());
    return table;
  }

  // var(_aeroMonthData->DebugString());

  QMap<int, QMap<int, float> > temp; //давление, (месяц, средняя температура)
  
  for (int idx = 0; idx < _aeroMonthData->all(0).val_size(); idx++) {
    QMap<int, float>& tlev = temp[_aeroMonthData->all(0).val(idx).layer()];
    QMap<int, int> cnt; // месяц, количество значений
    for (int tidx = 0; tidx < _aeroMonthData->all(0).val(idx).avglist_size(); tidx++) {
      const meteo::climat::AeroValuesList& val = _aeroMonthData->all(0).val(idx).avglist(tidx);
      QDateTime dt = QDateTime::fromString(QString::fromStdString(val.date()), Qt::ISODate);
      if (cnt.contains(dt.date().month())) {
	cnt[dt.date().month()] += 1;
	tlev[dt.date().month()] += val.avg();
      } else {
	cnt.insert(dt.date().month(), 1);
	tlev.insert(dt.date().month(), val.avg());
      }
    }
    for (int m = 1; m < 13; m++) { //месяц
      if (cnt.contains(m)) {
	tlev[m] /= cnt.value(m);
      }
    }
  }

  QList<int> levels;
  levels << 850 << 700 << 500 << 300 << 200 << 100 << 50 << 30;

  for (int row = 0; row < levels.size(); ++row) {
    boost::python::list trow;
    for (int col = 1; col < 13; col++) { //месяц
      if (temp.contains(levels.at(row)) && temp.value(levels.at(row)).contains(col)) {
	float val = round(temp.value(levels.at(row)).value(col) * 10) / 10;
	trow.append(QString::number(val).toStdString());
      } else {
	trow.append("");
      }
    }
    table.append(trow);
  }  

  return table;
}

boost::python::list VkoCalc::calcAeroIsoterm()
{
  boost::python::list table;
  
  if (_aeroMonthData == 0 || !_aeroMonthData->IsInitialized() ||
      _aeroMonthData->all_size() == 0) {
    return table;
  }

  if (_aeroMonthData->has_error()) {
    error_log << QString::fromStdString(_aeroMonthData->error());
    return table;
  }

  QMap<int, QMap<int, float> > temp; //температура, (месяц, средняя высота)
  QMap<int, QMap<int, int> > count; // температура, (месяц, количество значений)
  for (int idx = 0; idx < _aeroMonthData->all(0).val_size(); idx++) {
    float height = _aeroMonthData->all(0).val(idx).layer();
    for (int tidx = 0; tidx < _aeroMonthData->all(0).val(idx).avglist_size(); tidx++) {
      const meteo::climat::AeroValuesList& val = _aeroMonthData->all(0).val(idx).avglist(tidx);
      QDateTime dt = QDateTime::fromString(QString::fromStdString(val.date()), Qt::ISODate);
      int curtemp = round(val.avg());
      QMap<int, int>& cnt = count[curtemp];
      if (cnt.contains(dt.date().month())) {
	cnt[dt.date().month()] += 1;
	temp[curtemp][dt.date().month()] += height;
      } else {
	cnt.insert(dt.date().month(), 1);
	temp[curtemp].insert(dt.date().month(), height);
      }
    }
  }

  QList<int> tlist;
  tlist << 0 << -10 << -20;

  for (int row = 0; row < tlist.count(); ++row) {
    boost::python::list trow;
    for (int m = 1; m < 13; m++) { //месяц
      if (count.value(tlist.at(row)).contains(m)) {
	trow.append(QString::number(temp.value(tlist.at(row)).value(m) / count.value(tlist.at(row)).value(m), 'f', 0).toStdString());
      } else {
	trow.append("");
      }
    }
    table.append(trow);
  }  

  return table;
}

boost::python::list VkoCalc::calcAeroTropo()
{
  boost::python::list rtable;
  if (_aeroMonthData == 0 || !_aeroMonthData->IsInitialized() ||
      _aeroMonthData->all_size() == 0) {
    return rtable;
  }

  if (_aeroMonthData->has_error()) {
    error_log << QString::fromStdString(_aeroMonthData->error());
    return rtable;
  }

  QMap<int, QPair<float, float> > table; //месяц, (высота, темература)
  QMap<int, int> cnt; //месяц, количество значений

  for (int idx = 0; idx < _aeroMonthData->all(0).val_size(); idx++) {
    float height = _aeroMonthData->all(0).val(idx).layer();
    if (height <= 0) continue;
    for (int tidx = 0; tidx < _aeroMonthData->all(0).val(idx).avglist_size(); tidx++) {
      const meteo::climat::AeroValuesList& val = _aeroMonthData->all(0).val(idx).avglist(tidx);
      QDateTime dt = QDateTime::fromString(QString::fromStdString(val.date()), Qt::ISODate);
      float curtemp = val.avg();
      if (cnt.contains(dt.date().month())) {
	cnt[dt.date().month()] += 1;
	table[dt.date().month()].first += height;
	table[dt.date().month()].second += curtemp;
      } else {
	cnt.insert(dt.date().month(), 1);
	table.insert(dt.date().month(), QPair<float, float>(height, curtemp));
      }
    }
  }

  // var(table);
  // var(cnt);

  boost::python::list rowH;
  boost::python::list rowT;
  for (int m = 1; m < 13; m++) { //месяц
    if (cnt.contains(m)) {
      rowH.append(QString::number(table.value(m).first / cnt.value(m), 'f', 0).toStdString());
      rowT.append(QString::number(table.value(m).second / cnt.value(m), 'f', 0).toStdString());
    } else {
      rowH.append("");
      rowT.append("");
    }
  }
  rtable.append(rowH);
  rtable.append(rowT);
    
  return rtable;
}


//lim - пределы, разделяющие категории
//<категория, количество>
void VkoCalc::calcRepeatability(const climat::SurfAvgValues& vals, const QList<int>& lim, QMap<int, float>* monthval)
{
  for (int limIdx = 0; limIdx < lim.count(); limIdx++) {
    monthval->insert(lim.at(limIdx), 0);
  }
  for (int hidx = 0; hidx < vals.avg_size(); hidx++) {
    for (int limIdx = 0; limIdx < lim.count(); limIdx++) {
      if (vals.avg(hidx) >= lim.at(limIdx)) {
	(*monthval)[lim.at(limIdx)] += 1;
	break;
      }
    }
  } 
}

//месяц, (категория, количество)
boost::python::list VkoCalc::fillMonthScriptObject(const QMap<int, QMap<int, float> >& table, const QList<int>& lim)
{
  boost::python::list res;
  
  for (int mIdx = 0; mIdx < 12; mIdx++) { //месяц
    boost::python::list trow;
    for (int limIdx = lim.count() - 1; limIdx >= 0; limIdx--) {
      if (table.contains(mIdx + 1)) {
	float val = round(table.value(mIdx + 1).value(lim.at(limIdx)) * 10) / 10;
	trow.append(QString::number(val).toStdString());
      } else {
	trow.append("");
      }
    }
    res.append(trow);
  }
  return res;
}


//! все значения в %, в соответствии с количеством count
void VkoCalc::convertToPercent(QMap<int, float>* monthval, int count)
{
  QMap<int, float>::iterator it = monthval->begin();
  while (it != monthval->end()) {
    it.value() = it.value() / count * 100;
    ++it;
  }
}


QList<std::string> VkoCalc::typeToDescr(climat::VkoType type, int* levtype /*= 0*/)
{
  if (0 != levtype) {
    *levtype = 0;
  }

  QList<std::string> dlist;
  if (!_etc.IsInitialized()) {
    return dlist;
  }  
  
  for (int idx = 0; idx < _etc.prop_size(); idx++) {
    if (_etc.prop(idx).type() == type) {
      for (int kk = 0; kk < _etc.prop(idx).descr_size(); kk++ ) {
	dlist << _etc.prop(idx).descr(kk);
      }
      if (_etc.prop(idx).has_levtype() && 0 != levtype) {
	*levtype = _etc.prop(idx).levtype();
      } 
      break;
    }
  }
  
  return dlist;
}

//-----

boost::python::list VkoCalc::calcVkoddY()
{
  boost::python::list res;
  
  if (_monthAvgData == 0 || !_monthAvgData->IsInitialized()) {
    return res;
  }
  
  if (_monthAvgData->has_error()) {
    error_log << QString::fromStdString(_monthAvgData->error());
    return res;
  }
  
  //var(_monthAvgData->DebugString());

  QList<int> lim;
  lim << 8 << 7 << 6 << 5 << 4 << 3 << 2 << 1 << 0;
  QList<int> fillOrder;
  fillOrder << 0 << 7 << 6 << 5 << 4 << 3 << 2 << 1 << 8; //обратный, т.к. в функции fillMonthScriptObject так
  
  QMap<int, QMap<int, float> > table; //месяц, (категория, количество)
  for (int idx = 0; idx < _monthAvgData->val_size(); idx++) {
    const climat::SurfAvgValues& vals = _monthAvgData->val(idx);
    QMap<int, float> monthval;

    calcRepeatability(vals, lim, &monthval);
    int count = vals.avg_size() - monthval.value(0); //без штиля
    //debug_log << monthval << count << vals.avg_size();

    QMap<int, float>::iterator it = monthval.begin();
    while (it != monthval.end()) {
      if (it.key() == 0) {
	if (vals.avg_size() == 0) {
	  it.value() = 0;
	} else {
	  it.value() = it.value() / vals.avg_size() * 100;
	} 
      } else {
	if (count <= 0) {
	  it.value() = 0;
	} else {
	  it.value() = it.value() / count * 100;
	}	       
      }
      ++it;
    }

    table.insert(vals.month(), monthval);
  }

  //var(table);

  for (int limIdx = lim.count() - 1; limIdx >= 0; limIdx--) {
    boost::python::list trow;
    float sum = 0;
    int cnt = 0;
    for (int mIdx = 0; mIdx < 12; mIdx++) { //месяц
      if (table.contains(mIdx + 1)) {
	float val = round(table.value(mIdx + 1).value(lim.at(limIdx)) * 10) / 10;
	sum += table.value(mIdx + 1).value(lim.at(limIdx));
	cnt += 1;
	trow.append(QString::number(val).toStdString());
      } else {
	trow.append("");
      }
    }
    if (cnt != 0) {
      trow.append(QString::number(round(sum / cnt * 10)/10).toStdString());
    } else {
      trow.append("");
    }
    res.append(trow);
  }
  
  return res;
}


boost::python::list VkoCalc::calcVkoffxY()
{
  boost::python::list res;
  
  if (_yearData == 0 || !_yearData->IsInitialized() ||
      _dayData == 0  || !_dayData->IsInitialized()) {
    return res;
  }
  
  if (_yearData->has_error() || _dayData->has_error()) {
    error_log << QString::fromStdString(_yearData->error());
    return res;
  }

  QMap<int, ClimParam> table; //<месяц, параметры>
  for (int idx = 0; idx < _yearData->val_size(); idx++) {
    const climat::SurfMonthValues& vals = _yearData->val(idx);
    ClimParam param;
    for (int hidx = 0; hidx < vals.avglist_size(); hidx++) {
      param.avg += vals.avglist(hidx).avg();
    }
    param.avg /= vals.avglist_size();
    //debug_log << vals.month() << param.min  << param.minYear << param.avg << param.max << param.maxYear;

    table.insert(vals.month(), param);
  }

  boost::python::list trow;
  //Object::tr("Средняя"));
  float sum = 0;
  int cnt = 0;
  for (int mIdx = 1; mIdx <= 12; mIdx++) { //месяц
    if (table.contains(mIdx)) {
      float val = round(table.value(mIdx).avg * 10) / 10;
      sum += table.value(mIdx).avg;
      cnt += 1;
      trow.append(QString::number(val).toStdString());
    } else {
      trow.append(std::string(""));
    }
  }
  if (cnt != 0) {
    trow.append(QString::number(round(sum / cnt * 10) / 10).toStdString());
  } else {
    trow.append(std::string(""));
  }
  res.append(trow);

  int lim = 15;
  table.clear(); //месяц, параметры
  for (int idx = 0; idx < _dayData->val_size(); idx++) {
    ClimParam param;
    const climat::SurfDayValues& vals = _dayData->val(idx);
    
    for (int yIdx = 0; yIdx < vals.avglist_size(); yIdx++) {
      param.count = 0; //число дней в месяце с явлением
      for (int hidx = 0; hidx < vals.avglist(yIdx).avg_size(); hidx++) {
	if (vals.avglist(yIdx).avg(hidx) > lim) {
	  param.count += 1;
	}
      }
      if (param.max < param.count || param.maxYear == 0) {
	param.max = param.count;
	param.maxYear = vals.avglist(yIdx).year();
      }
      param.avg += param.count;
      // if (idx == 0) {
      //   debug_log <<  param.count << param.avg << param.max;
      // }
    }
    param.avg /= vals.avglist_size();
    // if (idx == 0) {
    //   	debug_log << param.avg;
    //   }
    
    table.insert(vals.month(), param);
  }
  
  boost::python::list trow1;
  boost::python::list trow2;
  float sum1 = 0;
  float sum2 = 0;
  int cnt1 = 0;
  int cnt2 = 0;
  for (int mIdx = 1; mIdx <= 12; mIdx++) { //месяц
    if (table.contains(mIdx)) {
      if (qFuzzyIsNull(table.value(mIdx).avg)) {
	trow1.append(std::string(""));	
      } else {
	float val = round(table.value(mIdx).avg * 10) / 10;
	trow1.append(QString::number(val).toStdString());
	sum1 += table.value(mIdx).avg;
	cnt1++;
      }
      if (qFuzzyIsNull(table.value(mIdx).max)) {
	trow2.append(std::string(""));
      } else {
	trow2.append(QString::number(table.value(mIdx).max, 'f', 0).toStdString());
	sum2 += table.value(mIdx).max;
	cnt2++;
      }
    } else {
      trow1.append(std::string(""));
      trow2.append(std::string(""));
    }
  }

  if (cnt1 != 0) {
    trow1.append(QString::number(round(sum1 / cnt1 * 10) / 10).toStdString());
  } else {
    trow1.append(std::string(""));
  }
  if (cnt2 != 0) {
    trow2.append(QString::number(round(sum2 / cnt2 * 10) / 10).toStdString());
  }
    
  res.append(trow1);
  res.append(trow2);
  
  return res;
}

boost::python::list VkoCalc::calcVkoNY()
{
  boost::python::list res;
  boost::python::list table;
  
  if (_allDataList == 0 || !_allDataList->IsInitialized() ||
      _allDataList->all_size() == 0 || _allDataList->all(0).val_size() == 0) {
    error_log << QObject::tr("Нет данных");
    return table;
  }

  //var(_allDataList->DebugString());  

  if (_allDataList->has_error()) {
    error_log << QString::fromStdString(_allDataList->error());
    return table;
  }
  
  QMap<int, QMap<int, float> > Nh;//месяц, (категория, число дней)

  for (int mIdx = 0; mIdx < _allDataList->all(0).val_size(); mIdx++) {
    const climat::SurfMonthAllValues& vals = _allDataList->all(0).val(mIdx);
    
    QMap<int, ClimAccum> Nsum; //!< дни, (сумма облачности, число значений)
    for (int hIdx = 0; hIdx < vals.avglist_size(); hIdx++) {
      QDate date(vals.avglist(hIdx).year(), 1, 1);
      date = date.addDays(vals.avglist(hIdx).day() - 1);
      Nsum[date.day()].sum += vals.avglist(hIdx).avg();
      Nsum[date.day()].cnt += 1;
    }
    
    QMapIterator<int, ClimAccum> it(Nsum);
    while (it.hasNext()) {
      it.next();
      int categ = 0;
      int Nb = round((it.value().sum / it.value().cnt)); //среднее в баллах за день
      if (Nb >= 8) {
	categ = 2;
      } else if (Nb >=3) {
	categ = 1;
      }
      
      // debug_log << it.key() << it.value().sum << it.value().cnt << it.value().sum / it.value().cnt
      // 		<< Nb << categ;
      Nh[mIdx + 1][categ] += 1;
    }
  }

  //  var(Nh);

  for (int limIdx = 0; limIdx < 3; limIdx++) {
    boost::python::list trow;
    float sum = 0;
    int cnt = 0;
    for (int mIdx = 1; mIdx <= 12; mIdx++) { //месяц
      if (Nh.contains(mIdx) && Nh.value(mIdx).contains(limIdx)) {
	float val = round(Nh.value(mIdx).value(limIdx) * 10) / 10;
	trow.append(QString::number(val).toStdString());
	sum += Nh.value(mIdx).value(limIdx);
	cnt++;
      } else {
	trow.append("");
      }
    }
    trow.append(QString::number(round(sum / cnt * 10) / 10).toStdString());
    table.append(trow);
  }
  
  return table;
}

boost::python::list VkoCalc::calcVkowRY()
{
  boost::python::list res;
  res = calcVkowY();
  res.append(calcVkoRY());
  return res;
}

boost::python::list VkoCalc::calcVkowY()
{
  boost::python::list res;
  
  if (_dayData == 0  || !_dayData->IsInitialized()) {
    return res;
  }

  if (_dayData->has_error()) {
    error_log << QString::fromStdString(_dayData->error());
    return res;
  }

  //  var(_dayData->DebugString());
  
  //гроза (0x1), туман (0x2), метель (0x4), град (0x8)
  QList<int> lim;
  lim << 0 << 1 << 2 << 3;

  for (int lidx = 0; lidx < lim.count(); lidx++) {
    QMap<int, ClimParam> table; //месяц, параметры
    for (int idx = 0; idx < _dayData->val_size(); idx++) {
      ClimParam param;
      const climat::SurfDayValues& vals = _dayData->val(idx);

      for (int yIdx = 0; yIdx < vals.avglist_size(); yIdx++) {
	param.count = 0;//число дней в месяце с явлением
	for (int hidx = 0; hidx < vals.avglist(yIdx).avg_size(); hidx++) {
	  int mask = 1 <<  lim.at(lidx);
	  if (0 != ((int)vals.avglist(yIdx).avg(hidx) & mask)) {
	    param.count += 1;
	  }
	}
	param.avg += param.count;
      }
      param.avg /= vals.avglist_size();
      
      //debug_log << vals.month() << param.avg << param.max;
      table.insert(vals.month(), param);
    }

    //var(table.keys());
    
    boost::python::list trow1;
    for (int mIdx = 1; mIdx <= 12; mIdx++) { //месяц
      if (table.contains(mIdx)) {
	float val = round(table.value(mIdx).avg * 10) / 10;
	if (qFuzzyIsNull(val)) {
	  trow1.append("");
	} else {
	  trow1.append(QString::number(val).toStdString());
	}
      } else {
	trow1.append("");
      }
    }
    res.append(trow1);
  }

  return res;
}

boost::python::list VkoCalc::calcVkoRY()
{
  boost::python::list trow;
  if (_yearData == 0 || !_yearData->IsInitialized()) {
    return trow;
  }

  if (_yearData->has_error()) {
    error_log << QString::fromStdString(_yearData->error());
    return trow;
  }

  QMap<int, float> table; //<месяц, среднее>
  for (int idx = 0; idx < _yearData->val_size(); idx++) {
    const climat::SurfMonthValues& vals = _yearData->val(idx);
    float avg = 0;
    for (int hidx = 0; hidx < vals.avglist_size(); hidx++) {
      avg += vals.avglist(hidx).avg();
    }
    avg /= vals.avglist_size();

    table.insert(vals.month(), avg);
  }

  //var(table);

  for (int mIdx = 1; mIdx <= 12; mIdx++) { //месяц
    if (table.contains(mIdx)) {
      float val = round(table.value(mIdx) * 10) / 10;
      trow.append(QString::number(val).toStdString());
    } else {
      trow.append("");
    }
  }
  
  return trow;
}

boost::python::list VkoCalc::calcVkoTY()
{
  boost::python::list res;
  
  if (_allDataList == 0 || !_allDataList->IsInitialized() ||
      _allDataList->all_size() == 0) {
    return res;
  }
  
  if (_allDataList->has_error()) {
    error_log << QString::fromStdString(_allDataList->error());
    return res;
  } 
  //var(_allDataList->DebugString());

  // QStringList categ;
  // categ << QObject::tr("Средняя месячная") << QObject::tr("Средняя максимальная") 
  // 	  << QObject::tr("Средняя минимальная");
  int categCnt = 3;

  QMap<int, QMap<int, float> > table; //месяц, (категория, количество)
  for (int idx = 0; idx < _allDataList->all(0).val_size(); idx++) {
    const climat::SurfMonthAllValues& vals = _allDataList->all(0).val(idx);

    QMap<int, ClimParam> yearval;//год, параметры
    for (int hidx = 0; hidx < vals.avglist_size(); hidx++) {
      ClimParam& param = yearval[vals.avglist(hidx).year()];
      ++param.count;
      param.avg += vals.avglist(hidx).avg();
      if (param.max < vals.avglist(hidx).avg() || param.maxYear == 0) {
	param.max = vals.avglist(hidx).avg();
	param.maxYear = vals.avglist(hidx).year();
      }
      if (param.min > vals.avglist(hidx).avg() || param.minYear == 0) {
	param.min = vals.avglist(hidx).avg();
	param.minYear = vals.avglist(hidx).year();
      }
    }

    QMap<int, float> monthval;
    for (int c = 0; c < categCnt; c++) {
      monthval.insert(c, 0);
    }
    monthval[3] = monthval[4] = -999;
    
    QMap<int, ClimParam>::iterator it = yearval.begin();
    while (it != yearval.end()) {
      it.value().avg /= it.value().count;
      monthval[0] += it.value().avg;
      monthval[1] += it.value().max;
      monthval[2] += it.value().min;
      ++it;
    }
    if (yearval.count() != 0) {
      monthval[0] /= yearval.count();
      monthval[1] /= yearval.count();
      monthval[2] /= yearval.count();
      
      table.insert(vals.month(), monthval);
    }
  }


  for (int row = 0; row < categCnt; row++) {
    boost::python::list trow;
    float sum = 0;
    float cnt = 0;
    for (int mIdx = 1; mIdx <= 12; mIdx++) { //месяц
      if (table.contains(mIdx)) {
	float val = round(table.value(mIdx).value(row) * 10) / 10;
	trow.append(QString::number(val).toStdString());
	sum += table.value(mIdx).value(row);
	cnt += 1;
      } else {
	trow.append(std::string(""));
      }
    }
    trow.append(QString::number(round(sum / cnt * 10) / 10).toStdString());
    res.append(trow);
  }
 
  return res;
}


//-----
//день в декаду (0-2)
int VkoCalc::dayToDecade(int day)
{
  int decIdx = 0;
  if (day > 20) {
    decIdx = 2;
  } else if (day > 10) {
    decIdx = 1;
  }
  return decIdx;
}

//облачность за месяц
boost::python::list VkoCalc::calcVkoNm()
{
  boost::python::list table;
 
  if (_allDataList == 0 || !_allDataList->IsInitialized() ||
      _allDataList->all_size() == 0 ||
      _allDataList->all(0).val_size() == 0) {
    error_log << QObject::tr("Нет данных");
    return table;
  }

  //var(_allDataList->DebugString());  

  if (_allDataList->has_error()) {
    error_log << QString::fromStdString(_allDataList->error());
    return table;
  }
  
  const climat::SurfMonthAllValues& vals = _allDataList->all(0).val(0);

  // var(vals.DebugString());
  
  //0-2, 3-7, 8-10, 0 баллов
  QVector<QVector<float> > Nd(4); //категория, (декада, число дней)
  for (int idx = 0; idx < Nd.size(); idx++) {
    Nd[idx].fill(0, 3);
  }

  //var(vals.avglist_size());
  
  QMap<int, ClimAccum> Nsum; //!< дни, (сумма облачности, число значений)
  QVector<int> cnt(3, 0); //число случаев для каждой декады
  for (int hIdx = 0; hIdx < vals.avglist_size(); hIdx++) {
    QDate date(vals.avglist(hIdx).year(), 1, 1);
    date = date.addDays(vals.avglist(hIdx).day() - 1);
    Nsum[date.day()].sum += vals.avglist(hIdx).avg();
    Nsum[date.day()].cnt += 1;
  }
  
  QMapIterator<int, ClimAccum> it(Nsum);
  while (it.hasNext()) {
    it.next();
    int categ = 0;
    int Nb = round((it.value().sum / it.value().cnt)); //среднее в баллах за день
    if (Nb >= 8) {
      categ = 2;
    } else if (Nb >=3) {
      categ = 1;
    }
    
    int decIdx = dayToDecade(it.key());
    cnt[decIdx] += 1;
    // debug_log << it.key() << it.value().sum << it.value().cnt << it.value().sum / it.value().cnt
    // 	      << Nb << categ << decIdx;
    Nd[categ][decIdx] += 1;
    if (Nb == 0) {
      Nd[3][decIdx] += 1;
    }
  }
  // var(Nd);
  
  for (int idx = 0; idx < Nd.size(); idx++) {
    boost::python::list trow;
    float sum = 0;
    int scnt = 0;
    for (int decIdx = 0; decIdx < Nd.at(idx).size(); decIdx++) {
      if (cnt[decIdx] == 0) {
	trow.append(std::string(""));
      } else {
	float val = Nd[idx][decIdx];
	if (idx == 3) {
	  val /= cnt[decIdx];
	  scnt += cnt[decIdx];
	}
	sum += val;
	trow.append(QString::number(round(val * 10) / 10).toStdString());
      }
    }
    if (idx == 3) {
      if (scnt == 0) {
	trow.append(std::string(""));
      } else {
	trow.append(QString::number(round(sum / scnt * 10) / 10).toStdString());
      }
    } else {
      trow.append(QString::number(round(sum * 10) / 10).toStdString());
    }
    table.append(trow);
  }

  return table;
}

//ВНГО, месяц
boost::python::list VkoCalc::calcVkohm()
{
  boost::python::list table;
  
  if (_allDataList == 0 || !_allDataList->IsInitialized() ||
      _allDataList->all_size() == 0 ||
      _allDataList->all(0).val_size() == 0) {
    error_log << QObject::tr("Нет данных");
    return table;
  }
  
  if (_allDataList->has_error()) {
    error_log << QString::fromStdString(_allDataList->error());
    return table;
  }
  
  const climat::SurfMonthAllValues& vals = _allDataList->all(0).val(0);

  // var(vals.DebugString());
  
  QList<int> lim = QList<int>() << 2500 << 1500 << 1000 << 600 << 300 << 100 << 0;

  QVector<QMap<int, int>> res(3);//декада, (категория, сумма повторов)
  QVector<int> cnt(3, 0); //число случаев для каждой декады

  for (int limIdx = 0; limIdx < lim.count(); limIdx++) {
    res[0].insert(lim.at(limIdx), 0);
    res[1].insert(lim.at(limIdx), 0);
    res[2].insert(lim.at(limIdx), 0);
  }

  
  for (int hIdx = 0; hIdx < vals.avglist_size(); hIdx++) {
    QDate date(vals.avglist(hIdx).year(), 1, 1);
    int day = date.addDays(vals.avglist(hIdx).day() - 1).day();
    int decade = dayToDecade(day);
    cnt[decade] += 1;
    for (int limIdx = 0; limIdx < lim.count(); limIdx++) {
      if (vals.avglist(hIdx).avg() >= lim.at(limIdx)) {
	res[decade][lim.at(limIdx)] += 1;
	break;
      }
    }
  }
  
  // var(res);
  // var(cnt);
  for (int didx = 0; didx < res.size(); didx++) {
    //до 1500 - сумма того, что ниже
    for (int idx = lim.size()-2; idx > 1; idx--) {
      res[didx][lim.at(idx)] += res[didx][lim.at(idx + 1)];
    }
    //от 1500 - сумма, что выше
    res[didx][1500] += res[didx][2500];
  }

  // var(res);
  lim.removeAt(3); //от 600 до 1000 не надо
  for (int limIdx = lim.count() - 1; limIdx >= 0; limIdx--) {
    boost::python::list trow;
    int sum = 0;
    int scnt = 0;
    for (int idx = 0; idx < res.size(); idx++) {
      if (cnt.at(idx) != 0) {
	float val = round(res.at(idx).value(lim.at(limIdx)) * 100. / cnt.at(idx));
	sum  += res.at(idx).value(lim.at(limIdx));
	scnt += cnt.at(idx);
	trow.append(QString::number(val, 'f', 0).toStdString());
      } else {
	trow.append(std::string(""));
      }
    }
    if (scnt != 0) {
      trow.append(QString::number(sum * 100. / scnt, 'f', 0 ).toStdString());
    } else {
      trow.append(std::string(""));
    }
    table.append(trow);
  }

  //от 300 до 1000м
  boost::python::list trow;
  int sum = 0;
  int scnt = 0;
  for (int idx = 0; idx < res.size(); idx++) {
    if (cnt.at(idx) != 0) {
      float val = round((res.at(idx).value(600) - res.at(idx).value(100)) * 100. / cnt.at(idx));
      sum  += res.at(idx).value(600) - res.at(idx).value(100);
      scnt += cnt.at(idx);
      trow.append(QString::number(val, 'f', 0).toStdString());
    } else {
      trow.append(std::string(""));
    }
  }
  if (scnt != 0) {
    trow.append(QString::number(sum * 100. / scnt, 'f', 0 ).toStdString());
  } else {
    trow.append(std::string(""));
  }
  table.append(trow);
  
  return table;
}

//МДВ, месяц
boost::python::list VkoCalc::calcVkoVm()
{
  boost::python::list table;
  
  if (_allDataList == 0 || !_allDataList->IsInitialized() ||
      _allDataList->all_size() == 0 ||
      _allDataList->all(0).val_size() == 0) {
    error_log << QObject::tr("Нет данных");
    return table;
  }
  
  if (_allDataList->has_error()) {
    error_log << QString::fromStdString(_allDataList->error());
    return table;
  }
  
  const climat::SurfMonthAllValues& vals = _allDataList->all(0).val(0);

  // var(vals.DebugString());
  
  QList<int> lim = QList<int>() << 4000 << 2000 << 1500 << 1000 << 500 << 100 << 0;

  QVector<QMap<int, int>> res(3);//декада, (категория, сумма повторов)
  QVector<int> cnt(3, 0); //число случаев для каждой декады
  for (int limIdx = 0; limIdx < lim.count(); limIdx++) {
    res[0].insert(lim.at(limIdx), 0);
    res[1].insert(lim.at(limIdx), 0);
    res[2].insert(lim.at(limIdx), 0);
  }
  
  for (int hIdx = 0; hIdx < vals.avglist_size(); hIdx++) {
    QDate date(vals.avglist(hIdx).year(), 1, 1);
    int day = date.addDays(vals.avglist(hIdx).day() - 1).day();
    int decade = dayToDecade(day);
    cnt[decade] += 1;
    for (int limIdx = 0; limIdx < lim.count(); limIdx++) {
      if (vals.avglist(hIdx).avg() >= lim.at(limIdx)) {
	res[decade][lim.at(limIdx)] += 1;
	break;
      }
    }
  }
  
  // var(res);
  // var(cnt);

  for (int didx = 0; didx < res.size(); didx++) {
    //до 4000 - сумма того, что ниже
    for (int idx = lim.size()-2; idx >= 1; idx--) {
      res[didx][lim.at(idx)] += res[didx][lim.at(idx + 1)];
    }
  }
  // var(res);

  for (int limIdx = lim.count() - 1; limIdx >= 0; limIdx--) {
    boost::python::list trow;
    int sum = 0;
    int scnt = 0;
    for (int idx = 0; idx < res.size(); idx++) {
      if (cnt.at(idx) != 0) {
	float val = round(res.at(idx).value(lim.at(limIdx)) * 100. / cnt.at(idx));
	sum  += res.at(idx).value(lim.at(limIdx));
	scnt += cnt.at(idx);
	// debug_log << idx << lim.at(limIdx) << res.at(idx).value(lim.at(limIdx))
	// 	  << val;
	trow.append(QString::number(val, 'f', 0).toStdString());
      } else {
	trow.append(std::string(""));
      }
    }
    if (scnt != 0) {
      trow.append(QString::number(sum * 100. / scnt, 'f', 0 ).toStdString());
    } else {
      trow.append(std::string(""));
    }
    table.append(trow);
  }
  
  return table;
}

 
//ВНГО/МДВ, месяц
boost::python::list VkoCalc::calcVkohVm()
{
  boost::python::list table;
  
  if (_allDataList == 0 || !_allDataList->IsInitialized() ||
      _allDataList->all_size() < 2 ||
      _allDataList->all(0).val_size() == 0 ||
      _allDataList->all(1).val_size() == 0) {
    error_log << QObject::tr("Нет данных");
    return table;
  }
  
  if (_allDataList->has_error()) {
    error_log << QString::fromStdString(_allDataList->error());
    return table;
  }

  // var(_allDataList->DebugString());
  
  QList<int> limh; 
  limh << 30  <<  60 <<  100 <<  150 <<  200 <<  250 <<  300 <<  300 <<  600;
  QList<int> limV;
  limV << 400 << 800 << 1000 << 1500 << 2000 << 2500 << 3000 << 4000 << 5000;

  QMap<QDateTime, int> h; //дата, предел в который попало значение (>= предел)
  QMap<QDateTime, int> V;
  QVector<int> cnt(3, 0); //общее число случаев по декадам

  const climat::SurfMonthAllValues& hval = _allDataList->all(1).val(0);
  const climat::SurfMonthAllValues& Vval = _allDataList->all(0).val(0);

  for (int hIdx = 0; hIdx < hval.avglist_size(); hIdx++) {
    QDateTime date = QDateTime(QDate(hval.avglist(hIdx).year(), 1, 1).addDays(hval.avglist(hIdx).day() - 1),
			       QTime(hval.avglist(hIdx).hour(), 0, 0));
    int day = date.date().day();
    int decade = dayToDecade(day);
    cnt[decade] += 1;
    for (int limIdx = 0; limIdx < limh.count(); limIdx++) {
      if (hval.avglist(hIdx).avg() <= limh.at(limIdx)) {
	h[date] = limIdx;
	break;
      }
    }
  }
 
  for (int hIdx = 0; hIdx < Vval.avglist_size(); hIdx++) {
    QDateTime date = QDateTime(QDate(Vval.avglist(hIdx).year(), 1, 1).addDays(Vval.avglist(hIdx).day() - 1),
			       QTime(Vval.avglist(hIdx).hour(), 0, 0));
    for (int limIdx = 0; limIdx < limV.count(); limIdx++) {
      if (Vval.avglist(hIdx).avg() <= limV.at(limIdx)) {
	V[date] = limIdx;
	break;
      }
    }
  }

  QVector<QMap<int, int>> res(3);//декада, (категория, сумма повторов)
  
  QMapIterator<QDateTime, int> hit(h);
  while (hit.hasNext()) {
    hit.next();
    int decade = dayToDecade(hit.key().date().day());
    // debug_log << hit.key() << decade << hit.value() << V.contains(hit.key()) << V.value(hit.key());
    
    for (int hidx = hit.value(); hidx < limh.count(); hidx++) {
      // if (decade == 0) {
      // 	debug_log << hidx << hit.value()
      // }
      if (V.contains(hit.key()) && V.value(hit.key()) <= hit.value()) {
	if (!res[decade].contains(hidx)) {
	  res[decade].insert(hidx, 1);
	} else {
	  res[decade][hidx] += 1;
	}
	// var(res[decade][hidx]);
      }      
    }
  }

  // qDebug() << "h" << h << "\n";
  // qDebug() << "V" << V;
  // var(cnt);
  // var(res);

  for (int limIdx = 0; limIdx < limh.count(); limIdx++) {
    boost::python::list trow;
    int sum = 0;
    int scnt = 0;
    for (int idx = 0; idx < res.size(); idx++) {
      if (cnt.at(idx) != 0) {
	float val = round(res.at(idx).value(limIdx) * 100. / cnt.at(idx));
	sum  += res.at(idx).value(limIdx);
	scnt += cnt.at(idx);
	trow.append(QString::number(val, 'f', 0).toStdString());
      } else {
	trow.append(std::string(""));
      }
    }
    if (scnt != 0) {
      trow.append(QString::number(sum * 100. / scnt, 'f', 0 ).toStdString());
    } else {
      trow.append(std::string(""));
    }
    table.append(trow);
  }   
  
  return table;
}

boost::python::list VkoCalc::calcVkoRm()
{
  boost::python::list table;

  if (_dayData == 0  || !_dayData->IsInitialized()) {
    return table;
  }
  
  if (_dayData->has_error()) {
    error_log << QString::fromStdString(_dayData->error());
    return table;
  }

  if (_dayData->val_size() == 0) {
    return table;
  }

  const climat::SurfDayValues& vals = _dayData->val(0);
  
  if (vals.avglist_size() == 0 ||
      vals.avglist(0).avg_size() !=  vals.avglist(0).day_size()) {
    return table;
  }

  //var(_dayData->DebugString());

  float maxDay = 0;
  float maxMonth = 0;
  float avgMonth = 0;
  QList<float> lim = QList<float>() << 0.1 << 1 << 20;
  QVector<QVector<float>> res(3);//декада, (категория, число дней)
  for (int dec = 0; dec < 3; dec++) {
    res[dec].fill(0, 3);
  }
    
    
  for (int idx = 0; idx < vals.avglist_size(); idx++) {
    float sumYear = 0; //за текущий год
      
    for (int yIdx = 0; yIdx < vals.avglist(idx).avg_size(); yIdx++) {
      QDate date(vals.avglist(idx).year(), 1, 1);
      for (int limIdx = 0; limIdx < lim.count(); limIdx++) {
	if (vals.avglist(idx).avg(yIdx) >= lim.at(limIdx)) {
	  int day = date.addDays(vals.avglist(idx).day(yIdx) - 1).day();
	  int decade = dayToDecade(day); 
	  res[decade][limIdx] += 1;
	}
	sumYear += vals.avglist(idx).avg(yIdx);
	maxDay = (maxDay < vals.avglist(idx).avg(yIdx)) ? vals.avglist(idx).avg(yIdx) : maxDay;
      }
    }
      
    avgMonth += sumYear;
    maxMonth = (maxMonth < sumYear) ? sumYear : maxMonth;
  }

  avgMonth /= vals.avglist_size();
  for (int dec = 0; dec < 3; dec++) {
    for (int limIdx = 0; limIdx < lim.count(); limIdx++) {
      res[dec][limIdx] /= vals.avglist_size();
    }
  }
    
  //debug_log << avgMonth << maxMonth << res;

  for (int limIdx = 0; limIdx < lim.count(); limIdx++) {
    boost::python::list trow;
    float sum = 0;
    for (int dec = 0; dec <= 2; dec++) { //декада
      float val = res[dec][limIdx];
      sum += val;
      trow.append(QString::number(round(val)).toStdString());
    }
    trow.append(QString::number(sum / 3, 'f', 0 ).toStdString());
    table.append(trow);
  }

  boost::python::list trow1, trow2, trow3;
  trow1.append("");
  trow1.append("");
  trow1.append("");
  QString mv = QString::number(round(avgMonth));
  trow1.append(mv.toStdString());
  table.append(trow1);
  trow2.append("");
  trow2.append("");
  trow2.append("");
  mv = QString::number(round(maxMonth));
  trow2.append(mv.toStdString());
  table.append(trow2);
  trow3.append("");
  trow3.append("");
  trow3.append("");
  mv = QString::number(round(maxDay));
  trow3.append(mv.toStdString());
  table.append(trow3);
 
  return table;
}

boost::python::list VkoCalc::calcVkoYfxm()
{
  boost::python::list table;
  if (_dayData == 0 || !_dayData->IsInitialized()) {
    return table;
  }

  if (_dayData->has_error()) {
    error_log << QString::fromStdString(_dayData->error());
    return table;
  }
  if (_dayData->val_size() == 0) {
    return table;
  }

  //var(_dayData->DebugString());
  
  QList<int> lim;
  lim << 8 << 15 << 20;
  const climat::SurfDayValues& vals = _dayData->val(0);

  for (int lidx = 0; lidx < lim.count(); lidx++) {
    float cnt = 0;
    for (int yIdx = 0; yIdx < vals.avglist_size(); yIdx++) {
      for (int hidx = 0; hidx < vals.avglist(yIdx).avg_size(); hidx++) {
	if (vals.avglist(yIdx).avg(hidx) > lim.at(lidx)) {
	  cnt += 1;
	}
      }
    }
    cnt /= vals.avglist_size();
    //debug_log << "lim" << lim.at(lidx) << cnt;

    table.append(QString::number(cnt, 'f', 0).toStdString());
  }    

  return table;
}

boost::python::list VkoCalc::calcVkoP0m()
{
  boost::python::list table;
  // var(_yearData);
  // var(_yearData->DebugString());
  if (_yearData == nullptr || !_yearData->IsInitialized()) {
    return table;
  }
  
  if (_yearData->has_error()) {
    error_log << QString::fromStdString(_yearData->error());
    return table;
  }
  if (_yearData->val_size() == 0) {
    return table;
  }

  // var(_yearData->DebugString());

  const climat::SurfMonthValues& vals = _yearData->val(0);
  float val = 0;
  for (int hidx = 0; hidx < vals.avglist_size(); hidx++) {
    val += vals.avglist(hidx).avg();
  }
  val /= vals.avglist_size();
  
  // var(val);

  boost::python::list row;
  row.append(QString::number(val, 'f', 0).toStdString());
  table.append(row);
  
  return table;
}

boost::python::list VkoCalc::calcVkowm()
{
  boost::python::list table;
  
  if (_dayData == 0  || !_dayData->IsInitialized()) {
    return table;
  }
  
  if (_dayData->has_error()) {
    error_log << QString::fromStdString(_dayData->error());
    return table;
  }

  if (_dayData->val_size() == 0) {
    return table;
  }

  // var(_dayData->DebugString());

  boost::python::list res;
    
  //гроза (0x1 (<<0)), гололед (0x10(<<4)), град (0x8 (<<3)), метель (0x4 (<<2)), туман (0x2 (<<1)),
  //мгла (0x40 (<<6)), пыль  (0x20(<<5)), шквал (0x80 (<<7))
  QList<int> lim;
  lim << 0 << 4 << 3 << 2 << 1 << 6 << 5 << 7;
  
  for (int lidx = 0; lidx < lim.count(); lidx++) {
    ClimParam param;
    const climat::SurfDayValues& vals = _dayData->val(0);
      
    for (int yIdx = 0; yIdx < vals.avglist_size(); yIdx++) {
      param.count = 0;//число дней в месяце с явлением
      for (int hidx = 0; hidx < vals.avglist(yIdx).avg_size(); hidx++) {
	int mask = 1 <<  lim.at(lidx);
	if (0 != ((int)vals.avglist(yIdx).avg(hidx) & mask)) {
	  param.count += 1;
	}
      }
      // if (param.max < param.count || param.maxYear == 0) {
      // 	param.max = param.count;
      // 	param.maxYear = vals.avglist(yIdx).year();
      // }
      param.avg += param.count;
    }
    param.avg /= vals.avglist_size();
    
    // var(param.avg);
    
    res.append(QString::number(param.avg).toStdString());
    
  }

  table.append(res);
  return table;
}

boost::python::list VkoCalc::calcVkoTm()
{
  boost::python::list res;

  if (_allDataList == nullptr || !_allDataList->IsInitialized() ||
      _allDataList->all_size() == 0 ||
      _allDataList->all(0).val_size() == 0) {
    return res;
  }
  
  if (_allDataList->has_error()) {
    error_log << QString::fromStdString(_allDataList->error());
    return res;
  }

  
  //var(_allDataList->DebugString());

  // QStringList categ;
  // categ << QObject::tr("Средняя месячная") << QObject::tr("Средняя максимальная") 
  // 	  << QObject::tr("Средняя минимальная") << QObject::tr("Абсолютный максимум") 
  // 	<< QObject::tr("Абсолютный минимум");
  int categCnt = 3;

  QMap<int, QMap<int, float> > table; //декада, (категория, количество)
  
  const climat::SurfMonthAllValues& vals = _allDataList->all(0).val(0);

  QMap<int, QMap<int, ClimParam>> yearval;//год, параметры
  for (int hidx = 0; hidx < vals.avglist_size(); hidx++) {
    QDate date(vals.avglist(hidx).year(), 1, 1);
    int day = date.addDays(vals.avglist(hidx).day() - 1).day();
    int decade = dayToDecade(day);
    
    ClimParam& param = yearval[decade][vals.avglist(hidx).year()];
    ++param.count;
    param.avg += vals.avglist(hidx).avg();
    if (param.max < vals.avglist(hidx).avg() || param.maxYear == 0) {
      param.max = vals.avglist(hidx).avg();
      param.maxYear = vals.avglist(hidx).year();
    }
    if (param.min > vals.avglist(hidx).avg() || param.minYear == 0) {
      param.min = vals.avglist(hidx).avg();
      param.minYear = vals.avglist(hidx).year();
    }
  }
  
  ClimParam absExtrem;
  QMapIterator<int, QMap<int, ClimParam>> itd(yearval);
  while (itd.hasNext()) {
    itd.next();

    QMap<int, float> decval;
    for (int c = 0; c < categCnt; c++) {
      decval.insert(c, 0);
    }
    
    QMapIterator<int, ClimParam> it(itd.value());
    while (it.hasNext()) {
      it.next();
    
      decval[0] += it.value().avg / it.value().count;
      decval[1] += it.value().max;
      decval[2] += it.value().min;
      if (absExtrem.max < it.value().max || absExtrem.maxYear == 0) {
	absExtrem.max = it.value().max;
	absExtrem.maxYear = it.value().maxYear;
      }
      if (absExtrem.min > it.value().min || absExtrem.minYear == 0) {
	absExtrem.min = it.value().min;
	absExtrem.minYear = it.value().minYear;
      }
    }
    
    if (yearval.count() != 0) {
      decval[0] /= yearval.count();
      decval[1] /= yearval.count();
      decval[2] /= yearval.count();
      
      table.insert(itd.key(), decval);
    }
  }

  //var(table);

  for (int row = 0; row < categCnt; row++) {
    boost::python::list trow;
    float sum = 0;
    for (int mIdx = 0; mIdx <= 2; mIdx++) { //декада
      if (table.contains(mIdx)) {
	float val = table.value(mIdx).value(row);
	sum += val;
	trow.append(QString::number(round(val)).toStdString());
      } else {
	trow.append(std::string(""));
      }
    }
    trow.append(QString::number(sum / 3, 'f', 0 ).toStdString());
    res.append(trow);
  }

  boost::python::list trow1, trow2;
  trow1.append("");
  trow1.append("");
  trow1.append("");
  QString mv = QString::number(round(absExtrem.max)) + " (" + QString::number(absExtrem.maxYear) + "г.)";
  trow1.append(mv.toStdString());
  res.append(trow1);
  trow2.append("");
  trow2.append("");
  trow2.append("");
  mv = QString::number(round(absExtrem.min)) + " (" + QString::number(absExtrem.minYear) + "г.)";
  trow2.append(mv.toStdString());
  res.append(trow2);
 
  return res;
}

boost::python::list VkoCalc::calcVkoUm()
{
  boost::python::list table;
  // var(_yearData);
  // var(_yearData->DebugString());
  if (_yearData == nullptr || !_yearData->IsInitialized()) {
    return table;
  }
  
  if (_yearData->has_error()) {
    error_log << QString::fromStdString(_yearData->error());
    return table;
  }
  if (_yearData->val_size() == 0) {
    return table;
  }

  //var(_yearData->DebugString());

  const climat::SurfMonthValues& vals = _yearData->val(0);
  float val = 0;
  for (int hidx = 0; hidx < vals.avglist_size(); hidx++) {
    val += vals.avglist(hidx).avg();
  }
  val /= vals.avglist_size();
  
  // var(val);

  boost::python::list row;
  row.append(QString::number(val, 'f', 0).toStdString());
  table.append(row);
  
  return table;
}

boost::python::list VkoCalc::calcVkoff1m()
{
  boost::python::list res;
  // var(_aeroMonthData);
  // var(_aeroMonthData->DebugString());
  if (_aeroMonthData == nullptr || !_aeroMonthData->IsInitialized() ||
      _aeroMonthData->all_size() == 0) {
    return res;
  }

  if (_aeroMonthData->has_error()) {
    error_log << QString::fromStdString(_aeroMonthData->error());
    return res;
  }

  //var(_aeroMonthData->DebugString());

  QVector<QMap<int, float>> mval(2);//дескриптор, (давление, среднее)
  for (int didx = 0; didx < _aeroMonthData->all_size(); didx++) {
    for (int idx = 0; idx < _aeroMonthData->all(didx).val_size(); idx++) {
      float tlev = 0;
      for (int tidx = 0; tidx < _aeroMonthData->all(didx).val(idx).avglist_size(); tidx++) {
	const meteo::climat::AeroValuesList& val = _aeroMonthData->all(didx).val(idx).avglist(tidx);
	tlev += val.avg();
      }
      tlev /= _aeroMonthData->all(didx).val(idx).avglist_size();
      mval[didx].insert(_aeroMonthData->all(didx).val(idx).layer(), tlev);
    }
  }

  //var(mval);
  
  QList<int> levels;
  levels <<   0 << 1000 << 925 << 850 << 700 << 500 << 400 << 300
	 << 250 <<  200 << 150 << 100 <<  70 <<  50 <<  30 <<  20 << 10;

  for (int row = 0; row < levels.size(); ++row) {
    boost::python::list trow;
    for (int didx = 0; didx < mval.size(); ++didx) {
      if (mval[didx].contains(levels.at(row))) {
	float val = round(mval[didx].value(levels.at(row)));
	trow.append(QString::number(val).toStdString());
	if (didx == 1) {
	  trow.append(QString::number(val * 3.6, 'f', 0).toStdString());
	}
      } else {
	trow.append("");
	trow.append("");
      }
    }
    
    res.append(trow);
  }

  return res;
}

boost::python::list VkoCalc::calcVkoTropom()
{
  boost::python::list rtable;
  if (_aeroMonthData == 0 || !_aeroMonthData->IsInitialized() ||
      _aeroMonthData->all_size() == 0 ||
      _aeroMonthData->all(0).val_size() == 0) {
    return rtable;
  }

  if (_aeroMonthData->has_error()) {
    error_log << QString::fromStdString(_aeroMonthData->error());
    return rtable;
  }

  //var(_aeroMonthData->DebugString());
  
  QPair<float, float> table = QPair<float, float>(0, 0); // (высота, темература)
  
  int cnt = 0; //количество значений

  for (int idx = 0; idx < _aeroMonthData->all(0).val_size(); idx++) {
    float height = _aeroMonthData->all(0).val(idx).layer();
    if (height <= 0) continue;
    for (int tidx = 0; tidx < _aeroMonthData->all(0).val(idx).avglist_size(); tidx++) {
      const meteo::climat::AeroValuesList& val = _aeroMonthData->all(0).val(idx).avglist(tidx);
      float curtemp = val.avg();
      cnt += 1;
      table.first += height;
      table.second += curtemp;
  }
  }

  // var(table);
  // var(cnt);
  
  boost::python::list rowH;
  boost::python::list rowT;
  rowH.append(QString::number(table.first / cnt, 'f', 0).toStdString());
  rowT.append(QString::number(table.second / cnt, 'f', 0).toStdString());
  
  rtable.append(rowH);
  rtable.append(rowT);
    
  return rtable;
}

boost::python::list VkoCalc::calcVkoTgm()
{
  boost::python::list res;
    
  if (_allDataList == nullptr || !_allDataList->IsInitialized() ||
      _allDataList->all_size() == 0 ||
      _allDataList->all(0).val_size() == 0) {
    error_log << QObject::tr("Нет данных");
    return res;
  }
  
  if (_allDataList->has_error()) {
    error_log << QString::fromStdString(_allDataList->error());
    return res;
  }

  
  //var(_allDataList->DebugString());

  // QStringList categ;
  // categ << QObject::tr("Средняя месячная") << QObject::tr("Средняя максимальная") 
  // 	  << QObject::tr("Средняя минимальная") << QObject::tr("Абсолютный максимум") 
  // 	<< QObject::tr("Абсолютный минимум");
  int categCnt = 3;

  QMap<int, QMap<int, float> > table; //декада, (категория, количество)
  
  const climat::SurfMonthAllValues& vals = _allDataList->all(0).val(0);

  QMap<int, QMap<int, ClimParam>> yearval;//год, параметры
  for (int hidx = 0; hidx < vals.avglist_size(); hidx++) {
    QDate date(vals.avglist(hidx).year(), 1, 1);
    int day = date.addDays(vals.avglist(hidx).day() - 1).day();
    int decade = dayToDecade(day);
    
    ClimParam& param = yearval[decade][vals.avglist(hidx).year()];
    ++param.count;
    param.avg += vals.avglist(hidx).avg();
    if (param.max < vals.avglist(hidx).avg() || param.maxYear == 0) {
      param.max = vals.avglist(hidx).avg();
      param.maxYear = vals.avglist(hidx).year();
    }
    if (param.min > vals.avglist(hidx).avg() || param.minYear == 0) {
      param.min = vals.avglist(hidx).avg();
      param.minYear = vals.avglist(hidx).year();
    }
  }
  
  ClimParam absExtrem;
  QMapIterator<int, QMap<int, ClimParam>> itd(yearval);
  while (itd.hasNext()) {
    itd.next();

    QMap<int, float> decval;
    for (int c = 0; c < categCnt; c++) {
      decval.insert(c, 0);
    }
    
    QMapIterator<int, ClimParam> it(itd.value());
    while (it.hasNext()) {
      it.next();
    
      decval[0] += it.value().avg / it.value().count;
      decval[1] += it.value().max;
      decval[2] += it.value().min;
      if (absExtrem.max < it.value().max || absExtrem.maxYear == 0) {
	absExtrem.max = it.value().max;
	absExtrem.maxYear = it.value().maxYear;
      }
      if (absExtrem.min > it.value().min || absExtrem.minYear == 0) {
	absExtrem.min = it.value().min;
	absExtrem.minYear = it.value().minYear;
      }      
    }
    
    if (yearval.count() != 0) {
      decval[0] /= yearval.count();
      decval[1] /= yearval.count();
      decval[2] /= yearval.count();
      
      table.insert(itd.key(), decval);
    }
  }

  //var(table);

  for (int row = 0; row < 1 /*categCnt*/; row++) {
    boost::python::list trow;
    float sum = 0;
    for (int mIdx = 0; mIdx <= 2; mIdx++) { //декада
      if (table.contains(mIdx)) {
	float val = table.value(mIdx).value(row);
	sum += val;
	trow.append(QString::number(round(val)).toStdString());
      } else {
	trow.append(std::string(""));
      }
    }
    trow.append(QString::number(sum / 3, 'f', 0 ).toStdString());
    res.append(trow);
  }
  boost::python::list trow1, trow2;
  trow1.append("");
  trow1.append("");
  trow1.append("");
  QString mv = QString::number(round(absExtrem.max)) + " (" + QString::number(absExtrem.maxYear) + "г.)";
  trow1.append(mv.toStdString());
  res.append(trow1);
  trow2.append("");
  trow2.append("");
  trow2.append("");
  mv = QString::number(round(absExtrem.min)) + " (" + QString::number(absExtrem.minYear) + "г.)";
  trow2.append(mv.toStdString());
  res.append(trow2);
 
  return res;
}

boost::python::list VkoCalc::calcVkossm()
{
  boost::python::list res;
  
  if (_decadeData == 0 || !_decadeData->IsInitialized() || _decadeData->val_size() == 0 ) {
    error_log << QObject::tr("Нет данных");
    return res;
  }

  if (_decadeData->has_error()) {
    error_log << QString::fromStdString(_decadeData->error());
    return res;
  }
  
  // var(_decadeData->DebugString());

  int idx = 0;
  
  const climat::SurfDecadeValues& vals = _decadeData->val(idx);
  QVector<ClimParam> decade(3);

  for (int hIdx = 0; hIdx < vals.avglist_size(); hIdx++) {
    if (vals.avglist(hIdx).decade() > 3) continue;
    if (vals.avglist(hIdx).avg() != 0) {
      int decIdx = vals.avglist(hIdx).decade() - 1;
      // var(decIdx);
      decade[decIdx].count += 1;
      decade[decIdx].avg += vals.avglist(hIdx).avg();
      if (decade[decIdx].max < vals.avglist(hIdx).avg() || decade[decIdx].maxYear == 0) {
	decade[decIdx].max = vals.avglist(hIdx).avg();
	decade[decIdx].maxYear = vals.avglist(hIdx).year();
      }
    } 
  }

  // var(decade[0].max);
  
  for (int i = 0; i < decade.size(); i++) {
    // if (decade.at(i).count == 0 || decade.at(i).count < vals.avglist_size() / 3 / 2) { //1 одна декада, 50 %
    //   decade[i].avg = 0;
    // } else 
    {
      decade[i].avg = round(decade[i].avg / decade.at(i).count);
    }
  }

  float sum = 0;
  int scnt = 0;
  boost::python::list trow;
  for (int d = 0 ; d < 3; d++) {
    if (decade.at(d).avg > 0) {
      sum += decade.at(d).avg;
      scnt++;
      trow.append(QString::number(decade.at(d).avg, 'f', 0).toStdString());
    } else {
      trow.append("");
    }
  }
  trow.append(QString::number(sum / scnt, 'f', 0).toStdString());
  res.append(trow);

  float max = 0;
  boost::python::list trow1;
  for (int d = 0 ; d < 3; d++) {
    if (decade.at(d).max > 0) {
      if (max < decade.at(d).max) {
	max = decade.at(d).max;
      }
      trow1.append(QString::number(decade.at(d).max, 'f', 0).toStdString());
    } else {
      trow1.append("");
    }
  }
  trow1.append(QString::number(max, 'f', 0).toStdString());
  res.append(trow1);  

  return res;
}

boost::python::list VkoCalc::calcVkoYssm()
{
 boost::python::list res;
  
  if (_dateData == 0 || !_dateData->IsInitialized()) {
    return res;
  }

  if (_dateData->has_error()) {
    error_log << QString::fromStdString(_dateData->error());
    return res;
  }

  //var(_dateData->DebugString());

  // образования средняя; разрушения средняя
  QMap<int, QDate> dt; //категория, дата
  float early = 0, late = 0;

  int startCnt = 0;
  int endCnt = 0;  
  for (int idx = 0; idx < _dateData->val_size(); idx++) {
    QDate start = QDate::fromString(QString::fromStdString(_dateData->val(idx).start_date()), Qt::ISODate);
    QDate end = QDate::fromString(QString::fromStdString(_dateData->val(idx).end_date()), Qt::ISODate);
    if (start.isValid()) {
      ++startCnt;
      early += start.dayOfYear();
    }
    if (end.isValid()) {
      
      ++endCnt;
      late += end.dayOfYear();
    }
  }

  QDate cur(1, 1, 1);
  if (early != 0 && startCnt != 0) {
    early /= startCnt;    
    dt.insert(0, cur.addDays(early - 1));
  }
  if (late != 0 && endCnt != 0) {
    late /= endCnt;    
    dt.insert(1, cur.addDays(late - 1));
  }

  // debug_log;
  // qDebug() << "dt" << dt;
  
  for (int idx = 0; idx < 2; idx++) {
    boost::python::list trow;
    if (!dt.contains(idx)) {
      trow.append(std::string(""));
    } else {
      trow.append(dt.value(idx).toString("dd.MM").toStdString());
    }
    res.append(trow);
  }
  
  return res;
}



BOOST_PYTHON_MODULE(libvkocalc)
{
  boost::python::class_<VkoCalc>("VkoCalc")
    .def("setPath", &VkoCalc::setPath,  boost::python::args("path"))
    .def("setStation", &VkoCalc::setStation, boost::python::args("station", "diffutc"))
    .def("setDt",  &VkoCalc::setDt, boost::python::args("start", "end", "month"))
    .def("isInit", &VkoCalc::isInit)
    .def("getClimatData",   &VkoCalc::getClimatData, boost::python::args("type"))
    .def("calcTableValues", &VkoCalc::calcTableValues)
    .def("calcTableParamValues", &VkoCalc::calcTableParamValues, boost::python::args("val1", "val2"));
  
}

