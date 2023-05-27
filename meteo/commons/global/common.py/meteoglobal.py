#!/usr/bin/python3
# -*- coding: utf-8 -*-

# в этот файл надо складывать общие функции

import math

# импорт прото-модулей обеспечивает возможность создания экземпляров прото-классов по наименованиям
import field_pb2
import weather_pb2
import document_service_pb2
import surface_pb2
import appconf_pb2
import sprinf_pb2
import surface_service_pb2
import services_pb2
import meteo_pb2
import customviewer_pb2
import msgcenter_pb2
import forecast_pb2
import climat_pb2
import cron_pb2
import map_radar_pb2
import map_isoline_pb2

import meteo_controller
import meteo_channel
import libglobalwrap
import librpcpywrap
import locale

import sys

DEG_PER_RAD = 180.0 / math.pi
RAD_PER_DEG = math.pi / 180.0

#Устанваливает локаль для даты. Данная локаль присутствует по умолчанию в ASTRA 1.6
#В дебиан может отсутствовать, отредактируйте файл /etc/locale.gen и запустите locale-gen
try:
  locale.setlocale(locale.LC_TIME, 'ru_RU.UTF-8')
except:
  sys.stderr.write('Ошибка при установке локали для даты\времени.\n')

def RemoteCall( servicecode, methodfullname, request, timeout_msecs ):
  '''
  вызов rpc-процедуры по ее полному имени (например 'field_pb2@FieldService.GetIsoLines' )
  :param servicecode: код сервиса, указанный в модуле services_pb2
  :param methodfullname: полное имя вызываемой процедуры = имямодуля@ИмяСервиса.ИмяФункции
  :param request: прото-структура с параметрами запроса
  :param timeout_msecs: таймаут ожидания ответа в миллисекундах
  :return: resp: ответ на запрос
  '''
  ctrl    = meteo_controller.ClientController()
  address = libglobalwrap.serviceAddress(servicecode)
  res     = ctrl.Connect( librpcpywrap.hostByAddress(address), librpcpywrap.portByAddress(address) )
  if ( False == res ):
    sys.stderr.write('Не удалось подключиться  к сервису = ' + str(services_pb2._SERVICECODE.values_by_number[
      servicecode].name) + ' по адресу:' + str(address) + '\n' )
    ctrl.close()
    return None

  module, classmethodname = str.split( methodfullname, '@', 1 )
  classname, methodname = str.split( classmethodname, '.', 1 )
  classname = classname + '_Stub'

  cls = getattr( globals()[module], classname )
  stub = cls( ctrl.Channel() )
  funcpointer = getattr( stub, methodname )
  resp = ctrl.RemoteCall( funcpointer, request, timeout_msecs )
  if ( None == resp ):
    sys.stderr.write('Не удалось получить ответ на запрос = ' + str(methodfullname) + '\n' )

  ctrl.close()

  return resp

def readStdinBytes():
  """
    Считываем бинарные данные со входа
  """
  szstr = sys.stdin.buffer.read(4)
  sz = int.from_bytes( szstr, "little");
  return sys.stdin.buffer.read(sz)


def trUtf8(data):
  if data is None:
    return ''
  if isinstance(data, str):
    data = data.encode('utf-8')
  # else:
  if isinstance(data, bytes):
    data = data.decode('utf-8')
  # if not isinstance(str(data), basestring):
  #   return ''
  return str(data)

def fromUtf8(data):
  if data is None:
    return None
  if isinstance(data, str):
    return data

  if isinstance(data, basestring) or isinstance(data, bytes):
    return data

def rad2deg(rad):
  '''
  Перевод радиан в градусы
  :param rad: радианы
  :return: градусы
  '''
  try:
    rad = float(rad)
  except ValueError:
    return None
  return rad * DEG_PER_RAD


def deg2rad(deg):
  '''
  Перевод градусов в радианы
  :param deg: градусы
  :return: радианы
  '''
  try:
    deg = float(deg)
  except ValueError:
    return None
  return deg * RAD_PER_DEG


def latToHuman(lat):
  '''
  Перевод широты в читаемый для человека формат
  :param lat: широта в радианах
  :return: читаемая строка
  '''
  degree = rad2deg(lat)
  if degree < 0:
    return '{:.1f} ю.ш..'.format(-degree)
  else:
    return '{:.1f} с.ш.'.format(degree)
  
