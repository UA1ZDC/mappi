#!/usr/bin/env python
# -*- coding: utf-8 -*-
# системные модули
import os
import sys
from sys import argv

# конфигурационный модуль
from conf import *

sys.path.append(PRJ_DIR + '/lib')

# модули времени и даты
from datetime import timedelta, datetime
import time

# модули для работы с документами
from lpod3 import document
from lpod3.document import *

# модули для работы с удаленным сервисом
from google.protobuf.service import RpcController
import document_service_pb2 as docServ


# модуль для парсинга аргументов строки командной
import argparse
import base64

# модули оболочек C++ классов Puanson, TMeteoData, Zond
import libglobalwrap
import tempfile

import meteoglobal

from document_service_pb2 import *
from sprinf_pb2 import *

# Qt-модули
from PyQt5.QtCore import *
from PyQt5.QtGui import *

# ипортируем главный класс для наслоедования
from documentform import DocumentFormal

class DonesenieCHS(DocumentFormal):

  def __init__(self):
    super().__init__()
    
    # self.hasData           = True    
    self.template_path     = "/share/meteo/odtsamples/hsprirodavoiska.odt"
  
  # 
  # Заполнение документа
  # 
  def fillDocument(self):
    
    dataTable = self.body.get_table(name='mainTable')

    cell = dataTable.get_cell(coord=(0,1), clone=False)
    cell.set_value(value='1', text='1')
    dataTable.set_cell(coord=(0,1), cell=cell, clone=False)

    cell = dataTable.get_cell(coord=(1,1), clone=False)
    cell.set_value(self.arg_date.strftime('%Y-%m-%d %H:%M') )
    dataTable.set_cell(coord=(1,1), cell=cell, clone=False)

    currentdate = datetime.now()
    day = self.body.get_variable_set("day")
    day.set_text(currentdate.strftime('%d.%m.%Y'))

    rank = self.body.get_variable_set("rank")
    name = self.body.get_variable_set("name")
    rank.set_text( meteoglobal.trUtf8( self.rank ) )
    name.set_text( meteoglobal.trUtf8( self.user ) )





# 
# ====================== MAIN FUNCTION ============================
# 

def main(argv):
  
  # Qt-приложение
  app = QCoreApplication(sys.argv)

  parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter,
                                    description=meteoglobal.trUtf8("Построение Прогноз распределения ветра по району"))
  parser.add_argument('--inputType', '-m', type=str, help=meteoglobal.trUtf8(
    "Тип источника входных данных  proto = прото файл или arg = коммандная строка"))
  parser.add_argument('--date', '-dt', type=str, help=meteoglobal.trUtf8("Дата начала прогноза"))
  parser.add_argument('--rank', '-r', type=str, help=meteoglobal.trUtf8("Звание автора"))
  parser.add_argument('--name', '-n', type=str, help=meteoglobal.trUtf8("Имя автора"))

  options = parser.parse_args()

  generator = DonesenieCHS()
  init      = False
  
    # парсим данные из файла или протки
  if str(options.inputType) == "arg":
    init = generator.init(options)
  elif str(options.inputType) == "proto":
    sa     = StartArguments()
    sa.ParseFromString(meteoglobal.readStdinBytes())
    init   = generator.initFromProto(sa)
  if not init:
    result = generator.resultProto()
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(1)

  # генерим из параметров
  generator.generateDocument()
  # получаем готовую протку
  result = generator.resultProto()
  
  if True == result.result:
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(0)
  else:
    sys.stdout.buffer.write(result.SerializeToString())
    sys.exit(1)
    
if __name__ == "__main__":
  main(argv)
