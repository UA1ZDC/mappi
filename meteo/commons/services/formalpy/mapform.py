#!/usr/bin/env python
# -*- coding: utf-8 -*-

# системные модули
import os
import sys
import base64
import logging
from sys import argv
import math

# модули времени и даты
import time
from datetime import timedelta, datetime

# конфигурационный модуль
from conf import *
sys.path.append(PRJ_DIR + '/lib')

# модули для работы с документами
from lpod3 import document
from lpod3.document import *

# модули оболочек C++ классов Puanson, TMeteoData, Zond
import libglobalwrap as globalWrap
import tempfile
import meteoglobal

from meteo_channel import *
from meteo_controller import *

from document_service_pb2 import *
import sprinf_pb2
import surface_pb2
import field_pb2
import services_pb2
import appconf_pb2
import state_pb2
import forecast_pb2
import meteo_pb2
import weather_pb2
import map_document_pb2

# ипортируем главный класс для наслоедования
from documentform import DocumentFormal

# 
# Скелет класса с базовыми функциями для запросов документов
# 
class MapFormal(DocumentFormal):
  
  def __init__(self):
    super().__init__()
    
    # имя джоба, по которому генерятся карты
    self.job_name          = "Prizemnaya-Prognoz-Eurasia.job"
    
    self.map               = False
    self.image             = ""
    self.job               = False
    self.center            = 74
    self.hour              = 24

  # 
  # Загружаем джоб по имени
  # 
  def load_job(self, name="Prizemnaya-Eurasia.job"):
    self.job = False
    req      = weather_pb2.JobList()
    try:
        response = meteoglobal.RemoteCall(  servicecode    = services_pb2.kMap,
                                        methodfullname = 'document_service_pb2@DocumentService.GetWeatherJobs',
                                        request        = req,
                                        timeout_msecs  = 90000 )
    except:
        response = False
    if response is None or response is False:
      return False
    for _ in response.job:
      if name == _.name:
        self.job = _
    return True

  # 
  # сохраняем картинку в файл и вставляем / заменяем фрейм
  # 
  def saveImg(self,current_page, frame_name):
    # сохраняем ее в файл и добавляем в презентацию
    fh      = tempfile.NamedTemporaryFile()
    outputs = fh.name + ".jpg"
    with open(outputs, 'wb') as f:
      f.write(self.image)
      f.flush()
      f.close()
      self.doc.del_part( current_page.get_frame(name=frame_name).get_image().get_url() )
      current_page.get_frame(name=frame_name).set_image(self.doc.add_file(outputs))
      os.remove(outputs)

  # 
  # устанавливаем дефолтные параметры для карты
  # 
  def set_map_attrs(self,coords,scale=14):
    self.map.hour                           = self.hour
    self.map.document.doctype               = map_document_pb2.kGeoMap
    self.map.document.map_center.lat_radian = coords.lat_radian
    self.map.document.map_center.lon_radian = coords.lon_radian
    self.map.document.doc_center.lat_radian = coords.lat_radian
    self.map.document.doc_center.lon_radian = coords.lon_radian
    self.map.document.docsize.width         = 1920
    self.map.document.docsize.height        = 1080
    self.map.document.geoloader             = "ptkpp"
    self.map.document.projection            = meteo_pb2.kMercat
    self.map.document.scale                 = scale
    self.map.center                         = 250
    # self.map.model                          = 250
    self.map.datetime                       = self.arg_date.strftime("%Y-%m-%dT%H:%M:%S")
    
    if 0 != self.map.hour:
      self.map.center = self.center
      # self.map.model  = 11
    
    # если параметры карты пришли из прото
    # if self.map_from_proto is not False:
    #   if self.map_from_proto.document.doc_center:
    #     self.map.document.doc_center.lat_deg = self.map_from_proto.document.doc_center.lat_deg
    #     self.map.document.doc_center.lon_deg = self.map_from_proto.document.doc_center.lon_deg
    #   if self.map_from_proto.document.map_center:
    #     self.map.document.map_center.lat_deg = self.map_from_proto.document.map_center.lat_deg
    #     self.map.document.map_center.lon_deg = self.map_from_proto.document.map_center.lon_deg
    #   if self.map_from_proto.document.scale:
    #     self.map.document.scale = self.map_from_proto.document.scale
    
    

  # 
  # Заполняем слои и параметры карты вручную
  # 
  def fill_map_proto(self, coords):
    
    self.map                                = weather_pb2.Map()
    # устанавливаем дефолнтные параметры на карту
    # self.set_map_attrs(coords)
    
    
    # mapRequest             = self.map
    self.map.allow_empty = True
    layer                  = self.map.data.add()
    layer.mode             = weather_pb2.kIsoline
    layer.level            = 850
    layer.type_level       = 100
    layer.template_name    = '10009'
    layer.transparency     = 0
    
    layer                  = self.map.data.add()
    layer.mode             = weather_pb2.kIsoline
    layer.level            = 850
    layer.type_level       = 100
    layer.template_name    = '12101'
    layer.transparency     = 0
    layer                  = self.map.data.add()
    layer.mode             = weather_pb2.kGradient
    layer.level            = 850
    layer.type_level       = 100
    layer.template_name    = '12108'
    layer.transparency     = 50
    layer                  = self.map.data.add()
    layer.mode             = weather_pb2.kRadar
    layer.type             = meteo_pb2.kLayerMrl
    layer.h1               = 0
    layer.h2               = 0
    layer.source           = weather_pb2.kSurface
    layer.template_name    = 'Явления МРЛ'
    layer.data_type.append( surface_pb2.kRadarMapType )
        
    layer                  = self.map.data.add()
    layer.mode             = weather_pb2.kPuanson
    layer.level            = 0
    layer.type_level       = 1
    layer.template_name    = 'kn01'
    layer.transparency     = 50
    
    layer.data_type.append( surface_pb2.kSynopFix )

    self.map.center = 250
    # self.map.model  = 250

    # self.map.path        = './tmp'
    self.map.datetime    = self.arg_date.strftime("%Y-%m-%dT%H:%M:%S")
    if 0 != self.map.hour:
      self.map.center = 4
      # self.map.model  = 11
    
    return self.map

  # 
  # заполняем группы регионов
  # @self.regiongroup { regiongroup_id,  }
  # 
  def getRegionGroups(self):
    
    req  = sprinf_pb2.RegionGroupsRequest()

    if self.region_group_id is not None:
      req.id = self.region_group_id
    
    try:
      response = meteoglobal.RemoteCall( servicecode    = services_pb2.kSprinf,
                                        methodfullname = 'sprinf_pb2@SprinfService.GetRegionGroups',
                                        request        = req,
                                        timeout_msecs  = 10000 )
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
        logging.error(coord)
        self.stations.append(
          {
            "station"   : _.station,
            "region_id" : _.region_id,
            "coord"     : coord,
            "title"     : _.title,
          }
        )
        
    return True
