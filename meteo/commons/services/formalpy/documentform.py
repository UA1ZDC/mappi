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

import libpuansonwrap
import librpcpywrap
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


# 
# Скелет класса с базовыми функциями для запросов документов
# 
class DocumentFormal():
  
  def __init__(self):
    # сервис по умолчанию
    self.service = False
    # адрес и порт сервиса
    self.address       = False
    self.host          = False
    self.port          = False
    self.ctrl          = ClientController()
    self.output_format = "odt"
    self.months        = ['января', 'февраля', 'марта', 'апреля', 'мая', 'июня', 'июля', 'августа', 'сентября', 'октября', 'ноября', 'декабря']
    self.month         = ['январь', 'февраль', 'март', 'апрель', 'май', 'июнь', 'июль', 'август', 'сентябрь', 'октябрь', 'ноябрь', 'декабрь']

    # путь к шаблону
    self.template_path = ""
    # словарь названий параметров к их значениям
    self.descrname2params = {
      'CH'    : 20012,
      'CM'    : 20012,
      'CL'    : 20012,
      'w'     : 20003,
      'N'     : 20010,
      'Nh'    : 20011,
      'h'     : 20013,
      'w_w_'  : 20019,
      'V'     : 20001,
      'VV'    : 20060,
      'PQNH'  : 10052,
      'precip': 13055,
      'dd'    : 11001,
      'ff'    : 11002,
      'T'     : 12101,
      'U'     : 13003,
      'P0'    : 10004,
      'R12'   : 13022,
      'Td'    : 12103,
      'wind'  : 7700001,
      'cloud' : 7700002,
      'Ptend' : 7700003,
    }

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

    # широта и долгота станции
    self.lat        = 0.0
    self.lon        = 0.0
    self.lat_radian = 0.0
    self.lon_radian = 0.0
    self.alt        = 0.0
    self.h0         = 0.0
    # станция в прото формате
    self.station = False 

    # документ
    self.body = None
    self.doc  = None

    # если надо сохранять документ
    # self.need_save = False
        
    self.login             = "" # логин пользователя
    self.user              = "" # имя пользователя
    self.rank              = "" # звание
    self.department        = "" # подразделение
    self.hasData           = False
    self.result            = ExportResponse()
    self.result.result     = False
    self.text              = ''
    # индикация
    self.indication        = [] #{'ump':[],'smu':[],'name':''}
    # Тип рекомендации
    # 0 - без ограничений
    # 1 - ограничения (СМУ)
    # 2 - не рекомендуется (УМП)
    self.recomendations    = [ 'Ограничений по ГМУ нет', 'Ограничить применение', 'Применение не рекомендуется' ]
    # параметры карты из прото
    self.map_from_proto = False

  # 
  # Устанавливаем формат презентации Impress на выходе
  # 
  def setODPFormat(self):
    self.output_format = "odp"
    return self

  # 
  # Устанавливаем формат екселя Calc на выходе
  # 
  def setODSFormat(self):
    self.output_format = "ods"
    return self

  # 
  # Инициализируем сервис, получаем его адрес порт и хост
  # return @address, @host, @port
  # 
  def init_service(self, service):
    address = globalWrap.serviceAddress(service)
    return address, librpcpywrap.hostByAddress(address), librpcpywrap.portByAddress(address)

  # 
  # Преобразуем названия параметров в значение дескриптора 
  # надо для запроса полей метеовеличин
  # 
  def get_param_by_descrname(self,descrname):
    if descrname in self.descrname2params.keys():
      return self.descrname2params[descrname]
    return 0

  # 
  # Подключаемся к нужному сервису
  # @service - номер нужного сервиса, порт и адрес достаются автоматом
  # @attemts - количество попыток подключения
  # return статус подключения - удалось или нет
  # 
  def connect(self, service=False):
    # подключение к сервису
    if service is False:
      # по умолчанию - просто сервис 
      service = services_pb2.kSprinf

    address, host, port = self.init_service(service)
    self.ctrl           = ClientController()
    
    if ( False == self.ctrl.Connect(host, port) ):
      self.result.comment = u"Не удалось подключиться к сервису данных"
      return False
    return True

  # безопасная 
  def safeCast(self,value,to_type=int,default=None):
    try:
      return to_type(value)
    except (ValueError, TypeError):
      return default
    return

  # 
  # заполняем данные по станции
  # @st_index - индекс станции
  # 
  def getStationData(self,st_index):
    request = sprinf_pb2.MultiStatementRequest()
    # request.type.append( sprinf_pb2.MeteostationType.Value('kStationSynop') )
    # request.type.append( sprinf_pb2.MeteostationType.Value('kStationAerodrome') )
    # request.type.append( sprinf_pb2.MeteostationType.Value('kStationAirport') )
    # request.type.append( sprinf_pb2.MeteostationType.Value('kStationHydro') )
    request.station.append(st_index)
    try:
      response = meteoglobal.RemoteCall(  servicecode    = services_pb2.kSprinf,
                                          methodfullname = 'sprinf_pb2@SprinfService.GetStations',
                                          request        = request,
                                          timeout_msecs  = 30000)
    except:
      response = None

    if response == None or response is False or response.result is False :
      return False
    
    if len(response.station):
      return response.station[0]
    return response.station


  # 
  # заполняем данные по станции
  # @st_index - индекс станции
  # 
  def getStationsData(self,st_indexes):
    request = sprinf_pb2.MultiStatementRequest()
    request.station.extend(st_indexes)
    try:
      response = meteoglobal.RemoteCall(  servicecode    = services_pb2.kSprinf,
                                          methodfullname = 'sprinf_pb2@SprinfService.GetStations',
                                          request        = request,
                                          timeout_msecs  = 30000)
    except:
      response = None

    if response == None or response.result is False :
      return False
    
    return response.station

  # 
  # Заполняем данные по станции
  # @station_data - прото ответ от сервиса
  # 
  def fillStationData(self,station_data):
    station_name = ""
    lat, lon, alt,lat_radian,lon_radian     = 0.0, 0.0, 0.0, 0.0,0.0
    
    try:
      lat_radian = station_data.position.lat_radian
      lon_radian = station_data.position.lon_radian
      alt        = station_data.position.height_meters
      lat        = math.degrees(lat_radian)
      lon        = math.degrees(lon_radian)
    except:
      pass
    # сохраняем в классе
    self.lat        = lat
    self.lon        = lon
    self.alt        = alt
    self.lat_radian = lat_radian
    self.lon_radian = lon_radian
    
    return lat, lon, alt
    
  # 
  # Заполняем данные пользователя
  # 
  def fillUserData(self, login=None):
    if login is None:
      login = self.login
      
    # 
    # получаем зхвания
    # 
    ranksList = meteoglobal.RemoteCall(services_pb2.kDiagnostic,
                                       'appconf_pb2@ControlService.LoadRanks',
                                       appconf_pb2.UserRequest(), 50000) #type appconf_pb2.UserResponce
    if ranksList is None:
      return

    ranks = dict()
    for rank in ranksList.ranks:
      ranks[rank.id] = meteoglobal.fromUtf8(rank.name)

    # 
    # получаем должности
    # 
    departmentList = meteoglobal.RemoteCall(services_pb2.kDiagnostic,
                                            'appconf_pb2@ControlService.LoadDepartmentList',
                                            state_pb2.Dummy(), 50000)
    if departmentList is None:
      return

    departments = dict()
    for dep in departmentList.list:      
      id              = meteoglobal.fromUtf8(dep.id)
      name            = meteoglobal.fromUtf8(dep.name)
      departments[id] = name

    userRequest = appconf_pb2.UserRequest()
    response    = meteoglobal.RemoteCall(services_pb2.kDiagnostic,
                                      'appconf_pb2@ControlService.LoadUsers',
                                      userRequest, 500000)

    for user in response.users:
      if user.login == login:        
        name         = meteoglobal.fromUtf8(user.name)
        surname      = meteoglobal.fromUtf8(user.surname)
        patron       = meteoglobal.fromUtf8(user.patron)
        departmentId = meteoglobal.fromUtf8(user.department)

        self.user       = u'{} {} {}'.format( name, surname, patron)
        self.rank       = u'{}'.format(ranks[user.rank])
        self.department = departments.get(departmentId, "")

  # генерим имя станции
  # если station не задан, то берется локальная станция класса
  def getStationName(self,station=False):
    if station is False:
      station = self.station
    result = ""

    try:
      if station.name.rus:
        result = station.name.rus
      elif station.name.international:
        result = station.name.international
      elif station.name.short:
        result = station.name.short
      else:
        result = self.getStationNumber(station.index)
    except:
      result = ""
      
    return result

  def getStationNumber(self,station=False):
    """Функция, которая исправляет представление номеров станций путем добавления 0 у станций,
       длина которых меньше 5 чисел

    Returns:
        _type_: _description_
    """
    if station is False:
      station = self.station
    if str(station).isnumeric():
      return str(station).rjust(5,'0')
    return station

  # получаем строковое представлени широты
  def getLatStr(self,lat=False):
    if lat is False:
      lat = self.lat
    return str('%.2f с.ш' % lat) if lat > 0 else  str('%.2f ю.ш' % abs(lat))

  # получаем строковое представлени долготы
  def getLonStr(self,lon=False):
    if lon is False:
      lon = self.lon
    return str('%.2f в.д' % lon) if lon > 0 else  str('%.2f з.д' % abs(lon))
    
  # 
  # Функция генерации документа
  # 
  def generateDocument(self):
    if not self.openTemplate():
      # print "Не удалось открыть шаблон документа"
      self.result.result = False
      return False
    else:
      self.body  = self.doc.get_body()
    # заполняем документ
    self.fillDocument()
    # проверяем все ли ок
    if not self.hasData:
      self.result.comment = u"Данных нет"
    if not self.saveDocument():
      self.result.result = False
      return False
    self.result.result = True
    return True

  
  # 
  # Заполнение документа
  # 
  def fillDocument(self):
    # 
    # необходимо описание функционала метода =========
    # 

    # инициализация пуансона из файла шаблона
    libpuansonwrap.loadPunch(str("is"))
    
    return False

  # 
  # Сохранение документа
  # 
  def saveDocument(self):
    fh      = tempfile.NamedTemporaryFile()
    outputs = fh.name + "." + self.output_format

    try:
      self.doc.save(target=outputs, pretty=True)
      file = open(outputs, 'rb')
      self.result.data = base64.b64encode(file.read())
      os.remove(outputs)
      # print(outputs)
      self.result.result = True
      sys.stdout.buffer.write(self.result.SerializeToString())
      return True

    except IOError as ioe:
      self.result.comment = u'Ошибка записи в файл'
      return False

  # 
  # функция открытия документа
  # 
  def openTemplate(self):
    try:
      # открываем документ
      self.doc = document.odf_get_document(PRJ_DIR + self.template_path)
      return True
    except IOError:
      self.result.comment = u"Файл шаблона указан неверно (%s)" % self.template_path
      return False
  
  # сохраняем результат в протофайле
  def resultProto(self):
    return self.result
  
  

  # 
  # инициализация с параметрами
  # 
  def init(self, options):
    if (None == options.datetime):
      self.result.result = False
      self.result.comment = u"Не заданы требуемые параметры (datetime)"
      return False
    self.arg_date          = datetime.strptime(options.datetime, "%Y-%m-%dT%H:%M:%S")
    self.data_date         = self.arg_date.strftime("%Y-%m-%dT%H:%M:%S")
    
    if options.name:
      self.user = options.name
    if options.rank:
      self.rank = options.rank
    if options.department:
      self.department = options.department
      
    return True


  # 
  # инициализация из прото параметров
  # 
  def initFromProto(self, proto):
    if not proto.datetime or 0 == proto.stations.__len__():
      self.result.result = False
      self.result.comment = u"Не заданы требуемые параметры proto"
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
  # Инициализация станций из списка, если он задан
  # 
  def initProtoStations(self,proto):
    proto_stations = [ _.index for _ in proto.stations ]
    if len(proto_stations)>0:
      stations = self.getStationsData( proto_stations )
      if stations is not False and len(stations)>0:
        self.stations = [ { "station"   : _.station,
                            "coord"     : _.position,
                            "title"     : _.name.rus  } for _ in stations ]
    return self
  
  # 
  # Базовые параметры при инициализации из прото
  # если надо что-то экзотическое -то прописываем отдельно
  # 
  def standartProtoInit(self,proto):
    self.arg_date   = datetime.strptime(proto.datetime, "%Y-%m-%dT%H:%M:%S")
    self.number     = proto.number
    
    # путь к документам
    if proto.filepath:
      self.filepath = proto.filepath
    else:
      self.filepath = PRJ_DIR + '/var/meteo/documents/'
    
    # параметры станции
    try:
    # logging.error(proto.stations[0])
      self.station      = self.getStationData( str( proto.stations[0].index ) )
      self.station_name = self.getStationName(self.station)
      self.alt          = self.station.position.height_meters
      self.h0           = self.station.position.height_meters
      self.station_type = self.station.type
      # задаем lat lon
      self.fillStationData(self.station)
    except:
      logging.error("[documentform.py] Cannot init station params from proto")
      
    try:
      self.initIndicationFromProto(proto)
    except:
      logging.error("[documentform.py] Indication cannot init from proto")
      
    self.printFileToStdOut = True
  
  # 
  # Парсим индикацию из прото
  # 
  def initIndicationFromProto(self, proto):
    self.indication = []
    
    if len( proto.indication ) >0:
      for _ in proto.indication:
        indication = {'ump':[],'smu':[],'preset_name':''}
        indication['preset_name'] = _.preset_name
        for u in _.preset_settings.ump:
          indication['ump'].append( self.getFunctionByOperator(u.param,u.operator,u.value) )
        for s in _.preset_settings.smu:
          indication['smu'].append( self.getFunctionByOperator(s.param,s.operator,s.value) )
        # logging.error(indication)
        self.indication.append(indication)
     
    return self
  
  # 
  # Функция генерации лямбда выражений которые возвращают результат сравнения
  # параметр ляимбда выражения ( @descr, @param ), где @descr - тип переменной (descrname),а
  # @param - это значение переменной
  # 
  # @descrname - тип переменной
  # @operator - признак сравнения
  # @value - значение параметра
  def getFunctionByOperator(self, descrname, operator, value):
    if self.is_float(value):
      value = float(value)
    if operator=='=':
      return ( lambda descr,param: descr==descrname and param==value )
    if operator=='<':
      return ( lambda descr,param: descr==descrname and param<value )
    if operator=='>':
      return ( lambda descr,param: descr==descrname and param>value )
    if operator=='in':
      return ( lambda descr,param: descr==descrname and str(param) in str(value) )
    return ( lambda x: False )
  
  # проверяем если число
  def is_float(self,value):
    try:
      float(value)
      return True
    except:
      return False
  
  # 
  # Получаем данные по станции
  # @_request - готовый прото-запрос, если он пуст, то генерим запрос по умолчанию
  # 
  def getMeteoData(self, _request = None,date=None):
    request = _request
    if request is None:
      date    = self.arg_date if date is None else date
      request = self.makeRequest(self.station, date, self.descs)
    
    try:
      response       = meteoglobal.RemoteCall(  servicecode    = services_pb2.kSrcData,
                                                methodfullname = 'surface_service_pb2@SurfaceService.GetMeteoDataOnStation',
                                                request        = request,
                                                timeout_msecs  = 30000 )
    except:
      response=False

    
    if response is False :
      self.result.comment = u'Ответ от сервиса meteo не получен'
      return False, []
    if response is None or (not response.meteodata_proto):
      self.result.comment = u'Ответ от сервиса meteo получен, но данных нет'
      return False, []

    # HasData Meteodata
    return True, response.meteodata_proto 
  
   
  # 
  # получаем последнюю погоду по станциям
  # 
  def getMeteoDataByDay(self, _request, date=None):
    request = _request
    if request is None:
      date    = self.arg_date if date is None else date
      request = self.makeRequest(self.station, date, self.descs)
    
    try:
      response       = meteoglobal.RemoteCall(  servicecode    = services_pb2.kSrcData,
                                                methodfullname = 'surface_service_pb2@SurfaceService.GetMeteoDataByDay',
                                                request        = request,
                                                timeout_msecs  = 30000 )
    except:
      response=False

    
    if response is False :
      self.result.comment = u'Ответ от сервиса meteo не получен'
      return False, []
    if response is None or (not response.meteodata_proto):
      self.result.comment = u'Ответ от сервиса meteo получен, но данных нет'
      return False, []

    # HasData Meteodata
    return True, response.meteodata_proto 


  # 
  # подготавливаем данные для отображения
  # @sttype - тип станции, потому что пуансон неправильно обрабатывает облачность, поэтому такой небольшой костыль
  # 
  def prepare_params_for_tablo(self, param_list, columns, sttype=2):
    indexed_ = {}
    str_array_items = {}
    val_array_items = {}
    for param in param_list:
      _d = param.descrname
      # если нашли дескриптор в повторяшках
      # то пытаемся его преобразовать, чтобы получилось как надо
      # это необходимо для того, чтобы учесть параметры, которые приходят в разных кодах  под разными дескрипторами
      for k in self.transform_descriptors:
        if _d in self.transform_descriptors[k]:
          _d = k
          break
      
      if _d not in indexed_:
        str_array_items[ _d ] = []
        val_array_items[ _d ] = []
        indexed_[ _d ] = {
          'code' : param.code,
          'val' : param.value,
          'str' : '' }
      
      # // обрабатываем и добавляем параметры в табло
      # // подготовленное значение для отображения
      # strval_ = param.value
      if sttype!=0 and (_d=='N' or (_d=='Nh' and param.value=='0')):
        strval_ = self.safeCast(param.value, int)
      else:
        strval_ = libpuansonwrap.stringFromRuleValue( _d, param.value )

      if len(str_array_items[_d])==0 and strval_!='':
        str_array_items[_d].append( str(strval_) )
        val_array_items[_d].append( self.safeCast(strval_, float) )
      
      # Если есть параметры, которые надо объеденить через /, то объединяем их предварительно проверив
      if _d in self.merge_cloud_descr:
        if indexed_[ _d ]['str'] != '' and strval_!='' and indexed_[ _d ]['str']!='0':
          param_val = param.value
          if indexed_[ _d ]['val']!=0 and indexed_[ _d ]['val']!=0.0 and indexed_[ _d ]['val']!='':
            # // проверяем, если такое значение уже есть, то нет необходимости его записывать
            if indexed_[ _d ]['val'] != param_val:
              indexed_[ _d ]['val'] =[ indexed_[ _d ]['val'], param_val ]
          else:
            if isinstance(indexed_[ _d ]['val'], list) and param_val not in indexed_[ _d ]['val']:
              indexed_[ _d ]['val'].append( param_val )
          # // добавляем строковой параметр до того, как смерджили или объединили несколько
          strval_ = str(strval_)
          if strval_!='' and strval_ not in str_array_items[ _d ]:
            str_array_items[ _d ].append( strval_ )
            val_array_items[ _d ].append( self.safeCast(strval_, float) )
        
        #  офифровываем для корректной сортировки
        _val = str_array_items[ _d ]
        # if _d not in state.map_descriptors.keys():
        #   _val = _val.map( state.map_descriptors[_d]
        
        # if ( Object.keys( state.sort_descriptors ).indexOf( _d )!==-1 ){
        #   _val.sort(state.sort_descriptors[_d]);
        # }
        
        strval_ = "/".join(_val)

      indexed_[ _d ]['str'] = strval_
    
    result = {}
    values = {}
    for _ in columns:
      if _ in indexed_:
        result[_] = indexed_[_]['str']
        values[_] = val_array_items[_][0] if len(val_array_items[_])==1 else val_array_items[_]
      else:
        result[_] = ''
        values[_] = None
        indexed_[_] = {
          'code' : 0,
          'val' : 0,
          'str' : '' }
    
    return result, indexed_


  # 
  # Получаем поля метеовеличин
  # @_request - готовый прото-запрос, если он пуст, то генерим запрос по умолчанию
  # 
  def getFieldData(self, _request=None,date=None):
    request = _request
    if request is None:
      date    = self.arg_date if date is None else date
      request = self.makeFieldRequest(date, self.descs)
    
    try:
      response       = meteoglobal.RemoteCall(  servicecode    = services_pb2.kField,
                                                methodfullname = 'field_pb2@FieldService.GetFieldDataForMeteosummary',
                                                request        = request,
                                                timeout_msecs  = 30000 )
    except:
      response=False
    if response is None or response is False:
      self.result.comment = u'Ответ от сервиса field получен, но данных нет'
      return False, []
    # HasData Meteodata
    return True, response.data 
  
  # 
  # Получаем имя центра
  # 
  def getCenterName(self, id):
    request = sprinf_pb2.MeteoCenterRequest()
    request.center_id.append(id)
    response = meteoglobal.RemoteCall(services_pb2.kSprinf,
                                      'sprinf_pb2@SprinfService.GetMeteoCenters',
                                      request,
                                      100000)
    if None == response:
      return None
    if 0 == len(response.center):
      return None
    if response.center[0].short_name:
      return response.center[0].short_name
    else:
      return response.center[0].name


  # 
  # Получаем данные зондирования
  # @_request - готовый прото-запрос, если он пуст, то генерим запрос по умолчанию
  # 
  def getAeroData(self,_request=None,date=None):
    request = _request
    if request is None:
      date    = self.arg_date if date is None else date        
      request = self.makeAeroRequest(self.station, date)
      
    try:
      response       = meteoglobal.RemoteCall(  servicecode    = services_pb2.kSrcData,
                                                methodfullname = 'surface_service_pb2@SurfaceService.GetTZondOnStation',
                                                request        = request,
                                                timeout_msecs  = 30000 )
    except:
      response=False
    
    if response is None or response is False:
      self.result.comment = u'Ответ от сервисa aero не получен'
      return False, []
    else:
      if len(response.data)==0 :
        self.result.comment = u'Ответ от сервиса aero получен, но данных нет'
        return False, []
    if response.data[0].date == "" or response.data[0].point.fi>990 or response.data[0].point.fi<-990:
      self.result.comment = u'Неверные данные'
      return False, []
    # HasData Meteodata
    return True, response.data 
  
  # 
  # Получаем данные зондирования из полей
  # @_request - готовый прото-запрос, если он пуст, то генерим запрос по умолчанию
  # 
  def getAeroFieldData(self,_request=None,date=None):
    request = _request
    if request is None:
      date    = self.arg_date if date is None else date
      request = self.makeFieldAeroRequest(date)
    try:
      response       = meteoglobal.RemoteCall(  servicecode    = services_pb2.kField,
                                                methodfullname = 'field_pb2@FieldService.GetADZond',
                                                request        = request,
                                                timeout_msecs  = 30000 )
    except:
      response=False
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
  # Загружаем прогностические центры
  # 
  def getCenters(self):
    # 
    # FieldService::GetAvailableCentersAD
    # 
    return

  # 
  # создаем запрос для списка станция на определнный срок
  # 
  def makeRequest(self, stations, date_start, descs, proto=True, date_end=None):
    if not stations:
      self.result.comment = u"Станции не заданы"
      return 0
    request            = surface_pb2.DataRequest()
    
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
      
    request.station.append(str(stations.station))
    request.meteo_descrname.extend(descs)
    request.type_level = 1
    request.level_p    = 0
    request.punchid    = 'is'
    request.type.extend([surface_pb2.kSynopFix, surface_pb2.kAirportRegType, surface_pb2.kAerodrome, surface_pb2.kAeroFix])
    request.as_proto   = proto
    request.meteotablo = True
    request.station_type.append(self.station_type)
    return request
  
  # 
  # Запрашиваем данные зондирования
  # 
  def makeAeroRequest(self, station, date):
    request            = surface_pb2.DataRequest()
    request.type.append(surface_pb2.kAeroFix)
    request.type_level = 1
    if station is not False:
      request.station.append(str(station.station))
      request.station_type.append(self.station_type)
    request.date_start = date.strftime("%Y-%m-%dT%H:%M:%S")
    request.date_end   = date.strftime("%Y-%m-%dT%H:%M:%S")
    request.only_last  = False
    return request
  
  # 
  # Запрашиваем поля
  # 
  def makeFieldRequest(self, date, descs):
    request                  = field_pb2.DataRequest()
    coords                   = surface_pb2.Point()
    coords.fi                = self.lat_radian
    coords.la                = self.lon_radian
    coords.height            = self.alt
    request.coords.extend([coords])
    request.date_start       = date.strftime("%Y-%m-%dT%H:%M:%S")
    # request.forecast_end   = date.strftime("%Y-%m-%dT%H:%M:%S")
    request.only_best        = True
    request.only_last        = True
    
    # заполняем дескрипторами поля
    meteo_descr = [] 
    for _d in descs:
      _val = self.get_param_by_descrname(_d) 
      if _val != 0:
        meteo_descr.append(_val)

    request.meteo_descr.extend(meteo_descr)
    # request.descrname.extend([descs])
    request.type_level.append(1)
    request.level.append(0)
    # request.date_start = date_start.strftime("%Y-%m-%dT%H:%M:%SZ")
    return request

  # 
  # Делаем запрос на получение аэрологии из полей
  # 
  def makeFieldAeroRequest(self, date):
    request                = field_pb2.DataRequest()
    coords                 = surface_pb2.Point()
    coords.fi              = self.lat_radian
    coords.la              = self.lon_radian
    coords.height          = self.alt
    request.coords.extend([coords])
    request.forecast_start = date.strftime("%Y-%m-%dT%H:%M:%S")
    request.forecast_end   = date.strftime("%Y-%m-%dT%H:%M:%S")
    request.only_best      = True
    request.only_last      = True
    return request
  
  # 
  # Получаем атоматом значение для представления из табло
  # @item - параметр типа Meteotablo
  # @param - параметр, значение которого хотим достать
  # @val_type = "", "code", "val" - одно на выбор
  # 
  def getParamFromTablo(self, item, param, val_type=""):
    try:
      fnn =  getattr( item.tablo, param + val_type )
      return fnn
    except:
      # logging.error("Error to find param %s" % (param+val_type) )
      return ""
  
  # 
  # Получаем значение из массива параметров для meteodata_proto
  # 
  def getParamVal(self, item, param, val_type="value"):
    for _param in item.param:
      if _param.descrname==param:
        return _param.value if val_type=="value" else _param.code
    return ""
  
  # 
  # Вытаскиваем значние параметра из OnePointData 
  # это поля метеовеличин
  # вытаскиваем по descrname (P, C, N, ...)
  # 
  def get_field_value_by_descrname(self, data, descrname):
    # получаем значение параметра из массива
    param = self.get_param_by_descrname(descrname)
    if param == 0:
      return False
    return self.get_field_value_by_param(data,param)
    
    
  # 
  # Вытаскиваем значние параметра из OnePointData 
  # это поля метеовеличин
  # вытаскиваем по meteoparam (20001, 20003 ...)
  # 
  def get_field_value_by_param(self, data, param):
    for item in data:
      if item.fdesc.meteodescr == param:
        return item.value
    return False


  # вычисляет влажность по заданным температуре и температуре росы
  def humidity(self, T, Td):
    if T < -10:
      a = 9.5
      b = 265.5
    else:
      a = 7.63
      b = 241.9
      
    tmp1 = (a * Td) / (b + Td)
    tmp2 = (a * T) / (b + T)
    tmp1 = tmp1 - tmp2
    tmp2 = 10 ** tmp1
    if tmp2 < 0:
      result = 0
    elif tmp2 > 1:
      result = 1
    else:
      result = tmp2
    return result * 100
