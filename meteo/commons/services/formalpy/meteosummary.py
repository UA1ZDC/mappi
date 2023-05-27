#!/usr/bin/env python
# -*- coding: utf-8 -*-

# системные модули
import os
import sys
from sys import argv
import logging

# конфигурационный модуль
from conf import *
import meteoglobal

sys.path.append(PRJ_DIR + '/lib')

# модули времени и даты
import datetime
import time
import math

# модули для работы с документами
from lpod3 import document, table

# модули для работы с удаленным сервисом
from google.protobuf.service import RpcController

import surface_pb2
from surface_service_pb2 import *
import field_pb2
import sprinf_pb2
import services_pb2

from meteo_channel import *
from meteo_controller import *

# модули оболочек C++ классов Puanson, TMeteoData, Zond
import libpuansonwrap
from libpuansonwrap import *
import libmeteodatawrap
from libmeteodatawrap import *
from libzondwrap import *
from libglobalwrap import *

# Qt-модули
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

from optparse import OptionParser
import argparse
import textwrap

import base64

# модуль настроек
# import productsettings

from document_service_pb2 import *
from sprinf_pb2 import *


# ипортируем главный класс для наслоедования
from documentform import DocumentFormal

class MeteosummaryGenerator(DocumentFormal):
  
  
  def __init__(self):
    super().__init__()
    
    self.service_code      = services_pb2.kSrcData
    
    self.descs             = ['CH', 'CM', 'CL', 'w', 'N', 'h', 'V', 'precip', 'dd', 'ff', 'T', 'U', 'P0', 'R12']  # дескрипторы идут в определенном порядке, не нарушать , 'Td', 'wind', 'cloud', 'Ptend'
    # перечень параметров, которые берем из табло
    self.tablo_list        = [ 'ch', 'cm', 'cl', 'w', 'n', 'h','v', 'r', 'dd','ff','t','u','p0','' ]
    self.hasData           = False
    self.result            = ExportResponse()
    self.result.result     = False
    self.printFileToStdOut = False
    self.station_type      = 0
    self.template_path     = "/share/meteo/odtsamples/Meteosummary.odt"
    
    # заполняем адрес
    self.address, self.host, self.port = self.init_service( self.service_code )


  # 
  # инициализация с параметрами
  # 
  def init(self, options):
    if (None == options.datetime) or \
        ( None == options.index ) or \
        (None == options.filepath):
      self.result.result = False
      self.result.comment = u"Не заданы требуемые параметры"
      return False
    self.arg_date          = datetime.datetime.strptime(options.datetime, "%Y-%m-%dT%H:%M:%S")
    self.data_date         = self.arg_date.strftime("%Y-%m-%dT%H:%M:%S")
    self.arg_station       = surface_pb2.Point()
    self.arg_station.index = options.index
    # делаем запрос в БД на получение данных по станции
    self.station           = self.getStationData( str( options.index ) )
    self.station_name      = self.getStationName()
    # задаем lat lon
    self.fillStationData(self.station)
    
    self.filepath          = options.filepath
    
    return True


  # 
  # заполняем данные из прото
  # 
  def initFromProto(self, proto):
    if (None == proto.datetime) or (0 == proto.stations.__len__()):
      self.result.result = False
      self.result.comment = u"Не заданы требуемые параметры"
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
    
    return True


  # 
  # Заполнение документа
  # 
  def fillDocument(self):
    
    # получаем тело документа
    date_time       = self.body.get_variable_set('date_time')
    station         = self.body.get_variable_set('station')
    self.atmosphere = self.body.get_table(name='Atmosphere')
    date_time.set_text(self.arg_date.strftime('%d-%m-%Y %H:%M') + ' UTC')
    
    station.set_text(self.station_name)
    
    zond_date_time  = self.body.get_variable_set('zond_date_time')
    zond_date_time.set_text(u'-')
    zond_attention  = self.body.get_variable_set('zond_attention')
    zond_attention.set_text(u'')
    attention       = self.body.get_variable_set('attention')
    attention.set_text(u'')
    
    latitude  = self.body.get_variable_set('latitude')
    longitude = self.body.get_variable_set('longitude')
    latitude.set_text(self.getLatStr())
    longitude.set_text(self.getLonStr())

    # пробуем заполнить данные по станции из наблюдений
    if not self.surface_from_src() and not self.surface_from_field():
      self.hasData        = False
      self.result.comment = u'Данных нет'
    else:
      self.hasData       = True
      self.result.result = True  
    
    if not self.zond_from_src() and not self.zond_from_field():
      self.hasData        = False
      self.result.comment = u'Данных нет'
    else:
      self.hasData       = True
      self.result.result = True  
      

  # 
  # Пробуем получить погоду по станции 
  # и заполнить таблицу метеосводки
  # 
  def surface_from_src(self):
    # получение данных
    self.hasData, surf_data = self.getMeteoData()
    
    # если данные есть
    if self.hasData and surf_data and len(surf_data)>0:
      # текущие данные
      self.cur_data  = surf_data[0]
      self.data_date = self.cur_data.dt
      # данные для заполнения таблицы
      table_data     = []
      # значения в таблице
      table_val      = []
    else:
      return False

    # заполняем 
    for _param in self.tablo_list:
      table_val.append( self.getParamFromTablo( self.cur_data, _param, 'val' ) )
      table_data.append( self.getParamFromTablo( self.cur_data, _param ) )
      
    # заполняем таблицу с данными наблюдений
    self.fillAtmosphereTable(table_data,table_val)
    
    attention = self.body.get_variable_set('attention')
    attention.set_text(u'')
    return True

  # 
  # Забираем значения метеопараметров из поля
  # 
  def surface_from_field(self):
    # получение данных
    self.hasData, surf_field = self.getFieldData()
    
    # если данные есть
    if self.hasData and surf_field and len(surf_field)>0:
      # текущие данные
      self.cur_data  = surf_field[0]
      self.data_date = self.cur_data.fdesc.date
      # данные для заполнения таблицы
      table_data     = []
      # значения в таблице
      table_val      = []
    else:
      return False
    
    libpuansonwrap.loadPunch(str("is"))                # загрузка шаблона пуансона
    # проходимся по дескрипторам  
    for descrname in self.descs:
      # изем значение параметра в пришедшем прото ответе
      _value = self.get_field_value_by_descrname( surf_field, descrname )
      table_val.append( _value )
      if _value is False:
        _value = '-'
      else:
        _value = libpuansonwrap.stringFromRuleValue( descrname,  _value )
      # заполняем массив с данными
      table_data.append( _value )
    
    # заполняем таблицу с данными наблюдений
    self.fillAtmosphereTable(table_data,table_val)

    attention = self.body.get_variable_set('attention')
    text      = u'Данные получены в результате объективного анализа. '
    hour      = self.cur_data.fdesc.hour/3600
    center    = self.getCenterName( self.cur_data.fdesc.center )

    if center is None:
      center = self.cur_data.fdesc.center
    if 0 == hour:
      text = text + u'Анализ. '
    else:
      text = text + u'Срок прогноза ' + str(hour) + u'ч. '
    text = text + (u'Центр ') + (center)
    attention.set_text(text)
    return True


  # 
  # Заполняем таблицу с аэрологическими данными на основе зонда
  # 
  def fillAeroTable(self,zond):
    heights = [0.5, 1.0, 1.5, 2.0, 2.5, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21]
    height  = self.body.get_table(name="Height")
    hh      = height.clone()
    height.delete()
    height = hh
    self.body.append(height)
    fstCellL = 'B'
    fstCellI = 2
    i        = 0
    libpuansonwrap.loadPunch(str("is"))                # загрузка шаблона пуансона
    # проходимся по высотам
    while i < len(heights):
      level = Uroven()
      zond.getUrPoH(heights[i] * 1000, level)
      
      if level.isGood(ValueType.UR_P):
        p_str = libpuansonwrap.stringFromRuleValue('P0', level.value(ValueType.UR_P))
      else:
        p_str = u'-'
      if level.isGood(ValueType.UR_T):
        t_str = libpuansonwrap.stringFromRuleValue('T', level.value(ValueType.UR_T))
      else:
        t_str = u'-'
      if level.isGood(ValueType.UR_T) and level.isGood(ValueType.UR_Td):
        h_str = libpuansonwrap.stringFromRuleValue('U',
                                                 self.humidity(level.value(ValueType.UR_T),
                                                               level.value(ValueType.UR_Td)))
      else:
        h_str = u' '
      if level.isGood(ValueType.UR_ff):
        ff_str = libpuansonwrap.stringFromRuleValue('ff', level.value(ValueType.UR_ff))
      else:
        ff_str = u'-'
      if level.isGood(ValueType.UR_dd):
        dd_str = libpuansonwrap.stringFromRuleValue('dd', level.value(ValueType.UR_dd))
      else:
        dd_str = u'-'
      
      data = [p_str, t_str, h_str, ff_str, dd_str]
      j = 0
      while j < 5:
        cell = height.get_cell(chr(ord(fstCellL) + j) + str(fstCellI + i))
        cell.set_text(meteoglobal.trUtf8(data[j]))
        height.set_cell(chr(ord(fstCellL) + j) + str(fstCellI + i), cell)
        j = j + 1
      i = i + 1
    zond_date = zond.dateTime()
    dt = self.arg_date.strftime("%Y-%m-%dT%H:%M:%S")
    if zond_date:
      dt = zond_date
    zond_date_time = self.body.get_variable_set('zond_date_time')
    zond_date_time.set_text(dt)
    return True

  # 
  # Заполняем таблицу с приземными данными
  # 
  def fillAtmosphereTable(self, datashow, dataval):
    fstCellI = 1
    fstCellL = 'C'
    i        = 0   
    # заполняем таблицу
    while i < 14:
      _cell_name = fstCellL + str(fstCellI + i)
      cell       = self.atmosphere.get_cell( _cell_name )
      cell.set_text(meteoglobal.trUtf8(datashow[i]))
      self.atmosphere.set_cell( _cell_name, cell)
      i = i + 1
    
    clonedrow = self.atmosphere.get_row(i-1,True)
    # self.atmosphere.delete_row(i)
    
    last_row = i
    # 
    # Проверяем есть ли индикация, если есть - то сравниваем и выводим текст
    # 
    if len(self.indication)>0:
      
      self.atmosphere.append_row( clonedrow.clone() )
      _cell_name_param = 'A' + str(fstCellI + last_row )
      cell       = self.atmosphere.get_cell( _cell_name_param )
      cell.set_text(meteoglobal.trUtf8("Рекомендации"))
      self.atmosphere.set_cell( _cell_name_param, cell)
      last_row+=1
      
      for k,_ in enumerate(self.indication):
        # 
        # Тип рекомендации
        # 0 - без ограничений
        # 1 - ограничения (СМУ)
        # 2 - не рекомендуется (УМП)
        # 
        recomendation_type = 0
        # проходимся по сложным условиям
        for s in _['smu']:
          # проходимся по параметрам в описании
          for j,descrname in enumerate(self.descs):
            if dataval[j] !='' and dataval[j] is not None and dataval[j] is not False:
              if s( descrname, dataval[j] ):
                recomendation_type = 1
        for u in _['ump']:
          # проходимся по параметрам в описании
          for j,descrname in enumerate(self.descs):
            if dataval[j] !='' and dataval[j] is not None and dataval[j] is not False:
              if u( descrname, dataval[j] ):
                recomendation_type = 2
        
        self.atmosphere.append_row( clonedrow.clone() )
        
        _cell_name_param = 'A' + str(fstCellI + last_row + k)
        cell       = self.atmosphere.get_cell( _cell_name_param )
        cell.set_text(meteoglobal.trUtf8(_['preset_name']))
        self.atmosphere.set_cell( _cell_name_param, cell)
        
        _cell_name_val   = fstCellL + str(fstCellI + last_row + k)
        cell       = self.atmosphere.get_cell( _cell_name_val )
        cell.set_text(meteoglobal.trUtf8(self.recomendations[recomendation_type]))
        self.atmosphere.set_cell( _cell_name_val, cell)
    return True

  # 
  # Заполняем таблицу с аэрологией
  # 
  def zond_from_src(self):
    zond    = Zond()
    # делаем запрос на получение данных от сервиса
    result, aerodata = self.getAeroData()
    # если получен ответ и есть данные
    if result is True and len(aerodata)>0:
      data = aerodata[0].meteodata
      zond.fromArrayObject(data)
      zond.preobr()
      # заполняем документ
      self.fillAeroTable(zond)
      # информация в таблице
      zond_attention = self.body.get_variable_set('zond_attention')
      zond_attention.set_text('')
    else:
      return False
    return True

  # 
  # Забираем данные зондирования из поля
  # 
  def zond_from_field(self):
    zond = Zond()
    result, aero_data = self.getAeroFieldData()
    if result is True and len(aero_data)>0:
      
      data = aero_data[0].meteodata
      zond.fromArrayObject(data)
      zond.preobr()
      # заполняем таблицу с аэрологией
      self.fillAeroTable(zond)
      
      zond_attention = self.body.get_variable_set('zond_attention')
      srok = str('. Анализ ')
      if 0 != aero_data[0].hour:
        srok = u'. Срок прогноза: ' + str(aero_data[0].hour/3600) 
      zond_attention.set_text(u'Результат объективного анализа. Центр: ' + aero_data[0].center + meteoglobal.trUtf8(srok))
    else:
      return False
    return True



