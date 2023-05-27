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
from lpod3.style import odf_create_style, odf_style,odf_create_table_cell_style
from lpod3.table import odf_create_cell, odf_create_row

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

class ODPTable(DocumentFormal):
  
  def __init__(self):
    super().__init__()
    
    self.service_code      = services_pb2.kSrcData
    
    self.hasData           = False
    self.result            = ExportResponse()
    self.result.result     = False
    self.text              = ''
    
    # количество добавляемых строк в таблице на одной странице
    # если количество строк превышает данное значение, то добавляется новая страница
    self.rows_on_page      = 12
    
    # количество и название столбцов в таблице
    self.table_col_names   = [ 'title', 'C', 'W', 'V', 'dd', 'ff', 'Tmax', 'Tmin', 'Twater', 'Sea', 'danger' ]
    
    
    
    self.station_type      = 0
    self.template_path     = "/share/meteo/odpsamples/single_forecast_regions_template.odp"
    
    

  # 
  # инициализация с параметрами
  # 
  def init(self, options):
    if (None == options.datetime) or (None == options.index) \
        or (None == options.filepath) or (None == options.number):
      self.result.result = False
      self.result.comment = u"Не заданы требуемые параметры"
      # return False
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
    
    # получаем тело документа
    body          = self.doc.get_body()
    # заголовок салйда
    title         = body.get_frame(name='slide_title_new')
    # подзаголовок слайда
    title_context = body.get_frame(name='context_title')
    # таблица с прогнозами
    table         = body.get_frame(name='table_forecast_day')
    table         = body.get_table()
    
    title.set_text_content("Доклад метеорологическеой обстановки") 
    title_context.set_text_content("IV. Восточный военный округ") 
    
    params = [{
      'title' : "Забайкальский край",
      'C'     : "переменная облачность",
      'W'     : "местами снег",
      'V'     : "6-10, в снеге 2-4",
      'dd'    : "С, СЗ",
      'ff'    : "5-10",
      'Tmax'  : "-17...-20",
      'Tmin'  : "-25...-28",
      'Twater': "",
      'danger': "Чукотский полуостров",
      'Sea'   : "",
    }]
    
    # odf_create_style
    style = odf_create_style('paragraph', 'table_style1', area='text',
                **{'style:name'   : 'Roboto5',
                   'fo:color'     : '#000000',
                   'fo:text-align': 'center',
                   'fo:font-size' : '8pt',
                   'fo:font-name' : 'Roboto Condensed' })
    style.set_properties({'fo:margin-top'   : "0cm",
                          'fo:margin-bottom': "0cm",
                          'fo:line-height'  : "100%",
                          'fo:text-align'   : "center" }, area="paragraph" )
    name = self.doc.insert_style(style=style, automatic=True)
    
    
    # клонируем первую страницу, чтобы потом из нее делать остальные страницы
    first_page   = body.get_draw_page().clone()
    # получаем текущую страницу
    current_page = body.get_draw_page()
    
    
    page = 0
    # проходимся по параметрам и заполняем ячейки
    for _ in params:
      # если вышли за рамки текущей страницы
      # надо создать новую и в ней продолжить
      if k > self.rows_on_page * ( page+1 ):
        body.append(first_page)
        current_page = body.get_draw_pages()[-1]
        table = current_page.get_table()
        page+=1
        
      row = odf_create_row()
      i=0
      # добавляем ячейки в строку
      for p in self.table_col_names:
        # проверяем, есть ли такой параметр
        if _[p] is not None:
          cell = odf_create_cell( value=_[p] )
          # добавляем стили к тектовому параграфу в ячейке
          cell.get_element('//text:p').set_style(name)
          row.append_cell(cell)
          i+=1
      table.append_row(row)
    
    
    table.set_style("style1")
    print(table.get_height())

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

  generator = ODPTable()
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
  
  # устанавливаем выходной формат
  generator.setODPFormat()
  # генерим из параметров
  generator.generateDocument()
  # получаем готовую протку
  result = generator.resultProto()
  
  if True == result.result:
    # sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(0)
  else:
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(1)

if __name__ == "__main__":
  main(argv)
