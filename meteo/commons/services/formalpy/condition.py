#!/usr/bin/python3
# -*- coding: utf-8 -*-

#системные модули
import os
import sys
from sys import argv
import datetime

import argparse

from document_service_pb2 import *
from sprinf_pb2 import *
import meteoglobal

# Qt-модули
from PyQt5.QtCore import *
from PyQt5.QtGui import *

# ипортируем главный класс для наслоедования
from documentform import DocumentFormal


class ConditionGenerator(DocumentFormal):
  
  def __init__(self):
    super().__init__()
    self.forecast          = False
    self.docNumberFileName = '/factcondition_counter'
    self.template_path     = "/share/meteo/odtsamples/Condition.odt"



  # 
  # инициализация из прото параметров
  # 
  def initFromProto(self, proto):
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
    if proto.forecast:
      self.forecast          = True
      self.docNumberFileName = '/forecastcondition_counter'
    if proto.text:
      self.text = proto.text

    return True

  # 
  # Заполнение документа
  # 
  def fillDocument(self):
    
    self.header()
    self.description()
    self.footer()
    


  def docNumberIncrement(self):
    meteodir = os.path.expanduser('~/.meteo/')
    cur_month = datetime.datetime.now().month
    if os.path.isfile(os.path.join(meteodir + self.docNumberFileName)):
      counter = open(os.path.join(meteodir + self.docNumberFileName), 'r')
      number = int(counter.readline())
      month = int(counter.readline())
      counter.close()
      if cur_month > month:
        month = cur_month
        number = 1
      else:
        number += 1
      counter = open(os.path.join(meteodir + self.docNumberFileName), 'w')
      counter.write(str(number) + '\n')
      counter.write(str(month))
      counter.close()
    else:
      if not os.path.exists(meteodir):
        os.mkdir(meteodir)
      counter = open(os.path.join(meteodir + self.docNumberFileName), 'w+')
      number = 1
      month = cur_month
      counter.write(str(number) + '\n')
      counter.write(str(month))
      counter.close()
    meteo = self.body.get_table(name='Header')
    n = meteo.get_cell('B1')
    n.set_text(str(number))
    meteo.set_cell('B1',n)




  def header(self):
    header = self.body.get_table(name='Header')
    self.docNumberIncrement()

    name = header.get_cell('B2')
    name.set_text(self.station_name)
    header.set_cell('B2',name)

    index = header.get_cell('B3')
    index.set_text( self.getStationNumber(self.station.station) )
    header.set_cell('B3', index)

    coords = header.get_cell('B4')
    coords.set_text( self.getLatStr() + " " + self.getLonStr() )
    header.set_cell('B4', coords)

    date = header.get_cell('B5')
    date.set_text(meteoglobal.dateToHumanTimeShort(self.arg_date))
    header.set_cell('B5', date)


  def description(self):
    pogodaPrognoz = self.body.get_variable_set('pogodaPrognoz')
    title         = self.body.get_variable_set('title')
    if self.forecast:
      pogodaPrognoz.set_text(u'Прогноз по району')
      title.set_text(u'Прогноз гидрометеорологических условий по району')

    text = self.body.get_table(name="Text")
    cell = text.get_cell('A1')
    cell.set_text(meteoglobal.trUtf8(self.text))
    text.set_cell('A1', cell)


  def footer(self):
    footer     = self.body.get_table(name='Footer')
    department = footer.get_cell('B1')
    user       = footer.get_cell('B2')
    dt         = footer.get_cell('B3')

    department.set_text(meteoglobal.trUtf8(self.department))
    user.set_text( meteoglobal.trUtf8(self.rank) + " " + meteoglobal.trUtf8(self.user) )
    dt.set_text(meteoglobal.dateToHumanTimeShort(datetime.datetime.now()))

    footer.set_cell('B1', department)
    footer.set_cell('B2', user)
    footer.set_cell('B3', dt)





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

  generator = ConditionGenerator()
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
