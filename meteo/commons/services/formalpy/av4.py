#!/usr/bin/python3
# coding: utf-8

import os
import sys
from sys import argv
import datetime

from conf import *
sys.path.append(PRJ_DIR + '/lib')

from lpod3 import document

from meteo_controller import *

from document_service_pb2 import *
from sprinf_pb2 import *
from libglobalwrap import *
import librpcpywrap
import surface_pb2
import meteoglobal

import argparse
import base64

class StormGenerator():
  def __init__(self):
    self.months = ['января', 'февраля', 'марта', 'апреля', 'мая', 'июня', 'июля', 'августа', 'сентября', 'октября', 'ноября',
          'декабря']
    self.result = ExportResponse()
    self.result.result = False
    self.printFileToStdOut = False

  def generate(self):
    if not self.openTemplate():
      self.result.result = False
      return False
    self.fillVariables()
    if not self.saveStorm():
      self.result.result = False
      return False
    self.result.result = True
    return True

  def init(self, options):
    if (None == options.index) or (None == options.date_start) or (None == options.date_end) or (
      None == options.description):
      self.result.result = False
      self.result.comment = u"Не заданы требуемые параметры"
      return False
    self.dts           = datetime.datetime.strptime(options.date_start, "%Y-%m-%dT%H:%M:%S")
    self.dte           = datetime.datetime.strptime(options.date_end, "%Y-%m-%dT%H:%M:%S")
    self.station       = surface_pb2.Point()
    self.station.index = options.index
    self.station_name  = self.getStationName(options.index)
    self.description   = options.description
    return True

  def initProto(self, proto):
    if (None == proto.datetime) or (None == proto.datetime_end) or (0 == proto.stations.__len__()) or (None == proto.text):
      self.result.result = False
      self.result.comment = u"Не заданы требуемые параметры"
      return False
    self.dts = datetime.datetime.strptime(proto.datetime, "%Y-%m-%dT%H:%M:%S")
    self.dte = datetime.datetime.strptime(proto.datetime_end, "%Y-%m-%dT%H:%M:%S")
    self.station = proto.stations[0]
    self.station_name = self.station.name
    self.description = proto.text
    self.printFileToStdOut = True
    return True

  def openTemplate(self):
    # заполнение документа
    try:
      # открвыаем документ
      self.doc = document.odf_get_document(PRJ_DIR + "/share/meteo/odtsamples/AV4.odt")
      return True
    except IOError:
      self.result.comment = u"Файл шаблона указан неверно"
      return False

  def fillVariables(self):
    # получаем тело документа
    self.body = self.doc.get_body()
    storm_number = self.body.get_variable_set('storm_number')
    day = self.body.get_variable_set('day')
    month = self.body.get_variable_set('month')
    year = self.body.get_variable_set('year')
    from_hour = self.body.get_variable_set('from_hour')
    to_hour = self.body.get_variable_set('to_hour')
    region = self.body.get_variable_set('region')
    description = self.body.get_variable_set('description')
    create_hour = self.body.get_variable_set('create_hour')
    create_min = self.body.get_variable_set('create_min')

    dt = datetime.datetime.utcnow()
    create_hour.set_text(str(dt.hour))
    create_min.set_text(str(dt.minute))

    region.set_text(meteoglobal.trUtf8(self.station_name))

    day.set_text(str(self.dts.day))
    month.set_text(meteoglobal.trUtf8(self.months[self.dts.month - 1]))
    year.set_text(self.dts.strftime('%y'))

    description.set_text(self.description)
    from_hour.set_text(str(self.dts.hour))
    to_hour.set_text(str(self.dte.hour))

    meteodir = os.path.expanduser('~/.meteo/')
    cur_month = datetime.datetime.now().month
    if os.path.isfile(os.path.join(meteodir + '/storm_counter')):
      counter = open(os.path.join(meteodir + '/storm_counter'), 'r')
      number = int(counter.readline())
      month = int(counter.readline())
      counter.close()
      if cur_month > month:
        month = cur_month
        number = 1
      else:
        number += 1
      counter = open(os.path.join(meteodir + '/storm_counter'), 'w')
      counter.write(str(number) + '\n')
      counter.write(str(month))
      counter.close()
    else:
      if not os.path.exists(meteodir):
        os.mkdir(meteodir)
      counter = open(os.path.join(meteodir + '/storm_counter'), 'w+')
      number = 1
      month = cur_month
      counter.write(str(number) + '\n')
      counter.write(str(month))
      counter.close()
    storm_number.set_text(str(number))

  def saveStorm(self):
    # сохраняем файл
    output = '/tmp/' + u'Storm_' + self.station.index + '_' + self.dts.strftime("%Y-%m-%d") + '.odt'
    try:
      self.doc.save(target=output, pretty=True)
      self.result.result = True
      self.result.filename = u'Storm_' + self.station.index + '_' + self.dts.strftime("%Y-%m-%d") + '.odt'
      self.result.datetime = self.dts.strftime("%Y-%m-%dT%H:%M:%S")

      if self.printFileToStdOut:
        resultFile = open(output, 'rb')
        data = resultFile.read()
        resultFile.close()
        self.result.data = base64.b64encode(data)
        os.remove(output)
      return True
    except IOError as ioe:
      self.result.comment = u"Ошибка записи в файл"
      return False

  def getStationName(self, index):
    ctrl = ClientController()
    sprinf_address = serviceAddress(14)
    sprinf_host = librpcpywrap.hostByAddress(sprinf_address)
    sprinf_port = librpcpywrap.portByAddress(sprinf_address)
    if (False == ctrl.Connect(sprinf_host, sprinf_port)):
      sys.stderr.write(u'Не удалось подключиться к сервису справочной информации')
      return str(index)
    srv = SprinfService_Stub(ctrl.Channel())
    req = MultiStatementRequest()
    if isinstance(index, (int, long)):
      req.index.append(int(index))
    else:
      req.cccc.append(index)
    resp = ctrl.RemoteCall(srv.GetStations, req, 30000)
    if resp is None:
      return False
    if not resp.station:
      station_name = index
    else:
      station_name = resp.station[0].name
      if station_name.rus:
        station_name = station_name.rus
      elif station_name.international:
        station_name = station_name.international
      elif station_name.short:
        station_name = station_name.short
      else:
        station_name = index
    return station_name

  def resultProto(self):
    return self.result

def main(argv):

  parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter,description="Сценарий формирования метеосводок")
  parser.add_argument('-m', '--mode', default='arg', help="Режим запуска сценария. "
                                                          "arg - Задание аргументов запуска "
                                                          "proto - Передача прото-структуры в стандартный поток")
  parser.add_argument('-i', '--index', help='Индекс станции')
  parser.add_argument('-dts', '--date_start',
                      help='Начальный срок штормового предупреждения, в формате YYYY-MM-DDThh:mm:ss')
  parser.add_argument('-dte', '--date_end',
                      help='Конечный срок штормового предупреждения, в формате YYYY-MM-DDThh:mm:ss')
  parser.add_argument('-d', '--description', help='Описание')

  options = parser.parse_args()

  generator = StormGenerator()

  if str(options.mode) == 'arg':
    generator.init(options)
  if str(options.mode) == 'proto':
    sa = StartArguments()
    sa.ParseFromString(meteoglobal.readStdinBytes())
    init = generator.initProto(sa)

  generator.generate()
  result = generator.resultProto()
  if True == result.result:
    result.comment = u'Успех'
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(0)
  else:
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(1)


if __name__ == "__main__":
  main(argv)
