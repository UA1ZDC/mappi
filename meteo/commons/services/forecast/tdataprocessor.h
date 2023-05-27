#ifndef METEO_COMMONS_FORECAST_TDATAPROCESSOR_H
#define METEO_COMMONS_FORECAST_TDATAPROCESSOR_H

#include <qobject.h>
#include <QtScript/qscriptvalue.h>

#include "tdataprovider.h"

#include <meteo/commons/zond/zond.h>
#include <meteo/commons/zond/turoven.h>

const float kBadValue = -9999.;
class TDataProcessor: public QObject
{
  Q_OBJECT

public:

  TDataProcessor(TDataProvider* adataProvider): dataProvider_(adataProvider){}

  Q_INVOKABLE bool resetData();

  Q_INVOKABLE float getZondValuePoUrType(const StationData&,int gr_type, int level, int type);
  Q_INVOKABLE float getZondValuePoDescr(const StationData&,int gr_type, int level, int descr);

  Q_INVOKABLE float getHpoT(const StationData&,double valT, int srok);

  Q_INVOKABLE float getUr(const StationData&,int level, int type); //!< Возвращает данные на заданном уровне давления
  Q_INVOKABLE float getDataFromHeight(const StationData&,int H, int t, const QScriptValue& valueType); //!< Возвращает данные на заданной высоте, если нужны прогностические то выбираем t не ноль
  Q_INVOKABLE float getDataFromP(const StationData&,int P, int t, const QScriptValue& valueType); //!< Возвращает данные на заданной высоте, если нужны прогностические то выбираем t не ноль

  //! Функции дла вычисления значений параметров
  Q_INVOKABLE float calcPNS(const StationData&,const QScriptValue &funcName);  //!< Потенциально неустойчивый слой - значение параметра
  Q_INVOKABLE double oprTPot(const QScriptValue& PVal, const QScriptValue& TdVal);
//  Q_INVOKABLE double oprTsost(const QScriptValue& PVal);
  Q_INVOKABLE void advData(const StationData &);//!< Заполнение зонда на основе прогностических (адвективных) данных
  Q_INVOKABLE double oprTsostAdv(const StationData &,const QScriptValue &PVal); //!< получение температуры на кривой состояния из прогностического зонда
//  Q_INVOKABLE QDateTime getSunRise(int year, int month, int day, double d_fi0, double d_la0, int i_N, const QScriptValue& typeSun);  //!< Восход солнца
  Q_INVOKABLE float getUrOsobPoTemp(const StationData&,int N, const QScriptValue& urValueType); //!< Список особых точек по температуре
  Q_INVOKABLE float getPrizInvData(const StationData&,const QScriptValue& urValueType); //!< Данные на уровне приземной инверсии
  Q_INVOKABLE float getInvData(const StationData&,const QScriptValue& urValueType); //!< Данные на уровне обычной инверсии
  //Q_INVOKABLE float getUrzData(const QScriptValue& dataType);//!< Для получения данных на уровне земли
  Q_INVOKABLE float getH_obled(const StationData&,int ); //!< Высота нижней границы обледенения,м
  Q_INVOKABLE float getT_obled(const StationData&,int ); //!< Температура воздуха на нижней границе обледенения,°С

  Q_INVOKABLE float getNalOblak(const StationData&,int, double ); //!< Наличие облачности на высоте

  Q_INVOKABLE float getUrzDataAdv(const StationData&,const QScriptValue& dataType);//!< Для получения данных на уровне земли из прогностического зонда

  Q_INVOKABLE float oprTsmUrzem0(const StationData&);
  Q_INVOKABLE float opredHPoT(const StationData&,double , int );

  Q_INVOKABLE float advParam(const StationData &, float level, int ); //!< получение прогностических данных из прогностического зонда. предварительно нужно заполнить этот зонд - метод advData()

  //! метод Вельтищева
  Q_INVOKABLE float oprPsostRaznAllFastDelta(const StationData &);
  Q_INVOKABLE float oprPsostRaznAllFastP(const StationData &);
  Q_INVOKABLE float oprHmaxVFastH(const StationData &,int srok);
//  Q_INVOKABLE float oprHmaxVFastHprog(int h); //!< прогностическая высота максимального ветра

  Q_INVOKABLE float oprHmaxVFastV(const StationData &,int srok);
  Q_INVOKABLE float oprHmaxVFastVprog(const StationData &,int srok);


  Q_INVOKABLE float oprHmaxVFastH1(const StationData &);
  Q_INVOKABLE float oprHmaxVFastH2(const StationData &);
  Q_INVOKABLE float oprMaxWind(const StationData &st, int min_h, int max_h, int srok); //!<скорость максимального ветра
  Q_INVOKABLE float oprHmaxWind(const StationData &st, int min_h, int max_h, int srok); //!<Высота максимального ветра

  Q_INVOKABLE float oprHSloyWind(const StationData &st, double, int min_h, int max_h, int srok); //!<слой максимального ветра

//  Q_INVOKABLE float oprVeljtishevHRes(const QScriptValue &HVal,const QScriptValue &deltaHVal, const QScriptValue &VmaxVal);//!< метод Вельтищева, изменение высоты
//  Q_INVOKABLE float oprVeljtishevVRes(const QScriptValue &HVal, const QScriptValue &VmaxVal);//!< метод Вельтищева, изменение макс. скорости
//  double pic9_19a(double Vmax, double dH); //!< метод Вельтищева, график 9_19а - прогноз изменения уровня макс. ветра на 12 ч, лето
//  double pic9_20a(double Vmax, double H); //!< метод Вельтищева, график 9_20а - прогноз изменения максимальной скорости ветра на 12 ч, лето
//  double pic9_19b(double Vmax, double dH); //!< метод Вельтищева, график 9_19а - прогноз изменения уровня макс. ветра на 12 ч, зима
//  double pic9_20b(double Vmax, double H); //!< метод Вельтищева, график 9_20а - прогноз изменения максимальной скорости ветра на 12 ч, зима

  Q_INVOKABLE double pic( const QScriptValue &arr1,const QScriptValue &arr2,const QScriptValue &arr3,const QScriptValue &val1, const QScriptValue &val2);

  Q_INVOKABLE float oprIntLivnOrl(const StationData &); //!< Интенсивность ливневых осадков по методу Орловой
  Q_INVOKABLE float oprSrKonvSkor(const StationData &,const QScriptValue &P1Val, const QScriptValue &P2Val ); //!< Средняя скорость перемещения конвективных облаков, км/ч
  Q_INVOKABLE double oprTsost(const StationData &,const QScriptValue &PVal); //!< получение температуры на кривой состояния по фактическим данным
  Q_INVOKABLE double calcDist(const meteo::GeoPoint&,const meteo::GeoPoint&);
  Q_INVOKABLE double getValue(const StationData &,int level, int type, const QString &descr,int srok);
  Q_INVOKABLE int offsetFromUtc();

private:

   TDataProvider* dataProvider_;

};


#endif
