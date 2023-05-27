#!/usr/bin/env python
# -*- coding: utf-8 -*-
# системные модули
import os
import sys
from sys import argv

# конфигурационный модуль
from conf import *

from meteoglobal import error_log

sys.path.append(PRJ_DIR + '/lib')

# модули времени и даты
from datetime import timedelta, datetime
import time

import logging

# модули для работы с документами
from lpod3 import document
from lpod3.document import *

# модули для работы с удаленным сервисом
from   google.protobuf.service import RpcController
import surface_pb2  as surface
import services_pb2 as services
import surface_service_pb2
import field_pb2 as field
import meteo_controller
import sprinf_pb2
import services_pb2
import surface_pb2
from   document_service_pb2 import *

# модуль для парсинга аргументов строки командной
import argparse
import base64

# модули оболочек C++ классов Puanson, TMeteoData, Zond
from  libzondwrap import *
import tempfile

import meteoglobal
import meteocalc
import math

# Qt-модули
from PyQt5.QtGui import *

# ипортируем главный класс для наслоедования
from documentform import DocumentFormal

class WindRegionDoc(DocumentFormal):

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
    self.department        = ''
    self.user_name         = ''
    self.user_rank         = ''
    
    self.station_type      = 0
    self.template_path     = "/share/meteo/odtsamples/forecastWindRegion.odt"


  # 
  # инициализация с параметрами
  # 
  def init(self, options):
    if (None == options.datetime) or (None == options.index) \
        or (None == options.filepath):
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
      self.user_name = proto.user_name
    if proto.user_rank:
      self.user_rank = proto.user_rank
    if proto.department:
      self.department = proto.department
            
    return True
    

  # 
  # Заполнение документа
  # 
  def fillDocument(self):
    self.fillInfoTable()
    self.fillWindTable()
    self.authorTable()

  # 
  # Заполняем информационную составляющую таблицы
  # 
  def fillInfoTable(self):
    meteodir    = os.path.expanduser('~/.meteo/')
    cur_month   = datetime.now().month
    nameCounter = '/forecastWindRegion_counter'
    if os.path.isfile(os.path.join(meteodir + nameCounter)):
      counter = open(os.path.join(meteodir + nameCounter), 'r')
      number  = int(counter.readline())
      month   = int(counter.readline())
      counter.close()
      if cur_month > month:
        month = cur_month
        number = 1
      else:
        number += 1
      counter = open(os.path.join(meteodir + nameCounter), 'w')
      counter.write(str(number) + '\n')
      counter.write(str(month))
      counter.close()
    else:
      if not os.path.exists(meteodir):
        os.mkdir(meteodir)
      counter = open(os.path.join(meteodir + nameCounter), 'w+')
      number  = 1
      month   = cur_month
      counter.write(str(number) + '\n')
      counter.write(str(month))
      counter.close()

    infotable = self.body.get_table(name='infotable')

    cell = infotable.get_cell('B1')
    cell.set_text(str(number))
    infotable.set_cell('B1', cell)

    cell = infotable.get_cell('B2')
    cell.set_text(meteoglobal.trUtf8(self.station_name))
    infotable.set_cell('B2', cell)

    cell = infotable.get_cell('B3')
    cell.set_text( self.getStationNumber(self.station.station) )
    infotable.set_cell('B3', cell)

    cell = infotable.get_cell('B4')
    cell.set_text(str(self.alt))
    infotable.set_cell('B4', cell)

    cell = infotable.get_cell('B5')
    cell.set_text( self.getLatStr() +" "+ self.getLonStr() )
    infotable.set_cell('B5', cell)

  # 
  # Подготавливаем запрос к сервису для диаграммы из полей прогноза
  # 
  def makeADRequest(self,date):
    request                = field.DataRequest()
    coords                 = surface_pb2.Point()
    coords.fi              = self.lat_radian
    coords.la              = self.lon_radian
    coords.height          = self.alt
    request.coords.extend([coords])
    request.forecast_start = date.strftime('%Y-%m-%dT%H:%M:%SZ')
    request.forecast_end   = date.strftime('%Y-%m-%dT%H:%M:%SZ')
    request.only_last      = True
    request.only_best      = True
    return request

    
  # 
  # Получаем данные зондирования из полей
  # 
  def getADData(self,request):
    try:
      response       = meteoglobal.RemoteCall(  servicecode    = services_pb2.kField,
                                                methodfullname = 'field_pb2@FieldService.GetADZond',
                                                request        = request,
                                                timeout_msecs  = 30000 )
    except:
      response=False
    # logging.error(request)  
    # logging.error(response.data)  
    if response is False:
      self.result.comment = u'Ответ от сервиса не получен'
      return False, []
    else:
      if not response.data:
        self.result.comment = u'Ответ от сервиса получен, но данных нет'
        return False, []
    # HasData Meteodata
    return True, response.data 

  # 
  # Заполняем таблицу ветра
  # 
  def fillWindTable(self):
    date                   = self.arg_date
    tm                     = timedelta(hours=6)
    date                   = date.replace(hour= int(date.strftime('%H'))//6*6 ) - tm
    windTable              = self.body.get_table(name='windtable')
    averagewindtable       = self.body.get_table(name='averagewindtable')
    lettersDate            = ['C', 'D', 'E', 'F', 'G', 'H', 'I']
    lettersDD              = ['C', 'E', 'G', 'I', 'K', 'M', 'O']
    lettersFF              = ['D', 'F', 'H', 'J', 'L', 'N', 'P']
    lettersADD             = ['B', 'D', 'F', 'H', 'J', 'L', 'N']
    lettersAFF             = ['C', 'E', 'G', 'I', 'K', 'M', 'O']
    heights                = [ 0, 925, 850, 700, 500, 400, 300, 200, 150, 100, 70, 50, 30, 20, 10]
    # делаем запрос
    request                = self.makeADRequest( date )
    self.hasData           = False
      
    for number in range(7):
      date                   = date + tm
      request.forecast_start = date.strftime('%Y-%m-%dT%H:%M:%SZ')
      request.forecast_end   = date.strftime('%Y-%m-%dT%H:%M:%SZ')
      status, data           = self.getADData(request)

      if status is False :
        continue      
      
      self.hasData           = True
      cell = windTable.get_cell(lettersDD[number] + '1')
      buf  = date.strftime('%H:%M:%S')
      cell.set_text(buf)
      windTable.set_cell(lettersDD[number] + '1', cell)


      cell = windTable.get_cell(lettersDD[number] + '2')
      buf  = meteoglobal.dateToHumanDateOnly(date)
      cell.set_text(buf)
      windTable.set_cell(lettersDD[number] + '2', cell)
      zond = Zond()
      data = data[0].meteodata
      zond.fromArrayObject(data)
      zond.preobr()

      # 
      # Проходимся по высотам
      # 


      for h in range(len(heights)):
        level = Uroven()
        if h == 0 :
          zond.getUrPoLevelType(103, level)
        else:
          zond.getUrPoP(heights[h], level)
        cell = windTable.get_cell(lettersDD[number] + str(h + 4))
        if level.isGood(ValueType.UR_dd):
          value = str(round(level.value(ValueType.UR_dd), 1) )
          cell.set_text(meteoglobal.trUtf8(value))
        else:
          cell.set_text('-')  
        windTable.set_cell(lettersDD[number] + str(h + 4), cell)

        cell = windTable.get_cell(lettersFF[number] + str(h + 4))
        if level.isGood((ValueType.UR_ff)):
          value = str(round(level.value(ValueType.UR_ff), 1 ) )
          cell.set_text(meteoglobal.trUtf8(value))
        else:
          cell.set_text('-')
        windTable.set_cell(lettersFF[number] + str(h + 4), cell)

      ur_station = Uroven()
      zond.getUrPoLevelType(103,ur_station)
      if ur_station.isGood(ValueType.UR_H):
        h0 = ur_station.value(ValueType.UR_H)
      else:
        h0 = int(zond.alt())
        ur_station.set(ValueType.UR_H, h0, 1)
    
      P = ur_station.value(ValueType.UR_P)
      T = ur_station.value(ValueType.UR_T)
    
      if  not ur_station.isGood(ValueType.UR_P) and ur_station.isGood(ValueType.UR_T):
        ur_101 = Uroven()
        zond.getUrPoLevelType(101, ur_101)
        pqnh = ur_101.value(ValueType.UR_P)
        P = meteocalc.pH_from_PQNH(pqnh, h0, T)
        ur_station.set(ValueType.UR_P, P, 1)

      zond.setData(103, 0, ur_station)
      h0 = math.ceil(h0)
      # zond.preobr()

      levels = [{'h1': h0, 'h2': 200, 'gr': '02'},
                {'h1': h0, 'h2': 400, 'gr': '04'},
                {'h1': h0, 'h2': 800, 'gr': '08'},
                {'h1': h0, 'h2': 1200, 'gr': '12'},
                {'h1': h0, 'h2': 1600, 'gr': '16'},
                {'h1': h0, 'h2': 2000, 'gr': '20'},
                {'h1': h0, 'h2': 2400, 'gr': '24'},
                {'h1': h0, 'h2': 3000, 'gr': '30'},
                {'h1': h0, 'h2': 4000, 'gr': '40'},
                {'h1': h0, 'h2': 5000, 'gr': '50'},
                {'h1': h0, 'h2': 6000, 'gr': '60'},
                {'h1': h0, 'h2': 8000, 'gr': '80'},
                {'h1': h0, 'h2': 10000, 'gr': '10'},
                {'h1': h0, 'h2': 12000, 'gr': '12'},
                {'h1': h0, 'h2': 14000, 'gr': '14'},
                {'h1': h0, 'h2': 18000, 'gr': '18'},
                {'h1': h0, 'h2': 22000, 'gr': '22'},
                {'h1': h0, 'h2': 26000, 'gr': '26'},
                {'h1': h0, 'h2': 30000, 'gr': '30'}]

      meteocalc.calcAvgWind(zond, levels)

      incrementedWind = None
      for level in levels:
        if 12000 == level['h2'] and 'dd' in level and 'ff' in level:
          incrementedWind = meteocalc.calcAvgWindBy12(level['dd'], level['ff'])
          break

      if incrementedWind:
        for level in levels:
          if level['h2'] <= 12000:
            continue
          if not 'dd' in level:
            level['dd'] = incrementedWind[level['h2']]['dd']
          if not 'ff' in level:
            level['ff'] = incrementedWind[level['h2']]['ff']

      cell = averagewindtable.get_cell(lettersADD[number] + '1')
      buf = date.strftime('%H:%M:%S')
      cell.set_text(buf)
      averagewindtable.set_cell(lettersADD[number] + '1', cell)

      cell = averagewindtable.get_cell(lettersADD[number] + '2')
      buf = meteoglobal.dateToHumanDateOnly(date)
      cell.set_text(buf)
      averagewindtable.set_cell(lettersADD[number] + '2', cell)
      i = 0
      for level in levels:
        if (not 'dd' in level) or (not 'ff' in level):
          devD = str('-')
          devF = str('-')
        else:
          devD = round(level['dd'], 2)
          devF = round(level['ff'], 2)

        cell = averagewindtable .get_cell(lettersADD[number] + str(i + 4))
        cell.set_text(meteoglobal.trUtf8(devD))
        averagewindtable.set_cell(lettersADD[number] + str(i + 4), cell)

        cell = averagewindtable .get_cell(lettersAFF[number] + str(i + 4))
        cell.set_text(meteoglobal.trUtf8(devF))
        averagewindtable.set_cell(lettersAFF[number] + str(i + 4), cell)
        i += 1

  # 
  # Заполняем таблицу авторов
  # 
  def authorTable(self):
    authortable = self.body.get_table(name='authortable')

    try:
      cell = authortable.get_cell('B1')
      cell.set_text( meteoglobal.trUtf8( self.department ) )
      authortable.set_cell('B1', cell)
    except:
      logging.error("Не могу заполнить поле department")

    try:
      cell = authortable.get_cell('B2')
      cell.set_text( meteoglobal.trUtf8(self.user_rank) + " " + meteoglobal.trUtf8(self.user_name) )
      authortable.set_cell('B2', cell)
    except:
      logging.error("Не могу заполнить поля данных пользователя")
  

    cell = authortable.get_cell('B3')
    cell.set_text( meteoglobal.trUtf8( meteoglobal.dateToHumanTimeShort(datetime.now()) ) )
    authortable.set_cell('B3', cell)



# 
# ====================== MAIN FUNCTION ============================
# 

def main(argv):
  QGuiApplication(sys.argv)

  parser  = argparse.ArgumentParser(description=meteoglobal.trUtf8("Построение Прогноз распределения ветра по району"))
  parser.add_argument('--inputType', '-m', type=str, help=meteoglobal.trUtf8(
    "Тип источника входных данных  proto = прото файл или arg = коммандная строка"))
  parser.add_argument('--index', '-i', type=str, help=meteoglobal.trUtf8("Индекс станции(аэродрома)"))
  parser.add_argument('--date', '-dt', type=str, help=meteoglobal.trUtf8("Дата начала прогноза"))
  parser.add_argument('--rank', '-r', type=str, help=meteoglobal.trUtf8("Звание автора"))
  parser.add_argument('--name', '-n', type=str, help=meteoglobal.trUtf8("Имя автора"))
  parser.add_argument('--department', '-dp', type=str, help=meteoglobal.trUtf8("Подразделение"))
  parser.add_argument('--fi', '-f', type=str, help=meteoglobal.trUtf8("fi координата"))
  parser.add_argument('--la', '-l', type=str, help=meteoglobal.trUtf8("la координата"))

  options   = parser.parse_args()

  generator = WindRegionDoc()
  init      = False
  if str(options.inputType) == "arg":
    init = generator.init(options)
  elif str(options.inputType) == "proto":
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
