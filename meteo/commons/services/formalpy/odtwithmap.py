#!/usr/bin/python3
# coding: utf-8

#системные модули
import os
import sys
from sys import argv

import inspect

from conf import *
sys.path.append(PRJ_DIR + '/lib')

#модули времени и даты
import datetime
import time

import math

import meteo_channel
import meteo_controller


import libglobalwrap
import librpcpywrap
from lpod3 import document

import document_service_pb2
from document_service_pb2 import StartArguments, ExportResponse, ExportRequest
import services_pb2
import weather_pb2

import tempfile
import base64
import meteoglobal
from meteoglobal import error_log
from meteo_channel import *
from meteo_controller import *

from PyQt5.QtCore import QCoreApplication


import optparse
import argparse

class OdtWithMapGenerator:
  def __init__(self):
    self.result = ExportResponse()
    self.result.result = False

  def init(self, proto):
    if not all([proto.map, proto.index, proto.text, proto.number, proto.datetime]):
      self.result.comment = u'Не заданы требуемые параметры'
      return False
    self.map = proto.map
    self.index = proto.index[0]
    self.title = proto.text
    self.number = proto.number
    self.datetime = proto.datetime

    self.username = proto.user_name if proto.user_name else u'Неизвестный пользователь'
    self.rank = proto.user_rank if proto.user_rank else u'Неизвестное звание'
    self.department = proto.department if proto.department else u'Неизвестное подразделение'

    return True

  def generate(self):
    if not self.openTemplate():
      return False
    if not self.createMap():
      return False
    self.fillDoc()
    self.save()
    self.result.result = True

  def createMap(self):
    def errFunc(error):
      error_log << error
      self.result.comment = error
      return False
    ctrl = ClientController()
    host = librpcpywrap.hostByAddress(libglobalwrap.serviceAddress(services_pb2.kMap))
    port = librpcpywrap.portByAddress(libglobalwrap.serviceAddress(services_pb2.kMap))
    if not ctrl.Connect(host, port):
      return errFunc('Не удалось подключиться к службе формирования документов')
    else:
      srv = document_service_pb2.DocumentService_Stub(ctrl.Channel())

    resp = ctrl.RemoteCall(srv.CreateDocument, self.map.document, 30000)
    if not resp or not resp.result:
      return errFunc('Не получен ответ от службы формирования документов при создании документа')

    mapRequest = self.map
    mapRequest.allow_empty = True
    if 0 is not self.map.hour:
      mapRequest.center = 4
      mapRequest.model = 11

    resp = ctrl.RemoteCall(srv.CreateMap, mapRequest, 100000)
    if not resp:
      return errFunc('Не получен ответ от службы формирования документов при создании карты')

    if not resp.result:
      req = ExportRequest()
      req.title = self.title
      resp = ctrl.RemoteCall(srv.SaveDocument, req, 30000)

    req = ExportRequest()
    resp = ctrl.RemoteCall(srv.GetDocument, req, 30000)
    if not resp or not resp.result or not resp.data:
      return errFunc(u'Не получен документ от службы формирования документов')

    self.image = resp.data
    return True




  def resultProto(self):
    return self.result

  def save(self):
    output = u'/tmp/odtwithmap_{0}_{1}.odt'.format(self.index, self.datetime)
    try:
      self.doc.save(target=output, pretty=True)
      self.result.result = True
      self.result.filename = u'odtwithmap_{0}_{1}.odt'.format(self.index, self.datetime)
      self.result.datetime = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:00")

      resultFile = open(output, 'rb')
      data = resultFile.read()
      resultFile.close()
      self.result.data = base64.b64encode(data)
      os.remove(output)
      return True
    except IOError as ioe:
      self.result.comment = u'Ошибка записи в файл'
      return False

  def fillDoc(self):
    self.body = self.doc.get_body()

    self.frame = self.body.get_frame()
    filename = tempfile.NamedTemporaryFile().name + '.jpg'
    with open(filename, 'a') as f:
      f.write(self.image)
      f.flush()
      f.close()
      self.doc.del_part(self.frame.get_image().get_url())
      self.frame.set_image(self.doc.add_file(filename))
      # width = '{}px'.format(self.map.document.docsize.width)
      # height = '{}px'.format(self.map.document.docsize.height)
      # self.frame.set_size((width, height))
      os.remove(filename)

    doctype = self.body.get_variable_set('doctype')
    doctype.set_text(self.map.title)

    header_table = self.body.get_table(name='HeaderTable')

    number = header_table.get_cell('B1')
    number.set_text(str(self.number))
    header_table.set_cell('B1', number)

    region = header_table.get_cell('B2')
    region.set_text(self.title)
    header_table.set_cell('B2', region)

    index = header_table.get_cell('B3')
    index.set_text(self.index)
    header_table.set_cell('B3', index)

    coords = header_table.get_cell('B4')
    corners = []
    for point in self.map.point:
      lat = meteoglobal.latToHuman(point.fi)
      lon = meteoglobal.lonToHuman(point.la)
      corners.append('{0} {1}'.format(lat, lon).decode('utf-8'))

    coordText = u'Координаты района (верхний левый угол; верхний правый угол; нижний правый угол; нижний левый угол): {0}, {1}, {2}, {3}'.format(*corners)
    coords.set_text(coordText)
    header_table.set_cell('B4', coords)

    if 0 == self.map.hour:
      forecast = ''
    else:
      self.datetime = self.datetime.replace('Z', '')
      forecast = datetime.datetime.strptime(self.datetime, "%Y-%m-%dT%H:%M:%S") + datetime.timedelta(hours = self.map.hour)
      #forecast = forecast.strftime("%Y-%m-%dT%H:%M:%SZ").replace('T', ' ').replace('Z', '')
      forecast = u' / Прогноз на {}'.format(forecast)

    hour = header_table.get_cell('B5')
    hour.set_text(self.datetime.replace('T', ' ').replace('Z', '') + forecast)
    header_table.set_cell('B5', hour)

    footer_table = self.body.get_table(name='FooterTable')

    department = footer_table.get_cell('B1')
    department.set_text(self.department)
    footer_table.set_cell('B1', department)

    user = footer_table.get_cell('B2')
    user.set_text(u'{0} {1}'.format(self.rank, self.username).replace('T',' '))
    footer_table.set_cell('B2', user)

    dt = footer_table.get_cell('B3')
    dt.set_text(datetime.datetime.now().strftime("%Y-%m-%d %H:%M"))
    footer_table.set_cell('B3', dt)


  def openTemplate(self):
    # заполнение документа
    try:
      # открвыаем документ
      self.doc = document.odf_get_document(PRJ_DIR + "/share/meteo/odtsamples/odtwithmap.odt")
      return True
    except IOError:
      self.result.comment = u"Файл шаблона указан неверно"
      return False


def main(argv):
  #Qt-приложение
  app = QCoreApplication (sys.argv)

  generator = OdtWithMapGenerator()
  init = False
  sa = StartArguments()
  sa.ParseFromString(meteoglobal.readStdinBytes())
  init = generator.init(sa)
  if not init:
    result = generator.resultProto()
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(1)

  generator.generate()
  result = generator.resultProto()
  if not result.result:
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(1)
  else:
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(0)


if __name__ == "__main__":
  main(argv)