def lonToHuman(lon):
  '''
  Перевод долготы в читаемый для человека формат
  :param lon: долгота в радианах
  :return: читаемая строка
  '''
  degree = rad2deg(lon)
  if degree < 0:
    return '{:.1f} з.д.'.format(degree)
  else:
    return '{:.1f} в.д.'.format(degree)

def frange(x,y,step=0.1):
  '''
  Генератор чисел с плавающей точкой в заданном диапазоне
  :param x: начало диапазона
  :param y: конец диапазона
  :param step: шаг диапазона, по умолчанию 0.1, если сделать меньше, возникнет неточность
  :return: генератор чисел
  '''
  while x < y:
    yield float("{0:.1f}".format(x))
    x += step

class Error:
  def __lshift__(self, other):
    sys.stderr.write(str(other) + '\n')
    return self

# использовать так: error_log << str1 << str2 << str3
error_log = Error()

#Форматы даты по умолчанию - месяц числом
dateHumanFullFormat = '%d.%m.%Y %H:%M:%S'
dateHumanTimeShortFormat = '%d.%m.%Y %H:%M'
dateHumanDateOnlyFormat = '%d.%m.%Y'

#MT-форматы - месяц текстом - месяц представлен локализованной строкой
dateHumanFullMTFormat = '%d %B %Y %H:%M:%S'
dateHumanTimeShortMTFormat = '%d %B %Y %H:%M'
dateHumanDateOnlyMTFormat = '%d %B %Y'


def dateToHumanFull(dt, textMonth=False):
  if textMonth is True:
    return trUtf8(dt.strftime(dateHumanFullMTFormat))
  else:
    return dt.strftime(dateHumanFullFormat)

def dateToHumanTimeShort(dt, textMonth=False):
  if textMonth is True:
    return trUtf8(dt.strftime(dateHumanTimeShortMTFormat))
  else:
    return dt.strftime(dateHumanTimeShortFormat)

def dateToHumanDateOnly(dt, textMonth=False):
  if textMonth is True:
    return trUtf8(dt.strftime(dateHumanDateOnlyMTFormat))
  else:
    return dt.strftime(dateHumanDateOnlyFormat)

def meteoparam_props( descr ):
  return libglobalwrap.meteoparam_props(descr)

def meteoparam_proplist_by_names( descrnames ):
  props = []
  descr_array = descrnames.split(',')
  for name in descr_array:
    prop = libglobalwrap.meteoparam_props_by_name(name)
    props.append(prop)
  return props

def meteoparam_proplist_by_descr( descrs ):
  props = []
  descr_array = descrs.split(',')
  for descr in descr_array:
    try:
      prop = libglobalwrap.meteoparam_props(int(descr))
      props.append(prop)
    except Exception as e:
      print('Error =', e )
  return props

def meteoparam_value_descriptions(descriptor):
  return libglobalwrap.meteoparam_value_descriptions(int(descriptor))

# 
# получаем строку, десериализуем в прото
# 
def loadClimatStations():
  stations_binary = libglobalwrap.loadClimatStations()
  stations        = climat_pb2.ClimatSaveProp()
  stations.ParseFromString(stations_binary)
  
  return stations

# 
# Загружаем настройки моделей машинного обучения
# 
def loadMLSettings():
  settings_binary = libglobalwrap.loadMLSettings()
  settings        = cron_pb2.Settings()
  settings.ParseFromString(settings_binary)
  
  return settings

# 
# получаем прото - для передачи сериализуем в строку
# 
def saveMLSettings(settings):
  return libglobalwrap.saveMLSettings( settings.SerializeToString() )


# 
# Глобальные пути
# 
def etcPath():
  return libglobalwrap.etcPath()

def sharePath():
  return libglobalwrap.sharePath()

def libPath():
  return libglobalwrap.libPath()

def binPath():
  return libglobalwrap.binPath()

def radarColors():
  colorstr = libglobalwrap.radarColors()
  colors = map_radar_pb2.RadarColors()
  colors.ParseFromString(colorstr)

  return colors

def isoColors():
  colorstr = libglobalwrap.isoColors()
  colors = map_isoline_pb2.FieldColors()
  colors.ParseFromString(colorstr)

  return colors

# 
# получаем прото - для передачи сериализуем в строку
# 
def saveClimatStations(stations):
  return libglobalwrap.saveClimatStations( stations.SerializeToString() )
