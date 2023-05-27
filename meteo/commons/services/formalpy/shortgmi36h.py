#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
import optparse
import argparse
from datetime import datetime, timedelta
import logging

import field_pb2
import surface_pb2
import appconf_pb2
import sprinf_pb2
import state_pb2
import services_pb2
import document_service_pb2
# конфигурационный модуль
from conf import *
sys.path.append(PRJ_DIR + '/lib')
from sys import argv

from libzondwrap import *
import meteoglobal
import libpuansonwrap
from document_service_pb2 import *
from sprinf_pb2 import *

# Qt-модули
from PyQt5.QtCore import *
# ипортируем главный класс для наслоедования
from documentform import DocumentFormal

class ShortGMIDoc(DocumentFormal):
  
  
  def __init__(self):
    super().__init__()
    
    self.service_code      = services_pb2.kSrcData
    
    self.descs             = ['CH', 'CM', 'CL', 'w', 'Nh', 'h', 'V', 'precip', 'dd', 'ff', 'T', 'U', 'P0', 'R12', 'Td', 'wind',
                              'cloud', 'Ptend']  # дескрипторы идут в определенном порядке, не нарушать
    #Дескрипторы и их коды для форматирования пуансонов
    self.descriptors = {  20010: 'N',  # общая облачность
                          11001: 'dd',  # Направление ветра
                          11002: 'ff',  # Скорость ветра
                          12111: 'T',  # Максимальная температура воздуха у земли
                          12112: 'T',  # Минимальная температура воздуха
                          10004: 'P0'  # Атмосферное давление на уровне станции
                        }

    # перечень параметров, которые берем из табло
    self.tablo_list        = [ 'ch', 'cm', 'cl', 'w', 'n', 'h','v', 'r', 'dd','ff','t','u','p0','' ]
    self.hasData           = False
    self.result            = ExportResponse()
    self.result.result     = False
    self.printFileToStdOut = False
    self.station_type      = 0
    self.template_path     = "/share/meteo/odtsamples/gmi.odt"
    self.template_long_path= "/share/meteo/odtsamples/gmilong.odt"
    
    # заполняем адрес
    self.address, self.host, self.port = self.init_service( self.service_code )


  # 
  # инициализация с параметрами
  # 
  def init(self, options):
    if (None == options.datetime) or \
        ( None == options.index ) :
      self.result.result = False
      self.result.comment = u"Не заданы требуемые параметры"
      return False
    self.arg_date          = datetime.strptime(options.datetime, "%Y-%m-%dT%H:%M:%S")
    self.data_date         = self.arg_date.strftime("%Y-%m-%dT%H:%M:%S")
    self.arg_station       = surface_pb2.Point()
    self.arg_station.index = options.index
    # делаем запрос в БД на получение данных по станции
    self.station           = self.getStationData( str( options.index ) )
    self.station_name      = self.getStationName()
    self.login             = options.author
    # задаем lat lon
    self.fillStationData(self.station)
    
    # сроки прогнозов
    if 'short' == options.type:
      self.docType = document_service_pb2.GmiArguments.kShort
    elif 'medium' == options.type:
      self.docType = document_service_pb2.GmiArguments.kMedium
    elif 'long' == options.type:
      self.docType = document_service_pb2.GmiArguments.kLong
    else:
      self.arg_date_end = None
    
    # рассчитываем окончание прогноза исходя из заданного типа
    self.arg_date_end = self.calc_forecast_date(self.arg_date, self.docType)
   
    return True


  # 
  # заполняем данные из прото
  # 
  def initFromProto(self, proto):
    if (None == proto.dt_start) or ("" == proto.station_index):
      self.result.result = False
      self.result.comment = u"Не заданы требуемые параметры"
      return False
    
    self.arg_date   = datetime.strptime(proto.dt_start, "%Y-%m-%dT%H:%M:%SZ")
    self.station    = self.getStationData( str( proto.station_index ) )
    # путь к документам
    self.filepath = PRJ_DIR + '/var/meteo/documents/'
    
    # параметры станции
    self.station_name = self.getStationName(self.station)
    self.alt          = self.station.position.height_meters
    self.h0           = self.station.position.height_meters
    self.station_type = self.station.type
    
    # задаем lat lon
    self.fillStationData(self.station)
      
    self.printFileToStdOut = True
    
    # сроки прогнозов
    self.docType   = proto.doc_type
    self.login     = proto.author
    self.docNumber = proto.document_number
    
    self.arg_date_end = self.calc_forecast_date(self.arg_date, self.docType)
    
    return True

  # 
  # Рассчитываем срок прогноза на основе типа прогноза
  # @forecast_type - тип прогноза 
  # 
  def calc_forecast_date(self, start_date, forecast_type):
    if forecast_type == document_service_pb2.GmiArguments.kShort:
      return start_date + timedelta(hours=36)
    if forecast_type == document_service_pb2.GmiArguments.kMedium:
      return start_date + timedelta(days=3)
    if forecast_type == document_service_pb2.GmiArguments.kLong:
      return start_date + timedelta(days=7)
    return start_date
    

  # 
  # Заполнение документа
  # 
  def fillDocument(self):
    
    
    libpuansonwrap.loadPunch("is")
    
    self.fillUserData()   
    
    for i,delay in enumerate(self.getDelay()):
      
      timestamp = self.arg_date + timedelta(hours=delay)
      
      # проходимся по всем датам и заполняем зонды
      if not self.zond_from_src(timestamp,i) and not self.zond_from_field(timestamp,i):
        self.hasData        = False
        self.result.comment = u'Данных нет'
      else:
        self.hasData       = True
        self.result.result = True 
      
      # пробуем заполнить данные по станции из наблюдений
      if not self.surface_from_src(timestamp,i) :
        self.hasData        = False
        self.result.comment = u'Данных нет'
      else:
        self.hasData       = True
        self.result.result = True  

    self.fillHeader()
    self.fillAuthorInfo()

    
  
  # 
  # Пробуем получить погоду по станции 
  # и заполнить таблицу метеосводки
  # @dt - дата
  # @offset - смещение в таблице (какой по счету срок заполняем)
  #  
  def surface_from_src(self, dt, offset):
    # получение данных
    self.hasData, surf_data = self.loadGmiData(date=dt)
    
    # если данные есть
    if self.hasData and surf_data and len(surf_data)>0:
      pass
    else:
      return False

    # заполняем таблицу с данными наблюдений
    self.fillGmiData(surf_data,dt,offset)
    
    return True
 
      

  # 
  # Заполняем таблицу с аэрологией
  # @dt - дата
  # @offset - смещение в таблице (какой по счету срок заполняем)
  # 
  def zond_from_src(self, dt, offset):
    zond    = Zond()
    # делаем запрос на получение данных от сервиса
    result, aerodata = self.getAeroData(date=dt)
    # если получен ответ и есть данные
    if result is True and len(aerodata)>0:
      data = aerodata[0].meteodata
      zond.fromArrayObject(data)
      zond.preobr()
      # заполняем документ
      self.fillHeightData(zond, dt, offset)
      self.fillFlowData(zond, dt, offset)
      # информация в таблице
      # zond_attention = self.body.get_variable_set('zond_attention')
      # zond_attention.set_text('')
    else:
      return False
    return True

  # 
  # Забираем данные зондирования из поля
  # @dt - дата
  # @offset - смещение в таблице (какой по счету срок заполняем)
  # 
  def zond_from_field(self,dt,offset):
    zond = Zond()
    result, aero_data = self.getAeroFieldData(date=dt)
    if result is True and len(aero_data)>0:
      
      data = aero_data[0].meteodata
      zond.fromArrayObject(data)
      zond.preobr()
      # заполняем таблицу с аэрологией
      self.fillHeightData(zond,dt,offset)
      self.fillFlowData(zond,dt,offset)
      
    else:
      return False
    return True


  # 
  # Заполняем данные ГМИ
  # 
  def fillGmiData(self, surf_data, dt, offset):
    tableRowIndexes = {  20010: 3,  # общая облачность
                         11001: 9,  # Направление ветра
                         11002: 10, # Скорость ветра
                         12111: 11, # Максимальная температура воздуха у земли
                         12112: 12, # Минимальная температура воздуха
                         10004: 13  # Атмосферное давление на уровне станции
                       }

    body     = self.doc.get_body()
    gmiTable = body.get_table(name="tableGmiParams")

    column = 1 + offset

    self.setCell(table_=gmiTable, row_=0, column_=column, data_=dt.strftime('%d.%m.%Y %H:%M') )

    descrNH = None
    descrC  = None
    descrH  = None

    for mdescr in surf_data:
      row   = tableRowIndexes[mdescr]
      value = surf_data[mdescr]
      self.setCell(table_=gmiTable, row_=row, column_=column, data_=value)

    column += 1



  # 
  # получаем сроки прогнозов
  # 
  def getDelay(self):
    if self.docType == document_service_pb2.GmiArguments.kShort:
      return [ 12, 24, 36 ]
    if self.docType == document_service_pb2.GmiArguments.kMedium:
      return [ 24, 48, 72 ]
    return [ 24, 48, 72, 96, 120, 144, 168 ]

  # 
  # заполняем заголовок
  # 
  def fillHeader(self):
    taskId = '0'
    
    body       = self.doc.get_body()
    titleTable = body.get_table(name='tableTitle')
    
    self.setCell(titleTable, 0, 1, self.docNumber)
    self.setCell(titleTable, 1, 1, self.station_name)
    self.setCell(titleTable, 2, 1, self.getStationNumber(self.station.station))
    self.setCell(titleTable, 3, 1, str(self.alt))
    self.setCell(titleTable, 4, 1, '{}, {}'.format( self.getLatStr(), self.getLonStr() ))
    
    self.setCell(titleTable, 5, 1, '{} часов'.format( self.getDelay()[-1] ))

  # 
  # Запрашиваем прогностические значения
  # 
  def loadGmiData(self, date):
    resp = dict()
    
    for descr in self.descriptors.keys():
      request                = field_pb2.DataRequest()
      request.forecast_start = str(date)
      request.forecast_end   = str(date)
      request.meteo_descr.append(descr)
      request.type_level.extend([1, 2])
      coord    = request.coords.add()
      coord.fi = self.lat_radian
      coord.la = self.lon_radian
      response = meteoglobal.RemoteCall(services_pb2.kField,
                                        'field_pb2@FieldService.GetForecastValues', request,
                                        500000)
      if response is None:
        return

      for data in response.data:
        mdescr = data.fdesc.meteodescr
        value  = data.value
        value  = libpuansonwrap.stringFromRuleValue(self.descriptors[mdescr], value)

        resp[mdescr] = value
      
    return True, resp


  # 
  # Заполняем данные по высотам на основе зонда
  # @dt - дата
  # @offset - смещение в таблице (какой по счету срок заполняем)
  # 
  def fillHeightData(self, zond, dt, offset):
    body        = self.doc.get_body()
    levelTable  = body.get_table(name="levelValues")
    
    columnIndex = 2 + 3*offset
    self.setCell(table_=levelTable, column_=columnIndex, row_=0, data_=dt.strftime('%d.%m.%Y %H:%M'))
        
    rowindex = 3
    for plevel in [ 925, 850, 700, 500, 400, 300, 200, 150, 100, 70, 50, 30, 20, 10 ]:
      ur = Uroven()
      zond.getUrPoP(plevel, ur)

      t = libpuansonwrap.stringFromRuleValue('T', ur.value(ValueType.UR_T) )
      if ur.isGood(ValueType.UR_dd):
        dd = libpuansonwrap.stringFromRuleValue('dd', ur.value(ValueType.UR_dd) )
      else:
        dd = '-'
      if ur.isGood(ValueType.UR_ff):
        ff = libpuansonwrap.stringFromRuleValue('ff', ur.value(ValueType.UR_ff) )
      else:
        ff = '-'  
          
      self.setCell(table_=levelTable, column_=columnIndex,     row_=rowindex, data_=dd)
      self.setCell(table_=levelTable, column_=columnIndex + 1, row_=rowindex, data_=ff)
      self.setCell(table_=levelTable, column_=columnIndex + 2, row_=rowindex, data_=t)
      rowindex += 1


  # 
  # Заполняем остальные данные таблицы
  # @dt - дата
  # @offset - смещение в таблице (какой по счету срок заполняем)
  # 
  def fillFlowData(self, zond, dt, offset):
    tableData = []
    levels    = {850: '1,5', 700: '3,0', 500: '5,5', 400: '7,0', 300: '9,0', 200: '12,0', 150: '14,0', 100: '16,0'}
    plevels   = levels.keys()
    plevels   = sorted(plevels) 

    for plevel in  plevels:
      h      = levels[plevel]
      data   = []
      isFlow = False

      if zond is None or isinstance(zond,dict):
        data.append([dt, h])
        continue

      ur = Uroven()
      zond.getUrPoP(plevel, ur)
      ff = ur.value(ValueType.UR_ff)
      dd = ur.value(ValueType.UR_dd)

      if ff >= 36.0:
        isFlow = True
      ffString = libpuansonwrap.stringFromRuleValue('ff', ff)
      ddString = libpuansonwrap.stringFromRuleValue('dd', dd)
      data.append([ dt, h, ffString, ddString])

      if isFlow:
        tableData.append(data)

    body       = self.doc.get_body()
    levelTable = body.get_table(name="strFlow")
    
    while levelTable is not None and 0 != len(tableData):
      clone = levelTable.clone()
      data  = tableData.pop(0)

      for col in range(0, len(data)):
        values = data[col]
        for row in range(0, len(values)):
          self.setCell( levelTable, row, col + 1, values[row] )

      if len(tableData) != 0:
        index     = body.index(levelTable)
        body.insert(element=paragraph.odf_create_paragraph('\n'), position=(index + 1))
        # logging.error(clone)
        body.insert(element=clone, position=index + 2)
        levelTable = clone
      else:
        levelTable = None
  
  # 
  # Заполняем таблицу с автором
  # 
  def fillAuthorInfo(self):
    currentDT   = datetime.now()
    currentDT   = currentDT.replace(microsecond=0)
    body        = self.doc.get_body()
    authorTable = body.get_table(name='tableFooter')

    self.setCell(table_=authorTable, column_=1, row_=0, data_=self.department)
    self.setCell(table_=authorTable, column_=1, row_=1, data_=self.user)
    self.setCell(table_=authorTable, column_=1, row_=2, data_=currentDT.strftime('%d-%m-%Y %H:%M'))


  # 
  # Устанавливаем в таблице данные
  # 
  def setCell(self,table_, row_, column_, data_):
    cell = table_.get_cell(coord=(column_, row_))        
    cell.set_value(meteoglobal.trUtf8(data_))
    table_.set_cell(coord=(column_, row_), cell=cell)



      

