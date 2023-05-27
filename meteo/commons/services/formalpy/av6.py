#!/usr/bin/python3
# coding: utf-8

#системные модули
import os
import sys
import logging
from sys import argv

import math

#конфигурационный модуль
from conf import *
sys.path.append(PRJ_DIR + '/lib')

# logging.error(sys.version)

import meteoglobal

#модули времени и даты
import datetime
import time

#модули для работы с документами
from lpod3 import document
from lpod3 import table

#модули для работы с удаленным сервисом
from google.protobuf.service import RpcController

from surface_pb2 import *
from surface_service_pb2 import *

from meteo_channel import *
from meteo_controller import *


#модули оболочек C++ классов Puanson, TMeteoData, Zond, Global
import libpuansonwrap
from libpuansonwrap import *
import libmeteodatawrap
from libmeteodatawrap import *
from libzondwrap import *

from document_service_pb2 import *
from sprinf_pb2 import *
import services_pb2

from libglobalwrap import *

#Qt-модули
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

import base64

# ипортируем главный класс для наслоедования
from documentform import DocumentFormal

class AV6Generator(DocumentFormal):

  def __init__(self):
    super().__init__()

    self.service_code      = services_pb2.kSrcData

    self.hasData           = False
    # перечень запрашиваемых в базе параметров
    self.descs             = ['N', 'Nh', 'CH', 'CM', 'CL', 'h', 'dd', 'ff', 'V', 'T', 'U', 'Td', 'P0', 'p', 'a', 'w', 'w_w_', 'VV', 'hgr', 'Tb', 'PQNH', 'h50', 'h230', 'w_tr']
    # перечень параметров, которые берем из табло
    self.tablo_list        = [ 'N','C','h','dd','ff','V','T','Tb','Td','U','P0','p','a','W' ]
    self.result            = ExportResponse()
    self.result.result     = False
    self.printFileToStdOut = False
    self.user              = ""
    self.rank              = ""
    self.full              = False
    self.all_hours         = False
    self.mDataList         = []
    self.offset            = 0  # если строк больше 25 сюда заносится сдвиг, чтобы правильно отобразить подпись
    self.station_type      = 0

    # заполняем адрес
    self.address, self.host, self.port = self.init_service( self.service_code )


  # 
  # инициализация с параметрами
  # 
  def init(self, options):
    if (None == options.datetime) or (None == options.index ) or (None == options.filepath):
      self.result.result  = False
      self.result.comment = u"Не заданы требуемые параметры"
      return False
    self.full              = options.full
    self.all_hours         = options.nonstandard
    self.arg_date          = datetime.datetime.strptime(options.datetime, "%Y-%m-%dT%H:%M:%S")
    self.date_start        = self.arg_date - datetime.timedelta(days=1)
    self.arg_station       = Point()
    self.arg_station.index = options.index
    self.filepath          = options.filepath
    # self.alt               = self.arg_station.position.height_meters
    self.station_type      = self.arg_station.type
    # задаем lat lon
    self.fillStationData(self.station)
    
    # делаем запрос в БД на получение данных по станции
    self.station           = self.getStationData( str( options.index ) )
    self.station_name      = self.getStationName()
    if not self.station_name:
      self.station_name = u"Неизвестно"
    return True


  # 
  # заполняем данные из прото
  # 
  def initFromProto(self, proto):
    if (None == proto.datetime):
      self.result.result  = False
      self.result.comment = u"Не задана дата"
      return False
    if (0 == proto.stations.__len__()):
      self.result.result = False
      self.result.comment = u"Не указана станция"
      return False

    # делаем стандартную прото-инициализацию
    # после этого получаем:
    # arg_date
    # number
    # station
    # filepath
    # station_name
    # alt
    # h0
    # station_type
    # lat, lon
    # 
    self.standartProtoInit(proto)

    # дельта по времени - 1 день
    self.date_start = self.arg_date - datetime.timedelta(days=1)   
    
    if proto.user_name:
      self.user = proto.user_name
    if proto.user_rank:
      self.rank = proto.user_rank

    self.full      = proto.av6_full
    self.all_hours = proto.av6_all_hours

    if proto.av6_all_hours:
      self.all_hours = True
    
    return True



  # 
  # подготавливаем данные для заполнения таблицы
  # 
  def formAV6(self):
    # получаем погоду по станции
    self.hasData, _meteodata = self.getMeteoData()
    
    for data in _meteodata:
      # проверяем, если не за все сроки - то пропускаем сроки, которые не 0
      if not self.all_hours:
        dt = datetime.datetime.strptime(data.dt, "%Y-%m-%dT%H:%M:%S")
        if dt.second != 0 or dt.minute != 0:
          continue
      self.mDataList.append(data)
 
    self.offset = len(self.mDataList) - 25
    if self.offset < 0:
      self.offset = 0

    # инициализация пуансона из файла шаблона
    libpuansonwrap.loadPunch(str("is"))

    params = {  'hours' : [],
                'N'     : [],
                'hgr'   : [],
                'C'     : [],
                'h'     : [],
                'hWay'  : [],
                'hVerh' : [],
                'dd'    : [],
                'ff'    : [],
                'V'     : [],
                'VWay'  : [],
                'T'     : [],
                'Tb'    : [],
                'Td'    : [],
                'U'     : [],
                'P0'    : [],
                'p'     : [],
                'W'     : [],
                'h50'   : [],
                'h230'  : [] }
    dArrays = [ params['hours'],  params['N'],  params['C'],  params['h'],  params['hWay'],  
                params['dd'],  params['ff'],  params['V'],  params['VWay'],  params['T'],  params['U'],  
                params['P0'],  params['p'], params['W'] ]
    # если полный вариант 
    if self.full:
      dArrays = [ params['hours'],  params['N'],  params['C'],  params['h'],  params['hWay'],  params['hVerh'],  
                  params['dd'],  params['ff'],  params['V'],  params['VWay'],  params['T'],  params['Tb'],  
                  params['Td'],  params['U'],  params['P0'],  params['p'],  params['W'],  params['h50'], 
                  params['h230'] ]

    for data in self.mDataList:
      
      hour   = str(datetime.datetime.strptime(data.dt, "%Y-%m-%dT%H:%M:%S").hour)
      minute = datetime.datetime.strptime(data.dt, "%Y-%m-%dT%H:%M:%S").minute

      if minute != 0:
        time = hour + ' (' + str(minute) + ')'
      else:
        time = hour
      params['hours'].append(time)
      
      # заполняем 
      for _param in self.tablo_list:
        if _param in params:
          # value = self.getParamVal( data, _param )
          item_val = self.safeCast( self.getParamVal( data, _param ), float, '')
          if item_val=='':
            value = item_val
          else:
            value = libpuansonwrap.stringFromRuleValue( _param, item_val )
          # обрабатываем облачность
          if _param == 'C':
            value = self.getParamClouds( data )
          elif _param in [ 'h', 'Nh' ]:
            value = self.getParamCloudCustom( data, param=_param )
            # if meteodata.hasParam('CH') or meteodata.hasParam('CM') or meteodata.hasParam('CL') or meteodata.hasParam('Cn'):
            #   descrs = ['CH', 'CM', 'CL', 'Cn']
            #   for descr in descrs:
            #       if meteodata.hasParam(descr):
            #           param = int(meteodata.meteoParam(descr).value())
            #           string = libpuansonwrap.stringFromRuleValue(descr, param)
            #           value3 = value3 + string
          
          # добавляем тенденцию
          elif _param == 'p':
            _a = self.getParamVal( data, 'a', 'value' )
            if _a != "":
              value += libpuansonwrap.stringFromRuleValue('a', self.safeCast( _a, float, 0.0 ) )
          params[_param].append( value )

      params['h50'].append(  self.getParamVal( data, 'h50' )  )
      params['h230'].append( self.getParamVal( data, 'h230' ) ) 
      # способы определения видимости и ВНГО
      params['hWay'].append('Инстр.')
      params['VWay'].append('Инстр.')
      
      # h    = data.  #"Высота нижней границы облачности"
      # dd   = None   # направление ветра
      # ff   = None   # скорость ветра
      # V    = None   # "Видимость"
      # VV   = None   # "Горизонтальная видимость у поверхности" 
      # T    = None   # температура
      # Tb   = None   # "Температура по смоченному термометру"
      # Td   = None   # "Температура точки росы"
      # U    = None   # влажность
      # P0   = None   # "Давление на уровне станции" 
      # P    = None   # "Давление, приведенное к уровню моря"
      # PQNH = None   # "Величина QNH"
      # p    = None   # "Барическая тенденция"
      # A    = None   # "Хар-ка бар.тенденции"
      # w    = []     # "Текущая погода"
      # h50  = None   # "ВНГО, измеренная вблизи БПРМ со стороны захода на посадку с магнитным курсом посадки 50°"
      # h230 = None   # "ВНГО, измеренная вблизи БПРМ со стороны захода на посадку с магнитным курсом посадки 230°"
      # w_w_ = []     # "Особые явления текущей и прогнозируемой погоды"
      # w_tr = []     # "Явления погоды"

    if self.full:
      self.tableFullCycle(dArrays)
    else:
      self.tableCycle(dArrays)


  # 
  # Цикл заполнения таблицы
  # 
  def tableCycle(self, arrays):
    # logging.error(arrays)
    fstCellL = 'C'
    fstCellI = 6
    i        = 0
    _table   = self.body.get_table()
    while i < len(self.mDataList):
      j, k = 0, 0
      while k < 14:
        cell = _table.get_cell(chr(ord(fstCellL) + k) + str(fstCellI + i))
        cell.set_repeated(1)
        try:
          cell.set_value(value=meteoglobal.trUtf8(arrays[j][i]), cell_type='string')
        except:
          logging.error((j,i, len(arrays[j])))
        _table.set_cell((chr(ord(fstCellL) + k) + str(fstCellI + i)), cell)
        j += 1
        k += 1
      i += 1

  # 
  # Цикл заполнения таблицы полный
  # 
  def tableFullCycle(self, arrays):
    skip     = [6,18,19]
    fstCellL = 'C'
    fstCellI = 6
    i        = 0
    _table   = self.body.get_table()
    while i < len(self.mDataList):
      j, k = 0, 0
      while k < 22:
        if k in skip:
          k = k + 1
          continue
        cell = _table.get_cell(chr(ord(fstCellL) + k) + str(fstCellI + i))
        cell.set_repeated(1)
        cell.set_value(value=meteoglobal.trUtf8(arrays[j][i]), cell_type='string')
        _table.set_cell((chr(ord(fstCellL) + k) + str(fstCellI + i)), cell)
        j += 1
        k += 1
      i += 1

  # 
  # Получаем формы облачности через слеш
  # 
  def getParamClouds(self, data):
    result = []
    for _ in ['CH','CM','CL']:
      val_ = self.getParamVal( data, _ )
      if val_!='':
        result.append( libpuansonwrap.stringFromRuleValue( _, val_ ) )
    return '/'.join(result)
  
  # 
  # Получаем высоты облачности через слеш
  # 
  def getParamCloudCustom(self, data, param='h'):
    result = []
    for _ in data.param:
      if _.descrname==param:
        val_ = _.value
        if val_!='':
          result.append( libpuansonwrap.stringFromRuleValue( param, val_ ) )
    return '/'.join( sorted( result, reverse=True ) )

  # 
  # Заполнение документа
  # 
  def fillDocument(self):
    # подготавливаем данные для заполнения документа
    self.formAV6()

    _table      = self.body.get_table()
    signature   = 32 + self.offset
    time_cell   = _table.get_cell('A1')
    time_string = u'с {} час {}  до {} час {} {} по станции {} '.format( self.date_start.hour, 
                                                                         self.date_start.strftime("%d-%m"),
                                                                         self.arg_date.hour,
                                                                         self.arg_date.strftime("%d-%m"),
                                                                         self.arg_date.year,
                                                                         self.getStationName() )

    time_cell.set_value(value=time_string, cell_type='string')
    _table.set_cell('A1', (time_cell) )

    rankName = self.rank + ' ' + self.user
    letter   = 'P'
    if self.full:
      letter = 'T'
    name_cell = table.odf_create_cell(value=rankName, cell_type="string")
    _table.set_cell(letter + str(signature), name_cell)

    if self.offset > 0:
      sign_cell = table.odf_create_cell(value='', cell_type="string")
      _table.set_cell('A32', sign_cell)
      sign_cell = table.odf_create_cell(value=u'Прогноз оценил', cell_type="string")
      _table.set_cell('A' + str(signature), sign_cell)

      sign_cell = table.odf_create_cell(value=u'Дневник вел', cell_type="string")
      _table.set_cell('K' + str(signature), sign_cell)



  # 
  # устанавливаем путь к щаблону
  # 
  def set_template_path(self,path):
    self.template_path = path
    return


