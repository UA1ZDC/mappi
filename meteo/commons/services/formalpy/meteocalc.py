#!/usr/bin/python3
# coding: utf-8

import sys

import math
import datetime
import logging

from meteoglobal import frange, error_log
from libzondwrap import *
import wind_increment

kKelvin = 273.15

# вычисляет влажность по заданным температуре и температуре росы
def humidity(T, Td):
  if T < -10:
    a = 9.5
    b = 265.5
  else:
    a = 7.63
    b = 241.9
  tmp1 = (a * Td) / (b + Td)
  tmp2 = (a * T) / (b + T)
  tmp1 = tmp1 - tmp2
  tmp2 = 10 ** tmp1
  if tmp2 < 0:
    result = 0
  elif tmp2 > 1:
    result = 1
  else:
    result = tmp2
  return result * 100


def calcAvgWindBy12(dd12, ff12):
  '''
  Функция для рассчет приращения направления и скорости среднеого ветра в слоях 0-14, 0-16, ..., 0-30 км
  к среднему ветру слоя 0-12
  :param dd12: направление ветра в слое 0-12км в градусах
  :param ff12: скорость ветра в слое 0-12км в м/с
  :return: словарь словарей. Ключ каждого словаря - верхняя граница слоя. В каждом внутреннем словаре
  хранятся поля dd, ff - направление и скорость ветра соответственно
  '''

  # проверка переданных значений
  if not dd12 or not ff12:
    return None

  # определение времени года
  month = datetime.datetime.now().month
  time_of_the_year = ''
  if 3 <= month <= 5:
    time_of_the_year = 'spring'
  elif 6 <= month <= 8:
    time_of_the_year = 'summer'
  elif 9 <= month <= 11:
    time_of_the_year = 'autumn'
  else:
    time_of_the_year = 'winter'

  # вычисления производятся с расчетом на то, что направление ветра передано как целое число
  try:
    dd12 = int(dd12)
  except:
    return None

  # определение направления ветра
  wind_direction = ''
  if 11.25 < dd12 < 33.75:
    wind_direction = 'ssv'
  elif 33.75 < dd12 < 56.25:
    wind_direction = 'sv'
  elif 56.25 < dd12 < 78.75:
    wind_direction = 'vsv'
  elif 78.75 < dd12 < 101.25:
    wind_direction = 'v'
  elif 101.25 < dd12 < 123.75:
    wind_direction = 'vyv'
  elif 123.75 < dd12 < 146.25:
    wind_direction = 'yv'
  elif 146.25 < dd12 < 168.75:
    wind_direction = 'yyv'
  elif 168.75 < dd12 < 192.25:
    wind_direction = 'y'
  elif 192.25 < dd12 < 213.75:
    wind_direction = 'yyz'
  elif 213.75 < dd12 < 236.25:
    wind_direction = 'yz'
  elif 236.25 < dd12 < 258.75:
    wind_direction = 'zyz'
  elif 258.75 < dd12 < 281.25:
    wind_direction = 'z'
  elif 281.25 < dd12 < 303.75:
    wind_direction = 'zsz'
  elif 303.75 < dd12 < 326.25:
    wind_direction = 'sz'
  elif 326.25 < dd12 < 348.75:
    wind_direction = 'ssz'
  else:
    wind_direction = 's'

  # опредление скорости ветра
  wind_speed = ''
  if ff12 <= 10:
    wind_speed = '10'
  else:
    wind_speed = '11'

  variable_dd_name = '{0}_{1}_{2}_{3}'.format(time_of_the_year, wind_speed, wind_direction, 'dd')
  variable_ff_name = '{0}_{1}_{2}_{3}'.format(time_of_the_year, wind_speed, wind_direction, 'ff')

  levels = {}

  for i in range(0, len(wind_increment.layers)):
    levels[wind_increment.layers[i]] = {'dd' : dd12 + getattr(wind_increment, variable_dd_name)[i],
                   'ff' : ff12 + getattr(wind_increment, variable_ff_name)[i]}
  return levels

def calcAvgWind(zond, levels):
  zond.DF2UV()
  for level in levels:
    # logging.error(level['h1'])
    h1 = level['h1']
    h2 = level['h2']
    dd = zond.getAverWindD(h1, h2)
    ff = zond.getAverWindF(h1, h2)
    if not dd == -9999:
      level['dd'] = dd
    if not ff == -9999:
      level['ff'] = ff

