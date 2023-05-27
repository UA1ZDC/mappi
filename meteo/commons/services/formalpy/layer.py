#!/usr/bin/python3
# coding: utf-8

#системные модули
import os
import sys
from sys import argv
#модули времени и даты
import datetime
import time
import math
import logging
from conf import *
sys.path.append(PRJ_DIR + '/lib')

#модули для работы с удаленным сервисом
from google.protobuf.service import RpcController


from optparse import OptionParser
import argparse

from meteo_controller import *
import libmeteodatawrap
from libmeteodatawrap import *
from libglobalwrap import *

#модули для работы с документами
from lpod3 import document

from document_service_pb2 import *
import meteocalc
import meteoglobal
from meteoglobal import error_log
from surface_pb2 import *
from surface_service_pb2 import *
import field_pb2
from libzondwrap import *
import services_pb2

import base64

from PyQt5.QtCore import QCoreApplication

# ипортируем главный класс для наслоедования
from documentform import DocumentFormal

class LayerGenerator(DocumentFormal):
  
  def __init__(self):
    super().__init__()
    
    self.service_code      = services_pb2.kSrcData
    
    self.kPa2mm            = 7.5064 * 0.001
    self.hasData           = False
    self.ur_station        = None
    self.temperature       = None
    self.pressure          = None
    self.zond              = None
    self.zond_datetime     = None
    self.result            = ExportResponse()
    self.result.result     = False
    self.text              = ''
    
    self.station_type      = 0
    self.template_path     = "/share/meteo/odtsamples/Layer.odt"

  # 
  # инициализация с параметрами
  # 
  def init(self, options):
    if (None == options.datetime) or (None == options.index) \
        or (None == options.filepath):
      self.result.result = False
      self.result.comment = u"Не заданы требуемые параметры"
      return False
    self.number            = options.number

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
  # Заполнение документа
  # 
  def fillDocument(self):
    
    self.levels = [ {'code': 1, 'h1': self.h0, 'h2': 1500},
                    {'code': 2, 'h1': self.h0, 'h2': 3000},
                    {'code': 3, 'h1': self.h0, 'h2': 6000},
                    {'code': 4, 'h1': self.h0, 'h2': 12000},
                    {'code': 5, 'h1': self.h0, 'h2': 18000},
                    {'code': 6, 'h1': self.h0, 'h2': 24000},
                    {'code': 7, 'h1': self.h0, 'h2': 30000},
              ]
    # получаем тело документа
    body      = self.doc.get_body()
    table     = body.get_table(name="Layer")
    layer     = body.get_variable_set("layer")
    dtime     = body.get_variable_set("datetime")
    index     = body.get_variable_set("index")
    height    = body.get_variable_set("height")
    attention = body.get_variable_set('attention')

    attention.set_text(meteoglobal.trUtf8(self.text))
    index.set_text( self.getStationNumber(self.station.station) + " (%s)" % self.station_name )
    height.set_text( str(int(self.h0)) )


    if not self.zond_from_src() and not self.zond_from_field():
      self.hasData        = False
      self.result.comment = u'Данных нет'
    else:
      self.hasData        = True
      self.result.result  = True 

    dt = self.zond_datetime
    dtime.set_text(meteoglobal.dateToHumanTimeShort(dt, True))
    
    params   = ['dd', 'ff']
    fstCellL = 'B'
    fstCellI = 2
    i        = 0
    while i < 2:
      for level in self.levels:
        cell = table.get_cell(fstCellL + str(fstCellI))
        if params[i] in level:
          if params[i] == 'ff':
            param = str(int(round(level[params[i]] * 3.6, 0)))
          else:
            param = str(int(round(level[params[i]], 0)))
          cell.set_text(param)
          table.set_cell(fstCellL + str(fstCellI), cell)
        fstCellI += 1
      fstCellL = chr(ord(fstCellL) + 1)
      fstCellI = 2
      i += 1
    layer.set_text(self.meteo)


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
      self.fillBulletin(zond)
      # информация в таблице
      # zond_attention = self.body.get_variable_set('zond_attention')
      # zond_attention.set_text('')
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
      self.fillBulletin(zond)
      
      # zond_attention = self.body.get_variable_set('zond_attention')
      # srok = str('. Анализ ')
      # if 0 != aero_data[0].hour:
      #   srok = u'. Срок прогноза: ' + str(aero_data[0].hour/3600) 
      # zond_attention.set_text(u'Результат объективного анализа. Центр: ' + aero_data[0].center + meteoglobal.trUtf8(srok))
    else:
      return False
    return True


  # 
  # Заполняем документ на основе зонда
  # 
  def fillBulletin(self, zond):
    self.ur_station = Uroven()
    zond.getUrz(self.ur_station)
    if self.ur_station.isGood(ValueType.UR_H):
      self.h0 = self.ur_station.value(ValueType.UR_H)
    else:
      self.h0 = int(zond.alt())
      self.ur_station.set(ValueType.UR_H, self.h0, 1)
      zond.setData(1, 0, self.ur_station)
    self.h0 = math.ceil(self.h0)


    try:
      self.zond_datetime = datetime.datetime.strptime( zond.dateTime(), "%Y-%m-%dT%H:%M:%S" )
    except:
      self.zond_datetime = self.arg_date

    meteocalc.calcAvgWind(zond, self.levels)

    incrementedWind = None
    for level in self.levels:
      if 12000 == level['h2'] and 'dd' in level and 'ff' in level:
        incrementedWind = meteocalc.calcAvgWindBy12(level['dd'], level['ff'])
        break

    if incrementedWind:
      for level in self.levels:
        if level['h2'] <= 12000:
          continue
        if not 'dd' in level:
          level['dd'] = incrementedWind[level['h2']]['dd']
        if not 'ff' in level:
          level['ff'] = incrementedWind[level['h2']]['ff']


    meteo  = u"СЛОЙ "
    dt     = self.zond_datetime
    day    = dt.day
    hour   = dt.hour
    meteo += self.station_name + ' ' + str(day).zfill(2) + str(hour).zfill(2) + '0'
    for level in self.levels:
      if ('dd' in level and 'ff' in level):
        dd = int(round(level['dd'] / 10, 0))
        meteo += ' ' + str(level['code']) + str(dd).zfill(2) + str(int(level['ff'])).zfill(2)
    meteo += '='
    self.meteo = meteo



# 
# ====================== MAIN FUNCTION ============================
# 


def main(argv):
  #Qt-приложение
  app = QCoreApplication (sys.argv)

  parser = argparse.ArgumentParser()
  parser.add_argument('-m', '--mode', default='arg', help="Режим запуска сценария. "
                                                          "arg - Задание аргументов запуска "
                                                          "proto - Передача прото-структуры в стандартный поток")

  parser.add_argument('-f', '--filepath', default=PRJ_DIR + '/var/meteo/documents/',
                      help="Путь сохранения файла.")
  parser.add_argument('-i', '--index', help='Индекс станции')
  parser.add_argument('-d', '--datetime', help='Дата и время измерений в формате YYYY-MM-DDThh:mm:ss')

  options = parser.parse_args()

  generator = LayerGenerator()
  init = False
  if str(options.mode) == "arg":
    init = generator.init(options)
  elif str(options.mode) == "proto":
    sa = StartArguments()
    sa.ParseFromString(meteoglobal.readStdinBytes())
    init = generator.initFromProto(sa)
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
