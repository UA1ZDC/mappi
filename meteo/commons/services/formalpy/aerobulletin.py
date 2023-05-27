#!/usr/bin/python3
# coding: utf-8

#системные модули
import os
import sys
from sys import argv

#конфигурационный модуль
from conf import *

#модули времени и даты
import datetime
import time

#модули для работы с документами
from lpod3 import document
from lpod3.paragraph import odf_create_paragraph

#модули для работы с удаленным сервисом
from google.protobuf.service import RpcController

from surface_pb2 import *
from surface_service_pb2 import *

from meteo_channel import *
from meteo_controller import *

#модули оболочек C++ классов Puanson, TMeteoData, Zond
import libpuansonwrap
from libpuansonwrap import *
import libmeteodatawrap
from libmeteodatawrap import *
from libzondwrap import *

#Qt-модули
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

#модуль настроек
import productsettings

from sprinf_pb2 import *

#класс для хранения полученных данных. Необходим для заполнения таблиц
class CityData():
  def __init__(self, index):
    self.times = []
    self.index = index

  def add_data(self, data):
    self.times.append(data)

  def get_data_on_time(self, time):
    return self.times[time]

class BulletinGenerator():
  def __init__(self):
    self.descs = ['CH', 'CM', 'CL', 'CN','N','Nh','h','dd','ff','P0','T','U','V','w']
    self.host = productsettings.host()
    self.srcdata = productsettings.service(productsettings.kSrcData)
    libpuansonwrap.loadPunch(str("is"))                # загрузка шаблона пуансона
    self.months = ['января', 'февраля', 'марта', 'апреля', 'мая', 'июня', 'июля', 'августа', 'сентября', 'октября', 'ноября',
              'декабря']                      # для интерпретации текущего месяца:
    self.ctrl = ClientController()
    self.ctrl_loc = ClientController()
    self.cityData = []
    self.nonLocalCityData = []
    self.stations = []                        # список индексов запрашиваемых станций
    self.non_local_stations = []              # не локальные
    self.times = []                           # список сроков
    self.non_loc_rqs = []                     # список запросов по нелокальным станциям
    self.nonLocalResponses = []               # список ответов по нелокальным станциям
    self.zonds = []

  # применение аргументов запуска
  def init(self, argv):
    self.bulletin_date = datetime.datetime.strptime(argv[1], "%Y-%m-%dT%H:%M:%S")
    i = 2
    while i < argv.__len__():
      self.stations.append(argv[i])
      i = i + 1
    self.getStationNames()
    self.analyzeStations()
    if not self.connectControllers():
      return False
    return True

  def generate(self):
    self.pickTimes()
    self.formNonLocalRequests()
    if self.non_local_stations:
      if not self.getNonLocalData():
        return False
    self.interpretResponds()
    if not self.openTemplate():
      return False
    if not self.fillVariables():
      return False
    self.buildGround()
    self.buildFact()
    self.formNonLocalZonds()
    self.buildZond()
    self.saveBulletin()
    return True

  # анализ станций, распределение на локальные и нелокальные
  def analyzeStations(self):
    self.numOfPoints = self.stations.__len__()
    for station in self.stations:
      self.non_local_stations.append(station)
      self.nonLocalCityData.append(CityData(station))

  def connectControllers(self):
    if self.non_local_stations:
      connected = False
      tries = 0
      while not connected and tries < 5:
        print "Выполняется попытка подключения к сервису раскодированных данных"
        connected = self.ctrl.Connect(self.srcdata.host, self.srcdata.port)
        tries += 1
        time.sleep(5)
      if not connected:
        print "Не удалось подключиться к сервису раскодированных данных"
        return False
      else:
        print "Успешно"
        self.non_loc_srv = SurfaceService_Stub(self.ctrl.Channel())
    return True

  def pickTimes(self):
    # формируем массив, в котором будут хранится часы, за которые нужно получить данные
    curHour = self.bulletin_date.hour
    t = 0
    while t <= curHour:
      self.times.append(str('%.2d' % (t)))
      t = t + 1
    self.numOfTimes = self.times.__len__()

  def formNonLocalRequests(self):
    if not self.non_local_stations:
      return
    else:
      day = self.bulletin_date.strftime('%Y-%m-%d')
      for time in self.times:
        date_start = (day + "T" + str(time) + ":00:00")
        req = self.makeRequest(self.non_local_stations, date_start, self.descs, False)
        self.non_loc_rqs.append(req)


  def formNonLocalZonds(self):
    for station in self.non_local_stations:
      request = self.makeAeroRequest(station, self.times[-1])
      respond = self.ctrl.RemoteCall(self.non_loc_srv.GetTZondOnStation, request, 10000)
      if (None == respond):
        print('Ошибка во время получения ответа')
        continue
      if not respond.data:
        print ('Данных нет')
        continue
      if not respond.data[0].meteodata:
        print ('Данных нет')
        continue
      data = str(respond.data[0].meteodata)
      zond = Zond()
      zond.fromArray(data, sys.getsizeof(data))
      zond_date = zond.dateTime()
      dt = datetime.datetime.strptime(request.date_end, "%Y-%m-%dT%H:%M:%S")
      if zond_date:
        dt = zond_date
