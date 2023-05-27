#!/usr/bin/env python
# -*- coding: utf-8 -*-
# системные модули
import os
import sys
from sys import argv
import re

# конфигурационный модуль
from conf import *

sys.path.append(PRJ_DIR + '/lib')

# модули времени и даты
from datetime import timedelta
from datetime import datetime
import time

# модули для работы с документами
from lpod3 import document
from lpod3.document import *

# модули для работы с удаленным сервисом
from google.protobuf.service import RpcController
from meteo_controller import *
from sprinf_pb2 import *

# модуль для парсинга аргументов строки командной
import argparse
import base64

# модули оболочек C++ классов Puanson, TMeteoData, Zond
import libpuansonwrap
from libpuansonwrap import *
import libmeteodatawrap
from libmeteodatawrap import *
from libglobalwrap import *
from libweatherwrap import *
import libastrowrap as astro
import librpcpywrap
# import surface_pb2
from services_pb2 import *
from surface_service_pb2 import *

# Qt-модули
from PyQt5.QtGui import *

from surface_pb2 import *
import field_pb2 as field_pb2
import meteoglobal
from document_service_pb2 import *
from libzondwrap import *
import tempfile
import av12common


class DocCreator():
  gPa2mm = 7.5064 * 0.1
  billHead = {
    0: '(ВРП)',
    1: '(ПОЛЁТЫ)',
    2: '(ПЕРЕЛЁТЫ)',
    3: '(БОЕВОЕ ДЕЖУРСТВО)'
  }
  cloud = {
    0: 'Ci',
    1: 'Cc',
    2: 'Cs',
    3: 'Ac',
    4: 'As',
    5: 'Ns',
    6: 'Sc',
    7: 'St',
    8: 'Cu',
    9: 'Cb',
    # Облака верхнего яруса
    10: '',
    11: 'Ci',
    12: 'Ci',
    13: 'Ci',
    14: 'Ci',
    15: 'Cs',
    16: 'Cs',
    17: 'Cs',
    18: 'Cs',
    19: 'Cc',
    # Облака среднего яруса
    20: '',
    21: 'As',
    22: 'Ns',
    23: 'Ac',
    24: 'Ac',
    25: 'Ac',
    26: 'Ac',
    27: 'Ac',
    28: 'Ac',
    29: 'Ac',
    # Облака нижнего яруса
    30: '',
    31: 'Cu',
    32: 'Cu',
    33: 'Cb',
    34: 'Sc',
    35: 'Sc',
    36: 'St',
    37: 'St',
    38: 'CuSc',
    39: 'Cb',
    # Дополнительно
    59: meteoglobal.trUtf8("Не видно"),
    60: meteoglobal.trUtf8("Не видно"),
    61: meteoglobal.trUtf8("Не видно"),
    62.0: meteoglobal.trUtf8("Не видно"),
    99: meteoglobal.trUtf8("Ошибка")  # для ошибки
  }
  rain = [21, 23, 24, 25, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 123, 160, 161, 162, 163, 164, 165, 166, 167, 168]
  smoug = [10, 110]
  snow = [22, 26, 124, 170, 171, 172, 173, 283]
  mist = [28, 120, 130, 131, 132, 133, 134, 135]

  def __init__(self):
    self.descs = ['h', 'N', 'w', 'V', 'dd', 'ff', 'T', 'U', 'Nh', 'C', 'P', 'P0', 'PQNH']
    self.months = ['января', 'февраля', 'марта', 'апреля', 'мая', 'июня', 'июля', 'августа', 'сентября', 'октября',
                   'ноября', 'декабря']
    self.respond          = None
    self.meteoDataRespond = []
    self.srcdataStub      = None
    self.doc              = None
    self.ctrl             = ClientController()
    self.address          = serviceAddress(8)
    self.host             = librpcpywrap.hostByAddress(self.address)
    self.port             = librpcpywrap.portByAddress(self.address)
    self.proto            = None
    self.bulletin_date    = None
    self.fieldStub        = None
    self.result           = ExportResponse()
    self.result.result    = False
    # инициализация пуансона из файла шаблона
    libpuansonwrap.loadPunch(str("is"))
    # print("Cервис с данными Хост:{}. Порт:{}.".format(self.host, self.port))


  def getMeteoDataForStation(self, station):
    request = self.makeRequestForStation(station)
    if request is None:
      return False
    respond = self.ctrl.RemoteCall(self.srcdataStub.GetMeteoData, request, 30000)
    if (respond is None) or "" == respond or "" == str(respond):
      return False
    else:
      if len(respond.meteodata) > 0:
        meteodata = libmeteodatawrap.TMeteoData()
        # meteodata.fromArray(respond.meteodata[0], sys.getsizeof(respond.meteodata[0]))
        data = str(respond.meteodata[0])
        meteodata.fromArray(data, sys.getsizeof(data))
        return meteodata
      else:
        return False

  def createDoc(self):
    self.result.result = False
    tries = 0
    connected = False
    while not connected and tries < 5:
      # print("Попытка подключения к сервису доступа к раскодированным данным.")
      # connected = self.ctrl.Connect(self.host, self.srcdata.port)
      connected = self.ctrl.Connect(self.host, self.port)
      tries += 1
    if not connected:
      self.result.comment = u"Не подключились к сервису с данными Хост:{}. Порт:{}.".format(self.host,
                                                                                            self.srcdata.port)
      # print("Не подключились к сервису с данными Хост:{}. Порт:{}.".format(self.host,self.srcdata.port))
      # print("Не подключились к сервису с данными Хост:{}. Порт:{}.".format(self.host, self.port))
      return False
    else:
      # print("Подключились к сервису с данными. Хост:{}. Порт:{}.".format(self.host,self.srcdata.port))
      self.srcdataStub = SurfaceService_Stub(self.ctrl.Channel())
    # print("Формируем документ")
    if not self.opendoc():
      self.result.comment = u"Не удалось открыть документ."
      # print("Не удалось открыть документ.")
      return False
    if self.bulletin_date is None:
      self.result.comment = u"Нет даты"
      # print("Нет даты.")
      return False
    self.getFieldsDoc()
    self.fillDoc()
    self.saveDoc()
    self.result.result = True
    sys.stdout.buffer.write(self.result.SerializeToString())
    # print(self.result.SerializeToString())

  def opendoc(self):
    try:
      # открываем документ
      self.doc = document.odf_get_document(PRJ_DIR + "/share/meteo/odtsamples/av12d.odt")
      # print("  Получили документ")
      return True
    except IOError:
      self.result.comment = u"Файл шаблона указан неверно"
      # print("  Файл шаблона указан неверно")
      return False

  def parsInput(self, argv):
    parser = argparse.ArgumentParser(description=meteoglobal.trUtf8("Построение бюллетеня АВ-12-Д"))
    parser.add_argument('--inputType', '-m', type=str, help=meteoglobal.trUtf8(
      "Тип источника входных данных  proto = прото файл или arg = коммандная строка"))
    parser.add_argument('--stations', '-s', type=str, help=meteoglobal.trUtf8("Станции в одну строку. Пример "
                                                                              "\"индекс,имя,роль,2018-10-01T00:00:00;индекс,имя,роль,2018-10-01T06:00:00\""))
    parser.add_argument('--executorName', '-en', type=str,
                        help=meteoglobal.trUtf8("Руководитель полётами(Командир экипажа)"))
    parser.add_argument('--vhnumber', '-n', type=str, help=meteoglobal.trUtf8("Номер войсковой части"))
    parser.add_argument('--dejsinop', '-djs', type=str, help=meteoglobal.trUtf8("Дежурный синоптик"))
    parser.add_argument('--commander', '-c', type=str, help=meteoglobal.trUtf8("Звание и ФИО командира части"))
    parser.add_argument('--weatherRoute', '-p4', type=str, help=meteoglobal.trUtf8("Прогноз погоды по маршруту"))
    parser.add_argument('--weatherArrival', '-p5', type=str, help=meteoglobal.trUtf8("Прогноз в пункте посадки"))
    # parser.add_argument('--birdsRoute', '-p6', type=str, help=meteoglobal.trUtf8("Орнитологическая обстановка по маршруту"))
    # parser.add_argument('--birdsArrival', '-p7', type=str,
    #                   help=meteoglobal.trUtf8("Орнитологическая обстановка в пункте посадки"))
    parser.add_argument('--datestart', '-ds', type=str, help=meteoglobal.trUtf8("Дата вылета"))
    parser.add_argument('--timestart', '-ts', type=str, help=meteoglobal.trUtf8("Время вылета"))
    parser.add_argument('--timelanding', '-te', type=str, help=meteoglobal.trUtf8("Время посадки"))
    parser.add_argument('--typ', '-t', type=str, help=meteoglobal.trUtf8("Тип бюллетеня"))
    parser.add_argument('--timetype', '-tt', type=str, help=meteoglobal.trUtf8("Тип времени"))
    parser.add_argument('--executortype', '-et', type=str, help=meteoglobal.trUtf8("Кому адресован документ"))
    parser.add_argument('--echelon', '-ec', type=int, help=meteoglobal.trUtf8("Эшелон полёта"))

    args = parser.parse_args()

    if None == args.inputType:
      # print(str("Отсутсвует тип источника входных данных").decode("utf-8"))
      self.result.comment = meteoglobal.trUtf8("Отсутсвует тип источника входных данных")
      return False

    if 'proto' == args.inputType:
      self.proto = av12script()
      self.proto.ParseFromString(meteoglobal.readStdinBytes())
      self.bulletin_date = datetime.strptime(self.proto.date, "%Y-%m-%d")
      # print("you typed this")
      # print(self.proto.__str__())
      # print(self.proto.commandor.encode("utf-8"))
      # print(self.proto.number.encode("utf-8"))
      # print(self.proto.date.encode("utf-8"))
    else:
      self.proto = av12script()
      if args.typ is not None:
        self.proto.type = typeBulleten.Value(args.typ)
      if args.executortype is not None:
        self.proto.reciver = typeReciver.Value(args.executortype)
      if args.executorName is not None:
        self.proto.reciverName = args.executorName
      if args.weatherRoute is not None:
        self.proto.weatherRoute = args.weatherRoute
      if args.weatherArrival is not None:
        self.proto.weatherArrival = args.weatherArrival
      if args.birdsRoute is not None:
        self.proto.birdsRoute = args.birdsRoute
      if args.birdsArrival is not None:
        self.proto.birdsArrival = args.birdsArrival
      if args.dejsinop is not None:
        self.proto.synoptic = args.dejsinop
      if args.datestart is not None:
        self.proto.date = args.datestart
        self.bulletin_date = datetime.strptime(self.proto.date, "%Y-%m-%d")
      if args.timestart is not None:
        self.proto.timeStart = args.timestart
      if args.timelanding is not None:
        self.proto.timeEnd = args.timelanding
      if args.commander is not None:
        self.proto.commander = args.commander
      if args.timetype is not None:
        self.proto.typeTime = typeTime.Value(args.timetype)
      if args.vhnumber is not None:
        self.proto.number = args.vhnumber
      if args.echelon is not None:
        self.proto.echelon = args.echelon
      if args.stations is not None:
        stations = args.stations.split(';')
        i = 0
        while i < len(stations):
          station = self.proto.stations.add()
          arrstation = stations[i].split(',')
          station.index = arrstation[0]
          station.name = arrstation[1]
          station.type = surface_pb2.StationType.Value(arrstation[2])
          station.date_time = arrstation[3]
          i = i + 1

  def getWeatherStation(self):
    # print("    Попробуем отправить запрос погоды")
    self.meteoDataRespond[:] = []
    stations = []
    i = 0
    while i < len(self.proto.stations):
      if self.proto.stations[i].av12type == kDeparture:
        stations.append(self.proto.stations[i])
      if self.proto.stations[i].av12type == kArrival:
        stations.append(self.proto.stations[i])
      i = i + 1
    i = 0
    while i < len(stations):
      request = self.makeRequest(stations[i])
      if request is None:
        i = i + 1
        continue
      respond = self.ctrl.RemoteCall(self.srcdataStub.GetMeteoData, request, 30000)
      if (respond is None) or "" == respond or "" == str(respond) or len(respond.meteodata) == 0:
        i = i + 1
        continue
      else:
        self.meteoDataRespond.append(respond.meteodata[0])
      i = i + 1
    return True

  def getFieldService(self):
    ctrlField = ClientController()
    address = serviceAddress(kField)
    host = librpcpywrap.hostByAddress(address)
    port = librpcpywrap.portByAddress(address)
    tries = 0
    connected = False
    while not connected and tries < 5:
      # print("Попытка подключения к сервису доступа к раскодированным данным.")
      # connected = self.ctrl.Connect(self.host, self.srcdata.port)
      connected = ctrlField.Connect(host, port)
      tries += 1
    if not connected:
      self.result.comment = u"Не подключились к сервису с данными Хост:{}. Порт:{}.".format(host, str(port))
      # print("Не подключились к сервису с данными Хост:{}. Порт:{}.".format(self.host,self.srcdata.port))
      # print("Не подключились к сервису с данными Хост:{}. Порт:{}.".format(self.host, self.port))
      return False
    else:
      # print("Подключились к сервису с данными. Хост:{}. Порт:{}.".format(self.host,self.srcdata.port))
      self.fieldStub = field_pb2.FieldService_Stub(ctrlField.Channel())
      return True

  def getWindOnStation(self, station):
    # print("    Попробуем отправить запрос ветра")
    request = self.makeWindRequest(station)
    respond = None
    self.respond = respond
    respond = self.ctrl.RemoteCall(self.fieldStub.GetADZond, request, 100000)
    if respond is None or respond == "" or str(respond) == "":
      respond = None
      if self.proto.findStation is True:
        station = av12common.getNearStation(station, 3, self.proto.radiusFind)
        request = self.makeWindRequest(station)
        respond = self.ctrl.RemoteCall(self.fieldStub.GetADZond, request, 100000)
        if respond is None or respond == "" or str(respond) == "":
          self.respond = respond
          return False
        self.respond = respond
        return True
      else:
        self.respond = respond
        return False
    else:
      self.respond = respond
      return True

  def makeRequest(self, station):
    request = DataRequest()
    request.station_type.append(station.type)
    request.type_level = 1
    request.station.append(station.index)
    if self.bulletin_date is None:
      return None
    date = self.bulletin_date
    request.meteo_descrname.extend(self.descs)
    dtbuf = self.proto.timeStart.split(":")
    hour_int = int(dtbuf[0]) - 12
    if self.proto.typeTime == kNoUTC:
      hour_int = hour_int - (self.proto.shiftTime/3600)
    if hour_int < 0:
      hour_int = 24 + hour_int
      date = date - timedelta(days=1)
    if hour_int < 10:
      hour = '0' + str(hour_int)
    else:
      hour = str(hour_int)
    request.date_start = date.strftime('%Y-%m-%d') + "T" + hour + ":" + dtbuf[1] + ":00"
    hour_int = int(dtbuf[0])
    date = self.bulletin_date
    if self.proto.typeTime == kNoUTC:
      hour_int = hour_int - (self.proto.shiftTime / 3600)
      if hour_int < 0 :
        hour_int = 24 + hour_int
        date = date - timedelta(days=1)
    if hour_int < 10:
      hour = '0' + str(hour_int)
      request.date_end = date.strftime('%Y-%m-%d') + "T" + hour + ":" + dtbuf[1] + ":00"
    else:
      request.date_end = date.strftime('%Y-%m-%d') + "T" + str(hour_int) + ":00"
    request.query_type = 2
    # print("      Запрос данных с {} по {}".format(request.date_start,request.date_end))
    return request

  def makeRequestForStation(self, station):
    if station is None:
      return None
    request = DataRequest()
    request.type.append(1)
    request.type.append(61)
    request.type.append(62)
    request.type.append(86)
    request.type.append(83)
    request.type.append(71)
    request.type.append(101)
    request.type_level = 1
    request.station.append(bytes(station))
    if self.bulletin_date is None:
      return None
    date = self.bulletin_date
    request.meteo_descrname.extend(self.descs)
    dtbuf = self.proto.timeStart.split(":")
    hour_int = int(dtbuf[0]) - 12
    if self.proto.typeTime == kNoUTC:
      hour_int = hour_int - (self.proto.shiftTime / 3600)
    if hour_int < 0:
      hour_int = 24 + hour_int
      date = date - timedelta(days=1)
    if hour_int < 10:
      hour = '0' + str(hour_int)
    else:
      hour = str(hour_int)
    request.date_start = date.strftime('%Y-%m-%d') + "T" + hour + ":" + dtbuf[1] + ":00"
    hour_int = int(dtbuf[0])
    date = self.bulletin_date
    if self.proto.typeTime == kNoUTC:
      hour_int = hour_int - (self.proto.shiftTime / 3600)
      if hour_int < 0 :
        hour_int = 24 + hour_int
        date = date - timedelta(days=1)
    if hour_int < 10:
      hour = '0' + str(hour_int)
      request.date_end = date.strftime('%Y-%m-%d') + "T" + hour + ":" + dtbuf[1] + ":00"
    else:
      request.date_end = date.strftime('%Y-%m-%d') + "T" + str(hour_int) + ":00"
    request.query_type = 2
    # print("      Запрос данных с {} по {}".format(request.date_start,request.date_end))
    return request

  def makeWindRequest(self, station):
    if len(self.proto.stations) == 0:
      self.result.comment = u'Станции не заданы'
      # print "    Станции не заданы"
      return 0
    coord = Point()
    if station is None:
      return False
    ctrl = ClientController()
    address = serviceAddress(kSprinf)
    host = librpcpywrap.hostByAddress(address)
    port = librpcpywrap.portByAddress(address)
    tries = 0
    connected = False
    while not connected and tries < 5:
      connected = ctrl.Connect(host, port)
      tries += 1
    if not connected:
      return False
    else:
      sprinfStub = SprinfService_Stub(ctrl.Channel())
    request = MultiStatementRequest()
    #result = re.match(r'[A-z,А-я]{1,4}', station)
    # if result:
    #   request.cccc.append(station)
    # else:
    #   request.index.append(int(station))
    try:
      request.index.append(int(station))
    except:
      request.cccc.append(station.encode('utf-8'))

    respond = ctrl.RemoteCall(sprinfStub.GetStations, request, 30000)
    if respond is not None and len(respond.station) > 0:
      coord.fi = respond.station[0].position.lat_radian
      coord.la = respond.station[0].position.lon_radian
      coord.height = respond.station[0].position.height_meters

    try:
      coord.index = str(station)
    except:
      str_station = station.encode('utf-8')
      coord.index = str_station
    request = field_pb2.DataRequest()
    request.coords.extend([coord])
    #request.center.append(4)
    date = self.bulletin_date
    dtbuf = self.proto.timeStart.split(":")
    hour_int = int(dtbuf[0]) - 12
    if self.proto.typeTime == kNoUTC:
      hour_int = hour_int - (self.proto.shiftTime / 3600)
    if hour_int < 0:
      hour_int = 24 + hour_int
      date = date - timedelta(days=1)
    if hour_int < 10:
      hour = '0' + str(hour_int)
    else:
      hour = str(hour_int)
    #request.date_start = date.strftime('%Y-%m-%d') + "T" + hour + ":" + dtbuf[1] + ":00"
    request.forecast_start = date.strftime('%Y-%m-%d') + "T" + hour + ":" + dtbuf[1] + ":00"
    hour_int = int(dtbuf[0])
    date = self.bulletin_date
    if self.proto.typeTime == kNoUTC:
      hour_int = hour_int - (self.proto.shiftTime / 3600)
      if hour_int < 0 :
        hour_int = 24 + hour_int
        date = date - timedelta(days=1)
    if hour_int < 10:
      hour = '0' + str(hour_int)
      #request.date_end = date.strftime('%Y-%m-%d') + "T" + hour + ":" + dtbuf[1] + ":00"
      request.forecast_end = date.strftime('%Y-%m-%d') + "T" + hour + ":" + dtbuf[1] + ":00"
    else:
      request.forecast_end = date.strftime('%Y-%m-%d') + "T" + str(hour_int) + ":00"
    # print("      Запрос данных с {} по {}".format(request.date_start,request.date_end))
    #request.meteo_descr.extend([20012, 20012, 20012, 20003, 20010, 20013, 20001, 13055, 11001, 11002, 12101, 13003, 10004,
    #                    13022, 12103])
    #request.level.extend([1000, 925, 850, 700, 500, 400, 300, 250, 200, 150, 100, 70, 50, 30, 20, 10, 5 , 0 ])
    #request.type_level.extend([100, 101,102,103])
    request.only_last = True
    request.only_best = True
    return request

  def getFieldsDoc(self):
    # print("    Получение полей документа")
    self.body = self.doc.get_body()
    self.billnum = self.body.get_variable_set("bill_num")
    self.billtype = self.body.get_variable_set("billtype")
    self.vhnumber = self.body.get_variable_sets("vhnumber")
    # self.commander = self.body.get_variable_set("commander")
    self.dejsinop = self.body.get_variable_set("dejsinop")
    self.route_type = self.body.get_variable_sets("route_type")
    self.route = self.body.get_variable_sets("route")
    self.executor = self.body.get_variable_sets("executor")
    self.executor_name = self.body.get_variable_sets("executor_name")
    self.executor_rank = self.body.get_variable_sets("executor_rank")
    self.day = self.body.get_variable_sets("day")
    self.month = self.body.get_variable_sets("month")
    self.year = self.body.get_variable_sets("year")
    self.validtimebegin = self.body.get_variable_set("validtimebegin")
    self.validtimeend = self.body.get_variable_set("validtimeend")
    # self.endpoint = self.body.get_variable_set("endpoint")
    self.forecastlandingtimebegin = self.body.get_variable_set("forecastlandingtimebegin")
    self.forecastlandingtimeend = self.body.get_variable_set("forecastlandingtimeend")
    # self.forecasttime = self.body.get_variable_set("forecasttime")
    self.punkt4 = self.body.get_variable_set("punkt4")
    self.punkt5 = self.body.get_variable_set("punkt5")
    self.time_now = self.body.get_variable_set("time_now")
    self.minute_now = self.body.get_variable_set("minute_now")

  def inputImage(self):
    # Вставка картинки
    vbuilder = VProfiler()
    vbuilder.setSerialProto(self.proto.SerializeToString())
    vbuilder.initAv12d()
    vbuilder.initFromProto()
    arr = vbuilder.createVProfile()
    fh = tempfile.NamedTemporaryFile()
    nameFile = fh.name + ".png"
    fh = open(nameFile, "wb")
    fh.write(arr)
    fh.close()
    self.frame = self.body.get_frame()
    self.doc.del_part(self.frame.get_image().get_url())
    self.frame.set_image(self.doc.add_file(nameFile))
    os.remove(nameFile)

  def fillDoc(self):
    # print("    Заполнение документа")
    startPoint = None
    endPoint = None
    i = 0
    while i < len(self.proto.stations):
      if self.proto.stations[i].av12type == kDeparture:
        startPoint = self.proto.stations[i].name
      if self.proto.stations[i].av12type == kArrival:
        endPoint = self.proto.stations[i].name
      i = i + 1
    if startPoint is None:
      startPoint = meteoglobal.trUtf8("Пункт вылета")
    if endPoint is None:
      endPoint = meteoglobal.trUtf8("Пункт посадки")
    if self.proto.type == kPerelet:
      route = startPoint + '-' + endPoint
    else:
      route = startPoint
      endPoint = startPoint

    try:
      buf = DocCreator.billHead[self.proto.type]
    except KeyError as e:
      # print('Ошибка при анализе типа бюллетеня')
      buf = "Заголовок"

    self.billtype.set_text(meteoglobal.trUtf8(buf))
    if self.proto.type == kPerelet:
      buf = "маршруту"
    else:
      buf = "району"
    i = 0
    while i < len(self.route_type):
      self.route_type[i].set_text(meteoglobal.trUtf8(buf))
      i = i + 1
    i = 0
    while i < len(self.executor):
      self.executor[i].set_text(meteoglobal.trUtf8(self.proto.reciver))
      i = i + 1
    i = 0
    while i < len(self.executor_name):
      self.executor_name[i].set_text(self.proto.reciverName)
      i = i + 1
    i = 0
    while i < len(self.executor_rank):
      self.executor_rank[i].set_text(self.proto.reciverRank)
      i = i + 1
    if self.bulletin_date is not None:
      i = 0
      while i < len(self.day):
        self.day[i].set_text(str(self.bulletin_date.day))
        i = i + 1
      i = 0
      while i < len(self.month):
        self.month[i].set_text(meteoglobal.trUtf8(self.months[self.bulletin_date.month - 1]))
        i = i + 1
      i = 0
      while i < len(self.year):
        self.year[i].set_text(str(self.bulletin_date.year))
        i = i + 1
    # self.endpoint.set_text(endPoint)
    arrTimeBegin = self.proto.timeStart.split(":")
    arrTimelanding = self.proto.timeEnd.split(":")
    bufTimeBegin = timedelta(hours=int(arrTimeBegin[0]), minutes=int(arrTimeBegin[1]))
    bufTimelanding = timedelta(hours=int(arrTimelanding[0]), minutes=int(arrTimelanding[1]))
    result = bufTimelanding - bufTimeBegin
    threehours = timedelta(hours=3)
    if result < threehours:
      arrTimelanding[0] = str(int(arrTimelanding[0]) + 1)
    else:
      arrTimelanding[0] = str(int(arrTimelanding[0]) + 2)
    if int(arrTimelanding[0]) > 24:
      arrTimelanding[0] = str(int(arrTimelanding[0]) - 24)
    if int(arrTimelanding[0]) == 24 :
      arrTimelanding[0] = str("00")
    bufStart = arrTimeBegin[0] + ":" + arrTimeBegin[1]
    buf = arrTimelanding[0] + ":" + arrTimelanding[1]
    self.forecastlandingtimebegin.set_text(meteoglobal.trUtf8(bufStart))
    self.forecastlandingtimeend.set_text(meteoglobal.trUtf8(buf))
    self.dejsinop.set_text(meteoglobal.trUtf8(self.proto.synoptic))
    i = 0
    while i < len(self.route):
      self.route[i].set_text(route)
      i = i + 1
    self.validtimebegin.set_text(meteoglobal.trUtf8(arrTimeBegin[0] + ":" + arrTimeBegin[1]))
    arrTimelanding = self.proto.timeEnd.split(":")
    if (kPerelet == self.proto.type or kPolet == self.proto.type) and (startPoint != endPoint or len(self.proto.stations) > 2):
      time = str(int(arrTimeBegin[0]) + 1)
      if int(time) > 24:
        time = str(int(time) - 24)
      if int(time) == 24:
        time = str("00")
      self.validtimeend.set_text(meteoglobal.trUtf8(time + ":" + arrTimelanding[1]))
    else :
      self.validtimeend.set_text(meteoglobal.trUtf8(arrTimelanding[0] + ":" + arrTimelanding[1]))
    i = 0
    while i < len(self.vhnumber):
      self.vhnumber[i].set_text(self.proto.number)
      i = i + 1
    self.punkt4.set_text(self.proto.weatherRoute)
    self.punkt5.set_text(self.proto.weatherArrival)

    if self.getWeatherStation():
      self.fillfactweathertable()
    self.fillforecastwindtable()
    self.inputImage()

  def fillAstroTable(self, station, number):
    if station is None:
      return False
    ctrl = ClientController()
    address = serviceAddress(kSprinf)
    host = librpcpywrap.hostByAddress(address)
    port = librpcpywrap.portByAddress(address)
    tries = 0
    connected = False
    while not connected and tries < 5:
      connected = ctrl.Connect(host, port)
      tries += 1
    if not connected:
      return False
    else:
      sprinfStub = SprinfService_Stub(ctrl.Channel())
    request = MultiStatementRequest()
    result = re.match(r'[A-z,А-я]{1,4}', str(station))
    if result:
      request.cccc.append(station)
    else:
      request.index.append(int(station))

    respond = ctrl.RemoteCall(sprinfStub.GetStations, request, 30000)

    if respond is not None and len(respond.station) > 0:
      astroData = astro.AstroWrap()
      astroData.setStation(respond.station[0].position.lat_radian, respond.station[0].position.lon_radian,
                           respond.station[0].position.height_meters)
      begin = str(self.proto.date + "T" + self.proto.timeStart + "Z")
      end = str(self.proto.date + "T" + self.proto.timeEnd + "Z")
      astroData.setDate(begin, end)
      astroData.calcPoint()

      cell = self.factweather.get_cell('L' + number)
      buf = astroData.getTwilightNauticalBegin()
      cell.set_text(buf)
      self.factweather.set_cell('L' + number, cell)

      cell = self.factweather.get_cell('M' + number)
      buf = astroData.getSunRise()
      cell.set_text(buf)
      self.factweather.set_cell('M' + number, cell)

      cell = self.factweather.get_cell('N' + number)
      buf = astroData.getSunSet()
      cell.set_text(buf)
      self.factweather.set_cell('N' + number, cell)

      cell = self.factweather.get_cell('O' + number)
      buf = astroData.getTwilightNauticalEnd()
      cell.set_text(buf)
      self.factweather.set_cell('O' + number, cell)

      cell = self.factweather.get_cell('P' + number)
      buf = astroData.getLunarRise()
      cell.set_text(buf)
      self.factweather.set_cell('P' + number, cell)

      cell = self.factweather.get_cell('Q' + number)
      buf = astroData.getLunarSet()
      cell.set_text(buf)
      self.factweather.set_cell('Q' + number, cell)

      cell = self.factweather.get_cell('R' + number)
      buf = astroData.getLunarQuartet()
      buf = meteoglobal.trUtf8(buf)
      cell.set_text(buf)
      self.factweather.set_cell('R' + number, cell)

  def fillFactWeatherTableString(self, number, meteodata):
    # время
    cell = self.factweather.get_cell('B' + number)
    buf = meteodata.getDateTime().split('T')
    buf = buf[1].split(':')
    if self.proto.typeTime == kNoUTC:
      timeBeg =  datetime.now().replace(hour=int(buf[0]), minute=int(buf[1]))
      delt = timedelta(seconds=self.proto.shiftTime)
      timeBeg = timeBeg + delt
      if timeBeg.hour  < 10 :
        buf[0] = '0' + str(timeBeg.hour)
      else:
        buf[0] =  str(timeBeg.hour)
      if timeBeg.minute < 10:
        buf[1] = '0' + str(timeBeg.minute)
      else:
        buf[1] = str(timeBeg.minute)
    cell.set_text(buf[0] + ':' + buf[1])
    self.factweather.set_cell('B' + number, cell)
    # облачность н.н.
    cell = self.factweather.get_cell('H' + number)
    param = int(meteodata.meteoParam('h').value())
    value = libpuansonwrap.stringFromRuleValue('h', param)
    cell.set_text(str(value))
    self.factweather.set_cell('H' + number, cell)
    # давление
    cell = self.factweather.get_cell('C' + number)
    param = meteodata.meteoParam('P0').value()
    value = libpuansonwrap.stringFromRuleValue('P0', param)
    cell.set_text(str(value))
    self.factweather.set_cell('C' + number, cell)
    # температура
    cell = self.factweather.get_cell('D' + number)
    param = int(meteodata.meteoParam('T').value())
    value = libpuansonwrap.stringFromRuleValue('T', param)
    cell.set_text(str(value))
    self.factweather.set_cell('D' + number, cell)
    # скорость ветра
    cell = self.factweather.get_cell('J' + number)
    param = meteodata.meteoParam('ff').value()
    value = libpuansonwrap.stringFromRuleValue('ff', param)
    try:
      cell.set_text(str(value))
    except:
      cell.set_text(meteoglobal.trUtf8(value))
    self.factweather.set_cell('J' + number, cell)
    # направление ветра
    cell = self.factweather.get_cell('I' + number)
    param = int(meteodata.meteoParam('dd').value())
    value = libpuansonwrap.stringFromRuleValue('dd', param)
    try:
      cell.set_text(str(value))
    except:
      cell.set_text(meteoglobal.trUtf8(value))
    self.factweather.set_cell('I' + number, cell)
    # видимость гор
    cell = self.factweather.get_cell('E' + number)
    param = int(meteodata.meteoParam('V').value())
    value = libpuansonwrap.stringFromRuleValue('V', param)
    try:
      value = int(value)
      if value > 2000:
        value = value / 1000
      if value < 4:
        value = round(value, 1)
      else:
        value = round(value)
    except BaseException as esc:
      # print("Ошибка записи в файл")
      value = value
    cell.set_text(str(value))
    self.factweather.set_cell('E' + number, cell)
    # явления погоды
    cell = self.factweather.get_cell('F' + number)
    param = int(meteodata.meteoParam('w').value())
    value = libpuansonwrap.stringFromRuleValue('w', param)
    cell.set_text(meteoglobal.trUtf8(value))
    self.factweather.set_cell('F' + number, cell)
    # облачность кол-во
    cell = self.factweather.get_cell('G' + number)
    param = int(meteodata.meteoParam('N').value())
    value = libpuansonwrap.stringFromRuleValue('N', param)
    param = int(meteodata.meteoParam('Nh').value())
    value2 = libpuansonwrap.stringFromRuleValue('Nh', param)

    # if value:
    #   value = int(round(int(value) / 10))
    # else:
    #   value = '-'
    # if value2:
    #   value2 = int(round(int(value2) / 8 * 10))
    # else:
    #   value2 = '-'
    if not value:
      value = '-'
    if not value2:
      value2 = '-'
    value3 = meteodata.meteoParam('C').value()
    if value3 < 0:
      value3 = 99
    # cell.set_text(str(value) + '/' + str(value2) + ' ' + DocCreator.cloud[value3])
    cell.set_text(meteoglobal.trUtf8(value) + '/' + meteoglobal.trUtf8(value2))
    self.factweather.set_cell('G' + number, cell)

  def fillfactweathertable(self):
    stations = []
    i = 0
    while i < len(self.proto.stations):
      if self.proto.stations[i].av12type == kDeparture:
        stations.append(self.proto.stations[i])
      if self.proto.stations[i].av12type == kArrival:
        stations.append(self.proto.stations[i])
      i = i + 1
    flag = False
    if 1 < len(stations):
      if stations[0].index == stations[1].index:
        flag = True
    self.factweather = self.body.get_table(name='factweather')
    if self.factweather is None:
      self.result.comment = u'Таблицы нет'
      return False
    mdList = []
    mdDopList = []
    i = 0
    index = None
    while i < len(stations):
      if stations[i].av12type == kDeparture:
        index = stations[i].index
        break
      i = i + 1
    i = 0
    while i < len(self.meteoDataRespond):
      meteodata = libmeteodatawrap.TMeteoData()
      data = str(self.meteoDataRespond[i])
      meteodata.fromArray(data, sys.getsizeof(data))
      mdList.append(meteodata)
      i = i + 1
    i = 0
    count = 3
    while i < len(stations):
      if stations[i].av12type == kArrival:
        if flag:
          i = i + 1
          continue
      number = str(count)
      cell = self.factweather.get_cell('A' + number)
      cell.set_text(stations[i].name)
      self.factweather.set_cell('A' + number, cell)
      find = False
      j = 0
      while j < len(mdList):
        try:
          proto_station = str(stations[i].index)
        except:
          proto_station = str(stations[i].index.encode('utf-8'))
        if proto_station == str(mdList[j].getStation()):
          find = True
          self.fillFactWeatherTableString(number, mdList[j])
          self.fillAstroTable(mdList[j].getStation(), number)
        if find:
          break
        else:
          j = j + 1
      if not find:
        if self.proto.findStation is True:
          station = av12common.getNearStation(stations[i].index, 3, self.proto.radiusFind)
          if station is not False:
            mData = self.getMeteoDataForStation(station)
            if mData is not False:
              mdDopList.append(mData)
              self.fillFactWeatherTableString(number, mData)
              self.fillAstroTable(station, number)
              if index is self.proto.stations[i].index:
                value = str(round(mData.meteoParam('P0').value() * DocCreator.gPa2mm, 1))
                findP = True

      count = count + 1
      i = i + 1

  def fillforecastwindtable(self):
    self.factwind = self.body.get_table(name='forecastweather')
    # print("    Заполняем таблицу ветра")
    # if self.factwind is None:
    # print("    Таблицы ветра нет")
    stations = []
    st_names = []
    i = 0
    while i < len(self.proto.stations):
      if self.proto.stations[i].av12type == kDeparture:
        stations.append(self.proto.stations[i].index)
        st_names.append(self.proto.stations[i].name)
      if self.proto.stations[i].av12type == kArrival:
        if self.proto.stations[i].index not in stations:
          stations.append(self.proto.stations[i].index)
          st_names.append(self.proto.stations[i].name)
      i = i + 1
    i = 0
    line = 0
    self.getFieldService()
    while i < len(stations):
      if not self.getWindOnStation(stations[i]):
        i = i + 1
        continue
      else:
        self.zond = Zond()
        if len(self.respond.data) < 1:
          self.result.comment = u'Ошибка. Нет данных.'
          # print("     Ошибка. Нет данных.")
          i = i + 1
          continue
        data = str(self.respond.data[0].meteodata)
        self.zond.fromArray(data, sys.getsizeof(data))
        #self.zond.show()
        # if not self.zond.haveData() :
        # print("Данных о ветре нет")
        number = str(4 + line)
        line = line + 1
        cell = self.factwind.get_cell('A' + number)
        cell.set_text(st_names[i])
        self.factwind.set_cell('A' + number, cell)
        arrDate = self.respond.data[0].date.split("T")
        arrDate = arrDate[1].split(":")
        if self.proto.typeTime == kNoUTC:
          timeBeg = datetime.now().replace(hour=int(arrDate[0]), minute=int(arrDate[1]))
          delt = timedelta(seconds=self.proto.shiftTime)
          timeBeg = timeBeg + delt
          if timeBeg.hour < 10:
            arrDate[0] = '0' + str(timeBeg.hour)
          else:
            arrDate[0] = str(timeBeg.hour)
          if timeBeg.minute < 10:
            arrDate[1] = '0' + str(timeBeg.minute)
          else:
            arrDate[1] = str(timeBeg.minute)
        cell = self.factwind.get_cell('B' + number)
        date = arrDate[0] + ":" + arrDate[1]
        cell.set_text(meteoglobal.trUtf8(date))
        self.factwind.set_cell('B' + number, cell)
        self.factwind.set_cell('O' + number, cell)

        level = Uroven()
        #self.zond.preobr()
        # self.zond.show()
        self.zond.getUrPoLevelType(101, level)  # результаты обанала с таким типом хранят значения на уровене моря
        # level.show()
        if level.isGood(ValueType.UR_P):
          value = level.value(ValueType.UR_P)
          value = round(value, 1)
          value = str(value)  
        else:
            value = "-"
            
        cell = self.factwind.get_cell('P' + number)
        cell.set_text(meteoglobal.trUtf8(value))
        self.factwind.set_cell('P' + number, cell)

        self.zond.getUrPoLevelType(103, level)  # московский центр передает темп с таким типом уровня
        if level.isGood(ValueType.UR_T):
          value = level.value(ValueType.UR_T)
          value = round(value, 1)
          value = str(value)
        else:
          value = "-"
        
        cell = self.factwind.get_cell('Q' + number)
        cell.set_text(meteoglobal.trUtf8(value))
        self.factwind.set_cell('Q' + number, cell)

        heights = [1500, 3000, 5500, 7000, 9000, 10500]
        columns = ['C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N']
        k = 0
        l = 0
        while k < len(heights):
          level = Uroven()
          self.zond.getUrPoH(heights[k], level)
          #level.show()
          cell = self.factwind.get_cell(columns[l] + number)
          if level.isGood(ValueType.UR_dd):
            value = str(level.value(ValueType.UR_dd))
            cell.set_text(meteoglobal.trUtf8(value))
          else:    
            cell.set_text('-')
          self.factwind.set_cell(columns[l] + number, cell)
          cell = self.factwind.get_cell(columns[l + 1] + number)
          if level.isGood(ValueType.UR_ff):
            value = str(level.value(ValueType.UR_ff))
            cell.set_text(meteoglobal.trUtf8(value))
          else:
            cell.set_text('-')
          self.factwind.set_cell(columns[l + 1] + number, cell)
          k = k + 1
          l = l + 2
        i = i + 1

  def saveDoc(self):
    # output  = PRJ_DIR + '/TEST.odt'
    fileName = '/av12d_counter' + str(self.proto.type)
    meteodir = os.path.expanduser('~/.meteo/')
    cur_month = datetime.now().month
    if self.proto.bullNumber != 0:
      number = self.proto.bullNumber
      if os.path.isfile(os.path.join(meteodir + fileName)):
        month = cur_month
        counter = open(os.path.join(meteodir + fileName), 'w')
        counter.write(str(number) + '\n')
        counter.write(str(month))
        counter.close()
      else:
        if not os.path.exists(meteodir):
          os.mkdir(meteodir)
        counter = open(os.path.join(meteodir + fileName), 'w+')
        month = cur_month
        counter.write(str(number) + '\n')
        counter.write(str(month))
        counter.close()
    else:
      if os.path.isfile(os.path.join(meteodir + fileName)):
        counter = open(os.path.join(meteodir + fileName), 'r')
        number = int(counter.readline())
        month = int(counter.readline())
        counter.close()
        if cur_month > month:
          month = cur_month
          number = 1
        else:
          number += 1
        counter = open(os.path.join(meteodir + fileName), 'w')
        counter.write(str(number) + '\n')
        counter.write(str(month))
        counter.close()
      else:
        if not os.path.exists(meteodir):
          os.mkdir(meteodir)
        counter = open(os.path.join(meteodir + fileName), 'w+')
        number = 1
        month = cur_month
        counter.write(str(number) + '\n')
        counter.write(str(month))
        counter.close()
    self.billnum.set_text(str(number))
    station_string = ''
    i = 0
    while i < len(self.proto.stations):
      station_string = station_string + self.proto.stations[i].index + '_'
      i = i + 1
    # сохраняем файл
    try:
      outputs = str('/tmp/' + u'AB12_' + station_string + datetime.now().strftime("%Y-%m-%dT%H:%M:%S") + '.odt')
    except:
      outputs = str('/tmp/' + u'AB12_' + datetime.now().strftime("%Y-%m-%dT%H:%M:%S") + '.odt')
    try:
      # print(datetime.now().strftime("%Y-%m-%dT%H:%M:%S"))  # выводится для сервиса генерации документов
      self.result.datetime = str(datetime.now().strftime("%Y-%m-%dT%H:%M:%S"))
      try:
        self.result.filename = str(u'AB12_' + station_string + datetime.now().strftime("%Y-%m-%dT%H:%M:%S") + '.odt')
      except:
        self.result.filename = str(u'AB12_' + datetime.now().strftime("%Y-%m-%dT%H:%M:%S") + '.odt')
      # print("{}".format(outputs))
      self.doc.save(target=outputs, pretty=True)
      file = open(outputs, 'rb')
      self.result.data = base64.b64encode(file.read())
      # os.remove(outputs)
      return True
    except IOError as ioe:
      self.result.comment = u'Ошибка записи в файл'
      # print("Ошибка записи в файл")
      return False


def main(argv):
  # Qt-приложение
  app2 = QGuiApplication(sys.argv)

  creator = DocCreator()
  creator.parsInput(argv)
  creator.createDoc()

  # if not generator.init(argv):
  ##print "Не удалось подключиться к сервису"
  # sys.exit(1)
  # if not (generator.generate()):
  # print "Ошибка в процессе генерации бюллетеня"
  # sys.exit(1)


if __name__ == "__main__":
  main(argv)
