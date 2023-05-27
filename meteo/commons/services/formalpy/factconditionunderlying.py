#!/usr/bin/python3
# -*- coding: utf-8 -*-

#системные модули
import os
import sys
from sys import argv
import datetime
import logging
# модули для работы с документами
from lpod3 import document

from optparse import OptionParser
import argparse

from conf import PRJ_DIR
import base64

import meteoglobal
from document_service_pb2 import *
import sprinf_pb2
import surface_pb2
import field_pb2
import services_pb2

import libpuansonwrap
from libzondwrap import *

meteodescrs            = ['N','Nh','CH','CM','CL','h','w','V','dd','ff','T','P0']
descriptors            = [20010, 20011, 20012, 20013, 20003, 20001, 11001, 11002, 12101, 10004, 20019, 20060, 10052]
meteodescrs_alter      = {'hgr':'h','w_w_':'w','w_tr':'w','VV':'V','PQNH':'P0'}
meteodescrs_hydro      = ['HHHH','S','Hw','Tw','zi','ci']
meteodescrs_underlying = ['sssp','sssf','MM1p','YY1p','MM1f','YY1f','MM2p','YY2p','MM2f','YY2f','Tg','R24','FHI']


from PyQt5.QtCore import QCoreApplication

# ипортируем главный класс для наслоедования
from documentform import DocumentFormal

class FactConditionUnderlyingGenerator(DocumentFormal):
  
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
    # перечень запрашиваемых в базе параметров
    self.descs             = ['N', 'Nh', 'CH', 'CM', 'CL', 'h', 'dd', 'ff', 'V', 'T', 'U', 'Td', 'P0', 
                              'p', 'a', 'w', 'w_w_', 'VV', 'hgr', 'Tb', 'PQNH', 'h50', 'h230', 'w_tr']
    
    self.station_type      = 0
    self.template_path     = "/share/meteo/odtsamples/FactConditionUnderlying.odt"


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
    
    if proto.user_name:
      self.user = proto.user_name
    if proto.user_rank:
      self.rank = proto.user_rank
    if proto.department:
      self.department = proto.department
      
    return True

