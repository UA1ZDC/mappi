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
import logging

import libpuansonwrap

from meteo_controller import *

import libmeteodatawrap
from libmeteodatawrap import *
from libglobalwrap import *

#модули для работы с документами
from lpod3 import document
from lpod3.style import odf_create_style, odf_style,odf_create_table_cell_style
from lpod3.table import odf_create_cell, odf_create_row

from document_service_pb2 import *
import argparse

import meteoglobal
import surface_pb2
from surface_service_pb2 import *
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
    self.rows_on_page      = 21
    
    
    # количество и название столбцов в таблице
    self.columns           = [ 'N', 'C', 'h', 'w', 'V', 'dd', 'ff', 'T', 'U' ]
    self.table_col_names   = [ 'name', 'time' ] + self.columns

    #  дескрипторы, по которым производится слияние через /
    self.merge_cloud_descr = ['N', 'C', 'h']
    # дескрипторы, которые переводим в названия колонок
    self.transform_descriptors = { 
        'C' : ['CN','CL','CM','CH'],
        'N' : ['Nh'],
        'w' : ['w_w_','w_tr'],
        'V' : ['VV'],
        'P0' : ['PQNH','P'],
      }
      
    self.station_type      = 0
    self.template_path     = "/share/meteo/odpsamples/single_fact_airports_table.odp"
    
    

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
    if (None == proto.datetime) or ( proto.regiongroup_id is None ):
      self.result.result = False
      self.result.comment = u"Не заданы требуемые параметры (datetime, number, stations )"
      return False
    
    self.arg_date        = datetime.datetime.strptime(proto.datetime, "%Y-%m-%dT%H:%M:%S")
    self.region_group_id = proto.regiongroup_id
    
    # путь к документам
    if proto.filepath:
      self.filepath = proto.filepath
    else:
      self.filepath = PRJ_DIR + '/var/meteo/documents/'
        
    if proto.user_name:
      self.user = proto.user_name
    if proto.user_rank:
      self.rank = proto.user_rank
    if proto.department:
      self.department = proto.department
    
      
    self.initProtoStations(proto)
    
    if proto.map:
      self.map_from_proto = proto.map
      
    self.printFileToStdOut = True
      
    return True



  # 
  # Заполнение документа
  # 
  def fillDocument(self):
    
    # загружаем регионы
    # self.getRegionGroups()
    # получаем фактическую погоду
    req            = surface_pb2.DataRequest()
    req.as_proto   = True
    req.query_type = surface_pb2.kLastValue
    req.level_p    = 0       #приземные данные
    req.type_level = 1       #приземные данные
    req.meteotablo = True
    req.punchid    = 'is'
    req.date_start = self.arg_date.strftime("%Y-%m-%dT00:00:00")
    req.date_end   = self.arg_date.strftime("%Y-%m-%dT23:59:59")
    req.country    = 164
    req.station_type.append( 14 )
    
    status,pogoda = self.getMeteoDataByDay(req)
    
    # инициализация пуансона из файла шаблона
    libpuansonwrap.loadPunch(str("is"))

    params = []
    for _ in pogoda:
      # получаем подготовленные для отображения колонки
      params_by_columns = self.prepare_params_for_tablo( _.param, self.columns, 14 )

      params.append({
        'name': _.station_info.name_rus,
        'time': datetime.datetime.strptime(_.dt, "%Y-%m-%dT%H:%M:%S").strftime("%H:%M"),
        **params_by_columns
      })

    params=sorted( params, key=lambda x: x['name'] )
    # получаем тело документа
    body          = self.doc.get_body()
    # заголовок салйда
    title         = body.get_frame(name='slide_title_new')
    # подзаголовок слайда
    title_context = body.get_frame(name='context_title')
    # таблица с прогнозами
    table         = body.get_frame(name='table_fact')
    table         = body.get_table()
    
    title.set_text_content("Доклад метеорологической обстановки") 
    title_context.set_text_content( "Фактическая погода за {}".format( self.arg_date.strftime("%d.%m.%Yг.") ) ) 
    
    
    # odf_create_style
    style = odf_create_style('paragraph', 'table_style1', area='text',
                **{'style:name'   : 'Roboto5',
                   'fo:color'     : '#34495e',
                   'fo:text-align': 'center',
                   'fo:font-size' : '8pt',
                   'fo:font-name' : 'Roboto Condensed' })
    style.set_properties({'fo:margin-top'   : "0cm",
                          'fo:margin-bottom': "0cm",
                          'fo:line-height'  : "100%",
                          'fo:text-align'   : "center" }, area="paragraph" )
       
    name = self.doc.insert_style(style=style, automatic=True)
    
    
    smu_style = odf_create_style('table-cell', 'cell-ump', area='graphic',
                **{'draw:fill-color':"#ffd9d5"})
    smu_style.set_properties({'fo:border': "0.03pt solid #000000" }, area="paragraph" )
    smu_style = self.doc.insert_style(style=smu_style, automatic=True)
    
    smu_style = odf_create_style('table-cell', 'cell-smu', area='graphic',
                **{'draw:fill-color':"#b3defb",
                   'fo:border': "0.03pt solid #000000" })
    smu_style.set_properties({'fo:border': "0.03pt solid #000000" }, area="paragraph" )
    smu_style = self.doc.insert_style(style=smu_style, automatic=True)
    
    smu_style = odf_create_style('table-cell', 'cell-pmu', area='graphic',
                **{'draw:fill-color':"#c0ffdb",
                   'fo:border': "0.03pt solid #000000" })
    smu_style.set_properties({'fo:border': "0.03pt solid #000000" }, area="paragraph" )
    smu_style = self.doc.insert_style(style=smu_style, automatic=True)
    
    # клонируем первую страницу, чтобы потом из нее делать остальные страницы
    first_page   = body.get_draw_page().clone()
    # получаем текущую страницу
    current_page = body.get_draw_page()
    
    
    page = 0
    k    = 0
    # проходимся по параметрам и заполняем ячейки
    if params is not False and params is not None:
      for _ in params:
        # если вышли за рамки текущей страницы
        # надо создать новую и в ней продолжить
        if k > self.rows_on_page * ( page+1 ):
          body.append(first_page.clone())
          current_page = body.get_draw_pages()[-1]
          table.set_style("table_style1")
          table        = current_page.get_table()
          page+=1
          
        row = odf_create_row()
        i=0
        
        rowstyle = self.get_rowstyle(_)
        
        # добавляем ячейки в строку
        for p in self.table_col_names:
          # проверяем, есть ли такой параметр
          value = ""
          if p in _ and  _[p] is not None:
            value = _[p]
          cell = odf_create_cell( value=value )
          # добавляем стили к тектовому параграфу в ячейке
          cell.get_element('//text:p').set_style(name)
          cell.set_style("cell-"+rowstyle)
          row.append_cell(cell)
          i+=1
        
        
        row.set_style(rowstyle)
        # row.set_background(color="#005599")
        
        table.append_row(row)
        k+=1
    table.set_style("table_style1")
    
    return

  # назначаем стиль
  def get_rowstyle(self,row):
    h = self.safeCast(row['h'],int,0)
    V = self.safeCast(row['V'],int,0)
    if V < 20:
      V = V*1000
    if (h>0 and h<=200) or (V>0 and V<=2000) :
      return "ump"
    n_count = row['N'].split('/')    
    if (len(n_count)>0 and  self.safeCast(n_count[0],int,0)>=7) or (self.safeCast(row['N'],int,0)>=7) or (V>0 and V<=4000):
      return "smu"
    return "pmu"


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
