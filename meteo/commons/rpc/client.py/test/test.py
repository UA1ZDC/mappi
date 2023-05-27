#!/usr/bin/python3
# coding: utf-8

import sys
import signal


from conf import *
sys.path.append(PRJ_DIR + '/lib')

import logging

from meteo_controller import *

import libmeteodatawrap
from libmeteodatawrap import *
from libglobalwrap import *



from google.protobuf.service import RpcController

from meteo_channel import *
from meteo_controller import *

from meteo_pb2 import *
from document_service_pb2 import *
from map_document_pb2 import *

import weather_pb2

class Responser(object):
  def __init__( self, ctrl):
    self.controller = ctrl

  def Callback( self, response ):
    if ( True == self.controller.Failed() ):
      print( 'Oshibka vypolnenia')
      print( self.controller.ErrorText())
    else:
      print( 'async')
#      print( response.__unicode__())

def async_call( ctrl, app ):
  srv = Service_Stub( ctrl.Channel() )
  request = Document()
  request.scale = 12
  rspr = Responser(ctrl)
  srv.CreateDocument( ctrl, request, rspr.Callback )
  app.exec_() #eventLoop для асинхронного выполнения

def main(argv):
  signal.signal( signal.SIGINT, signal.SIG_DFL )

#  app = QCoreApplication(argv)

  ctrl = ClientController()
  if ( False == ctrl.Connect( "10.10.11.143", 9153 ) ):
    print("error connect to mapservice")
    return 1

#  async_call( ctrl, app )

  request = Document()
  request.scale = 16
  request.map_center.lat_deg = 65
  request.map_center.lon_deg = 90
  request.doc_center.lat_deg = 65
  request.doc_center.lon_deg = 90
  request.projection = kConical
  request.docsize.width = 1920
  request.docsize.height = 1080
  srv = DocumentService_Stub(ctrl.Channel())
  response = ctrl.RemoteCall( srv.CreateDocument, request, 10000 )
  if ( None == response ):
    print('Ошибка во время получения ответа1')
    return 1;
  
  map_proto = weather_pb2.Map()
  layer = weather_pb2.WeatherLayer()
  layer.mode =  weather_pb2.ViewMode.kIsoline
  layer.level =  0
  layer.type_level = 1
  layer.template_name = '10051'
  layer.transparency =  0
  map_proto.data.append(layer)
  map_proto.path = './tmp'
  map_proto.datetime = '2021-03-06T00:00:00'
#  print( response.__unicode__())

  request = ExportRequest()
  request.uuid = response.uuid
  print( 'normas poshel')
  response = ctrl.RemoteCall( srv.GetDocument, request, 10000 )
  print( 'normas prishel')
  if ( None == response ):
    print( 'Ошибка во время получения ответа2')
    return 1;

  f = open( './map.jpeg', 'wb' )
#  f.open( QIODevice.WriteOnly );
  f.write( response.data )
#  f.flush()
#  f.close()

  request = Document()
  response = ctrl.RemoteCall( srv.RemoveDocument, request, 10000 )
  if ( None == response ):
    print( 'Ошибка во время получения ответа3')
    return 1;

#  print( response.__unicode__())

  return 0

if __name__ == "__main__":
  main(sys.argv)
