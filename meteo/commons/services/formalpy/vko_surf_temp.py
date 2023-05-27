#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys

import os
import argparse
# модули времени и даты
from datetime import timedelta
from datetime import datetime
import time

#Qt-модули
#from PyQt5.QtCore import *

# import base64

# конфигурационный модуль
from conf import *
sys.path.append(PRJ_DIR + '/lib')

from climat_pb2 import *
import vkodoc

kTemplate = PRJ_DIR + "/share/meteo/odtsamples/vko_temp.odt"
kBaseName = 'vko_temp_'


def createTemp(args):
  doc = vkodoc.VkoDoc()

  doc.init(args)

  #outname = str(kBaseName + self.station + datetime.now().strftime("%Y-%m-%dT%H:%M:%S") + '.odt')
  outname = str(kBaseName + doc.getStationNumber() + '_' + datetime.now().strftime("%Y-%m-%dT") + '.odt')
  ok = doc.opendoc(kTemplate)
  if (ok):
    doc.fillheader()
    doc.fillOne(kVkoT, 'tableT', 'B', 3)
    doc.setImage('vkoT.png', 'imgT')
    doc.fillByMonth(kVkodT, 'tabledT', 'B', 3)
    doc.setImages('vkodT_', 'imgdT_', 1, 13)
    doc.fillOne(kVkoU, 'tableU', 'B', 3)
    doc.save(outname)
    sys.stdout.buffer.write(doc.result.SerializeToString())
  
  # if doc.result.comment:
  #   print datetime.now().strftime("%Y-%m-%dT%H:%M:%S"), "[E]", sys.argv[0], ":", doc.result.comment
  
   
    
def main(argv):
  #app = QApplication (sys.argv)

  parser = argparse.ArgumentParser(description=str("Построение климатического описания"))
  # parser.add_argument('-t', '--type', dest='vtype', type=int, help=str(
  #   "Тип военно-климатической характеристики"))
  parser.add_argument('-s', '--station', type=str, help=str("Станция"))
  parser.add_argument('-b', '--beg', type=str, help=str(
    "Дата начала периода отбора в формате yyyy-MM-dd"))
  parser.add_argument('-e', '--end', type=str, help=str(
    "Дата конца периода отбора в формате yyyy-MM-dd"))
  parser.add_argument('-u', '--utc', type=int, help=str(
    "Разница с UTC"))
  parser.add_argument('-m', '--mode', default='arg', help=str(
    'Режим запуска сценария: arg - аргументы запуска, proto - прото-структура'))
  parser.add_argument('-c', '--coord', type=str, help=str(
    "Координаты"))
  parser.add_argument('-a', '--name', type=str, help=str(
    "Название станции"))

  
  args = parser.parse_args()

  createTemp(args)
  sys.exit(0)
  
        
if __name__ == '__main__':
  main(sys.argv)
