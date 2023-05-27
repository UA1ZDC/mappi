#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys

import os
import argparse
# модули времени и даты
from datetime import timedelta
from datetime import datetime
import time
import re

#Qt-модули
from PyQt5.Qt import *

# import base64

# конфигурационный модуль
from conf import *
sys.path.append(PRJ_DIR + '/lib')

from climat_pb2 import *
import vkodoc

kTemplate = PRJ_DIR + "/share/meteo/odtsamples/vko_month.odt"
kBaseName = 'vko_month_'

months = {1:'Январь', 2:'Февраль', 3:'Март', 4:'Апрель', 5:'Май', 6:'Июнь', 7:'Июль', 8:'Август', 9:'Сентябрь', 10:'Октябрь', 11:'Ноябрь', 12:'Декабрь'}

def fillWind(doc, vtype):
  try:
    doc.calc.getClimatData(vtype)
    res = doc.calc.calcTableValues()
    doc.fill(res, 'tableff1', 'C', 4)
    if len(res) < 12:
      return
    
    height = [ 0 ]
    h = re.search(u'Высота:\s*\d+', doc.coord)
    if h is not None:
      height = h.group(0).split(':')[1].strip()
    doc.fill(height, 'tableff1', 'B', 4)
    
    ff = 0.0
    dd = 0.0
    fx = 0.0
    cnt = 0
    for ur in range(3, 12):
      if res[ur] is not None and res[ur][2] != '' and float(res[ur][2]) > 100:
        cnt += 1
        ff += float(res[ur][2])
        dd = float(res[ur][0])
        if fx < float(res[ur][2]):
          fx = float(res[ur][2])
    if cnt != 0:
      ff /= cnt
      w = [[str(int(round(dd)))], [str(int(round(ff)))], [str(int(round(fx)))]]
      doc.fill(w, 'tableTrop', 'B', 6)
    
  except IOError:
    sys.stderr.write("Ошибка заполнения таблицы " + tname)

def createMonth(args):
  doc = vkodoc.VkoDoc()
  doc.init(args, setmonth = True)
  
  outname = str(kBaseName + doc.getStationNumber() + '_' + datetime.now().strftime("%Y-%m-%dT") + '.odt')
  ok = doc.opendoc(kTemplate)
  if (ok):
    doc.fillheader()
    doc.body.get_variable_set("month").set_text(months[doc.month]) 
    doc.fillOne(kVkoNm,  'tableNh', 'B', 3)
    doc.fillOne(kVkohm,  'tableNh', 'B', 7)
    doc.fillOne(kVkoVm,  'tableV', 'B', 3)
    doc.fillOne(kVkohVm, 'tablehV', 'B', 3)
    doc.fillOne(kVkoP0m, 'tableRP', 'E', 12)
    doc.fillOne(kVkoRm,  'tableRP', 'B', 3)
    doc.fillOne(kVkoYfxm, 'tableRP', 'E', 9)
    doc.fillOne(kVkowm,   'tablew', 'A', 3)
    doc.fillOne(kVkoTm,   'tableT', 'B', 3)
    doc.fillOne(kVkoUm,   'tableT', 'E', 8)
    fillWind(doc, kVkoff1m)
    doc.fillOne(kVkoTropom, 'tableTrop', 'B', 3)
    doc.fillOne(kVkoTgm,  'tableTg', 'B', 3)
    doc.fillOne(kVkossm,  'tableTg', 'B', 7)
    doc.fillOne(kVkoYssm, 'tableTg', 'E', 9)
    doc.save(outname)
    sys.stdout.buffer.write(doc.result.SerializeToString())


    
def main(argv):
#  app = QApplication (sys.argv)

  parser = argparse.ArgumentParser(description=str("Построение климатического описания"))
  # parser.add_argument('-t', '--type', dest='vtype', type=int, help=str(
  #   "Тип военно-климатической характеристики"))
  parser.add_argument('-s', '--station', type=str, help=str("Станция"))
  parser.add_argument('-b', '--beg', type=str, help=str(
    "Дата начала периода отбора в формате yyyy-MM-dd"))
  parser.add_argument('-n', '--month', type=int, help=str(
    "Месяц"))
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

  createMonth(args)

  sys.exit(0)
  
        
if __name__ == '__main__':
  main(sys.argv)