# 
# ====================== MAIN FUNCTION ============================
# 


def main(argv):
  # Qt-приложение
  app = QCoreApplication(sys.argv)

  parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter,
                                   description="Сценарий формирования метеосводок")
  parser.add_argument('-m', '--mode', default='arg', help=textwrap.dedent('''\
                  Режим запуска сценария.
                  arg - Задание аргументов запуска
                  proto - Передача прото-структуры в стандартный поток'''))
  parser.add_argument('-f', '--filepath', default=PRJ_DIR + '/var/meteo/documents/',
                      help="Путь сохранения файла.")
  parser.add_argument('-i', '--index', help='Индекс станции')
  parser.add_argument('-d', '--datetime', help='Дата и время измерений в формате YYYY-MM-DDThh:mm:ss')

  options = parser.parse_args()

  generator = MeteosummaryGenerator()
  init      = False
  
  # парсим данные из файла или протки
  if str(options.mode) == "arg":
    init = generator.init(options)
  elif str(options.mode) == "proto":
    sa     = StartArguments()
    sa.ParseFromString(meteoglobal.readStdinBytes())
    init   = generator.initFromProto(sa)
  if not init:
    result = generator.resultProto()
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(1)

  # генерим из параметров
  generator.generateDocument()
  # получаем готовую протку
  result = generator.resultProto()
  
  if True == result.result:
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(0)
  else:
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(1)


if __name__ == "__main__":
  main(argv)
