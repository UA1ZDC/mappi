#!/usr/bin/env python
# -*- coding: utf-8 -*-

from google.protobuf.service import RpcController
import services_pb2 as services
import sprinf_pb2 as sprif
import surface_pb2 as src_data
import surface_service_pb2 as src
import meteo_controller as meteoCtrl
import libglobalwrap as globalWrap
import meteoglobal
import librpcpywrap

def getNearStation( station, typeStation, radius):
  if station is None:
    return False
  ctrl = meteoCtrl.ClientController()
  address = globalWrap.serviceAddress(services.kSprinf)
  host = meteoCtrl.hostByAddress(address)
  port = meteoCtrl.portByAddress(address)
  tries = 0
  connected = False
  while not connected and tries < 5:
    connected = ctrl.Connect(host, port)
    tries += 1
  if not connected:
    return False
  else:
    sprinfStub = sprif.SprinfService_Stub(ctrl.Channel())
  request = sprif.MultiStatementRequest()
  try:
    station = int(station)
    request.index.append(station)
  except:
    request.cccc.append(station)
  respond = ctrl.RemoteCall(sprinfStub.GetStations, request, 30000)
  if respond is not None and len(respond.station) > 0:
    request = sprif.CircleRegionRequest()
    request.circle.radius_meter = radius * 1000
    request.circle.center.lon_radian = respond.station[0].position.lon_radian
    request.circle.center.lat_radian = respond.station[0].position.lat_radian
    request.station_type = typeStation
    respond = ctrl.RemoteCall(sprinfStub.GetStationsByPosition, request, 30000)
    if respond is not None and len(respond.station) > 0:
      i = 0
      while i < len(respond.station):
        if respond.station[i].distance_to > 0 and respond.station[i].main.index != station:
          return respond.station[i].main.index
        i = 1 + i
    else:
      return False
  return False

def getNearStations( station, typeStation, radius):
  if station is None:
    return False
  ctrl = meteoCtrl.ClientController()
  address = globalWrap.serviceAddress(services.kSprinf)
  host = meteoCtrl.hostByAddress(address)
  port = meteoCtrl.portByAddress(address)
  tries = 0
  connected = False
  while not connected and tries < 5:
    connected = ctrl.Connect(host, port)
    tries += 1
  if not connected:
    return False
  else:
    sprinfStub = sprif.SprinfService_Stub(ctrl.Channel())
  request = sprif.MultiStatementRequest()
  try:
    station = int(station)
    request.index.append(station)
  except:
    request.cccc.append(station)
  respond = ctrl.RemoteCall(sprinfStub.GetStations, request, 30000)
  if respond is not None and len(respond.station) > 0:
    request = sprif.CircleRegionRequest()
    request.circle.radius_meter = radius * 1000
    request.circle.center.lon_radian = respond.station[0].position.lon_radian
    request.circle.center.lat_radian = respond.station[0].position.lat_radian
    request.station_type = typeStation
    respond = ctrl.RemoteCall(sprinfStub.GetStationsByPosition, request, 30000)
    if respond is not None and len(respond.station) > 0:
      return respond
    else:
      return False
  return False

def getStation( index):
  if index is None:
    return False
  ctrl = meteoCtrl.ClientController()
  address = globalWrap.serviceAddress(services.kSprinf)
  host = meteoCtrl.hostByAddress(address)
  port = meteoCtrl.portByAddress(address)
  tries = 0
  connected = False
  while not connected and tries < 5:
    connected = ctrl.Connect(host, port)
    tries += 1
  if not connected:
    return False
  else:
    sprinfStub = sprif.SprinfService_Stub(ctrl.Channel())
  request = sprif.MultiStatementRequest()
  try:
    station = int(index)
    request.index.append(station)
  except:
    request.cccc.append(index)
  respond = ctrl.RemoteCall(sprinfStub.GetStations, request, 30000)
  if respond is not None and len(respond.station) > 0:
    return respond.station[0]
  else:
    return False
  return False

