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
from libglobalwrap import *
import librpcpywrap
#Qt-модули
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

from optparse import OptionParser
import argparse

from document_service_pb2 import *
from sprinf_pb2 import *

import base64


class ForecastGenerator():
  def __init__(self):
    self.hasData           = False
    self.address           = serviceAddress(13)
    self.host              = librpcpywrap.hostByAddress(self.address)
    self.port              = librpcpywrap.portByAddress(self.address)
    self.result            = ExportResponse()
    self.result.result     = False
    self.printFileToStdOut = False

  def resultProto(self):
    return self.result


  def init(self, options):
    if (None == options.datetime) or (None == options.index) \
        or (None == options.filepath) or (None == options.period):
      self.result.result = False
      # sys.stderr.write("no args")
      self.result.comment = u"Не заданы требуемые параметры"
      return False
    self.arg_date = datetime.datetime.strptime(options.datetime, "%Y-%m-%dT%H:%M:%S")
    self.station = options.index
    self.period = options.period
    self.filepath = options.filepath
    # загрузка шаблона пуансона
    libpuansonwrap.loadPunch(str("is"))
    return True


  def initFromProto(self, proto):
    if (None == proto.datetime) or (0 == proto.index.__len__()) or (None == proto.forecast):
      self.result.result = False
      # sys.stderr.write("no args")
      self.result.comment = u"Не заданы требуемые параметры"
      return False
    self.arg_date = datetime.datetime.strptime(proto.datetime, "%Y-%m-%dT%H:%M:%S")
    self.station = proto.index[0]
    self.period = proto.forecast
    if proto.filepath:
      self.filepath = proto.filepath
    else:
      self.filepath = PRJ_DIR + '/var/meteo/documents/'
    # загрузка шаблона пуансона
    libpuansonwrap.loadPunch(str("is"))
    self.printFileToStdOut = True
    return True


  def connect(self):
    self.ctrl = ClientController()
    if (False == self.ctrl.Connect(self.host, self.port)):
      self.result.comment = u"Не удалось подключиться к сервису прогностических данных"
      return False
    return True


  def request(self):
    srv = ForecastData_Stub(self.ctrl.Channel())
    forecast_time = self.arg_date + datetime.timedelta(hours=int(self.period))
    req = DataRequest()
    req.date_start = forecast_time.strftime("%Y-%m-%dT%H:%M:%S")
    req.date_end = forecast_time.strftime("%Y-%m-%dT%H:%M:%S")
    req.station.append(self.station)
    resp = self.ctrl.RemoteCall(srv.GetMeteoData, req, 30000)
    if not resp:
      # sys.stderr.write("no resp")
      self.result.comment = u"Ответ от сервиса не получен"
      return False
    if not resp.meteodata:
      # sys.stderr.write("no data")
      self.result.comment = u"Данных нет"
      return False
    for data in resp.meteodata:
      meteodata = TMeteoData()
      data = str(data)
      meteodata.fromArray(data, sys.getsizeof(data))
      if not (meteodata.meteoParam('category').value() == 61):
        self.meteodata = meteodata
        return True


  def formBulletin(self):
    forecast = self.body.get_table(name="Forecast")
    CH = ('CH', forecast.get_cell('C4'), 'C4')
    CM = ('CM', forecast.get_cell('D4'), 'D4')
    CL = ('CL', forecast.get_cell('E4'), 'E4')
    Nh = ('Nh', forecast.get_cell('C5'), 'C5')
    h = ('h', forecast.get_cell('C6'), 'C6')
    N = ('N', forecast.get_cell('C7'), 'C7')
    w = ('w', forecast.get_cell('C8'), 'C8')
    V = ('V', forecast.get_cell('C9'), 'C9')
    dd = ('dd', forecast.get_cell('C11'), 'C11')
    ff = ('ff', forecast.get_cell('C12'), 'C12')
    T = ('T', forecast.get_cell('C13'), 'C13')
    param_cells = [CH, CM, CL, Nh, h, N, w, V, dd, ff, T]
    for cell in param_cells:
      if self.meteodata.meteoParam(cell[0]).isInvalid() :
        str_data = '-'
      else: 
        str_data = libpuansonwrap.stringFromRuleValue(cell[0], self.meteodata.meteoParam(cell[0]).value())
      cell[1].set_text(str_data.decode('utf-8'))
      forecast.set_cell(cell[2], cell[1])


  def generate(self):
    if not self.openTemplate():
      # sys.stderr.write("no template")
      self.result.comment = u"Не удалось открыть файл шаблона"
      self.result.result = False
      return False
    else:
      self.fillDoc()
    if self.connect():
      if self.request():
        self.hasData = True
        self.formBulletin()
      else:
        self.hasData = False
        return False
    else:
      self.result.result = False
      return False
    if self.save():
      self.result.result = True
      return True
    else:
      self.result.result = False
      return False


  def save(self):
    output = str('/tmp/' + u'Forecast' + str(self.period) + '_' + self.station + '_' + self.arg_date.strftime(
                            "%Y-%m-%dT%H:%M:%S") + '.odt')
    try:
      self.doc.save(target=output, pretty=True)
      self.result.result = True
      self.result.filename = str(u'Forecast' + str(self.period) + '_' + self.getStationNumber(self.station) + '_' +
                                 self.arg_date.strftime("%Y-%m-%dT%H:%M:%S") + '.odt')
      self.result.datetime = self.arg_date.strftime("%Y-%m-%dT%H:%M:00")

      if self.printFileToStdOut:
        resultFile = open(output, 'rb')
        data = resultFile.read()
        resultFile.close()
        self.result.data = base64.b64encode(data)
        os.remove(output)
      return True
    except IOError as ioe:
      # print ioe
      # sys.stderr.write("cant save")
      self.result.comment = u"Ошибка записи в файл"
      return False


  def openTemplate(self):
    try:
      self.doc = document.odf_get_document(PRJ_DIR + "/share/meteo/odtsamples/Forecast.odt")
      return True
    except IOError:
      self.result.comment = u"Файл шаблона указан неверно"
      return False


  def fillDoc(self):
    self.body = self.doc.get_body()
    station_var = self.body.get_variable_set("station")
    period_var = self.body.get_variable_set("period")
    date_var = self.body.get_variable_set("date")
    station_var.set_text(self.getStationName())
    period_var.set_text(str(self.period))
    date_var.set_text(self.arg_date.strftime("%Y-%m-%d %H:%M:%S"))


  def getStationName(self):
    ctrl = ClientController()
    sprinf_address = serviceAddress(14)
    sprinf_host = librpcpywrap.hostByAddress(sprinf_address)
    sprinf_port = librpcpywrap.portByAddress(sprinf_address)

    if (False == ctrl.Connect(sprinf_host, sprinf_port)):
      # sys.stderr.write("no sprinf connection")
      self.result.comment = u"Не удалось подключиться к сервису справочной информации"
      return self.station
    srv = SprinfService_Stub(ctrl.Channel())
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


def main(argv):
  #Qt-приложение
  app = QCoreApplication (sys.argv)

  parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter,
                                   description="Сценарий формирования метеосводок")
  parser.add_argument('-m', '--mode', default='arg', help="Режим запуска сценария. "
                                                          "arg - Задание аргументов запуска "
                                                          "proto - Передача прото-структуры в стандартный поток")
  parser.add_argument('-f', '--filepath', default=PRJ_DIR + '/var/meteo/documents/',
                      help="Путь сохранения файла.")
  parser.add_argument('-i', '--index', help='Индекс станции')
  parser.add_argument('-d', '--datetime', help='Дата и время измерений в формате YYYY-MM-DDThh:mm:ss')
  parser.add_argument('-p', '--period', help='Срок прогноза (2/24 часа)')

  options = parser.parse_args()

  generator = ForecastGenerator()
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

  generator.generate()
  result = generator.resultProto()
  if True == result.result:
    # sys.stderr.write("success")
    result.comment = u'Успех'
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(0)
  else:
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(1)

if __name__ == "__main__":
  main(argv)
