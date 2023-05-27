#!/usr/bin/python3
# -*- coding: utf-8 -*-

#системные модули
import os
import sys
from sys import argv
import datetime

# модули для работы с документами
from lpod3 import document

from conf import PRJ_DIR

#Qt-модули
from   PyQt5.QtCore import QCoreApplication
import base64

from   document_service_pb2 import *
import meteoglobal
import services_pb2


# ипортируем главный класс для наслоедования
from documentform import DocumentFormal

class FloodSituationGenerator(DocumentFormal):
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
    
    self.station_type      = 0
    self.template_path     = "/share/meteo/odtsamples/FloodSituation.odt"

  # 
  # инициализация с параметрами
  # 
  def init(self, proto):
    if not proto.datetime or 0 == proto.stations.__len__():
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
    day   = self.body.get_variable_set('day')
    month = self.body.get_variable_set('month')
    year  = self.body.get_variable_set('year')

    day.set_text(str(self.arg_date.day))
    month.set_text(meteoglobal.trUtf8(self.months[self.arg_date.month - 1]))
    year.set_text(str(self.arg_date.year))

    table  = self.body.get_table(name='Table')
    cell   = table.get_cell('A1')
    region = u'Паводковая обстановка по району (территории): ' + self.station_name
    cell.set_text(meteoglobal.trUtf8(region))
    table.set_cell('A1', cell)



# 
# ====================== MAIN FUNCTION ============================
# 

def main(argv):
  generator = FloodSituationGenerator()
  sa        = StartArguments()
  sa.ParseFromString(meteoglobal.readStdinBytes())
  init      = generator.init(sa)

  # генерим из параметров
  generator.generateDocument()
  # получаем готовую протку
  result = generator.resultProto()
  
  if not result.result:
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(1)
  else:
    result.comment = u"Успех"
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(0)


if __name__ == "__main__":
  main(argv)
