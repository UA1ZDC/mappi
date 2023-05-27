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

from optparse import OptionParser
import argparse

import base64
import meteoglobal
import surface_pb2
from surface_service_pb2 import *
import field_pb2
import meteocalc
from libzondwrap import *
import services_pb2

from PyQt5.QtCore import QCoreApplication

# ипортируем главный класс для наслоедования
from documentform import DocumentFormal

class Meteo11Generator(DocumentFormal):
  
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
    self.template_path     = "/share/meteo/odtsamples/Meteo11.odt"
    

  # 
  # инициализация с параметрами
  # 
  def init(self, options):
    if (None == options.datetime) or (None == options.index) \
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
    
      
    return True



  # 
  # Заполнение документа
  # 
  def fillDocument(self):
    
    self.levels = [{'h1': self.h0, 'h2': 200,   'gr': '02'},
                   {'h1': self.h0, 'h2': 400,   'gr': '04'},
                   {'h1': self.h0, 'h2': 800,   'gr': '08'},
                   {'h1': self.h0, 'h2': 1200,  'gr': '12'},
                   {'h1': self.h0, 'h2': 1600,  'gr': '16'},
                   {'h1': self.h0, 'h2': 2000,  'gr': '20'},
                   {'h1': self.h0, 'h2': 2400,  'gr': '24'},
                   {'h1': self.h0, 'h2': 3000,  'gr': '30'},
                   {'h1': self.h0, 'h2': 4000,  'gr': '40'},
                   {'h1': self.h0, 'h2': 5000,  'gr': '50'},
                   {'h1': self.h0, 'h2': 6000,  'gr': '60'},
                   {'h1': self.h0, 'h2': 8000,  'gr': '80'},
                   {'h1': self.h0, 'h2': 10000, 'gr': '10'},
                   {'h1': self.h0, 'h2': 12000, 'gr': '12'},
                   {'h1': self.h0, 'h2': 14000, 'gr': '14'},
                   {'h1': self.h0, 'h2': 18000, 'gr': '18'},
                   {'h1': self.h0, 'h2': 22000, 'gr': '22'},
                   {'h1': self.h0, 'h2': 26000, 'gr': '26'},
                   {'h1': self.h0, 'h2': 30000, 'gr': '30'}]
    
    # получаем тело документа
    body      = self.doc.get_body()
    table11   = body.get_table(name="Meteo11")

    meteo11   = body.get_variable_set("meteo11")
    dtime     = body.get_variable_set("datetime")
    index     = body.get_variable_set("index")
    height    = body.get_variable_set("height")
    temp      = body.get_variable_set("temp")
    press     = body.get_variable_set("press")
    attention = body.get_variable_set('attention')
    

    if not self.zond_from_src() and not self.zond_from_field():
      self.hasData        = False
      self.result.comment = u'Данных нет'
    else:
      self.hasData        = True
      self.result.result  = True 
      
    
    dt        = self.zond_datetime
    dtime.set_text(meteoglobal.dateToHumanTimeShort(dt, True)) 
    index.set_text( self.getStationNumber(self.station.station) + " (%s)" % self.station_name )
    
    
    height.set_text(    str(int(self.alt))             )
    temp.set_text(      str(self.temperature)         )
    press.set_text(     str(self.pressure)            )
    attention.set_text( meteoglobal.trUtf8(self.text) )

    params    = ['avgDevD', 'avgDevT', 'dd', 'ff']
    fstCellL  = 'B'
    fstCellI  = 2
    i         = 0
    while i < 4:
      for level in self.levels:
        cell = table11.get_cell(fstCellL + str(fstCellI))
        if params[i] in level:
          cell.set_text(str(int(round(level[params[i]], 0))))
          table11.set_cell(fstCellL + str(fstCellI), cell)
        fstCellI += 1
      fstCellL = chr(ord(fstCellL) + 1)
      fstCellI = 2
      i += 1
    meteo11.set_text(self.meteo)
    
    


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
      # logging.error("SRC")
      # logging.error(data)
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
      # logging.error(data)
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
      # print "Не удалось определить высоту приземного уровня (высоту станции над уровнем моря), недостаточно данных"
      # sys.exit(1)
    self.h0 = math.ceil(self.h0)
    
    
    # logging.error(  type(zond.dateTime()) )
    # logging.error(  zond.dateTime() )
    try:
      self.zond_datetime = datetime.datetime.strptime( zond.dateTime(), "%Y-%m-%dT%H:%M:%S" )
    except:
      self.zond_datetime = self.arg_date

    meteocalc.calcAvgWind(zond, self.levels)
    meteocalc.calcAvgDevD(zond, self.levels)
    
    for level in self.levels:
      level['avgDevT'] = meteocalc.calcDeltaTemp(zond, level['h1'], level['h2'])

    meteo   = u'Метео 11' + str(self.number).zfill(2) + ' - '
    dt      = self.zond_datetime
    day     = dt.day
    hour    = dt.hour
    minute  = dt.minute / 10
    meteo  += str(day).zfill(2) + str(hour).zfill(2) + str(minute)
    height  = int(round(self.h0 / 10) * 10)
    tt      = u'-'
    ppp     = u'-'

    if height >= 0:
      height = str(height).zfill(4)
    else:
      height = str(5000 - height).zfill(4)
    meteo += ' - ' + height + ' - '

    sa = [{'h2': self.h0}]
    meteocalc.calcSA81(sa)
    if self.ur_station.isGood(ValueType.UR_T):
      tt = int(self.ur_station.value(ValueType.UR_T))
      self.temperature = tt
    else:
      self.temperature = u'-'
    if self.ur_station.isGood(ValueType.UR_P):
      ppp = int(self.ur_station.value(ValueType.UR_P))
      self.pressure = ppp
    else:
      self.pressure = u'-'
    if not tt == u'-' and not ppp == u'-':
      Tv = oprTvirt(tt, ppp)
      tt = int(round(Tv - sa[0]['T'], 0))
    if not ppp == u'-':
      delta = ppp - sa[0]['P']
      ppp = int(math.ceil(delta * 100 * self.kPa2mm))

    self.pressure = ppp
    if not ppp == u'-' and ppp < 0:
      ppp = int(500 - ppp)
    mtt = tt
    self.temperature = tt
    if not mtt == u'-' and not (tt >= 0 or tt <= -50):
      mtt = 50 - tt
    if not mtt == u'-' and tt <= -50:
      mtt = tt * (-1)

    meteo += str(ppp).zfill(3) + str(mtt).zfill(2)

    for level in self.levels:
      if (not 'avgDevT' in level) or (not 'avgDevD' in level) or (not 'dd' in level) or (not 'ff' in level):
        continue
      meteo += ' - '

      devD = int(round(level['avgDevD'], 0))
      devT = int(round(level['avgDevT'], 0))

      if devD >= 0:
        devD = str(devD).zfill(2)
      else:
        devD = str(50 - devD).zfill(2)
      if devT >= 0:
        devT = str(devT).zfill(2)
      else:
        devT = str(50 - devT).zfill(2)
      meteo += level['gr'] + devD + ' - '
      meteo += devT + str(int(round(level['dd'] / 6, 0))).zfill(2) + str(int(round(level['ff'], 0))).zfill(2)

    windH = 0
    tempH = 0
    urs = zond.urovenList()
    for ur in urs.values():
      if not ur.isGood(ValueType.UR_H) or ur.quality(ValueType.UR_H) < 0 or ur.quality(ValueType.UR_H) > 5:
        continue
      ddqual = ur.quality(ValueType.UR_dd)
      if ddqual >= 0 and ddqual <= 5:
        windH = max(windH, ur.value(ValueType.UR_H))
      Tqual = ur.quality(ValueType.UR_T)
      if Tqual >= 0 and Tqual <= 5:
        tempH = max(tempH, ur.value(ValueType.UR_H))

    meteo += ' - ' + str(int(math.floor(tempH / 100))).zfill(2) + str(int(math.floor(windH / 100))).zfill(2)
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

  options   = parser.parse_args()

  generator = Meteo11Generator()
  init      = False
  if str(options.mode) == "arg":
    init = generator.init(options)
  elif str(options.mode) == "proto":
    sa   = StartArguments()
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