# 
# ====================== MAIN FUNCTION ============================
# 


def main(argv):
  # Qt-приложение
  app = QCoreApplication(sys.argv)

  parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter,
                                   description="Сценарий формирования метеосводок")
  parser.add_argument('-m', '--mode', default='proto', help="Режим запуска сценария. "
                                                            "arg - Задание аргументов запуска "
                                                            "proto - Передача прото-структуры в стандартный поток")

  parser.add_argument('-n', '--number', help='Номер документа', default='-')
  parser.add_argument('-s', '--station', help='Индекс станции для формирования', type=int)
  parser.add_argument('-d', '--date', help='Дата и время начала формирования документа YYYY-MM-DDThh:mm:ssZ')
  parser.add_argument('-t', '--type', help='тип документа', choices=['short', 'medium', 'long'])
  parser.add_argument('-a', '--author', help='автор документа')

  options = parser.parse_args()
  
  # sa     = StartArguments()
  # sa.ParseFromString(meteoglobal.readStdinBytes())
  # logging.error(sa)
  
  generator = ShortGMIDoc()
  init      = False
  
  # парсим данные из файла или протки
  if str(options.mode) == "arg":
    init = generator.init(options)
  elif str(options.mode) == "proto":
    sa     = GmiArguments()
    sa.ParseFromString(meteoglobal.readStdinBytes())
    # logging.error(sa)
    init   = generator.initFromProto(sa)
  if not init:
    result = generator.resultProto()
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(1)

 
  
  # корректируем шаблон документа
  if generator.docType == document_service_pb2.GmiArguments.kLong:
    generator.template_path = generator.template_long_path
    
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
