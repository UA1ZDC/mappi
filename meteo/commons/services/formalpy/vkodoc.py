#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
#reload(sys)
#sys.setdefaultencoding('utf8')

import os

# модули времени и даты
from datetime import timedelta
from datetime import datetime
import time

import base64
import meteoglobal
import argparse

from conf import *
sys.path.append(PRJ_DIR + '/lib')

# модули для работы с документами
from lpod3 import document
from lpod3.document import *

# модули оболочек C++ классов
from document_service_pb2 import *
from libvkocalc import *

from climat_pb2 import *

kTempSavePath = "/tmp/"


class VkoDoc():
  def __init__(self):
    self.doc = None
    self.result = ExportResponse()
    self.result.result = False
    self.begdt = ''
    self.enddt = ''
    self.month = 0
    self.name = ''
    self.coord = ''
    self.calc = VkoCalc()
    self.calc.setPath(kTempSavePath)

  def init(self, args, setmonth = False):
    if str(args.mode) == 'arg':
      filled = (args.station is not None) and (args.beg is not None) \
               and (args.end is not None) and (args.utc is not None) \
               and (setmonth == False or args.month is not None)
      if not filled:
        sys.stderr.write("Заданы не все аргументы. Вызов справки -h")
        sys.exit(1)
      if (setmonth == True and args.month is not None):
        self.month = args.month
      self.calc.setStation(args.station, args.utc)
      self.calc.setDt(args.beg, args.end, self.month)
      self.station = args.station
      if (args.name is not None):
        self.name = str(args.name)
      if (args.coord is not None):
        self.coord = str(args.coord)
      self.begdt = args.beg
      self.enddt = args.end
      
    elif str(args.mode) == 'proto':
      sa = StartArguments()
      sa.ParseFromString(meteoglobal.readStdinBytes())
      filled = (sa.index.__len__() != 0) and (sa.datetime is not None) \
               and (sa.datetime_end is not None) and (sa.number is not None) \
               and (setmonth == False or sa.month is not None)
      if not filled:
        sys.stderr.write("Заданы не все аргументы")
        sys.exit(1)
      if (setmonth == True and sa.month is not None):
        self.month = sa.month
      self.station = str(sa.index[0])
      if (sa.name is not None):
        self.name = str(sa.name)
      if (sa.coord is not None):
        self.coord = str(sa.coord)
      self.begdt = str(sa.datetime)
      self.enddt = str(sa.datetime_end)
      self.calc.setStation(str(sa.index[0]), sa.number)
      self.calc.setDt(str(sa.datetime), str(sa.datetime_end), self.month)

    if not self.calc.isInit():
      sys.stderr.write("Ошибка инициализации.")
      sys.exit(1)

  def opendoc(self, docname):
    try:
      self.doc = document.odf_get_document(docname)
      return True
    except IOError:
      self.result.comment = u"Файл шаблона указан неверно"
      return False

  # заполнение заголовка
  def fillheader(self):
    try:
      self.body = self.doc.get_body()
      self.body.get_variable_set("station").set_text( self.getStationNumber() )
      self.body.get_variable_set("name").set_text(self.name)      
      self.body.get_variable_set("coord").set_text(self.coord)
      # по умолчанию задаем текущий день и дату
      end = datetime.today()
      beg = end-timedelta(days=31)
      if self.begdt:  
        try:
          if 'T' in self.begdt:
            beg = datetime.strptime(self.begdt, "%Y-%m-%dT%H:%M:%S")
          else:
            beg = datetime.strptime(self.begdt, "%Y-%m-%d")
        except:
          self.result.comment = u"Ошибка распознавания даты"
        self.body.get_variable_set("begin").set_text(beg.strftime("%d.%m.%Y"))
      if self.enddt:
        try:
          if 'T' in self.begdt:
            end = datetime.strptime(self.enddt, "%Y-%m-%dT%H:%M:%S")
          else:
            end = datetime.strptime(self.enddt, "%Y-%m-%d")
        except:
          self.result.comment = u"Ошибка распознавания даты"
        # end = datetime.strptime(self.enddt, "%Y-%m-%d")
        self.body.get_variable_set("end").set_text(end.strftime("%d.%m.%Y"))
        # print "fill header", self.station, beg.strftime("%d.%m.%Y"), end.strftime("%d.%m.%Y")
    except IOError:
      self.result.comment = u"Ошибка заполнения заголовка"
      return False

  # заполнение таблицы
  def fill(self, vals, tname, scellL, scellI):
    # print "fill", tname, scellL, scellI
    self.body = self.doc.get_body()
    self.table = self.body.get_table(name=tname)
    if self.table is None:
      self.result.comment = u'Таблицы %s не существует' % tname
      return False
    # поля таблицы
    cellL = scellL
    cellI = scellI
    for row in vals:
      for val in row:
        if len(val) > 0:
          cellId = cellL + str(cellI)
          cell = self.table.get_cell(cellId, clone=False)
          cell.set_text(val)
          self.table.set_cell(cellId, cell, clone=False)
        cellL = chr(ord(cellL) + 1)
      cellI += 1
      cellL = scellL
    return True

  def save(self, outname):
    outputs = kTempSavePath + outname
    try:
      # print(datetime.now().strftime("%Y-%m-%dT%H:%M:%S")) # выводится для сервиса генерации документов
      self.result.datetime = str(datetime.now().strftime("%Y-%m-%dT%H:%M:%S"))
      self.result.filename = str(outname)
      self.doc.save(target=outputs, pretty=True)

      file = open(outputs, 'rb')
      self.result.data = base64.b64encode(file.read())
      file.close()
      self.result.result = True
      # print(self.result.SerializeToString())
      os.remove(outputs)
      return True
    except IOError as ioe:
      self.result.comment = u'Ошибка записи в файл'
      sys.stderr.write("Ошибка записи в файл")
      return False

  def fillOne(self, vtype, tname, scellL, scellI):
    try:
      self.calc.getClimatData(vtype)
      res = self.calc.calcTableValues()
      #print(res)
      self.fill(res, tname, scellL, scellI)
    except IOError:
      sys.stderr.write("Ошибка заполнения таблицы " + tname)

  def fillByMonth(self, vtype, tname, scellL, scellI):
    try:
      self.calc.getClimatData(vtype);
      for month in range(12):
        res = self.calc.calcTableParamValues(month + 1, -1);
        # print res
        if not self.fill(res, tname + "_" + str(month + 1).zfill(2), scellL, scellI):
          break
    except IOError:
      sys.stderr.write("Ошибка заполнения таблицы " + tname)

  def fillByMonthHour(self, vtype, tname, scellL, scellI):
    try:
      self.calc.getClimatData(vtype);
      for month in range(12):
        for hour in [0, 3, 6, 9, 12, 15, 18, 21]:
          res = self.calc.calcTableParamValues(month + 1, hour);
          # print res
          if not self.fill(res, tname + "_" + str(month + 1).zfill(2) + "_" + str(hour).zfill(2), scellL, scellI):
            break
    except IOError:
      sys.stderr.write("Ошибка заполнения таблицы " + tname)

  def setImage(self, img, imgname):
    try:
      self.frame = self.body.get_frame(name=imgname)
      fullname = kTempSavePath + img
      if os.path.exists(fullname):               
        imgfile = self.doc.add_file(fullname)
        self.doc.set_part(self.frame.get_image().get_url(), self.doc.get_part(imgfile))               
        os.remove(fullname)
    except IOError:
      sys.stderr.write("Ошибка вставки изображения " + img)

  def setImages(self, img, imgname, snum, enum):
    for month in range(snum, enum):
      try:
        frame = self.body.get_frame(name=imgname + str(month))
        if frame:
          fullname = kTempSavePath + img + str(month) + ".png"
          if os.path.exists(fullname):
            imgfile = self.doc.add_file(fullname)
            self.doc.set_part(frame.get_image().get_url(), self.doc.get_part(imgfile))                      
            os.remove(fullname)
      except IOError:
        sys.stderr.write("Ошибка вставки изображения " + img + str(month) + ".png")

  def getStationNumber(self):
    """Функция, которая исправляет представление номеров станций путем добавления 0 у станций,
       длина которых меньше 5 чисел

    Returns:
        _type_: _description_
    """
    if str(self.station).isnumeric():
      return str(self.station).rjust(5,'0')
    return self.station
      