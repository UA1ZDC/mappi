#include "atmosphysics.h"

#include "stdio.h"


/*!
 * \brief Расчет абсолютной влажности воздуха (г/м3)
 * \param partialPressure - парциальное давление водяного пара, гПа
 * \param temperature - температура воздуха, К
 */
double MnMath::absolutMoisture(double wetPartialPressure, double temperature) //g/m3 from hPa
{
  return 217.*wetPartialPressure/temperature;
}

/*!
 * \brief Расчет парциального давления водяного пара в воздухе (гПа)
 * \param wetMassFraction - массовая доля водяного пара, г/кг
 * \param wetAirPressure - давление влажного воздуха, гПа 
 */
double MnMath::wetPartialPressure1(double wetMassFraction, double wetAirPressure) //hPa from g/kg
{
 return wetAirPressure*wetMassFraction/(621.98+0.378*wetMassFraction);
}

/*!
 * \brief Расчет парциального давления водяного пара в воздухе (гПа)
 * \param wetDensity - плотность водяного пара, кг/м3
 * \param temperature - температура, К 
 */
double MnMath::wetPartialPressure2(double wetDensity, double temperature) //hPa from kg/m3
{
 return 0.01*SPECIFIC_WATER_VAPOUR_CONSTANT*wetDensity*temperature;
}

/*!
 * \brief Расчет плотности водяного пара в воздухе (кг/м3)
 * \param dryAirDensity - плотность сухого воздуха, кг/м3
 * \param temperature - температура, К 
 */
double MnMath::wetDensity(double dryAirDensity, double wetMassFraction) //kg/m3 from g/kg
{
 return dryAirDensity*wetMassFraction/(1-wetMassFraction);
}


/*!
 * \brief Расчет давления сухого воздуха (гПа)
 * \param dryAirDensity - плотность сухого воздуха, кг/м3
 * \param temperature - температура, К 
 */
double MnMath::dryAirPressure(double dryAirDensity, double temperature) //hPa from kg/m3
{
 return 0.01*SPECIFIC_DRY_AIR_CONSTANT*dryAirDensity*temperature;
}

/*!
 * \brief Расчет давления влажного воздуха (гПа)
 * \param dryAirDencity - плотность сухого воздуха, кг/м3
 * \param temperature - температура, К 
 * \param wetMassFraction - массовая доля водяного пара, г/г (доля единицы)
 */
double MnMath::wetAirPressure(double dryAirDensity, double temperature, double wetMassFraction) //hPa from kg/m3 K g/g
{
 return 0.01*SPECIFIC_DRY_AIR_CONSTANT*dryAirDensity*temperature*(0.608*wetMassFraction+1)/(1-wetMassFraction);
}

/*!
 * \brief Расчет давления влажного воздуха (гПа)
 * \param dryAirPressure - давление сухого воздуха, кг/м3
 * \param wetMassFraction - массовая доля водяного пара, г/кг
 */
double MnMath::wetAirPressure(double dryAirPressure, double wetMassFraction) //hPa from g/kg
{
 return dryAirPressure*(0.608*wetMassFraction+1)/(1-wetMassFraction);
}

/*!
 * \brief Расчёт максимальной  концентрации Nm электронов по критической частоте f0
 * \param Nm Максимальная концентрация электронов, см-3
 * \param f0 Критическая частота, Гц
 */
double MnMath::f0ToNm(double f0)
{
 return f0*f0/8.06e+7;
}

/*!
 * \brief Расчёт критической частоты f0 по максимальной  концентрации Nm электронов
 * \param Nm Максимальная концентрация электронов, см-3
 * \param f0 Критическая частота, Гц
 */
double MnMath::NmTof0(double Nm)
{
 return sqrt(8.06e+7*Nm);
}

/*!Расчет индекса F10.7 по известному значению числа Вольфа
 */
double MnMath::WtoF107(double W)
{
 return 63.75+W*(0.728+W*0.00089);
}

/*!
 * \brief Расчет средней относительной скорости движения двух частиц, см/сек
 * \param particleMass1 - масса первой частицы, г
 * \param particleMass2 - масса второй частицы, г 
 * \param temperature - температура газа, градус
 */
double MnMath::average2ParticlesRelateVelocity(double particleMass1, double particleMass2, double temperature)
{
   return sqrt(8.*BOLTSMAN_CONSTANT*temperature*(particleMass1+particleMass2)/(M_PI*particleMass1*particleMass2));
}

/*!
 * \brief Расчет средней скорости движения частиц, см/сек
 * \param particleMass - масса частицы, г
 * \param temperature - температура газа, градус Кельвина
 */
