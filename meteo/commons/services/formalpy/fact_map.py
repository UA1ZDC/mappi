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
import tempfile

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

import  document_service_pb2

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
import sprinf_pb2
import forecast_pb2
import meteo_pb2
import weather_pb2
import map_document_pb2

from PyQt5.QtCore import QCoreApplication

# ипортируем главный класс для наслоедования
from mapform import MapFormal

class ODPTable(MapFormal):
  
  def __init__(self):
    super().__init__()
    
    self.service_code      = services_pb2.kSrcData
    
    self.hasData           = False
    self.result            = document_service_pb2.ExportResponse()
    self.result.result     = False
    self.text              = ''
    
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
    
    # имя джоба, по которому генерятся карты
    self.job_name          = "Prizemnaya-Eurasia.job"
        
    self.station_type      = 0
    self.template_path     = "/share/meteo/odpsamples/single_forecast_map_template.odp"
    
    

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
    if proto.center:
      self.center          = proto.center
    
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
        
    # если не задан регион, то показываем просто карту
    if proto.map:
      self.map_from_proto = proto.map
      
    self.printFileToStdOut = True
      
    return True



  # 
  # Заполнение документа
  # 
  def fillDocument(self):
    
    # загружаем регионы
    self.getRegionGroups()
    
    # получаем красивые прогнозы
    # params = self.getRegionPrettyForecasts()
    
    
    # получаем тело документа
    body          = self.doc.get_body()
    # заголовок салйда
    title         = body.get_frame(name='slide_title_new')
    # подзаголовок слайда
    title_context = body.get_frame(name='context_title')
    # таблица с прогнозами
    forecastmap   = body.get_frame(name='forecast_map')
    # table         = body.get_table()
    
    title.set_text_content("Доклад метеорологической обстановки {}".format(self.region_group_title) )
    
    # клонируем первую страницу, чтобы потом из нее делать остальные страницы
    first_page   = body.get_draw_page().clone()
    # получаем текущую страницу
    current_page = body.get_draw_page()
    
    
    page = 0
    k    = 0
    
    # проходимся по параметрам и заполняем ячейки
    if self.stations is not False and self.stations is not None and len(self.stations)>0:
      for _ in self.stations:
        # если вышли за рамки текущей страницы
        # надо создать новую и в ней продолжить
        if k > 0:
          body.append(first_page.clone())
          current_page = body.get_draw_pages()[-1]
          # отрисовываем заголовок
          title_context = current_page.get_frame(name='context_title')
          
          page+=1
        
        
        title_context.set_text_content(     
            "Карта погоды по территории {} за {}".format(
                _['title'], 
                self.arg_date.strftime("%H:%M %d.%m.%Yг.")
            ) 
        ) 
        # подготавливаем карту
        self.createMap( self.arg_date.strftime("%Y-%m-%dT%H:%M:%S"), _['coord'] )
        
        self.saveImg(current_page,'forecast_map')
        
        k+=1   
    elif self.map_from_proto is not False:
      # 
      # Если заданы параметры с карты
      # 
      
      title_context.set_text_content(     
          "Карта погоды за {}".format(
              self.arg_date.strftime("%H:%M %d.%m.%Yг.")
          ) 
      ) 
      
      coords = meteo_pb2.GeoPointPb()
      coords.lat_radian =  math.radians(self.map_from_proto.document.doc_center.lat_deg)
      coords.lon_radian =  math.radians(self.map_from_proto.document.doc_center.lon_deg)
       
      # подготавливаем карту
      self.createMap( self.arg_date.strftime("%Y-%m-%dT%H:%M:%S"), coords, self.map_from_proto.document.scale )
      
      self.saveImg(current_page,'forecast_map')
      
      k+=1   
    
    return



  # 
  # Генерим карту
  # @dt - строка с датой
  # @coords - proto_dict с координатами в роадианах 'lat_radian', 'lon_radian'
  def createMap(self, dt,coords,scale=14):
    def errFunc(error):
      logging.error(  error )
      self.result.comment = error
      return False
    
    if self.connect(services_pb2.kMap) is False:
      logging.error("connection error")
      return False
    
    self.fill_map_proto(coords)
    
    
    # если джобы еще не загружены - пробуем загрузить их
    if self.job is False:
      self.load_job(self.job_name)
    
    # если джобы загрузились, то назначаем их реквесту
    # logging.error(self.job)
    if self.job is not False:
      self.map = self.job.map
    
    # устанавливаем дефолнтные параметры на карту
    self.set_map_attrs( coords, scale )
    self.map.hour   = 0
    self.map.center = 250
    self.map.model  = 250
    
    
    # logging.error(self.map)
    
    srv = document_service_pb2.DocumentService_Stub( self.ctrl.Channel() )
    response = self.ctrl.RemoteCall( srv.CreateDocument, self.map.document, 90000 )
    
    
    # logging.error(self.map)
    response = self.ctrl.RemoteCall( srv.CreateMap, self.map, 90000 )

    request      = document_service_pb2.ExportRequest()
    request.uuid = response.uuid
    response     = self.ctrl.RemoteCall( srv.GetDocument, request, 90000 )
    self.image   = response.data
    self.ctrl.close()
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
  parser.add_argument('-i', '--index',    help='Индекс станции')
  parser.add_argument('-d', '--datetime', help='Дата и время измерений в формате YYYY-MM-DDThh:mm:ss')
  parser.add_argument('-n', '--number',   help='Условный номер станции')

  options   = parser.parse_args()

  generator = ODPTable()
  init      = False
 
  if str(options.mode) == "arg":
    init = generator.init(options)
  elif str(options.mode) == "proto":
    sa   = document_service_pb2.StartArguments()
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
