#!/usr/bin/env python
# -*- coding: utf-8 -*-
# системные модули
import os
import sys
from sys import argv
import logging

# конфигурационный модуль
from conf import *

sys.path.append(PRJ_DIR + '/lib')

# модули времени и даты
from datetime import timedelta
from datetime import datetime
import math
import time

# модули для работы с документами
from lpod3 import document
from lpod3.document import *
from lpod3 import const
from lpod3 import style
from lpod3 import element


# модули для работы с удаленным сервисом
from google.protobuf.service import RpcController
import surface_pb2
from services_pb2 import *
from surface_pb2 import *
from surface_service_pb2 import *
from document_service_pb2 import *
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
from libzondwrap import *
from libglobalwrap import *
from libweatherwrap import *
import librpcpywrap
import meteoglobal

# Qt-модули
from PyQt5.QtGui import *

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

  def __init__(self):
    self.descs = ['h', 'N', 'w', 'V', 'dd', 'ff', 'T', 'U', 'Nh', 'C', 'P', 'P0', 'PQNH']
    self.months = ['января', 'февраля', 'марта', 'апреля', 'мая', 'июня', 'июля', 'августа', 'сентября', 'октября',
                   'ноября', 'декабря']
    self.respond             = None
    self.meteoDataRespond    = []
    self.srcdataStub         = None
    self.doc                 = None
    self.ctrl                = ClientController()
    self.address             = serviceAddress(8)
    self.host                = librpcpywrap.hostByAddress(self.address)
    self.port                = librpcpywrap.portByAddress(self.address)
    self.proto               = None
    self.bulletin_date       = None
    self.result              = ExportResponse()
    self.result.result       = False
    self.stationPressureCalc = False
    self.oceanPressureCalc   = False
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
        meteodata.fromArrayObject(respond.meteodata[0])
        return meteodata
      else:
        return False

  def createDoc(self):
    self.result.result = False
    tries = 0
    connected = False
    while not connected and tries < 5:
      connected = self.ctrl.Connect(self.host, self.port)
      tries += 1
    if not connected:
      self.result.comment = u"Не подключились к сервису с данными Хост:{}. Порт:{}.".format(self.host,self.srcdata.port)
      return False
    else:
      self.srcdataStub = SurfaceService_Stub(self.ctrl.Channel())
    if not self.opendoc():
      self.result.comment = u"Не удалось открыть документ."
      return False
    if self.bulletin_date is None:
      self.result.comment = u"Нет даты"
      return False
    self.getFieldsDoc()
    self.fillDoc()
    self.saveDoc()
    self.result.result = True
    sys.stdout.buffer.write(self.result.SerializeToString())

  def opendoc(self):
    try:
      self.doc = document.odf_get_document(PRJ_DIR + "/share/meteo/odtsamples/av12.odt")
      return True
    except IOError:
      self.result.comment = u"Файл шаблона указан неверно"
      return False

  def parsInput(self, argv):
    parser = argparse.ArgumentParser(description=meteoglobal.trUtf8("Построение бюллетеня АВ-12"))
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
    parser.add_argument('--weatherArrival', '-p5', type=str,
                        help=meteoglobal.trUtf8("Орнитологическая обстановка по маршруту"))
    parser.add_argument('--birdsRoute', '-p6', type=str, help=meteoglobal.trUtf8("Прогноз в пункте посадки"))
    parser.add_argument('--birdsArrival', '-p7', type=str,
                        help=meteoglobal.trUtf8("Орнитологическая обстановка в пункте посадки"))
    parser.add_argument('--datestart', '-ds', type=str, help=meteoglobal.trUtf8("Дата вылета"))
    parser.add_argument('--timestart', '-ts', type=str, help=meteoglobal.trUtf8("Время вылета"))
    parser.add_argument('--timelanding', '-te', type=str, help=meteoglobal.trUtf8("Время посадки"))
    parser.add_argument('--typ', '-t', type=str, help=meteoglobal.trUtf8("Тип бюллетеня"))
    parser.add_argument('--timetype', '-tt', type=str, help=meteoglobal.trUtf8("Тип времени"))
    parser.add_argument('--executortype', '-et', type=str, help=meteoglobal.trUtf8("Кому адресован документ"))
    parser.add_argument('--echelon','-ec', type=int,help=meteoglobal.trUtf8("Эшелон полёта"))

    args = parser.parse_args()

    if None == args.inputType:
      # print(str("Отсутсвует тип источника входных данных").decode("utf-8"))
      self.result.comment = meteoglobal.trUtf8("Отсутсвует тип источника входных данных")
      return False

    if 'proto' == args.inputType:
      self.proto = av12script()
      self.proto.ParseFromString(meteoglobal.readStdinBytes())
      self.bulletin_date = datetime.strptime(self.proto.date, "%Y-%m-%d")
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
          station.type =  surface_pb2.StationType.Value(arrstation[2])
          station.date_time = arrstation[3]
          i = i + 1

  def getWeatherStation(self):
    # print("    Попробуем отправить запрос погоды")
    self.meteoDataRespond[:] = []
    i = 0
    while i < len(self.proto.stations):
      request = self.makeRequest(self.proto.stations[i])
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


  def getWindOnStation(self, station):
    request = self.makeWindRequest(station)
    respond = None
    self.respond = respond
    self.stStation = None
    respond = self.ctrl.RemoteCall(self.srcdataStub.GetTZondOnStation, request, 600000)
    if respond is None or respond == "" or str(respond) == "":
      if self.proto.findStation is True:
        stations = av12common.getNearStations(station, 3, self.proto.radiusFind)
        if not (stations is False or stations is None):
          i = 0
          while (i < len(stations.station) ) :
            station = stations.station[i].main.index
            request = self.makeWindRequest(station)
            respond = self.ctrl.RemoteCall(self.srcdataStub.GetTZondOnStation, request, 600000)
            if respond is None or respond == "" or str(respond) == "":
              self.respond = respond
            else:
              self.respond = respond
              self.stStation = av12common.getStation(station)
              return True
            i = i+1
      else:
        self.respond = None
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
    # request.type = kSynopFix
    request.type.append(1)
    request.type.append(61)
    request.type.append(62)
    request.type.append(86)
    request.type.append(83)
    request.type.append(71)
    request.type.append(101)
    request.type_level = 1
    request.station.append(bytes(station) )
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
    request = DataRequest()
    request.type.append(kAeroType)
    request.type.append(61)
    request.type.append(62)
    request.type.append(86)
    request.type.append(83)
    request.type.append(71)
    request.type.append(101)
    try:
      request.station.extend([str(station)])
    except:
      str_station = station.encode('utf-8')
      request.station.extend([str_station])


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
    # print("      Запрос данных с {} по {}".format(request.date_start,request.date_end))
    request.only_last = True
    return request



  def getFieldsDoc(self):
    # print("    Получение полей документа")
    self.body                     = self.doc.get_body()
    self.commander                = self.body.get_variable_set("commander")
    self.commander_rank           = self.body.get_variable_set("commander_rank")
    self.dejsinop                 = self.body.get_variable_set("dejsinop")
    self.billtype                 = self.body.get_variable_set("billtype")
    self.day                      = self.body.get_variable_sets("day")
    self.year                     = self.body.get_variable_sets("year")
    self.endpoint                 = self.body.get_variable_set("endpoint")
    self.forecastlandingtimebegin = self.body.get_variable_set("forecastlandingtimebegin")
    self.forecastlandingtimeend   = self.body.get_variable_set("forecastlandingtimeend")
    self.forecasttime             = self.body.get_variable_set("forecasttime")
    self.pressure                 = self.body.get_variable_set("pressure")
    self.minpressure              = self.body.get_variable_set("minpressure")
    self.month                    = self.body.get_variable_sets("month")
    self.route                    = self.body.get_variable_sets("route")
    self.validtimebegin           = self.body.get_variable_set("validtimebegin")
    self.validtimeend             = self.body.get_variable_set("validtimeend")
    self.vhnumber                 = self.body.get_variable_sets("vhnumber")
    self.punkt4                   = self.body.get_variable_set("punkt4")
    self.punkt5                   = self.body.get_variable_set("punkt5")
    self.punkt6                   = self.body.get_variable_set("punkt6")
    self.punkt7                   = self.body.get_variable_set("punkt7")
    self.billnum                  = self.body.get_variable_set("bill_num")
    self.route_type               = self.body.get_variable_sets("route_type")
    self.executor                 = self.body.get_variable_sets("executor")
    self.executor_name            = self.body.get_variable_sets("executor_name")
    self.executor_rank            = self.body.get_variable_sets("executor_rank")
    self.dop_date                 = self.body.get_variable_set('dop_date')
    self.dop_executor             = self.body.get_variable_set('dop_executor')
    self.dop_vhnumber             = self.body.get_variable_set('dop_vhnumber')
    self.dop_rank                 = self.body.get_variable_set('dop_rank')
    self.dop_name                 = self.body.get_variable_set('dop_name')
    self.dop_anno                 = self.body.get_variable_set('dop_anno')
    styles                        = self.doc.get_styles(family='master-page')
    for var in styles :
      self.colon = var.get_variable_set('colon')


  def deleteDopSign(self):
    self.dop_date.delete()
    self.dop_executor.delete()
    self.dop_vhnumber.delete()
    self.dop_rank.delete()
    self.dop_name.delete()
    self.dop_anno.delete()

  def inputImage(self):
    # Вставка картинки
    vbuilder = VProfiler()
    # logging.error(self.proto.SerializeToString())
    vbuilder.setSerialProto(self.proto.SerializeToString())
    vbuilder.initFromProto()
    arr      = vbuilder.createVProfile()
    fh       = tempfile.NamedTemporaryFile()
    nameFile = fh.name + ".png"
    fh       = open(nameFile, "wb")
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
      self.executor[i].set_text( meteoglobal.trUtf8( self.proto.reciver) )
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
        self.month[i].set_text(meteoglobal.trUtf8(self.months[self.bulletin_date.month - 1]))
        self.year[i].set_text(str(self.bulletin_date.year))
        i = i + 1
    self.endpoint.set_text(endPoint)
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
      arrTimelanding[0] = str(int(arrTimelanding[0])-24)
    if int(arrTimelanding[0]) == 24:
      arrTimelanding[0] = str("00")
    buf = arrTimelanding[0] + ":" + arrTimelanding[1]
    self.forecasttime.set_text(meteoglobal.trUtf8(buf))
    bufStart = arrTimeBegin[0] + ":" + arrTimeBegin[1]
    self.forecastlandingtimebegin.set_text(meteoglobal.trUtf8(bufStart))
    self.forecastlandingtimeend.set_text(meteoglobal.trUtf8(buf))
    i = 0
    while i < len(self.route):
      self.route[i].set_text(route)
      i = i + 1
    arrTimelanding = self.proto.timeEnd.split(":")
    self.validtimebegin.set_text(meteoglobal.trUtf8(arrTimeBegin[0] + ":" + arrTimeBegin[1]))
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
    self.punkt5.set_text(self.proto.birdsRoute)
    self.punkt6.set_text(self.proto.weatherArrival)
    self.punkt7.set_text(self.proto.birdsArrival)
    self.commander.set_text(self.proto.commander)
    self.commander_rank.set_text(self.proto.commanderRank)
    if self.proto.dopSign == True:
      self.dop_executor.set_text(self.proto.dopReciver)
      self.dop_vhnumber.set_text(self.proto.number)
      self.dop_rank.set_text(self.proto.dopReciverRank)
      self.dop_name.set_text(self.proto.dopReciverName)
    else:
      self.deleteDopSign()
    buf = self.proto.synoptic.split(" ")
    dejsinop = buf[0] + "                  "
    i = 1
    while i < len(buf):
      dejsinop = dejsinop + " " + buf[i]
      i = i + 1
    self.dejsinop.set_text(dejsinop)
    if self.getWeatherStation():
      self.fillfactweathertable()
    self.fillfactwindtable()
    self.inputImage()

  def fillFactWeatherTableString(self, number, meteodata):
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
    if meteodata.hasParam('h'):
      cell = self.factweather.get_cell('D' + number)
      param = int(meteodata.meteoParam('h').value())
      #value = libpuansonwrap.stringFromRuleValue('h', param)
      value = param
      cell.set_text(str(value))
      self.factweather.set_cell('D' + number, cell)
    cell = self.factweather.get_cell('L' + number)
    param = meteodata.meteoParam('U').value()
    value = libpuansonwrap.stringFromRuleValue('U', param)
    cell.set_text(str(value))
    self.factweather.set_cell('L' + number, cell)
    cell = self.factweather.get_cell('K' + number)
    param = int(meteodata.meteoParam('T').value())
    value = libpuansonwrap.stringFromRuleValue('T', param)
    cell.set_text(str(value))
    self.factweather.set_cell('K' + number, cell)
    cell = self.factweather.get_cell('J' + number)
    param = meteodata.meteoParam('ff').value()
    value = libpuansonwrap.stringFromRuleValue('ff', param)
    try:
      cell.set_text(str(value))
    except:
      cell.set_text(meteoglobal.trUtf8(value))
    self.factweather.set_cell('J' + number, cell)
    cell = self.factweather.get_cell('I' + number)
    param = int(meteodata.meteoParam('dd').value())
    value = libpuansonwrap.stringFromRuleValue('dd', param)
    try:
      cell.set_text(str(value))
    except:
      cell.set_text(meteoglobal.trUtf8(value))
    self.factweather.set_cell('I' + number, cell)
    cell = self.factweather.get_cell('G' + number)
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
      value = value
    cell.set_text(str(value))
    self.factweather.set_cell('G' + number, cell)
    cell = self.factweather.get_cell('F' + number)
    param = int(meteodata.meteoParam('w').value())
    value = libpuansonwrap.stringFromRuleValue('w', param)
    cell.set_text(meteoglobal.trUtf8(value))
    self.factweather.set_cell('F' + number, cell)
    cell = self.factweather.get_cell('C' + number)
    value = None
    value2 = None
    value3 = ""
    if meteodata.hasParam('N'):
      param = int(meteodata.meteoParam('N').value())
      value = libpuansonwrap.stringFromRuleValue('N', param)
    if meteodata.hasParam('Nh'):  
      param = int(meteodata.meteoParam('Nh').value())
      value2 = libpuansonwrap.stringFromRuleValue('Nh', param)
    if meteodata.hasParam('CH') or meteodata.hasParam('CM') or meteodata.hasParam('CL') or meteodata.hasParam('Cn'):
        descrs = ['CH', 'CM', 'CL', 'Cn']
        for descr in descrs:
            if meteodata.hasParam(descr):
                param = int(meteodata.meteoParam(descr).value())
                string = libpuansonwrap.stringFromRuleValue(descr, param)
                value3 = value3 + string
    if value and value2:
      value = value + '/' + value2
    elif not value:
      value = value2
    if value3:
        value = value + ' ' + value3
    cell.set_text(meteoglobal.trUtf8(value))
    self.factweather.set_cell('C' + number, cell)

  def fillfactweathertable(self):
    # print("    Заполняем таблицу погоды")
    stations = []
    i = 0
    while i < len(self.proto.stations):
      if self.proto.stations[i].av12type == kDeparture:
        stations.append(self.proto.stations[i].index)
      if self.proto.stations[i].av12type == kArrival:
        stations.append(self.proto.stations[i].index)
      i = i + 1
    flag = False
    if 1 < len(stations):
      if stations[0] == stations[1]:
        flag = True
    self.factweather = self.body.get_table(name='factweather')
    if self.factweather is None:
      self.result.comment = u'Таблицы нет'
      #print("    Таблицы нет")
      return False
    mdList = []
    mdDopList = []
    i = 0
    index = None
    while i < len(self.proto.stations):
      if self.proto.stations[i].av12type == kDeparture:
        index = self.proto.stations[i].index
        break
      i = i + 1
    findP = False
    value = None
    i = 0
    while i < len(self.meteoDataRespond):
      meteodata = libmeteodatawrap.TMeteoData()
      meteodata.fromArrayObject(self.meteoDataRespond[i])
      mdList.append(meteodata)
      i = i + 1
    i = 0
    count = 3
    while i < len(self.proto.stations):
      if self.proto.stations[i].av12type == kArrival:
        if flag:
          i = i + 1
          continue
      find = False
      j = 0
      number = str(count)
      while j < len(mdList):
        try:
          proto_station = str(self.proto.stations[i].index)
        except:
          proto_station = str(self.proto.stations[i].index.encode('utf-8'))
        if proto_station == str(mdList[j].getStation()):
          find = True
          cell = self.factweather.get_cell('A' + number)
          cell.set_text(self.proto.stations[i].name)
          self.factweather.set_cell('A' + number, cell)
          self.fillFactWeatherTableString(number, mdList[j])
          count = count + 1
        if find:
          break
        else:
          j = j + 1
      if not find:
        if self.proto.findStation is True:
          station = av12common.getNearStation(self.proto.stations[i].index, 3, self.proto.radiusFind)
          if station is not False:
            mData = self.getMeteoDataForStation(station)
            if mData is not False:
              mdDopList.append(mData)
              stStation = av12common.getStation(station)
              if stStation.name.rus:
                cell = self.factweather.get_cell('A' + number)
                cell.set_text(meteoglobal.trUtf8(stStation.name.rus))
                self.factweather.set_cell('A' + number, cell)
              else:
                cell = self.factweather.get_cell('A' + number)
                cell.set_text(meteoglobal.trUtf8(stStation.name.international))
                self.factweather.set_cell('A' + number, cell)
              self.fillFactWeatherTableString(number, mData)
              count = count + 1
              if str(index) == str(self.proto.stations[i].index):
                value = self.getStationPressure(mData)
                if value is not None:
                  value = str(round(value, 1))
                  findP = True

      i = i + 1
    i = 0
    while i < len(mdDopList):
      mdList.append(mdDopList[i])
      i = i + 1
    if not findP:
      value = "N/A"
      i = 0
      while i < len(mdList):
        if meteoglobal.trUtf8(index) == meteoglobal.trUtf8(mdList[i].getStation()):
          value = self.getStationPressure(mdList[i])
          if value is None:
            value = "N/A"
          else:
            value = str(round(value, 1))
          break
        i = i + 1
    if self.stationPressureCalc:
      value = value+"*"
    self.pressure.set_text(meteoglobal.trUtf8(value))

    minPressure = self.getMinPressure(mdList)
    if minPressure is None:
      minPressure = "N/A"
    if self.oceanPressureCalc:
      minPressure = minPressure + "*"
    self.minpressure.set_text(meteoglobal.trUtf8(minPressure))
    self.makeColon()


  def makeColon(self):
    if self.oceanPressureCalc or self.stationPressureCalc:
      colon = str("* - значение рассчитано")
      self.colon.set_text(meteoglobal.trUtf8(colon))
    else:
      colon = str(".")
      self.colon.set_text(meteoglobal.trUtf8(colon)) 

  def getStationPressure(self, mData):
    pressure = mData.meteoParam('P0').value()
    if pressure is None or pressure <= 0:
      h = mData.meteoParam('h0_station').value()
      T = mData.meteoParam('T').value()
      P0 = mData.meteoParam('PQNH').value()
      if P0 is None or P0 <= 0:
        P0 = mData.meteoParam('P').value()
      pressure = pressureOnStationLevel(h, P0, T)
      self.stationPressureCalc = True
    pressure = pressure * DocCreator.gPa2mm
    if pressure <= 0 or type(pressure)  is not float:
      return None
    return pressure

  def getMinPressure(self, mdList):
    minPressure = None
    pressures = []
    i = 0
    while i < len(mdList):
      p = mdList[i].meteoParam('P').value() * DocCreator.gPa2mm
      if p <= 0 or type(p) is not float:
        p = mdList[i].meteoParam('PQNH').value() * DocCreator.gPa2mm
        if p <= 0 or type(p) is not float:
          # p = self.getOceanPressure(mdList[i])
          h = mdList[i].meteoParam('h0_station').value()
          P = mdList[i].meteoParam('P0').value()
          T = mdList[i].meteoParam('T').value() + 273
          p = pressureOnOceanLevel( h, P, T) * DocCreator.gPa2mm
          self.oceanPressureCalc = True
          if p <= 0 or type(p) is not float:
            i = i + 1
            continue
      pressures.append(p)
      i = i + 1
    if 0 < len(pressures):
      minPressure = str(round(min(pressures), 1))
    return minPressure

  def fillfactwindtable(self):
    self.factwind = self.body.get_table(name='factwind')
    i = 0
    line = 0
    if len(self.proto.factWindList) > 2:
      row = self.factwind.get_rows()
      row = row[3]
      for i in range(len(self.proto.factWindList)-2) :
        self.factwind.append_row(row)
    i = 0
    while i < len(self.proto.factWindList):
      self.stStation = None
      if not self.getWindOnStation( self.proto.factWindList[i].index ):
        i = i + 1
        continue
      else:
        self.zond = Zond()
        if len(self.respond.data) < 1:
          self.result.comment = u'Ошибка. Нет данных.'
          i = i + 1
          continue
        
        # self.zond.fromArray(self.respond.data[0].meteodata, sys.getsizeof(self.respond.data[0].meteodata))
        # data = str(self.respond.data[0].meteodata)
        self.zond.fromArrayObject(self.respond.data[0].meteodata)

        number = str(3 + line)
        line = line + 1
        cell = self.factwind.get_cell('A' + number)
        if self.stStation != None:
          cell.set_text(meteoglobal.trUtf8(self.stStation.name.rus))
        else:
          cell.set_text( meteoglobal.trUtf8( self.proto.factWindList[i].name ) )
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
        heights = [1500, 3000, 5500, 7000, 9000, 10500]
        columns = ['C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N']
        k = 0
        l = 0
        while k < len(heights):
          level = Uroven()
          self.zond.getUrPoH(heights[k], level)
          if level.isGood(ValueType.UR_dd) :
            value = str(level.value(ValueType.UR_dd))
            cell = self.factwind.get_cell(columns[l] + number)
            cell.set_text(meteoglobal.trUtf8(value))
          else :
            cell.set_text('-')
          self.factwind.set_cell(columns[l] + number, cell)
          if level.isGood(ValueType.UR_ff):
            value = str(level.value(ValueType.UR_ff))
            cell = self.factwind.get_cell(columns[l + 1] + number)
            cell.set_text(meteoglobal.trUtf8(value))
          else:
            cell.set_text('-')
          self.factwind.set_cell(columns[l + 1] + number, cell)
          k = k + 1
          l = l + 2
        i = i + 1

  def saveDoc(self):
    # output  = PRJ_DIR + '/TEST.odt'
    fileName = '/av12_counter' + str(self.proto.type);
    meteodir = os.path.expanduser('~/.meteo/')
    cur_month = datetime.now().month
    if self.proto.bullNumber != 0 :
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
      self.result.datetime = str(datetime.now().strftime("%Y-%m-%dT%H:%M:%S"))
      try:
        self.result.filename = str(u'AB12_' + station_string + datetime.now().strftime("%Y-%m-%dT%H:%M:%S") + '.odt')
      except:
        self.result.filename = str(u'AB12_' + datetime.now().strftime("%Y-%m-%dT%H:%M:%S") + '.odt')
      self.doc.save(target=outputs, pretty=True)
      file = open(outputs, 'rb')
      self.result.data = base64.b64encode(file.read())
      return True
    except IOError as ioe:
      self.result.comment = u'Ошибка записи в файл'
      return False


def main(argv):
  # Qt-приложение
  app2 = QGuiApplication(sys.argv)

  creator = DocCreator()
  creator.parsInput(argv)
  creator.createDoc()



if __name__ == "__main__":
  main(argv)