double MnMath::averageParticleVelocity(double particleMass, double temperature)
{
   return sqrt(8.*BOLTSMAN_CONSTANT*temperature/(M_PI*particleMass));
}
/*!
 * \brief Расчет средней кинетической энергии частиц, Эрг
 * \param temperature - температура газа, градус Кельвина
 */
double MnMath::averageParticleKineticEnergy(double temperature)
{
  return 3./2.*BOLTSMAN_CONSTANT*temperature;
}

/*!
 * \brief Расчет частот соударений частиц двух типов без заряда, Гц
 * \param particleRadius1 - радиус первой частицы, г
 * \param particleRadius2 - радиус второй частицы, г 
 * \param particleMass1 - масса первой частицы, г
 * \param particleMass2 - масса второй частицы, г 
 * \param temperature - температура газа, градус Кельвина
 */
double MnMath::twoUnchargedParticlesCollisionFrequency(double particleRadius1, double particleRadius2, double particleMass1, double particleMass2, double temperature)
{
 return 4./3.*M_PI*(particleRadius1+particleRadius2)*(particleRadius1+particleRadius2)*average2ParticlesRelateVelocity(particleMass1, particleMass2, temperature);
}

/*!
 * \brief Расчет частоты соударений электронов и ионов, Гц
 * \param electronDensity - концентрация электронов, см-3
 * \param electronTemperature - температура электронов, К
 */
double MnMath::electronIonCollisionFrequency(double electronDensity, double electronTemperature)
{
   return 5.5*electronDensity*pow(electronTemperature,-1.5)*log(220.*electronTemperature*pow(electronDensity,-0.3333));
}

/*!
 * \brief Расчет частоты соударений электронов и ионов в двухтемпературной плазме, Гц
 * \param electronDensity - концентрация электронов, см-3
 * \param electronTemperature - температура электронов, К
 * \param ionTemperature - температура ионов, К
 */
double MnMath::electronIonCollisionFrequency(double electronDensity, double electronTemperature, double ionTemperature)
{
   return 5.5*electronDensity*pow(electronTemperature,-1.5)*(log(280.*electronTemperature*pow(electronDensity,-0.3333))+log(ionTemperature/electronTemperature)/3.);
}

/*!
 * \brief Быстрый расчет приблизительной частоты соударений электронов и ионов, Гц
 * \param electronDensity - концентрация электронов, см-3
 * \param electronTemperature - температура электронов, К
 */
double MnMath::electronIonCollisionFrequencyFast(double electronDensity, double electronTemperature)
{
   return 54.5*electronDensity*pow(electronTemperature,-1.5);
}

/*!
 * \brief Расчет частоты соударений электронов и молекул O2, Гц
 * \param neutralO2Density - концентрация молекул O2, см-3
 * \param electronTemperature - температура электронов, К
 */
double MnMath::electronO2CollisionFrequency(double neutralO2Density, double electronTemperature)
{
   return 1.82E-10*neutralO2Density*(1+3.6E-2*sqrt(electronTemperature))*sqrt(electronTemperature);
}

/*!
 * \brief Расчет частоты соударений электронов и молекул N2, Гц
 * \param neutralN2Density - концентрация молекул N2, см-3
 * \param electronTemperature - температура электронов, К
 */
double MnMath::electronN2CollisionFrequency(double neutralN2Density, double electronTemperature)
{
   return 2.33E-11*neutralN2Density*(1-1.21E-4*electronTemperature)*electronTemperature;
}

/*!
 * \brief Расчет частоты соударений электронов и атомов O, Гц
 * \param neutralODensity - концентрация атомов O, см-3
 * \param electronTemperature - температура электронов, К
 */
double MnMath::electronOCollisionFrequency(double neutralODensity, double electronTemperature)
{
   return 8.90E-10*neutralODensity*(1+5.7E-4*electronTemperature)*sqrt(electronTemperature);
}

/*!
 * \brief Расчет частоты соударений электронов и атомов H, Гц
 * \param neutralHDensity - концентрация атомов H, см-3
 * \param electronTemperature - температура электронов, К
 */
double MnMath::electronHCollisionFrequency(double neutralHDensity, double electronTemperature)
{
   return 4.50E-9*neutralHDensity*(1-1.35E-4*electronTemperature)*sqrt(electronTemperature);
}

/*!
 * \brief Расчет частоты соударений электронов и атомов He, Гц
 * \param neutralHeDensity - концентрация атомов He, см-3
 * \param electronTemperature - температура электронов, К
 */
