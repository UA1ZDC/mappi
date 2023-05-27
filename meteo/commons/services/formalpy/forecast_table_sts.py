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
import json

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

import libpuansonwrap

import base64
import meteoglobal
import surface_pb2
from surface_service_pb2 import *
import field_pb2
import meteocalc
from libzondwrap import *
import services_pb2
import sprinf_pb2
import forecast_pb2

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
    self.table_col_names   = [ 'title', 'N', 'W', 'V', 'dd', 'ff', 'Tmax', 'Tmin', 'Twater', 'Sea', 'danger' ]
    
    # групповой айди
    self.region_group_id    = None
    # заголовок группы
    self.region_group_title = ""
    # станции/регионы в группе
    self.stations          = []
    self.regions           = []
    # скаченные прогнозы
    self.forecasts         = []
    self.gradacii          = {}
    
    self.station_type      = 0
    self.template_path     = "/share/meteo/odpsamples/single_forecast_regions_template.odp"
    
    

  # 
  # инициализация с параметрами
  # 
  def init(self, options):
    if (None == options.datetime) or (None == options.index) \
        or (None == options.filepath) or (None == options.number):
      self.result.result = False
      self.result.comment = u"Не заданы требуемые параметры (datetime)"
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
      
    self.printFileToStdOut = True
      
    return True



  # 
  # Заполнение документа
  # 
  def fillDocument(self):
    # загружаем пуансон
    libpuansonwrap.loadPunch('is')
    
    # загружаем регионы
    self.getRegionGroups()
    # получаем красивые прогнозы
    params = self.getRegionPrettyForecasts()
    # logging.error(params)
    # получаем тело документа
    body          = self.doc.get_body()
    # заголовок салйда
    title         = body.get_frame(name='slide_title_new')
    # подзаголовок слайда
    title_context = body.get_frame(name='context_title')
    # таблица с прогнозами
    table         = body.get_frame(name='table_forecast_day')
    table         = body.get_table()
    
    title.set_text_content("Доклад метеорологической обстановки") 
    title_context.set_text_content( self.region_group_title ) 
    
    # params = []
    # for _ in self.stations:
    #   params.append({
    #     'title' : _['title'],
    #     'C'     : "переменная облачность",
    #     'W'     : "местами снег",
    #     'V'     : "6-10, в снеге 2-4",
    #     'dd'    : "С, СЗ",
    #     'ff'    : "5-10",
    #     'Tmax'  : "-17...-20",
    #     'Tmin'  : "-25...-28",
    #     'Twater': "",
    #     'danger': "Чукотский полуостров",
    #     'Sea'   : "",
    #   })
    
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
    k    = 0
    # проходимся по параметрам и заполняем ячейки
    if params is not False and params is not None:
      for _ in params:
        # если вышли за рамки текущей страницы
        # надо создать новую и в ней продолжить
        if k > self.rows_on_page * ( page+1 ):
          body.append(first_page)
          current_page = body.get_draw_pages()[-1]
          table        = current_page.get_table()
          page+=1
          
        row = odf_create_row()
        i=0
        # добавляем ячейки в строку
        for p in self.table_col_names:
          # проверяем, есть ли такой параметр
          if p in _['pretty_params'] and  _['pretty_params'][p] is not None:
            cell = odf_create_cell( value=_['pretty_params'][p] )
            # добавляем стили к тектовому параграфу в ячейке
            cell.get_element('//text:p').set_style(name)
            row.append_cell(cell)
            i+=1
        table.append_row(row)
        k+=1
    table.set_style("style1")
    
    return


  # 
  # заполняем группы регионов
  # @self.regiongroup { regiongroup_id,  }
  # 
  def getRegionGroups(self):
    
    req  = sprinf_pb2.RegionGroupsRequest()

    if self.region_group_id is not None:
      req.id = self.region_group_id
    
    try:
      # logging.error(req)
      response = meteoglobal.RemoteCall( servicecode    = services_pb2.kSprinf,
                                        methodfullname = 'sprinf_pb2@SprinfService.GetRegionGroups',
                                        request        = req,
                                        timeout_msecs  = 10000 )
      # logging.error(response)
    except:
      response = False
    
    if response is None or response is False or response.data is None:
      return False
    
    if len(response.data)>0:
      self.stations = []
      self.region_group_title = response.data[0].title
      for _ in response.data[0].regions:
        coord = False
        if _.station_coord.lat_radian != 0.0 and _.station_coord.lon_radian != 0.0:
          coord = _.station_coord
        else:
          coord = _.region_coord
        # logging.error(coord)
        self.stations.append(
          {
            "station"   : _.station,
            "region_id" : _.region_id,
            "coord"     : coord,
            "title"     : _.title,
          }
        )
        
    # logging.error(response)
    
    return True

  # 
  # Заполняем массив с прогнозами 
  # 
  def getRegionPrettyForecasts(self):
    
    regions_id  = [ _['region_id']  for _ in self.stations if _['region_id'] is not None and _['region_id']!=0 ]
    stations = []
       
    req  = forecast_pb2.ForecastManualRequest()
    
    req.from_dt       = self.arg_date.strftime("%Y-%m-%dT00:00:00")
    req.to_dt         = self.arg_date.strftime("%Y-%m-%dT23:59:59")
    # запрашиваем прогнозы на 24 часа
    req.forecast_hour = 24
    
    if regions_id is not None:
      req.region_id.extend(regions_id)
    for _ in self.stations:
      logging.error(_['station'])
      st_ = req.station.add()
      st_.CopyFrom(_['station'])
      # req.station.extend(stations)
    
    # logging.error(req)
    response       = meteoglobal.RemoteCall( servicecode    = services_pb2.kForecastData,
                                     methodfullname = 'forecast_pb2@ForecastData.GetManualForecast',
                                     request        = req,
                                     timeout_msecs  = 30000 )
    
    if response is None or response is False or response.data is None:
      return False, False
    
    forecasts = []
    # те прогнозы, что уже есть
    already_exists_regions  = {}
    already_exists_stations = {}
    if len(response.data)>0:
      for _ in response.data:
        # если такой прогноз уже есть - смотри дату составления прогноза
        # и забираем уже последний
        # logging.error( "region_id: ======== "+str(_.region_id) +" == "+_.region_name + " -- "+",".join( map(str, already_exists_regions.keys() ) ) +":"+ str(_.region_id in already_exists_regions.keys()) )
        if _.region_id and _.region_id in already_exists_regions.keys():
          if _.dt <= already_exists_regions[_.region_id]:
            continue
          else:
            # удаляем предыдущий прогноз
            forecasts = [ f for f in forecasts if f['region_id'] != _.region_id ]
            already_exists_regions[_.region_id] = _.dt
            
        # logging.error( "======== "+str(_.station)+",".join( map(str, already_exists_stations.keys() ) ) +":"+ str(_.station in already_exists_stations.keys()) )
        if _.station and _.station in already_exists_stations.keys():
          if _.dt <= already_exists_stations[_.station]:
            continue
          else:
            # удаляем предыдущий прогноз
            forecasts = [ f for f in forecasts if f['station'] != _.station ]
            already_exists_stations[_.station] = _.dt
          
        # добавляем или заменяем в массиве существубющих прогнозоы
        if _.station !=0:
          already_exists_stations[_.station] = _.dt
        if _.region_id !=0:
          already_exists_regions[_.region_id] = _.dt
          
        item = {
          "region_id"      : _.region_id,
          "station"        : _.station,
          "title"          : (_.station_name if _.station_name!="" else _.region_name)+"",
          "forecast_text"  : _.forecast_text,
          "forecast_params": _.forecast_params,
          "pretty_params"  : {}
        }
        forecasts.append(self.params2Pretty(item,_.forecast_params))
    return forecasts

  # 
  # Превращаем числа в красивые прогнозы для таблички
  # 
  def params2Pretty(self,item,params):
        
    if "pretty_params" in item:
      decoded = json.loads( params )
      item['pretty_params'] = {
          'title'          : item['title'],
          'N'              : self.param2str( decoded,"N" ),
          'W'              : self.param2str( decoded,"w", False ),
          'V'              : self.param2str( decoded,"V" ),
          'dd'             : self.param2str( decoded,"dd" ),
          'ff'             : self.param2str( decoded,"ff" ),
          'Tmax'           : self.param2str( decoded,"T" ),
          'Tmin'           : self.param2str( decoded,"T" ),
          'Twater'         : "",
          'danger'         : "",
          'Sea'            : "",
        }
    return item

  # 
  # получаем градации для параметра
  # уровень 0, прогнозы у земли
  # 
  def getGradacii(self, descrname):
    req            = forecast_pb2.Gradacii()
    req.descrname  = descrname
    req.level      = 0
    req.type_level = 1
    
    response       = meteoglobal.RemoteCall( servicecode    = services_pb2.kForecastData,
                                     methodfullname = 'forecast_pb2@ForecastData.FillGradacii',
                                     request        = req,
                                     timeout_msecs  = 5000 )
    
    if response is None or response is False:
      return False, False

    gradacii = []
    delta    = False
    if response.dopusk_grad==1:
      delta = response.delta*2
    else:
      i=0
      for _ in response.gradacii_min:
        gradacii.append( [ _, response.gradacii_max[i] ] )      
        i+=1

    return gradacii, delta
    
    
  
  # ищет только первое вхождение
  # а если их несколько?! через запятую надо объединять
  def get_param(self,items,descrname):
    for _ in items:
      if _['descrname'] == descrname:
        return _['value']
    return ""
  
  # превращаем параметр в строку для таблицы
  # @single один или все параметры сливаем
  def param2str(self,items,descrname, single=True):
    params = []
    if single:
      params.append( self.get_param( items,descrname ) )
    else:
      params = [ _['value']  for _ in items if _['descrname'] == descrname ]
    
    if len(params)>0:
      str_params = []
      for x in params:
        str_params.append(self.val2str(x,descrname))
      return ", ".join( str_params )
      # return ", ".join( map( lambda x: self.val2str(x,descrname), params ) )
    return ""
  
  def val2str(self,x,descrname):
    # то, что не градуируем
    nogradations = ['w']
    if descrname in nogradations:
      # для явлений надо забирать названия явлений
      x = libpuansonwrap.stringFromRuleValueCode(descrname, x, str(x) )
      return "{}".format(x)
    # разделитель - точки ...
    # для остальных -
    dotsseparatoritems = [ 'T', 'Td' ]
    
    # выставляем разделитель
    separator = "-"
    if descrname in dotsseparatoritems:
      separator="..."
    
    # приводим значения к числу
    
    x  = self.safeCast(x,int,False)
    x2 = self.safeCast(x,float,False)
    if x is False:
      return ""
    
    # забираем градации для переменной, если есть
    cur_gradacii = False
    cur_delta    = False
    if descrname not in self.gradacii:
      self.gradacii[descrname] = self.getGradacii(descrname)
    
    cur_gradacii, cur_delta = self.gradacii[descrname]
    
    # logging.error([cur_gradacii, cur_delta])
    
    # если градаций нет в базе -в овзращаем как есть
    if (cur_gradacii is False or len(cur_gradacii)==0) and cur_delta is False:
      # по умолчанию
      cur_delta = 3
      # return "{}".format(x)
    # 
    if cur_gradacii is not False and len(cur_gradacii)>0:
      for _ in cur_gradacii:
        if _[0]==x:
          x2 = _[1]
    else:
      # в отрицательную сторону градации уменьшаются
      if x<0:
        x2 -= cur_delta
      else:
        x2 += cur_delta
    
    x2 = self.safeCast(x2,int,False)
    
    return "{}{}{}".format(x, separator, x2 )
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
  parser.add_argument('-i', '--index',    help='Индекс станции')
  parser.add_argument('-d', '--datetime', help='Дата и время измерений в формате YYYY-MM-DDThh:mm:ss')
  parser.add_argument('-n', '--number',   help='Условный номер станции')

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
    # logging.error(result)
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(0)
  else:
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(1)

if __name__ == "__main__":
  main(argv)
