#!/usr/bin/env python
# -*- coding: utf-8 -*-
# системные модули
import os
import sys
from sys import argv
import logging

# конфигурационный модуль
from conf import *

sys.path.append(PRJ_DIR + '/lib')

# модули времени и даты
from datetime import timedelta, datetime
import time

# модули для работы с документами
from lpod3 import document
from lpod3.document import *

# модули для работы с удаленным сервисом
from google.protobuf.service import RpcController
from   document_service_pb2 import *
import services_pb2 
from  surface_pb2 import *
import sprinf_pb2 as sprinf

import libpuansonwrap
import libmeteodatawrap
import libglobalwrap
import libweatherwrap


# модуль для парсинга аргументов строки командной
import argparse
import base64

# модули оболочек C++ классов Puanson, TMeteoData, Zond
import libglobalwrap
import tempfile

import meteoglobal
import meteocalc

# Qt-модули
from PyQt5.QtGui import *
# ипортируем главный класс для наслоедования
from documentform import DocumentFormal

class GidroDoc(DocumentFormal):

  def __init__(self):
    super().__init__()
    
    self.hasData           = False
    self.ur_station        = None
    self.result            = ExportResponse()
    self.result.result     = False
    self.text              = ''
    self.descs             = ['HHHH','HiHiHiK', 'Cw']
    
    self.station_type      = 0
    self.template_path     = "/share/meteo/odtsamples/gidrospravka.odt"
    
    

  # 
  # инициализация с параметрами
  # 
  def init(self, options):
    if (None == options.datetime) or (None == options.index) \
        or (None == options.filepath):
      self.result.result = False
      self.result.comment = u"Не заданы требуемые параметры"
      return False

    self.arg_date          = datetime.strptime(options.datetime, "%Y-%m-%dT%H:%M:%S")
    self.arg_date_end      = datetime.strptime(options.datetime_end, "%Y-%m-%dT%H:%M:%S")
    self.arg_station       = Point()
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
    
    self.text = proto.text
    
    if proto.datetime_end: 
      self.arg_date_end = datetime.strptime(proto.datetime_end, "%Y-%m-%dT%H:%M:%S")
    else:
      self.arg_date_end = self.arg_date + timedelta( days=1 )
    
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
    
    # получаем таблицу BUFR
    request = sprinf.BufrParametersTableRequest()
    request.table_code.extend(["5515012"])
    self.bufrtable = meteoglobal.RemoteCall(services_pb2.kSprinf, "sprinf_pb2@SprinfService.GetBufrParametersTables", request, 30000)

    
    begin     = self.body.get_variable_set("begin")
    end       = self.body.get_variable_set("end")
    index     = self.body.get_variable_set("index")
    obj       = self.body.get_variable_set("object")
    dateBegin = self.arg_date
    dateEnd   = self.arg_date_end
    
    begin.set_text( meteoglobal.trUtf8( dateBegin.strftime('%Y-%m-%d %H:%M') ) )
    end.set_text  ( meteoglobal.trUtf8( dateEnd.strftime('%Y-%m-%d %H:%M') ) )
    index.set_text( self.getStationNumber( self.station.station ) )
    obj.set_text  ( meteoglobal.trUtf8( self.text ) )
    
    # logging.error(dateEnd)
    status, meteodata_reply = self.getGidroData()
    self.hasData      = status
    



    if status is False:
      return False
    
    table = self.body.get_table(name='table')
    if len(meteodata_reply) > 3:
      row = table.get_rows()
      row = row[1]
      for i in range(len(meteodata_reply)-3) :
        table.append_row(row)
    for i in range(len(meteodata_reply)) :
      meteodata = libmeteodatawrap.TMeteoData()
      meteodata.fromArrayObject( meteodata_reply[i] )
      
      cell = table.get_cell('A' + str(i+2))

      date,time = meteodata.getDateTime().split('T')
      
      time = time.split(':')
      cell.set_text(meteoglobal.trUtf8(date + ' ' + time[0] + ':' + time[1]))
      table.set_cell('A' + str(i+2), cell)

      cell = table.get_cell('B' + str(i+2))
      if meteodata.meteoParam('HHHH').isInvalid():
        buf = '-'
      else:
        buf = meteodata.meteoParam('HHHH').value()
      cell.set_text(meteoglobal.trUtf8(buf))
      table.set_cell('B' + str(i+2), cell)

      cell = table.get_cell('C' + str(i+2))
      if meteodata.meteoParam('HiHiHiK').isInvalid():
        buf = '-'
      else:
        buf = meteodata.meteoParam('HiHiHiK').value()
        
      cell.set_text(meteoglobal.trUtf8(buf))
      table.set_cell('C' + str(i+2), cell)

      cell = table.get_cell('D' + str(i+2))
      buf = meteodata.meteoParam('Cw').value()
      buf = self.getBufrValue(buf)
      cell.set_text(meteoglobal.trUtf8(buf))
      table.set_cell('D' + str(i+2), cell)


  def getBufrValue(self, code):
    if self.bufrtable is None:
      return "-"
    if len(self.bufrtable.table) == 0:
      return "-"
    for param in self.bufrtable.table[0].parameter:
      if int(code) == int(param.code):
        return param.name
    return "-"


  # 
  # Получаем данные по станции
  # 
  def getGidroData(self):
    request            = DataRequest()
    
    # 
    # если задана конечная дата, то берем ее
    # 
    if hasattr(self, 'arg_date_end') :
      request.date_start = self.arg_date.strftime("%Y-%m-%dT%H:00:00")
      request.date_end   = self.arg_date_end.strftime("%Y-%m-%dT%H:00:00")
    # иначе отсчитываем сутки назад
    else:
      request.date_start = (self.arg_date - timedelta(days=1)).strftime("%Y-%m-%dT%H:00:00")
      request.date_end   = self.arg_date.strftime("%Y-%m-%dT%H:00:00")
      
    request.station.append(str(self.station.station))
    request.meteo_descrname.extend(self.descs)
    request.type_level = 1
    # request.as_proto   = proto
   
    try:
      response       = meteoglobal.RemoteCall(  servicecode    = services_pb2.kSrcData,
                                                methodfullname = 'surface_service_pb2@SurfaceService.GetMeteoDataOnStation',
                                                request        = request,
                                                timeout_msecs  = 30000 )
    except:
      response=False
      
    # logging.error(response.meteodata[0])
    if response is False :
      self.result.comment = u'Ответ от сервиса meteo не получен'
      return False, []
    try:
      if response is None or (not response.meteodata):
        self.result.comment = u'Ответ от сервиса meteo получен, но данных нет'
        return False, []
    except:
      self.result.comment = response.comment
      return False, []

    # HasData Meteodata
    return True, response.meteodata


# 
# ====================== MAIN FUNCTION ============================
# 

def main(argv):
  QGuiApplication(sys.argv)

  parser  = argparse.ArgumentParser(description=meteoglobal.trUtf8("Построение Прогноз распределения ветра по району"))
  parser.add_argument('--inputType', '-m', type=str, help=meteoglobal.trUtf8(
      "Тип источника входных данных  proto = прото файл или arg = коммандная строка"))
  parser.add_argument('--date', '-dt', type=str, help=meteoglobal.trUtf8("Дата начала прогноза"))
  parser.add_argument('--rank', '-r', type=str, help=meteoglobal.trUtf8("Звание автора"))
  parser.add_argument('--name', '-n', type=str, help=meteoglobal.trUtf8("Имя автора"))

  options   = parser.parse_args()

  generator = GidroDoc()
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