double MnMath::electronHeCollisionFrequency(double neutralHeDensity, double electronTemperature)
{
   return 4.60E-10*neutralHeDensity*sqrt(electronTemperature);
}

/*!
 * \brief Расчет частоты соударений электронов и нейтралов одного из видов (атомов, молекул), Гц
 * \param anyNeutralRadius - радиус нейтрала, см 
 * \param anyNeutralDensity - концентрация нейтралов, см-3
 * \param electronTemperature - температура электронов, К
 */
double MnMath::electronAnyNeutralCollisionFrequency(double anyNeutralRadius, double anyNeutralDensity, double electronTemperature)
{
   return 8.3E+5*M_PI*anyNeutralRadius*anyNeutralRadius*anyNeutralDensity*sqrt(electronTemperature);
}

/*!
 * \brief Расчет частоты соударений электронов и нейтралов всех видов, Гц
 * \param collisionFrequencies - частоты соударений электронов с нейтралами каждого из видов, Гц
 * \param countNeutrals - количество видов нейтралов
 */
double MnMath::electronNeutralCollisionFrequency(const double* collisionFrequencies, int countNeutrals)
{
  double val = 0.;
  for(int i = 0; i < countNeutrals;i++)
  {
    val+=collisionFrequencies[i];
  }
  return val;
}

/*!
 * \brief Расчет частоты соударений электронов с другими частицами, Гц
 * \param collisionFrequencies - частоты соударений электронов с нейтралами каждого из видов, Гц
 * \param countNeutrals - количество видов нейтралов
 * \param electronIonCollisionFrequency - частота соударений электронов с ионами, Гц
 */
double MnMath::electronCollisionFrequency(const double* collisionFrequencies, int countNeutrals, double electronIonCollisionFrequency)
{
 return MnMath::electronNeutralCollisionFrequency(collisionFrequencies, countNeutrals)+electronIonCollisionFrequency;
}

/*!
 * \brief Расчет частоты ионно-электронных соударений, Гц
 * \param ionDensity - концентрация ионов данного типа, см-3
 * \param electronDensity - концентрация электронов, см-3* 
 * \param electronIonCollisionFrequency - частота электронно-ионных соударений, Гц
 */
double MnMath::ionElectronCollisionFrequency(double ionDensity, double electronDensity, double electronIonCollisionFrequency)
{
 return electronDensity/ionDensity*electronIonCollisionFrequency;
}

/*!
 * \brief Расчет частоты ионно-ионных соударений, Гц
 * \param ionMass - масса иона данного вида, г
 * \param ionDensity - концентрация ионов всех видов, см-3
 * \param ionTemperature - температура ионов, градус Кельвина
 */
double MnMath::ionIonCollisionFrequency(double ionMass, double ionsDensity, double ionTemperature)
{
 return 3.9*ionsDensity*pow(ionTemperature,-1.5)*sqrt(ELECTRON_MASS*1E3/ionMass)*log(220.*ionTemperature*pow(ionsDensity, -0.33333));
}

/*!
 * \brief Расчет частоты соударений иона с нейтралами, Гц
 * \param ionMass - масса иона данного вида, г
 * \param neutralsDensity - концентрация нейтралов всех видов, см-3
 * \param ionTemperature - температура ионов, градус Кельвина
 */
double MnMath::ionNeutralCollisionFrequency(double ionMass, double neutralsDensity, double ionTemperature)
{
  return 16.*sqrt(2.)*4.5E-16/3.*MnMath::averageParticleVelocity(ionMass,ionTemperature)*neutralsDensity;
}

/*!
 * \brief Расчет частоты соударений иона, Гц
 * \param ionMass - масса иона, г
 * \param ionsDensity - концентрация ионов данного вида, см-3
 * \param neutralDensity - концентрация нейтралов, см-3
 * \param ionTemperature - температура ионов, градус Кельвина
 * \param electronDensity - концентрация электронов, см-3* 
 * \param electronIonCollisionFrequency - частота электронно-ионных соударений, Гц
 */
double MnMath::ionCollisionFrequency(double ionMass, double ionDensity, double neutralsDensity, double electronsDensity, double ionTemperature, double electronTemperature)
{
  return MnMath::ionNeutralCollisionFrequency(ionMass, neutralsDensity, ionTemperature)+
         MnMath::ionIonCollisionFrequency(ionMass,electronsDensity,ionTemperature)+
	 MnMath::ionElectronCollisionFrequency(ionDensity, electronsDensity, electronIonCollisionFrequency(electronsDensity, electronTemperature, ionTemperature));
}


