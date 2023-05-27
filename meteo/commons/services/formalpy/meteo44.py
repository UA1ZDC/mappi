#!/usr/bin/python3
# coding: utf-8

#системные модули
import os
import sys
from sys import argv

from conf import *
sys.path.append(PRJ_DIR + '/lib')

#модули для работы с удаленным сервисом
from google.protobuf.service import RpcController

#модули времени и даты
import datetime
import time

import math
import logging

from meteo_controller import *

import libmeteodatawrap
from libmeteodatawrap import *
from libglobalwrap import *

#модули для работы с документами
from lpod3 import document

from document_service_pb2 import *

import meteocalc
import meteoglobal
import surface_pb2
from surface_service_pb2 import *
import field_pb2
from libzondwrap import *
import services_pb2

import base64

from optparse import OptionParser
import argparse

from PyQt5.QtCore import QCoreApplication

# ипортируем главный класс для наслоедования
from documentform import DocumentFormal

class Meteo44Generator(DocumentFormal):
  
  def __init__(self):
    super().__init__()
    
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
    self.template_path     = "/share/meteo/odtsamples/Meteo44.odt"
    

  # 
  # инициализация с параметрами
  # 
  def init(self, options):
    if (None == options.datetime) or (0 == options.index) \
        or (None == options.filepath) or (None == options.number):
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
  # заполняем данные из прото
  # 
  def initFromProto(self, proto):
    if (None == proto.datetime) or (0 == proto.stations.__len__()) or (None == proto.number):
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
    
    if proto.user_name:
      self.user = proto.user_name
    if proto.user_rank:
      self.rank = proto.user_rank
    if proto.department:
      self.department = proto.department
      
    return True

  # 
  # Заполнение документа
  # 
  def fillDocument(self):
    
    self.levels = [     {'h1': self.h0, 'h2': 24000, 'gr': 24},
                        {'h1': self.h0, 'h2': 34000, 'gr': 34},
                        {'h1': self.h0, 'h2': 44000, 'gr': 44},
                        {'h1': self.h0, 'h2': 54000, 'gr': 54},
                        {'h1': self.h0, 'h2': 64000, 'gr': 64}]

    self.temp_levels = [{'h1': self.h0, 'h2': 16000, 'gr': 16},
                        {'h1': self.h0, 'h2': 20000, 'gr': 20},
                        {'h1': self.h0, 'h2': 24000, 'gr': 24},
                        {'h1': self.h0, 'h2': 28000, 'gr': 28},
                        {'h1': self.h0, 'h2': 30000, 'gr': 30}]

    self.wind_levels = [{'h1':self.h0,  'h2': 10000, 'gr': 10},
                        {'h1': self.h0, 'h2': 24000, 'gr': 24},
                        {'h1': self.h0, 'h2': 30000, 'gr': 30}]
    # получаем тело документа
    body    = self.doc.get_body()
    table44 = body.get_table(name="Meteo44")

    meteo44   = body.get_variable_set("meteo44")
    dtime     = body.get_variable_set("datetime")
    index     = body.get_variable_set("index")
    height    = body.get_variable_set("height")
    temp      = body.get_variable_set("temp")
    press     = body.get_variable_set("press")
    attention = body.get_variable_set('attention')

    attention.set_text(meteoglobal.trUtf8(self.text))
    index.set_text( self.getStationNumber(self.station.station) + " (%s)" % self.station_name )
    height.set_text(str(int(self.h0)))


    if not self.zond_from_src() and not self.zond_from_field():
      self.hasData        = False
      self.result.comment = u'Данных нет'
    else:
      self.hasData       = True
      self.result.result = True  

    dt      = self.zond_datetime
    dtime.set_text(meteoglobal.dateToHumanTimeShort(dt, True))
    
    if self.ur_station.isGood(ValueType.UR_T):
      toSetTemp = self.ur_station.value(ValueType.UR_T)
      toSetTemp += meteocalc.virtualCorrection(toSetTemp)
      toSetTemp = '%.1f' % toSetTemp
    else:
      toSetTemp = '-'
      
    temp.set_text(toSetTemp)
    if self.ur_station.isGood(ValueType.UR_P):
      toSetPress = str(int(self.ur_station.value(ValueType.UR_P)))
    else:
      toSetPress = '-'
    press.set_text(toSetPress)

    params   = ['avgDevT', 'dd', 'ff']
    fstCellL = 'B'
    fstCellI = 2
    i        = 0
    while i < 3:
      for level in self.levels:
        cell = table44.get_cell(fstCellL + str(fstCellI))
        if params[i] in level:
          cell.set_text(str(int(round(level[params[i]], 0))))
          table44.set_cell(fstCellL + str(fstCellI), cell)
        fstCellI += 1
      fstCellL = chr(ord(fstCellL) + 1)
      fstCellI = 2
      i += 1
    meteo44.set_text(self.meteo)
    
  
  
  
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
  def fillBulletin(self,zond):
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

    meteocalc.calcAvgWind(zond, self.wind_levels)
    meteocalc.calcAvgWind(zond, self.levels)
    
    # logging.error(self.wind_levels)
    # logging.error(self.levels)
    
    for level in self.temp_levels:
      level['avgDevT'] = meteocalc.calcDeltaTemp(zond, level['h1'], level['h2'])

    meteo   = u'Метео 44' + str(self.number).zfill(2) + ' - '
    dt      = self.zond_datetime
    day     = dt.day
    hour    = dt.hour
    minute  = dt.minute / 10
    meteo  += str(day).zfill(2) + str(hour).zfill(2) + str(minute)
    height  = int(round(self.h0 / 10) * 10)
    
    if height >= 0:
      height = str(height).zfill(4)
    else:
      height = str(5000 - height).zfill(4)
    meteo += ' - ' + height + ' - '
    pppp = int(round(self.ur_station.value(ValueType.UR_P), 0))
    if pppp >= 0:
      pppp = str(pppp).zfill(4)
    else:
      pppp = str(5000 - pppp).zfill(4)
    tt = int(round(self.ur_station.value(ValueType.UR_T), 0))
    if tt >= 0:
      tt = str(tt).zfill(2)
    elif tt <= -50:
      tt = str(tt * (-1)).zfill(2)
    else:
      tt = str(50 - tt).zfill(2)
    meteo += pppp + tt

    level_index = 0
    for level in self.levels:
      if 'avgDevT' in self.temp_levels[level_index]:
        level['avgDevT'] = self.temp_levels[level_index]['avgDevT']
        meteo += ' - '
        devT = int(round(level['avgDevT'], 0))
        if devT >= 0:
          devT = str(devT).zfill(2)
        elif -50 < devT < 0:
          devT = str(50 - devT).zfill(2)
        else:
          devT = str(devT * -1).zfill(2)
        meteo += str(level['gr']) + devT
      if level['gr'] == 24:
        if 'dd' in self.wind_levels[0] and 'dd' in self.wind_levels[1]:
          level['dd'] = -0.5 * self.wind_levels[0]['dd'] + 1.5 * self.wind_levels[1]['dd']
          if 'dd' in level:
            dd = str(int(round(level['dd'], 0)))
          if 'ff' in level:
            ff = str(int(round(level['ff'], 0)))
          if dd or ff:
            meteo += ' - '
            if dd:
              meteo += dd
            if ff:
              meteo += ff
      if level['gr'] == 34:
        dd = ff = None
        if 'dd' in self.wind_levels[0] and 'dd' in self.wind_levels[2]:
          level['dd'] = -0.3 * self.wind_levels[0]['dd'] + 1.3 * self.wind_levels[2]['dd']
          if 'dd' in level:
            dd = str(int(round(level['dd'], 0)))
          if 'ff' in level:
            ff = str(int(round(level['ff'], 0)))
          if dd or ff:
            meteo += ' - '
            if dd:
              meteo += dd
            if ff:
              meteo += ff
      level_index += 1
    self.meteo = meteo
    return True


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
  parser.add_argument('-n', '--number', help='Условный номер станции')

  options = parser.parse_args()

  generator = Meteo44Generator()
  init = False
  if str(options.mode) == "arg":
    init = generator.init(options)
  elif str(options.mode) == "proto":
    sa = StartArguments()
    logging.error(sa)
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
