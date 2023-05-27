# -*- coding: utf-8 -*-
import sys

import time

import libpuansonwrap
from libpuansonwrap import *
import libmeteodatawrap
from libmeteodatawrap import *

from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

import puanson_pb2
from puanson_pb2 import *

from google.protobuf.service import RpcController

from surface_pb2 import *
from surface_service_pb2 import *

from meteo_channel import *
from meteo_controller import *

app = QApplication (sys.argv)

puanson = libpuansonwrap.Puanson()
print puanson.setPunch(path = "/home/viktor/build_dir/share/meteo/punch/test.template")

meteodata = libmeteodatawrap.TMeteoData()

ctrl = ClientController()
if ( False == ctrl.Connect( "10.10.11.201", 38765 ) ):
  print("failed to connect to surfaceservice")

request = DataRequest()
#request.type = kSynopFix
request.type.append(1)
request.type_level = 1
request.level_p = 0
request.station.append("26063")
request.meteo_descrname.append('w')
t = '03'
request.date_start = ('2017-04-26' + "T" + t + ":00:00")
#request.as_proto = True

srv = SurfaceService_Stub(ctrl.Channel())

respond = ctrl.RemoteCall(srv.GetMeteoData, request, 10000)
array = respond.meteodata[0]
print sys.getsizeof(array)
meteodata.fromArray(array,sys.getsizeof(array))

puanson.setMeteoData(meteodata)

print puanson.stringForParam('w')