#        dt = datetime.datetime.strptime(dt, "%Y-%m-%dT%H:%M:%S")
      self.zonds.append((self.station_names[station], zond, dt))


  def getNonLocalData(self):
    # получаем ответы
    i = 0
    while i < self.times.__len__():
      res = self.ctrl.RemoteCall(self.non_loc_srv.GetMeteoData, self.non_loc_rqs[i], 30000)
      self.nonLocalResponses.append(res)
      if (None == res):
        print('Ответ от сервиса не получен') #TODO обрабокта
        return False
      i = i + 1
    return True


  def getStationNames(self):
    sprinf = settings.service(settings.kSprinf)
    ctrl = ClientController()
    if (False == ctrl.Connect(sprinf.host, sprinf.port)):
      print("Не удалось подключиться к сервису справочной информации")
      return 1
    srv = Service_Stub(ctrl.Channel())
    req = MultiStatementRequest()
    for station in self.stations:
      req.index.append(int(station))
    resp = ctrl.RemoteCall(srv.GetStations, req, 30000)
    self.station_names  = {}
    if not resp.station:
      for station in self.stations:
        self.station_names[station] = station
    else:
      for station in resp.station:
        station_name = station.name
        if station_name.rus:
          station_name = station_name.rus
        elif station_name.international:
          station_name = station_name.international
        elif station_name.short:
          station_name = station_name.short
        else:
          station_name = station.index
        self.station_names[str(station.index)] = station_name


  #создаем запрос для списка станция на определнный срок
  def makeRequest(self, stations, date_start, descs, proto = True, date_end = None):
    if not stations:
      print "Станции не заданы"
      return 0
    request = DataRequest()
    request.type = kSynopFix
    request.type_arr.append(1)
    request.type_level = 1
    request.level_p = 0
    request.station.extend(stations)
    request.meteo_descrname.extend(descs)
    request.date_start = date_start
    if date_end is not None:
      request.date_end = date_end
    request.as_proto = proto
    return request


  def makeAeroRequest(self, station, t):
    if not station:
      print "Станции не заданы"
      return 0
    request = DataRequest()
    request.type = kAeroFix
    request.station.append(station)
    daye = self.bulletin_date.strftime('%Y-%m-%d')
    #daye = datetime.datetime.utcnow().strftime('%Y-%m-%d')
    day = self.bulletin_date - datetime.timedelta(days=1)
    day = day.strftime('%Y-%m-%d')
    request.date_start = (day + "T" + t + ":00:00")
    request.date_end = (daye + "T" + t + ":00:00")
    request.only_last = True
    return request


  def processParam(self, meteodata, name):
    if meteodata.meteoParam(name).isInvalid():
      return '-'
    else:
      pustr = libpuansonwrap.stringFromRuleValue(name,meteodata.meteoParam(name).value())
      if len(pustr) > 0 and not pustr == 'err':
        return pustr
      else:
        return '%.1f'%meteodata.meteoParam(name).value()


  #функция, которая преобразует полученный ответ в простую структуру данных, которую можно будет использовать для заполнения таблиц
  def interpretRespond(self, rsp, city, descs):
    index = city.index
    data = []
    for item in rsp.meteodata:
      meteodata = self.interpretData(item)
      if int(meteodata.getStationIndex()) == int(index):
        params = []
        for desc in descs:
          params.append(self.processParam(meteodata, desc))
        data.append(str(params[0]) + '/' + str(params[1]) + '/' + str(params[2]) + '/' + str(params[3]))
        data.append(str(params[4]) + '/' + str(params[5]))
        i = 6
        while i < params.__len__():
          data.append(str(params[i]))
          i = i + 1
        break
    city.add_data(data)
    return True


  #преобразует полученный массив байт в понятный для класса Puanson вид
  def interpretData(self, bar):
    #создание объекта класса TMeteoData
    meteodata = libmeteodatawrap.TMeteoData()
    meteodata.fromArray(bar,sys.getsizeof(bar))
    return meteodata


  def interpretResponds(self):
    aux = {}
    # составляем полученные данные в структуру, удобную для заполнения таблиц
    i = 0
    while i < self.non_local_stations.__len__():
      j = 0
      while j < self.nonLocalResponses.__len__():
        self.interpretRespond(self.nonLocalResponses[j], self.nonLocalCityData[i], self.descs)
        aux[self.nonLocalCityData[i].index] = self.nonLocalCityData[i]
        j = j + 1
      i = i + 1
    for station in self.stations:
      for index in aux.keys():
        if index == station:
          self.cityData.append(aux[index])


  #вычисляет влажность по заданным температуре и температуре росы
  def humidity(self, T, Td):
    if T < -10:
      a = 9.5
      b = 265.5
    else:
      a = 7.63
      b = 241.9
    tmp1 = (a * Td)/(b + Td)
    tmp2 = (a * T)/(b + T)
    tmp1 = tmp1 - tmp2
    tmp2 = 10 ** tmp1
    if tmp2 < 0:
      return 0
    elif tmp2 > 1:
      return 1
    else:
      return tmp2


  def openTemplate(self):
    try:
      # открвыаем документ
      self.doc = document.odf_get_document(PRJ_DIR + "/share/meteo/odtsamples/Aerobill.odt")
      return True
    except IOError:
      print("Файл шаблона указан неверно")
      return False


  def fillVariables(self):
    # получаем тело документа
    self.body = self.doc.get_body()
    # получаем внутренние переменные документа
    self.unit_number = self.body.get_variable_sets("unit_number")
    self.cur_user = self.body.get_variable_set("cur_user")
    self.cur_user_rank = self.body.get_variable_set("cur_user_rank")
    self.example = self.body.get_variable_set("example")
    self.day = self.body.get_variable_sets("day")
    self.year = self.body.get_variable_sets("year")
    self.month = self.body.get_variable_sets("month")
    self.bulletin_number = self.body.get_variable_set("bulletin_number")
    self.region = self.body.get_variable_set("region")
    self.fio_cmdr = self.body.get_variable_set("fio_cmdr")
    self.rank_cmdr = self.body.get_variable_set("rank_cmdr")
    self.hour = self.body.get_variable_sets("hour")
    self.minute = self.body.get_variable_sets("minute")
    try:
      # заполняем документ параметрами из файла настроек
      self.cur_user.set_text(settings.curUserShortName().decode("utf-8"))
      self.cur_user_rank.set_text(settings.curUserRank().decode("utf-8"))
      self.example.set_text("1")
      i = 0
      while i < len(self.day):
        self.day[i].set_text(self.bulletin_date.strftime("%d"))
        self.year[i].set_text(self.bulletin_date.strftime("%y"))
        self.month[i].set_text(self.months[self.bulletin_date.month - 1].decode('utf-8'))
        i = i + 1
      for item in self.unit_number:
        item.set_text(("TODO: номер части").decode("utf-8"))
      i = 0
      while i < len(self.hour):
        self.hour[i].set_text(self.bulletin_date.strftime("%H"))
        self.minute[i].set_text(self.bulletin_date.strftime("%M"))
        i = i + 1
      self.bulletin_number.set_text("#")
      self.region.set_text(u"полигона")
      self.fio_cmdr.set_text(("TODO: фамилия командира").decode("utf-8"))
      self.rank_cmdr.set_text(("TODO: звание командира").decode("utf-8"))
      return True
    except:
      print "Ошибка при формировании документа (переменных документа). Вероятно, шаблон документа некорректен"
      return False


  def buildGround(self):
    # ЗАПОЛНЯЕМ ПЕРВУЮ ТАБЛИЦУ
    ground = self.body.get_table(name='Ground')
    if ground is None:
      print("Шаблон документа некорректен")
      return 1
    nameRow = ground.get_row('1')
    paramRow = ground.get_row('2')
    cell = ground.get_cell('A1')
    cell.set_text(self.station_names[self.stations[0]])
    ground.set_cell('A1', cell)
    i = 0
    while i < self.numOfTimes - 1:
      ground.append_row(paramRow)
      i = i + 1
    i = 0
    while i < self.numOfPoints - 1:
      ground.append_row(nameRow)
      crd = 'A' + str(1 * (i + 1) + (self.times.__len__()) * (i + 1) + 1)
      cell = ground.get_cell(crd)
      cell.set_text(self.station_names[self.stations[i + 1]])
      ground.set_cell(crd, cell)
      j = 0
      while j < self.numOfTimes:
        ground.append_row(paramRow)
        j = j + 1
      i = i + 1
    fstCellI = 2
    fstCellL = 'A'
    #try:
    # указываем сроки
    i = 0
    while i < self.numOfPoints:
      j = 0
      while j < self.numOfTimes:
        cell = ground.get_cell(fstCellL + str(fstCellI + (self.numOfTimes + 1) * i + j))
        cell.set_text(self.times[j] + ".00")
        ground.set_cell(fstCellL + str(fstCellI + (self.numOfTimes + 1) * i + j), cell)
        j = j + 1
      i = i + 1
    # заполняем параметры
    rowsToDelete = []
    fstCellL = 'B'
    i = 0
    while i < self.numOfPoints:
      j = 0
      while j < self.numOfTimes:
        if not self.cityData[i].get_data_on_time(j):
          rowsToDelete.append(fstCellI + j + ((self.times.__len__() + 1) * i - 1))
          j = j + 1
          continue
        k = 0
        while k < 10:
          crd = chr(ord(fstCellL) + k) + str(fstCellI + j + ((self.times.__len__() + 1) * i))
          cell = ground.get_cell(crd)
          datastr = str(self.cityData[i].get_data_on_time(j)[k])
          cell.set_text(datastr.decode("utf-8"))
          ground.set_cell(crd, cell)
          k = k + 1
        j = j + 1
      i = i + 1
    # except:
    #   print "Ошибка заполнения первой таблицы"
    # удаление пустых строк
    j = -1
    while j >= -(rowsToDelete.__len__()):
      ground.delete_row(rowsToDelete[j])
      j = j - 1


  def buildFact(self):
    # заполняем вторую таблицу
    fact = self.body.get_table(name="Fact")
    if fact is None:
      print("Шаблон документа некорректен")
      return 1
    if self.numOfTimes > 1:
      r = fact.get_row(0)
      i = 1
      while i < self.numOfTimes:
        fact.append_row(r)
        i = i + 1
      fact.set_span('A1:A' + str(self.numOfTimes))
      fact.set_span('H1:H' + str(self.numOfTimes))
    i = 0
    while i < (self.numOfPoints / 2) - 1 + self.numOfPoints % 2:
      rws = str((self.numOfTimes * i) + 1) + ':' + str(self.numOfTimes * (i + 1))
      rowsForMore = fact.get_rows(rws)
      fact.extend_rows(rowsForMore)
      i = i + 1
    #try:
    # заполняем станции
    j = 0
    while j < 2:
      ind = 1
      fstCellL = chr(ord('A') + (j * 7))
      i = 0
      while ind <= (fact.get_height()):
        if (i + ((fact.get_height() / self.numOfTimes) * j)) >= self.numOfPoints:
          break
        cell = fact.get_cell(fstCellL + str(ind))
        cell.set_text(self.station_names[argv[2 + i + ((fact.get_height() / self.numOfTimes) * j)]])
        fact.set_cell(fstCellL + str(ind), cell)
        i = i + 1
        ind = ind + self.numOfTimes
      j = j + 1
    # заполняем сроки
    j = 0
    while j < 2:
      fstCellL = chr(ord('B') + (j * 7))
      i = 0
      t = 0
      while i < (fact.get_height()):
        if t > self.numOfTimes - 1:
          t = 0
        cell = fact.get_cell(fstCellL + str(i + 1))
        cell.set_text(self.times[t] + ".00")
        fact.set_cell(fstCellL + str(i + 1), cell)
        t = t + 1
        i = i + 1
      j = j + 1
    # заполняем параметры
    # меняем индексы парметров в выданном ответе для удобства заполнения
    pe = [1, 8, 3, 4, 6]

    fstCellI = 1
    l = 0
    while l < 2:
      fstCellL = chr(ord('C') + (7 * l))
      shift = l * (fact.get_height() / self.numOfTimes)
      i = 0
      while i < (fact.get_height() / self.numOfTimes):
        if (i + shift) >= self.numOfPoints:
          break
        j = 0
        while j < self.numOfTimes:
          if not self.cityData[i + shift].get_data_on_time(j):
            j = j + 1
            continue
          k = 0
          while k < 5:
            crd = chr(ord(fstCellL) + k) + str(fstCellI + j + (self.numOfTimes * i))
            cell = fact.get_cell(crd)
            datastr = self.cityData[i + shift].get_data_on_time(j)[pe[k]]
            cell.set_text(datastr)
            fact.set_cell(crd, cell)
            k = k + 1
          j = j + 1
        i = i + 1
      l = l + 1
    # except:
    #   print("Ошибка при заполнении 2-й таблицы")


  def buildZond(self):
    empty = odf_create_paragraph(u' ')
    tables = []  # список будущих таблиц с данными по каждому пунтку
    pars = []  # список с заголовками этих таблиц
    dataPar = self.body.get_paragraph(content=u'Header')
    tables.append(self.body.get_table(name="Zond"))  # копируем первую таблицу
    pars.append(dataPar)  # копируем первый заголовок
    if dataPar is None or tables[0] is None:
      print("Шаблон документа некорректен")
      return 1
    if not self.zonds:
      print 'Нет данных от зондов'
      pars[0].delete()
      tables[0].delete()
      return
    # вырезаем подпись документа, чтобы добавить ее после всех таблиц
    implementer = self.body.get_paragraph(content=u'Исполнил')
    commander = self.body.get_paragraph(content=u'Начальник')
    commAppendix = self.body.get_paragraph(content=self.uloc.fio_cmdr)
    implementer.delete()
    commander.delete()
    commAppendix.delete()
    # уровни высоты, по которым требуются данные
    heights = [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
               23, 24, 25, 26, 27, 28, 29, 30]
    # список параметров, значения которых нужны по каждой высоте
    values = [ValueType.UR_dd, ValueType.UR_ff, ValueType.UR_P, ValueType.UR_T]
    valueDescs = ['dd', 'ff', 'P0', 'T']
    if self.zonds.__len__() > 1:
      i = 1
      while i < self.zonds.__len__():
        pars.append(dataPar.clone())
        tables.append(tables[0].clone())
        self.body.append(pars[i])
        self.body.append(tables[i])
        i += 1
    i = 0
    while i < self.zonds.__len__():
      pars[i].set_text(
        u'Данные метеорологического радиозондирования атмосферы за «' + str(self.zonds[i][2].day) + u'» ' + self.months[
          int(self.zonds[i][2].month) - 1].decode('utf-8') + ' ' + str(self.zonds[i][2].year) + u' года по району ' +
          self.zonds[i][0] + u' за ' + self.zonds[i][2].strftime("%H:%M") + u' UTC')
      fstCellL = 'B'
      row = 0
      j = 0
      while j < heights.__len__():
        if j == 17:
          row = 1
          fstCellL = 'B'
        level = Uroven()
        self.zonds[i][1].getUrPoH(heights[j] * 1000, level)
        fstCellI = 2
        k = 0
        while k < 5:
          if k == 4:
            if not level.isGood(ValueType.UR_T) or not level.isGood(ValueType.UR_Td):
              datastr = '-'
            else:
              data = self.humidity(level.value(ValueType.UR_T), level.value(ValueType.UR_Td))
              datastr = str('%.1f' % (data * 100))
          else:
            try:
              if level.isGood(values[k]) :
                datastr = libpuansonwrap.stringFromRuleValue(valueDescs[k], level.value(values[k]))
              else:
                datastr = '-'
            except IndexError:
              datastr = '-'
          cell = tables[i].get_cell(fstCellL + str(fstCellI + 6 * row))
          cell.set_text(datastr)
          tables[i].set_cell(fstCellL + str(fstCellI + 6 * row), cell)
          fstCellI = fstCellI + 1
          k = k + 1
        fstCellL = chr(ord(fstCellL) + 1)
        j = j + 1
      self.body.append(empty.clone())
      i += 1
      ######################
      # конец работы с третьей таблицей
      ######################
      # добавляем подпись документа
      self.body.append(implementer)
      self.body.append(commander)
      self.body.append(commAppendix)


  def saveBulletin(self):
    # присваимаем бюллетеню номер, по порядку от начала месяца
    meteodir = os.path.expanduser('~/.meteo/')
    cur_month = datetime.datetime.now().month
    if os.path.isfile(os.path.join(meteodir + '/bulletin_counter')):
      counter = open(os.path.join(meteodir + '/bulletin_counter'), 'r')
      number = int(counter.readline())
      month = int(counter.readline())
      counter.close()
      if cur_month > month:
        month = cur_month
        number = 1
      else:
        number += 1
      counter = open(os.path.join(meteodir + '/bulletin_counter'), 'w')
      counter.write(str(number) + '\n')
      counter.write(str(month))
      counter.close()
    else:
      if not os.path.exists(meteodir):
        os.mkdir(meteodir)
      counter = open(os.path.join(meteodir + '/bulletin_counter'), 'w+')
      number = 1
      month = cur_month
      counter.write(str(number) + '\n')
      counter.write(str(month))
      counter.close()
    self.bulletin_number.set_text(str(number))
    station_string = ''
    for station in self.stations:
      station_string = station_string + str(station) + '_'
    # сохраняем файл
    if not os.path.exists(PRJ_DIR + '/var/ukaz/documents/' + self.bulletin_date.strftime("%Y-%m-%d")):
      os.makedirs(PRJ_DIR + '/var/ukaz/documents/' + self.bulletin_date.strftime("%Y-%m-%d"))
    output = os.path.join(PRJ_DIR + '/var/ukaz/documents/' + self.bulletin_date.strftime("%Y-%m-%d"),
                          u'AerometeorologicalBulletin_' + station_string + self.bulletin_date.strftime(
                            "%Y-%m-%dT%H:%M:%S") + '.odt')
    print output
    try:
      self.doc.save(target=output, pretty=True)
    except IOError as ioe:
      # print ioe
      print("Ошибка записи в файл")


def main(argv):
  if argv.__len__() < 2:
    print "Не заданы необходимые параметры запуска"
    sys.exit(1)
  #Qt-приложение
  app = QCoreApplication (sys.argv)
  generator = BulletinGenerator()
  if not generator.init(argv):
    print "Не удалось подключиться к сервису"
    sys.exit(1)
  if not (generator.generate()):
    print "Ошибка в процессе генерации бюллетеня"
    sys.exit(1)


if __name__ == "__main__":
  main(argv)
