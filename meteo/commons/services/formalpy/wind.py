#!/usr/bin/python3
# coding: utf-8

#системные модули
import os
import sys
from sys import argv

#конфигурационный модуль
from conf import *
sys.path.append(PRJ_DIR + '/lib')

#модули времени и даты
import datetime
import time

#модули для работы с документами
from lpod3 import document
from lpod3.paragraph import odf_create_paragraph

#модули для работы с удаленным сервисом
from google.protobuf.service import RpcController

from meteo_channel import *
from meteo_controller import *

from forecast_pb2 import *
from surface_pb2 import *

#модули оболочек C++ классов Puanson, TMeteoData
import libpuansonwrap
from libpuansonwrap import *
import libmeteodatawrap
from libmeteodatawrap import *
from libzondwrap import *


#Qt-модули
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

from sprinf_pb2 import *


class WindGenerator():
  def __init__(self):
    self.isobara = [1000, 925, 850, 700, 500, 400, 300, 200, 100, 50]
    self.hasData = False


  def init(self, argv):
    self.arg_date = datetime.datetime.strptime(argv[2], "%Y-%m-%dT%H:%M:%S")
    self.station = argv[1]
    self.period = 2
    self.host = productsettings.host()
    self.forecastdata = productsettings.service(productsettings.kForecastData)
    # загрузка шаблона пуансона
    libpuansonwrap.loadPunch("is")


  def generate(self):
    if not self.openTemplate():
      return False
    else:
      self.fillDoc()
    if self.connect():
      if self.request():
        self.hasData = True
        self.formBulletin()
    if self.save():
      return True
    else:
      return False


  def fillDoc(self):
    self.body = self.doc.get_body()
    station_var = self.body.get_variable_set("station")
    date_var = self.body.get_variable_set("date")
    station_var.set_text(self.getStationName())
    date_var.set_text(self.arg_date.strftime("%Y-%m-%d %H:%M:%S"))


  def formBulletin(self):
    wind = self.body.get_table(name="Wind")
    fstCellL = 'C'
    fstCellI = 3
    i = 0
    while i < self.isobara.__len__():
      ur = Uroven()
      self.zond.getUrPoP(self.isobara[i], ur)
      dd = wind.get_cell(fstCellL + str(fstCellI + i))
      ff = wind.get_cell(chr(ord(fstCellL) + 1) + str(fstCellI + i))
      if ur.isGood(ValueType.UR_dd):
        ugol = libpuansonwrap.stringFromRuleValue('dd', ur.value(ValueType.UR_dd))
      else:
        ugol = '-'
      dd.set_text(ugol)
      if ur.isGood(ValueType.UR_ff):
        skorost = libpuansonwrap.stringFromRuleValue('ff', ur.value(ValueType.UR_ff)*3,6)
      else:
        skorost = '-'
      ff.set_text(skorost)
      wind.set_cell(fstCellL + str(fstCellI + i), dd)
      wind.set_cell(chr(ord(fstCellL) + 1) + str(fstCellI + i), ff)
      i += 1


  def save(self):
    if not os.path.exists(PRJ_DIR + '/var/meteo/documents/' + self.arg_date.strftime("%Y-%m-%d")):
      os.makedirs(PRJ_DIR + '/var/meteo/documents/' + self.arg_date.strftime("%Y-%m-%d"))
    output = os.path.join(PRJ_DIR + '/var/meteo/documents/' + self.arg_date.strftime("%Y-%m-%d"),
                          u'ForecastWind_' + argv[1] + '_' + self.arg_date.strftime("%Y-%m-%dT%H:%M:%S") + '.odt')
    print output
    print self.arg_date
    try:
      self.doc.save(target=output, pretty=True)
      return True
    except IOError as ioe:
      print("Ошибка записи в файл")
      return False


  def getStationName(self):
    sprinf = productsettings.service(productsettings.kSprinf)
    ctrl = ClientController()
    if (False == ctrl.Connect(self.host, sprinf.port)):
      print("Не удалось подключиться к сервису справочной информации")
      return self.station
    srv = Service_Stub(ctrl.Channel())
    req = MultiStatementRequest()
    req.index.append(int(self.station))
    resp = ctrl.RemoteCall(srv.GetStations, req, 30000)
    if not resp.station:
      station_name = self.station
    else:
      station_name = resp.station[0].name
      if station_name.rus:
        station_name = station_name.rus
      elif station_name.international:
        station_name = station_name.international
      elif station_name.short:
        station_name = station_name.short
      else:
        station_name = self.station
    return station_name


  def openTemplate(self):
    try:
      self.doc = document.odf_get_document(PRJ_DIR + "/share/meteo/odtsamples/ForecastWind.odt")
      return True
    except IOError:
      print("Файл шаблона указан неверно")
      return False


  def connect(self):
    self.ctrl = ClientController()
    if (False == self.ctrl.Connect(self.host, self.forecastdata.port)):
      print("Не удалось подключиться к сервису прогностических данных")
      return False
    return True


  def request(self):
    srv            = ForecastData_Stub(self.ctrl.Channel())
    forecast_time  = self.arg_date + datetime.timedelta(hours=int(self.period))
    req            = DataRequest()
    req.date_start = forecast_time.strftime("%Y-%m-%dT%H:%M:%S")
    req.date_end   = forecast_time.strftime("%Y-%m-%dT%H:%M:%S")
    req.station.append(self.station)
    resp = self.ctrl.RemoteCall(srv.GetMeteoData, req, 30000)
    if not resp:
      print "Ответ от сервиса не получен"
      return False
    if not resp.meteodata:
      print "Данных нет"
      return False
    for data in resp.meteodata:
      meteodata = TMeteoData()
      data      = str(data)
      meteodata.fromArray(data, sys.getsizeof(data))
      if meteodata.meteoParam('category').value() == 61:
        break
    self.zond = Zond()
    self.zond.setDataNoPreobr(meteodata)
    return True


def main(argv):
  #Qt-приложение
  app = QCoreApplication (sys.argv)
  generator = WindGenerator()
  generator.init(argv)
  if not generator.generate():
    print "Генерация прогноза параметров ветра не успешна"
    sys.exit(1)
  if not generator.hasData:
    sys.exit(2)

if __name__ == "__main__":
  main(argv)