def calcBallisticWind2434(zond, h0=None):
  '''
  Функция, вычисляющая значение скорости баллистического ветра на высотах 24 и 34 км
  :param zond: зонд в виде TZond
  :param h0: высота на уровне станции
  :return: скорость ветра на высотах 24 и 34 в м/с
  '''
  if h0 is None:
    ur_station = Uroven()
    zond.getUrz(ur_station)
    if ur_station.isGood(ValueType.UR_H):
      h0 = ur_station.value(ValueType.UR_H)
    else:
      h0 = int(zond.alt())
      ur_station.set(ValueType.UR_H, h0, 1)
      zond.setData(1,0, ur_station)
  zond.DF2UV()
  af10 = zond.getAverWindf(h0, 10000)
  af24 = zond.getAverWindf(h0, 24000)
  af30 = zond.getAverWindf(h0, 30000)
  ff24 = ff34 = -9999
  if af10 != -9999 and af24 != -9999:
    ff24 = -0.5*af10 + 1.5*af24
  if af10 != -9999 and af24 != -9999:
    ff34 = -0.3*af10 + 1.3*af34
  return ff24, ff34

def calcDeltaTemp(zond, h0, h1):
  '''
  Вычисляет среднее отклонение температуры от табличного значения на заданной высоте
  :param zond: Зонд TZond
  :param h0: Нижняя граница слоя (обычно высота станции)
  :param h1: Верхняя граница слоя
  :return: Среднее отклонение температуры в градусах цельсия
  '''
  zond.DF2UV()
  urList = zond.urovenList()
  sum = 0
  hs = []
  hs.append(h0)

  for ur in urList:
    if not urList[ur].isGood(ValueType.UR_H):
      continue
    urh = urList[ur].value(ValueType.UR_H)
    if h0 <= urh <= h1:
      hs.append(urh)
  hs.sort()

  for ur in urList:
    urh = urList[ur].value(ValueType.UR_H)
    if urh < h0 or urh > h1:
      continue
    if hs.index(urh) == 0:
      continue
    if not urList[ur].isGood(ValueType.UR_T):
      continue
    T = urList[ur].value(ValueType.UR_T)
    T += virtualCorrection(T)
    T -= tableT(urh)
    previousH = hs[hs.index(urh) - 1]
    sum += T*(urh - previousH)
  return sum/float(h1)

def calcAvgDevT(zond, levels):
  h1 = []
  h2 = []
  zond.DF2UV()
  for level in levels:
    h1.append(level['h1'])
    h2.append(level['h2'])
  z81 = Zond()
  z81.copy(zond)
  # z81 = zond
  zLvl = zond.urovenList()
  z81Lvl = z81.urovenList()
  for l in zLvl.keys():
    if not zLvl[l].isGood(ValueType.UR_H): continue
    if not zLvl[l].isGood(ValueType.UR_P): continue
    if not zLvl[l].isGood(ValueType.UR_T): continue
    h = zLvl[l].value(ValueType.UR_H)
    P = zLvl[l].value(ValueType.UR_P)
    T = zLvl[l].value(ValueType.UR_T)
    Td = zLvl[l].value(ValueType.UR_Td)
    dt = 0
    if zLvl[l].isGood(ValueType.UR_Td):
      dt = calcVirtK(T, Td, P)
    else:
      dt = virtualCorrection(T)
    Tvirt = T + dt
    zLvl[l].set(ValueType.UR_D, Tvirt, 1)
    tp = SA81_P(z81Lvl[l].value(ValueType.UR_H))
    T = tp[0]
    P = tp[1]
    T -= kKelvin
    z81Lvl[l].set(ValueType.UR_T, T, 1)
  i = 0
  isz = len(h1)
  for i in range(0, len(h1)):
    if (h1[i] == -9999) or (h2[i] == -9999):
      continue
    sr_param = zond.oprSrParamPoPH(ValueType.UR_D, ValueType.UR_H, h1[i], h2[i])
    sr_param81 = z81.oprSrParamPoPH(ValueType.UR_T, ValueType.UR_H, h1[i], h2[i])
    if not sr_param[0]: continue
    if not sr_param81[0]: continue
    delta = sr_param[1] - sr_param81[1]
    levels[i]['avgDevT'] = delta
    i += 1

