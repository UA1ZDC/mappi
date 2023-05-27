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

from meteo_controller import *

import libpuansonwrap
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
    self.rows_on_page      = 20
    
    # количество и название столбцов в таблице
    self.columns           = []
    self.table_cols        = []
    self.table_col_names   = []

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
    # наборы подготовленных стилей
    self.styles = {}
    # индикация (название и настройки)
    self.indication_name = ""
    self.indication = {}
    
    self.operator = {
      '>' : lambda x, y: x>y,
      '>=': lambda x, y: x>=y,
      '<' : lambda x, y: x<y,
      '<=': lambda x, y: x<=y,
      '=' : lambda x, y: x==y,
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
    
    # заполняем колонки с данными
    self.columns = []
    for _ in proto.columns:
        if _.visible:
            if _.param not in ['name', 'time']:
                self.columns.append( _.param )
            self.table_cols.append( _.param )
            self.table_col_names.append( _.title )
        
    self.stations = {}
    if proto.selected_stations:
      for _ in proto.selected_stations:
        cobj_ = {
            'include': [],
            'exclude': [],
            'sttype' : [],
            'full'   : False,
          }
        if _.include:
          cobj_['include'] = _.include
        if _.exclude:
          cobj_['exclude'] = _.exclude
        if _.sttype:
          cobj_['sttype'] = _.sttype
        if _.full:
          cobj_['full'] = _.full
        self.stations.update( { _.country: cobj_ } )
        
    self.initProtoStations(proto)
    
    # добавляем индикацию
    if proto.indication:
      # logging.error(proto.indication)
      for _ in proto.indication:
        # logging.error(_)
        self.indication[_.preset_name] = _.preset_settings
    # logging.error(self.indication)
    if proto.map:
      self.map_from_proto = proto.map
      
    self.printFileToStdOut = True
      
    return True


  # 
  # Получаем данные фактической погоды по станциям 
  # @dt - дата
  # @td_hour - часы (срок)
  # @stations - объект запроса станций по странам
  # 
  def get_meteodata( self, dt, dt_hour, stations ):
    response = False
    exclude_stations = []
    
    #
    # Инициализируем запрос каждый раз заново
    #
    def init_req( dt_start, dt_end ):
      req            = surface_pb2.DataRequest()
      req.as_proto   = True
      req.query_type = surface_pb2.kLastValue
      req.level_p    = 0       #приземные данные
      req.type_level = 1       #приземные данные
      req.meteotablo = True
      req.punchid    = 'is'
      req.date_start = dt_start
      req.date_end   = dt_end
      return req


    # функция отправки запроса
    # чтобы можно было отправлять несколько запросов
    def station_request(_req):
      # try:
      response = meteoglobal.RemoteCall(  servicecode    = services_pb2.kSrcData,
                                          methodfullname = 'surface_service_pb2@SurfaceService.GetMeteoDataByDay',
                                          request        = _req,
                                          timeout_msecs  = 180000 )
      # logging.error(response)
      # except:
      #   response=False
      return response

    if dt!='':
      if dt_hour!='':
        date_start,date_end = self.get_date_time( dt, dt_hour, onehour=True )
      else:
        date_start,date_end = self.get_date_time( dt )
    else:
      date_start,date_end = self.get_date_time(False)

    # logging.error(stations)
    
    # проверяем заданы ли станции в новом формате
    if stations and len(stations.keys())>0:
      for country, item in stations.items():
        req = init_req( date_start, date_end )

        if 'exclude' in item:
          exclude_stations = exclude_stations + item['exclude']
        
        # если полностью страну - добавляем
        if item['full'] is True:
          req.country = int(country)
          for sttype in item['sttype']:
            req.station_type.append( int(sttype) )
        else:
          for station in item['include']:
            st_and_type_ = station.split('_')
            if len(st_and_type_)>1:
              req.station.append(st_and_type_[1])
              req.station_type.append( int(st_and_type_[0]) )
            else:
              req.station.append(station)
              for sttype in item['sttype']:
                req.station_type.append( int(sttype) )

        # отправляем запрос
        _response = False
        if item['full'] is True or len(item['include'])>0:
          _response = station_request(req)

        # пытаемся слить прошлые запросы с текущим
        if response is not False and response is not None and \
          _response is not False and _response is not None:
          response.meteodata_proto.extend(_response.meteodata_proto)
        else:
          response = _response
    
    if None == response or response is False :
      return False
    
    # пробуем удалить станцию из списка, чтобы не показывать ее
    try:
      if len(exclude_stations)>0:
        for i in range(len(response.meteodata_proto)-1,-1,-1 ):
          st = response.meteodata_proto[i]
          st_item = str(st.station_info.station_type)+'_'+st.station_info.cccc
          if st_item in exclude_stations:
            del response.meteodata_proto[i]
    except:
      logging.error("Не могу удалить станцию из списка")
      
    return response

  # 
  # возвращаем дату и время за которые нужны данные
  # 
  # @mdate - дата за которую нужны данные
  #      в формате 2018-12-31, если дата не указана - берется текущее время
  # @hour - если указано, то возвращаем данные за указанный час суток
  # @minute - если указано, то возвращаем за указанный час, и минуты 
  # @onehour - если True, то разница в выбираемых сроках = 1 часу, если нет - то 12 часов
  # 
  def get_date_time( self, mdate, hour=False, minute=False, onehour=False):
    date_start = False
    date_end   = False

    # разница в часах, на которую смотрим
    hour_delta = 1 if onehour is True else 12

    if mdate is False:
      sdate = datetime.datetime.now()
    else:
      sdate = datetime.datetime.strptime(mdate, '%Y-%m-%d' )

    # если не установлен час - тогда за весь день
    if minute==False:
      if hour == False:
        date_start =  sdate.strftime('%Y-%m-%dT00:00:00') 
        date_end =  sdate.strftime('%Y-%m-%dT23:59:59')   
      else:
        sdate=sdate.replace(hour= int(hour) )
        date_start =  (sdate - timedelta(hours=hour_delta)).strftime('%Y-%m-%dT%H:00:00') 
        # а это показываем погоду за крайний час
        # date_start =  sdate.strftime('%Y-%m-%dT'+str(hour)+':00:00')   
        date_end =  sdate.strftime('%Y-%m-%dT'+str(hour)+':00:00')
        # это погода за ча запрашивалась
        # date_end =  sdate.strftime('%Y-%m-%dT'+str(hour)+':59:59')   
    else:
      date_start = sdate.strftime('%Y-%m-%dT'+str(hour)+':'+str(minute)+':00')
      date_end   = sdate.strftime('%Y-%m-%dT'+str(hour)+':'+str(minute)+':00')
    return (date_start,date_end)


  # 
  # Заполнение документа
  # 
  def fillDocument(self):
    
    pogoda = self.get_meteodata( self.arg_date.strftime("%Y-%m-%d"), '', self.stations )
    
    # получаем фактическую погоду
    # req            = surface_pb2.DataRequest()
    # req.as_proto   = True
    # req.query_type = surface_pb2.kLastValue
    # req.level_p    = 0       #приземные данные
    # req.type_level = 1       #приземные данные
    # req.meteotablo = True
    # req.punchid    = 'is'
    # req.date_start = self.arg_date.strftime("%Y-%m-%dT00:00:00")
    # req.date_end   = self.arg_date.strftime("%Y-%m-%dT23:59:59")
    # req.country    = 164
    # req.station_type.append( 0 )
    
    # status,pogoda = self.getMeteoDataByDay(req)
    
    # инициализация пуансона из файла шаблона
    libpuansonwrap.loadPunch(str("is"))

    params = []
    param_values = []
    i = 0
    # logging.error(pogoda.meteodata_proto)
    if pogoda:
      for _ in pogoda.meteodata_proto:
        # получаем подготовленные для отображения колонки
        # if i==0:
        #   logging.error( _ )
        #   i=1
        params_by_columns, indexed_ = self.prepare_params_for_tablo( _.param, self.columns, 0 )

        params.append({
          'name': _.station_info.name_rus,
          'time': datetime.datetime.strptime(_.dt, "%Y-%m-%dT%H:%M:%S").strftime("%H:%M"),
          **params_by_columns
        })
        param_values.append({
          'name': {'str':_.station_info.name_rus},
          'time': {'str':datetime.datetime.strptime(_.dt, "%Y-%m-%dT%H:%M:%S").strftime("%H:%M")},
          **indexed_
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
                **{'style:name'   : 'Arial',
                   'fo:color'     : '#34495e',
                   'fo:text-align': 'center',
                   'fo:font-size' : '9pt',
                   'fo:font-name' : 'Arial' })
    style.set_properties({'fo:margin-top'   : "0cm",
                          'fo:margin-bottom': "0cm",
                          'fo:line-height'  : "100%",
                          'fo:text-align'   : "center" }, area="paragraph" )
       
    self.styles['name'] = self.doc.insert_style(style=style, automatic=True)
    
    header_style = odf_create_style('table-cell', 'cell-header', area='graphic',
                **{'draw:fill-color':"#FFFFFF", 'draw:fill':'solid'})
    header_style.set_properties({
                   'fo:border': "0.03pt solid #777777",
                   'fo:color'        : '#34495e',
                   'fo:text-align'   : 'center',
                   'fo:font-size'    : '8pt',
                   'fo:margin-left'  : '0cm',
                   'fo:margin-right' : '0cm',
                   'fo:margin-top'   : '0cm',
                   'fo:margin-bottom': '0cm',
                   'fo:line-height'  : '100%',
                   'fo:text-align'   : 'center',
                   'fo:text-indent'  : '0cm',
                   'fo:font-name'    : 'Roboto Condensed' },   area="paragraph" )
    self.styles['header_style'] = self.doc.insert_style(style=header_style, automatic=True)
    
    c1 = odf_create_style('table-column', 'c1', area="table-column", 
                            **{ 'style:column-width':'1.897cm','style:use-optimal-column-width':'false' } )
    self.styles['c1'] = self.doc.insert_style(style=c1, automatic=True)
    
    smu_style = odf_create_style('table-cell', 'cell-ump', area='graphic',
                **{'draw:fill-color':"#ffd9d5",'draw:fill':'solid','fo:border': "0.03pt solid #000000"})
    smu_style.set_properties({'fo:border': "0.03pt solid #000000" }, area="paragraph" )
    self.styles['ump_style'] = self.doc.insert_style(style=smu_style, automatic=True)
    
    smu_style = odf_create_style('table-cell', 'cell-smu', area='graphic',
                **{'draw:fill-color':"#b3defb",
                   'draw:fill':'solid',
                   'fo:border': "0.03pt solid #000000" })
    smu_style.set_properties({'fo:border': "0.03pt solid #000000" }, area="paragraph" )
    self.styles['smu_style'] = self.doc.insert_style(style=smu_style, automatic=True)
    
    smu_style = odf_create_style('table-cell', 'cell-pmu', area='graphic',
                **{'draw:fill-color':"#c0ffdb",
                    'draw:fill':'solid',
                   'fo:border': "0.03pt solid #000000" })
    smu_style.set_properties({'fo:border': "0.03pt solid #000000" }, area="paragraph" )
    # smu_style = self.doc.insert_style(style=smu_style, automatic=True)
    self.styles['pmu_style'] = self.doc.insert_style(style=smu_style, automatic=True)
    # клонируем первую страницу, чтобы потом из нее делать остальные страницы
    first_page   = body.get_draw_page().clone()
    # получаем текущую страницу
    current_page = body.get_draw_page()
    
    self.table_create(table)
    
    page = 0
    k    = 0
    # проходимся по параметрам и заполняем ячейки
    if param_values is not False and param_values is not None:
      for _ in param_values:
        # если вышли за рамки текущей страницы
        # надо создать новую и в ней продолжить
        if k > self.rows_on_page * ( page+1 ):
          body.append(first_page.clone())
          current_page = body.get_draw_pages()[-1]
          table.set_style("table_style1")
          table        = current_page.get_table()
          self.table_create(table)
          page+=1
          
        row = odf_create_row()
        i   = 0
        
        rowstyle = self.get_rowstyle( param_values[k] )
        
        # добавляем ячейки в строку
        for p in self.table_cols:
          # проверяем, есть ли такой параметр
          value = ""
          if p in _ and  _[p] is not None:
            if 'str' in _[p]:
              value = _[p]['str']
          cell = odf_create_cell( value=value )
          # добавляем стили к тектовому параграфу в ячейке
          cell.get_element('//text:p').set_style(self.styles['name'])
          cell.set_style("cell-"+rowstyle)
          row.append_cell(cell)
          i+=1
        
        
        row.set_style(rowstyle)
        # row.set_background(color="#005599")
        
        table.append_row(row)
        k+=1
    table.set_style("table_style1")
    
    return


  def table_create(self,table):
    # делаем первую строку в таблице
    table.clear()
    table.set_style("table_style1")
    row = odf_create_row()
    row.set_style_attribute('table:default-cell-style-name', 'header_style')
    c = 0
    for _ in  self.table_col_names:
      col_ = table.set_column(c)
      if c==0:
        col_.set_style(self.styles['c1'])
      cell = odf_create_cell( value=_ )
      # добавляем стили к тектовому параграфу в ячейке
      cell.get_element('//text:p').set_style(self.styles['header_style'])
      cell.set_style(self.styles['header_style'])
      row.append_cell(cell)
      c+=1    
    table.set_row( "A1", row )

  # назначаем стиль
  def get_rowstyle(self,row):
    
    # если индикация не выбрана - красим по стандартной индикации
    if len(self.indication.values())==0:
      h = self.safeCast(row['h']['val'],int,0)
      V = self.safeCast(row['V']['val'],int,0)
      if V < 20:
        V = V*1000
      if (h>0 and h<=200) or (V>0 and V<=2000) :
        return "ump"
      n_count = row['N']['str'].split('/')    
      if (len(n_count)>0 and  self.safeCast(n_count[0],int,0)>=7) or (self.safeCast(row['N']['str'],int,0)>=7) or (V>0 and V<=4000):
        return "smu"
      return "pmu"
    
    # проходимся по табличным колонкам
    for colname_ in self.columns:
      # проходимся по индикации
      for ind_ in self.indication.values():
        # logging.error(ind_)
        # сначала по УМП
        for u_ in ind_.ump:
          # если есть значение в индикации
          if u_.param == colname_:
            # сравниваем со значением в таблице
            if u_.operator in self.operator:
              if isinstance( row[colname_]['val'], list ):
                for v_ in row[colname_]['val']:
                  if v_ is not None and self.operator[u_.operator]( v_, self.safeCast(u_.value,int) ):
                    return 'ump'
              if row[colname_]['val'] is not None and self.operator[u_.operator]( row[colname_]['val'], self.safeCast(u_.value,int) ):
                return 'ump'

        # потом по СМУ
        for s_ in ind_.smu:
          # logging.error(s_)
          # logging.error(colname_)
          # если есть значение в индикации
          if s_.param == colname_:
            # сравниваем со значением в таблице
            if s_.operator in self.operator:
              if isinstance( row[colname_]['val'], list ):
                for v_ in row[colname_]['val']:
                  if v_ is not None and self.operator[s_.operator]( v_, self.safeCast(s_.value,int) ):
                    return 'smu'
              if row[colname_]['val'] is not None and self.operator[s_.operator]( row[colname_]['val'], self.safeCast(s_.value,int) ):
                return 'smu'
      
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