# 
  # Заполнение документа
  # 
  def fillDocument(self):
    
    self.fillHeaderTable()
    
    libpuansonwrap.loadPunch('is')
    
    # пробуем заполнить данные по станции из наблюдений
    if not self.surface_from_src() and not self.surface_from_field():
      self.hasData        = False
      self.result.comment = u'Данных нет'
    else:
      self.hasData       = True
      self.result.result = True  
    
    
    if not self.zond_from_src() and not self.zond_from_field():
      self.hasData        = False
      self.result.comment = u'Данных нет'
    else:
      self.hasData       = True
      self.result.result = True  
      
    self.footer()
      

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
      zond.fromArrayObject(data)
      zond.preobr()
      # заполняем документ 
      self.fillZond(zond)
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
      zond.fromArrayObject(data)
      zond.preobr()
      
      # заполняем таблицу с аэрологией
      self.fillZond(zond)
      
      # zond_attention = self.body.get_variable_set('zond_attention')
      # srok = str('. Анализ ')
      # if 0 != aero_data[0].hour:
      #   srok = u'. Срок прогноза: ' + str(aero_data[0].hour/3600) 
      # zond_attention.set_text(u'Результат объективного анализа. Центр: ' + aero_data[0].center + meteoglobal.trUtf8(srok))
    else:
      return False
    return True



  # 
  # Пробуем получить погоду по станции 
  # и заполнить таблицу метеосводки
  # 
  def surface_from_src(self):
    # получение данных
    self.hasData, surf_data = self.meteoSurf( )
    
    # если данные есть
    if self.hasData and surf_data and len(surf_data)>0:
      # текущие данные
      cur_data       = surf_data[0]
      self.data_date = cur_data.dt
    else:
      return False

    self.fillMeteo(cur_data)
    self.fillHydro(cur_data)
    self.fillUnderlying(cur_data)
    
    return True

  # 
  # Забираем значения метеопараметров из поля
  # 
  def surface_from_field(self):
    # получение данных
    self.hasData, surf_field = self.meteoField()
    
    # если данные есть
    if self.hasData and surf_field and len(surf_field.data)>0:
      # текущие данные
      cur_data       = surf_field.data[0]
      self.data_date = cur_data.fdesc.date
    else:
      return False
    
    self.fillMeteoField(surf_field)
    self.fillHydro(surf_field)
    self.fillUnderlying(surf_field)
    
    return True



  # 
  # Заполняем гидроданные
  # 
  def fillHydro(self,meteodata_proto):
    hydro = self.body.get_table(name='Hydro')
    if self.station.type != 1:
      self.deleteHydro()
      return

    HHHH  = hydro.get_cell('B1')
    S     = hydro.get_cell('B2')
    Hw    = hydro.get_cell('B3')
    Tw    = hydro.get_cell('B4')
    zi_ci = hydro.get_cell('B5')

    if isinstance(meteodata_proto, surface_pb2.MeteoDataProto):
      meteodata = {}
      for param in meteodata_proto.param:
        meteodata[param.descrname] = param
      zicis = []
      for descr in meteodescrs_hydro:
        if descr not in meteodata or meteodata[descr].quality in [12,9999]:
          continue
        val = libpuansonwrap.stringFromRuleValueCode(descr, meteodata[descr].value, str(meteodata[descr].code))
        if descr in ['zi','ci']:
          zicis.append(val)
          continue
        d = descr
        cell = locals()[d]
        cell.set_text(meteoglobal.trUtf8(val))
      zicis = ' '.join(zicis)
      zi_ci.set_text(meteoglobal.trUtf8(zicis))
      hydro.set_cell('B1',HHHH)
      hydro.set_cell('B2', S)
      hydro.set_cell('B3', Hw)
      hydro.set_cell('B4', Tw)
      hydro.set_cell('B5', zi_ci)


  # 
  # Заполняем метеоинформацию
  # 
  def fillMeteo(self,meteodata_proto):
    meteo     = self.body.get_table(name='Meteo')
    N         = meteo.get_cell('B1')
    Nh        = meteo.get_cell('B2')
    C         = meteo.get_cell('B3')
    h         = meteo.get_cell('B4')
    w         = meteo.get_cell('B5')
    V         = meteo.get_cell('B6')
    dd        = meteo.get_cell('B7')
    ff        = meteo.get_cell('B8')
    T         = meteo.get_cell('B9')
    P0        = meteo.get_cell('B10')
    meteodata = {}
    
    for param in meteodata_proto.param:
      meteodata[param.descrname] = param
    Cs = []
    Ns = []
    hs = 0
    
        
    for descr in meteodescrs:
      if descr not in meteodata or meteodata[descr].quality in [12,9999]:
        continue
      if descr in ['CH','CM','CL']:
        Cs.append(libpuansonwrap.stringFromRuleValueCode(descr, meteodata[descr].value, str(meteodata[descr].code)))
        continue
      elif descr in ['Nh']:
        Ns.append(libpuansonwrap.stringFromRuleValueCode(descr, meteodata[descr].value, str(meteodata[descr].code)))
        Ns = sorted(list(set(Ns)), reverse=True)
        continue
      elif descr in ['h']:
        hs = meteodata[descr].value
        continue
      val = libpuansonwrap.stringFromRuleValueCode(descr, meteodata[descr].value, str(meteodata[descr].code))
      if descr in ['dd'] and meteodata['ff'].value == 0:
        val = ''
      d = descr
      cell = locals()[d]
      cell.set_text(meteoglobal.trUtf8(val))
    for descr in list(meteodescrs_alter.keys()):
      if descr not in meteodata or meteodata[descr].quality in [12,9999]:
        continue
      if descr in 'hgr' and meteodata[descr].value < h:
        hs = meteodata[descr].value
        continue
      if descr in 'w_tr':
        val = meteodata[descr].code.encode('utf-8').decode('utf-8')
      else:
        val = meteoglobal.trUtf8(libpuansonwrap.stringFromRuleValueCode(descr, meteodata[descr].value, meteodata[descr].code.encode('utf-8')))
      d = meteodescrs_alter[descr]
      cell = locals()[d]
      cell.set_text(val)
      Cs = '/'.join(list(dict.fromkeys(Cs)))
      Ns = '/'.join(list(dict.fromkeys(Ns)))
      Nh.set_text(meteoglobal.trUtf8(Ns))
      C.set_text(meteoglobal.trUtf8(Cs))
      h.set_text(meteoglobal.trUtf8(libpuansonwrap.stringFromRuleValue('h', hs)))
    meteo.set_cell('B1', N)
    meteo.set_cell('B2', Nh)
    meteo.set_cell('B3', C)
    meteo.set_cell('B4', h)
    meteo.set_cell('B5', w)
    meteo.set_cell('B6', V)
    meteo.set_cell('B7', dd)
    meteo.set_cell('B8', ff)
    meteo.set_cell('B9', T)
    meteo.set_cell('B10', P0)
    
    
  # 
  # Заполняем метеоинформацию
  # 
  def fillMeteoField(self,meteodata_proto):
    meteo     = self.body.get_table(name='Meteo')
    N         = meteo.get_cell('B1')
    Nh        = meteo.get_cell('B2')
    C         = meteo.get_cell('B3')
    h         = meteo.get_cell('B4')
    w         = meteo.get_cell('B5')
    V         = meteo.get_cell('B6')
    dd        = meteo.get_cell('B7')
    ff        = meteo.get_cell('B8')
    T         = meteo.get_cell('B9')
    P0        = meteo.get_cell('B10')
    meteodata = {}
    Cs = []
    Ns = []
    hs = 0
    
    
    descrs = { 'dd':11001, 'ff':11002, 'T':12101, 'U':13003, 'P0':10004, 'V':20001, 'Nh':20011, 'N':20010}
    for data in meteodata_proto.data:
      meteodata[data.fdesc.meteodescr] = data
    for key in descrs.keys():
        if descrs[key] in meteodata.keys():
          value = meteodata[descrs[key]].value #raw
          value = meteoglobal.trUtf8(libpuansonwrap.stringFromRuleValue(key, value))
          if key == 'dd':
            dd.set_text(value)
          elif key == 'ff':    
            ff.set_text(value)
          elif key == 'T':    
            T.set_text(value)
          elif key == 'V':    
            V.set_text(value)
          elif key == 'P0':    
            P0.set_text(value)
          elif key == 'Nh':    
            Nh.set_text(value)
          elif key == 'N':    
            N.set_text(value)  
    meteo.set_cell('B1', N)
    meteo.set_cell('B2', Nh)
    meteo.set_cell('B3', C)
    meteo.set_cell('B4', h)
    meteo.set_cell('B5', w)
    meteo.set_cell('B6', V)
    meteo.set_cell('B7', dd)
    meteo.set_cell('B8', ff)
    meteo.set_cell('B9', T)
    meteo.set_cell('B10', P0)



  # 
  # Заполняем на основе данных наблюдений
  # 
  def fillUnderlying(self, meteodata_proto):
    underlying = self.body.get_table(name='Underlying')
    sssp       = underlying.get_cell('B1')
    sssf       = underlying.get_cell('B2')
    MY1p       = underlying.get_cell('B3')
    MY1f       = underlying.get_cell('B4')
    MY2p       = underlying.get_cell('B5')
    MY2f       = underlying.get_cell('B6')
    Tg         = underlying.get_cell('B7')
    R24        = underlying.get_cell('B8')
    R240       = underlying.get_cell('B9')
    FHI        = underlying.get_cell('B10')

    if isinstance(meteodata_proto, surface_pb2.MeteoDataProto):
      meteodata = {}
      for param in meteodata_proto.param:
        meteodata[param.descrname] = param
      snow = {'MM1p': '', 'YY1p': '','MM1f': '', 'YY1f': '', 'MM2p': '', 'YY2p': '','MM2f': '', 'YY2f': ''}
      for descr in meteodescrs_underlying:
        if descr not in meteodata or meteodata[descr].quality in [12,9999]:
          continue
        val = libpuansonwrap.stringFromRuleValueCode(descr, meteodata[descr].value, str(meteodata[descr].code))
        if descr in ['MM1p','YY1p','MM1f','YY1f','MM2p','YY2p','MM2f','YY2f']:
          snow[descr] = val
          continue
        d = descr
        cell = locals()[d]
        cell.set_text(meteoglobal.trUtf8(val))

      MY1p.set_text(meteoglobal.trUtf8(snow['YY1p'] + ' ' + snow['MM1p']))
      MY1f.set_text(meteoglobal.trUtf8(snow['YY1f'] + ' ' + snow['MM1f']))
      MY2p.set_text(meteoglobal.trUtf8(snow['YY2p'] + ' ' + snow['MM2p']))
      MY2f.set_text(meteoglobal.trUtf8(snow['YY2f'] + ' ' + snow['MM2f']))
      underlying.set_cell('B3', MY1p)
      underlying.set_cell('B4', MY1f)
      underlying.set_cell('B5', MY2p)
      underlying.set_cell('B6', MY2f)

      underlying.set_cell('B1', sssp)
      underlying.set_cell('B2', sssf)
      underlying.set_cell('B7', Tg)
      underlying.set_cell('B8', R24)
      underlying.set_cell('B10', FHI)

      r240value = self.calculateR240()
      if r240value:
        r240value = libpuansonwrap.stringFromRuleValue('R24',r240value)
        R240.set_text(meteoglobal.trUtf8(r240value))
        underlying.set_cell('B9', R240)


  # 
  # Заполняем головную таблицу
  # 
  def fillHeaderTable(self):
    header = self.body.get_table(name='Header')
    self.docNumberIncrement()

    name = header.get_cell('B2')
    name.set_text(self.station_name)
    header.set_cell('B2',name)

    index = header.get_cell('B3')
    index.set_text( self.getStationNumber(self.station.station) )
    header.set_cell('B3', index)

    height = header.get_cell('B4')
    height.set_text(meteoglobal.trUtf8(self.alt))
    header.set_cell('B4',height)

    coords = header.get_cell('B5')
    coords.set_text(meteoglobal.trUtf8( self.getLatStr()+" "+self.getLonStr() ) )
    header.set_cell('B5', coords)

    date = header.get_cell('B6')
    date.set_text(self.arg_date.strftime('%d-%m-%Y %H:%M'))
    header.set_cell('B6', date)











  # 
  # Увеличиваем количество документов
  # 
  def docNumberIncrement(self):
    meteodir  = os.path.expanduser('~/.meteo/')
    cur_month = datetime.datetime.now().month
    if os.path.isfile(os.path.join(meteodir + '/ogmu_counter')):
      counter = open(os.path.join(meteodir + '/ogmu_counter'), 'r')
      number = int(counter.readline())
      month = int(counter.readline())
      counter.close()
      if cur_month > month:
        month = cur_month
        number = 1
      else:
        number += 1
      counter = open(os.path.join(meteodir + '/ogmu_counter'), 'w')
      counter.write(str(number) + '\n')
      counter.write(str(month))
      counter.close()
    else:
      if not os.path.exists(meteodir):
        os.mkdir(meteodir)
      counter = open(os.path.join(meteodir + '/ogmu_counter'), 'w+')
      number = 1
      month = cur_month
      counter.write(str(number) + '\n')
      counter.write(str(month))
      counter.close()
    meteo = self.body.get_table(name='Header')
    n = meteo.get_cell('B1')
    n.set_text(str(number))
    meteo.set_cell('B1',n)

  def meteoSurfRequest(self):
    request            = surface_pb2.DataRequest()
    request.type_level = 1
    request.level_p    = 0
    request.station.append( meteoglobal.trUtf8(self.station.station)  )
    request.station_type.append(self.station.type)
    request.meteo_descrname.extend(meteodescrs)
    request.meteo_descrname.extend(list(meteodescrs_alter.keys()))
    request.meteo_descrname.extend(meteodescrs_hydro)
    request.meteo_descrname.extend(meteodescrs_underlying)
    request.date_start = self.arg_date.strftime("%Y-%m-%dT%H:%M:%S")
    request.as_proto = True
    request.type.extend([surface_pb2.kSynopFix, surface_pb2.kAirportRegType, surface_pb2.kAerodrome, surface_pb2.kSynopMobSea, surface_pb2.kSynopSnow, surface_pb2.kAeroFix])
    return request


  def meteoSurf(self):
    request  = self.meteoSurfRequest()
    response = meteoglobal.RemoteCall(services_pb2.kSrcData, 'surface_service_pb2@SurfaceService.GetMeteoData', request, 10000)
    if not response:
      self.result.comment = u'Ответ от сервиса раскодированных данных не получен'
      return False, []
    if 0 == len(response.meteodata_proto):
      # self.result.comment = u"Ответ от сервиса получен, но данных нет"
      return False, []
    meteodata_proto = response.meteodata_proto[0]
    # logging.error(response.meteodata_proto)
    return True, response.meteodata_proto


  def meteoFieldRequest(self):
    request       = field_pb2.DataRequest()
    coords        = surface_pb2.Point()
    coords.fi     = self.lat_radian
    coords.la     = self.lon_radian
    request.coords.extend([coords])
    request.meteo_descr.extend(descriptors)
    request.type_level.append(1)
    request.level.append(0)
    request.date_start = self.arg_date.strftime("%Y-%m-%dT%H:%M:%S")
    return request


  def meteoField(self):
    request = self.meteoFieldRequest()
    # logging.error(request)
    response = meteoglobal.RemoteCall(services_pb2.kField, 'field_pb2@FieldService.GetFieldDataForMeteosummary', request, 10000)
    # logging.error(response.comment)
    # logging.error(response.data)
    if not response:
      self.result.comment = u'Ответ от сервиса результатов объективного анализа не получен'
      return False, []
    if len(response.data) == 0:
      self.result.comment = u"Ответ от сервиса получен, но данных нет"
      return False, []
    return True, response



  def calculateR240(self):
    request            = surface_pb2.DataRequest()
    request.type_level = 1
    request.level_p    = 0
    request.station.append( meteoglobal.trUtf8(self.station.station)  )
    request.station_type.append(self.station.type)
    request.meteo_descrname.extend(meteodescrs_underlying)
    dte = self.arg_date
    dts = self.arg_date - datetime.timedelta(days=10)
    request.date_start = dts.strftime("%Y-%m-%dT%H:%M:%S")
    request.date_end = dte.strftime("%Y-%m-%dT%H:%M:%S")
    request.as_proto = True
    request.type.extend([surface_pb2.kSynopFix, surface_pb2.kAirportRegType, surface_pb2.kAerodrome, surface_pb2.kSynopMobSea, surface_pb2.kSynopSnow])
    response = meteoglobal.RemoteCall(services_pb2.kSrcData, 'surface_service_pb2@SurfaceService.GetMeteoDataOnStation', request,
                                      10000)
    if not response:
      self.result.comment = u'Ответ от сервиса раскодированных данных не получен'
      return None
    if 0 == response.meteodata_proto.__len__():
      self.result.comment = u"Ответ от сервиса получен, но данных нет"
      return None
    R240 = 0
    for data in response.meteodata_proto:
      for param in data.param:
        if param.descrname in ['R24']:
          if param.value != 999.9:
            R240 += param.value
          break
    return R240


  def deleteHydro(self):
    hydro       = self.body.get_table(name='Hydro')
    hydroHeader = self.body.get_variable_set('HydroHeader')
    hydroHeader.set_text('')
    hydro.delete()



  # 
  # Заполняем данные зонда
  # 
  def fillZond(self, zond):
    windtemp = self.body.get_table(name='WindTemp')
    standard = [925, 850, 700, 500, 400, 300, 200, 150, 100, 70, 50, 30, 20, 10]
    i        = 3
    jet1     = jet2 = None
    zond.preobr()
    
    for s in standard:
      level = Uroven()
      zond.getUrPoP(s, level)
      dd = ff = T = h = None
      
      if level.isGood(ValueType.UR_dd):
        dd = libpuansonwrap.stringFromRuleValue('dd', level.value(ValueType.UR_dd))
        ddCell = windtemp.get_cell('C' + str(i))
        ddCell.set_text(meteoglobal.trUtf8(dd))
        windtemp.set_cell('C' + str(i), ddCell)
      if level.isGood(ValueType.UR_ff):
        ff = libpuansonwrap.stringFromRuleValue('ff', level.value(ValueType.UR_ff) * 3.6)
        ffCell = windtemp.get_cell('D' + str(i))
        ffCell.set_text(meteoglobal.trUtf8(ff))
        windtemp.set_cell('D' + str(i), ffCell)
      if level.isGood(ValueType.UR_T):
        T = libpuansonwrap.stringFromRuleValue('T', level.value(ValueType.UR_T))
        TCell = windtemp.get_cell('E' + str(i))
        TCell.set_text(meteoglobal.trUtf8(T))
        windtemp.set_cell('E' + str(i), TCell)
      if level.isGood(ValueType.UR_H):
        h = level.value(ValueType.UR_H)
        hCell = windtemp.get_cell('A' + str(i))
        meters = ''
        if h < 1000:
          meters = u' м'
        else:
          meters = u' км'
          h = h / 1000.0
        hCell.set_text(meteoglobal.trUtf8(libpuansonwrap.stringFromRuleValue('h', h)) + meters)
        windtemp.set_cell('A' + str(i), hCell)
      if h and h >= 1.5 and h <= 16 and ff and level.value(ValueType.UR_ff) >= 27.78:
        if not jet1:
          jet1 = {'h': h, 'dd' : dd if dd else '', 'ff' : ff}
        elif not jet2:
          jet2 = {'h': h, 'dd' : dd if dd else '', 'ff' : ff}
      i += 1
    if jet1:
      h = windtemp.get_cell('D17')
      ff = windtemp.get_cell('D18')
      dd = windtemp.get_cell('D19')
      h.set_text(meteoglobal.trUtf8(libpuansonwrap.stringFromRuleValue('h', jet1['h'])))
      ff.set_text(meteoglobal.trUtf8(jet1['ff']))
      dd.set_text(meteoglobal.trUtf8(jet1['dd']))
      windtemp.set_cell('D17', h)
      windtemp.set_cell('D18', ff)
      windtemp.set_cell('D19', dd)
    if jet2:
      h = windtemp.get_cell('D20')
      ff = windtemp.get_cell('D21')
      dd = windtemp.get_cell('D22')
      h.set_text(meteoglobal.trUtf8(libpuansonwrap.stringFromRuleValue('h', jet2['h'])))
      ff.set_text(meteoglobal.trUtf8(jet2['ff']))
      dd.set_text(meteoglobal.trUtf8(jet2['dd']))
      windtemp.set_cell('D20', h)
      windtemp.set_cell('D21', ff)
      windtemp.set_cell('D22', dd)
    tropo = Uroven()
    zond.getUrPoLevelType(7, tropo)
    tropoh      = windtemp.get_cell('D23')
    tropoT      = windtemp.get_cell('D24')
    tropohvalue = tropo.value(ValueType.UR_H)
    if tropohvalue > 1000:
      tropohvalue = tropohvalue / 1000.0
      
    tropohvalue = meteoglobal.trUtf8(libpuansonwrap.stringFromRuleValue('h', tropohvalue))
    tropoh.set_text(tropohvalue)
    tropotvalue = meteoglobal.trUtf8(libpuansonwrap.stringFromRuleValue('T', tropo.value(ValueType.UR_T)))
    tropoT.set_text(tropotvalue)
    windtemp.set_cell('D23', tropoh)
    windtemp.set_cell('D24', tropoT)


  # 
  # Заполняем подвал документа
  # 
  def footer(self):
    footer     = self.body.get_table(name='Footer')
    department = footer.get_cell('B1')
    user       = footer.get_cell('B2')
    dt         = footer.get_cell('B3')

    department.set_text(meteoglobal.trUtf8(self.department))
    rankuser = meteoglobal.trUtf8(self.rank) + " " + meteoglobal.trUtf8(self.user)
    rankuser = meteoglobal.trUtf8(rankuser)
    user.set_text(rankuser)
    dt.set_text(meteoglobal.dateToHumanTimeShort(datetime.datetime.now()))

    footer.set_cell('B1', department)
    footer.set_cell('B2', user)
    footer.set_cell('B3', dt)







# 
# ====================== MAIN FUNCTION ============================
# 
def main(argv):
  app = QCoreApplication(sys.argv)

  parser = argparse.ArgumentParser(description="Сценарий формирования документа \"Фактическая информация о гидрометеорологических условиях и состоянии подстилающей поверхности по району\"")
  parser.add_argument('-m', '--mode', default='arg', help="Режим запуска сценария. "
                                                          "arg - Задание аргументов запуска "
                                                          "proto - Передача прото-структуры в стандартный поток")
  parser.add_argument('-f', '--filepath', default=PRJ_DIR + '/var/meteo/documents/',
                      help="Путь сохранения файла.")
  parser.add_argument('-i', '--index', help='Индекс станции')
  parser.add_argument('-d', '--datetime', help='Дата и время измерений в формате YYYY-MM-DDThh:mm:ss')

  options = parser.parse_args()

  generator = FactConditionUnderlyingGenerator()
  init = False
  if str(options.mode) == "arg":
    init = generator.init(options)
  elif str(options.mode) == "proto":
    sa = StartArguments()
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
  
  if not result.result:
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(1)
  else:
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(0)


if __name__ == "__main__":
  main(argv)