def calcAvgDevD(zond, levels):
 # zond.show()
  h1 = []
  h2 = []
  for level in levels:
    h1.append(level['h1'])
    h2.append(level['h2'])
  zond.DF2UV()
  z81 = Zond()
  z81.copy(zond)
  zLvl = zond.urovenList()
  z81Lvl = z81.urovenList()
  for l in zLvl.keys():

    if not zLvl[l].isGood(ValueType.UR_H): continue
    if not zLvl[l].isGood(ValueType.UR_P): continue
    if not zLvl[l].isGood(ValueType.UR_T): continue
    h = zLvl[l].value(ValueType.UR_H)
    P = zLvl[l].value(ValueType.UR_P)
    T = zLvl[l].value(ValueType.UR_T)
    Td = zLvl[l].value(ValueType.UR_Td)
    dt = 0
    if zLvl[l].isGood(ValueType.UR_Td):
      dt = calcVirtK(T, Td, P)
    else:
      dt = virtualCorrection(T)
    Tvirt = T + dt
    zLvl[l].set(ValueType.UR_D, Tvirt, 1)
    tp = SA81_P(z81Lvl[l].value(ValueType.UR_H))
    T = tp[0]
    P = tp[1]
    T -= kKelvin
    z81Lvl[l].set(ValueType.UR_T, T, 1)
  for l in zLvl.keys():
    if not zLvl[l].isGood(ValueType.UR_H): continue
    if not zLvl[l].isGood(ValueType.UR_P): continue
    if not zLvl[l].isGood(ValueType.UR_T): continue
    P = zLvl[l].value(ValueType.UR_P)
    Tv = zLvl[l].value(ValueType.UR_D)
    Dens = (P*100.0)/(287.*(Tv+273.15))
    zLvl[l].set(ValueType.UR_D, Dens, 1)
    P = z81Lvl[l].value(ValueType.UR_P)
    T = z81Lvl[l].value(ValueType.UR_T)
    Dens81 = (P*100.0)/(287.*(T+273.15))
    z81Lvl[l].set(ValueType.UR_D, Dens81, 1)
  for i in range(0, h1.__len__()):
    if (h1[i] == -9999) or (h2[i] == -9999):
      continue
    sr_param = zond.oprSrParamPoPH(ValueType.UR_D, ValueType.UR_H, h1[i], h2[i])
    sr_param81 = z81.oprSrParamPoPH(ValueType.UR_D, ValueType.UR_H, h1[i], h2[i])

    #sys.stderr.write( str(ValueType.UR_D)+" " +str( ValueType.UR_H)+" "+str(h1[i])+" "+str( h2[i]))
#    sys.stderr.write( str(sr_param81))
    if (not sr_param[0]) or ( not sr_param81[0]):
      continue
    delta = (sr_param[1] - sr_param81[1])/sr_param81[1]*100
    levels[i]['avgDevD'] = delta
    #sys.stderr.write( str(delta))


def calcSA81(levels):
  for level in levels:
    h2 = level['h2']
    tp = SA81_P(h2)
    saT = tp[0]
    saP = tp[1]
    saT -= kKelvin
    dt = virtualCorrection(saT)
    saDensity = (saP*100.)/(287.*(saT+dt+273.15))
    level['P'] = saP
    level['T'] = saT
    level['Dens'] = saDensity


def calcVirtK(T, Td, P):
    e = 6.1*pow(10, 7.45*Td/(235.+Td))
    return (0.378*(T+273.15)*e/P)


def virtualCorrection(T):
  '''
  Функция, возвращающая вирутальную поправку для заданной температуры
  :param T: температура в градусах цельсия
  :return: виртуальная поправка температуры в градусах цельсия
  '''
  if T < -70:
    T = -70
  if T > 50:
    T = 50
  tableT = [-70,-10,-5,0,1,2,4,5,6,7,8,9,10,11,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,30,32,34,36,38,40,42,44,46,48,50]
  deltaT = [0,0,0.2,0.3,0.3,0.4,0.4,0.5,0.5,0.6,0.6,0.7,0.7,0.8,0.8,0.9,0.9,1,1,1.1,1.2,1.3,1.4,1.5,1.6,1.7,1.8,1.9,2,2.2,2.4,2.7,3.1,3.5,4,4.4,5,5.6,6.3,7,7.8]
  if T in tableT:
    return deltaT[tableT.index(T)]
  firstT = secondT = -9999
  for t in tableT:
    if t < T:
      firstT = t
    elif t > T:
      secondT = t
      break
  delta = deltaT[tableT.index(firstT)] + (deltaT[tableT.index(secondT)] - deltaT[tableT.index(firstT)]) / (secondT - firstT) * (T - firstT)
  return delta

def tableT(h):
  '''
  Функция, возвращающая табличное значение температуры для заданной высоты
  :param h: высота в метрах
  :return: температура в грудсах цельсия
  '''
  h = float("{0:.1f}".format(h/1000.0))
  if h == 0:
    return 15.9
  elif h in frange(0, 9.3):
    return 15.9 - 6.328 * h
  elif h in frange(9.3, 12):
    return -43 - 6.328*(h - 9.3) + 1.172*(h - 9.3)**2
  elif h > 12:
    return -51.5

def pH_from_PQNH(pqnh, h, T):
  '''
  Функция вычисления давления на заданной высоте
  :param pqnh: давление на уровне моря в гектапаскалях
  :param h: высота над уровнем моря в метрах
  :param T: температура на заданной высоте в градусах цельсия
  :return: давление на заданной высоте в гектапаскалях
  '''
  return pqnh * (math.e**(-0.029 * 9.81 * h / (8.31*(T+273))))
  pass