# 
# ====================== MAIN FUNCTION ============================
# 

from optparse import OptionParser
import argparse

def main(argv):
  #Qt-приложение
  app = QCoreApplication (sys.argv)

  parser = argparse.ArgumentParser(description="Сценарий формирования дневника погоды")
  parser.add_argument('-m', '--mode', default='arg', help="Режим запуска сценария. "
                                                          "arg - Задание аргументов запуска "
                                                          "proto - Передача прото-структуры в стандартный поток")

  parser.add_argument('-f', '--filepath', default=PRJ_DIR + '/var/meteo/documents/',
                      help="Путь сохранения файла.")
  parser.add_argument('-i', '--index', help='Индекс станции')
  parser.add_argument('-d', '--datetime', help='Дата и время измерений в формате YYYY-MM-DDThh:mm:ss')
  parser.add_argument('--nonstandard', action='store_true', help='Сформировать дневник погоды по всем имеющимся данным за последние сутки (нестандартные сроки)', default=False )
  parser.add_argument('--full', action='store_true', help='Сформировать дневникпогоды в полной форме (для аэродромов)')

  options = parser.parse_args()

  generator = AV6Generator()
  init      = False

  # парсим данные из файла или протки
  if str(options.mode) == "arg":
    init = generator.init(options)
  elif str(options.mode) == "proto":
    sa   = StartArguments()
    sa.ParseFromString(meteoglobal.readStdinBytes())
    init = generator.initFromProto(sa)
  # если не получилось - выходим
  if not init:
    result = generator.resultProto()
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(1)

  if generator.full:
    generator.set_template_path( "/share/meteo/odtsamples/AV6full.ods" )
  else:
    generator.set_template_path( "/share/meteo/odtsamples/AV6.ods" )

  # генерим из параметров
  generator.generateDocument()
  # получаем готовую протку
  result = generator.resultProto()
  if not result.result:
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(1)
  else:
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(0)


if __name__ == "__main__":
  main(argv)
