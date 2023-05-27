#!/usr/bin/python3
# coding: utf-8

import libglobalwrap

from google.protobuf.service import RpcController
from meteo_channel import *

class ClientController(RpcController):
  def __init__(self):
    self.channel_ = ClientChannel(self)
    self.failed_ = False
    self.reason_ = 'No errors'
    self.message_ = None
    self.donesucces_ = False

  def __exit__(self):
    self.close()

  def close(self):
    self.channel_.close()


  def Callback( self, response ):
    self.message_ = response
    if ( False == self.Failed() and True == self.channel_.client.received ):
      self.donesucces_ = True
    else:
      #print 'Ответ отсервиса не получен.'
      self.donesucces_ = False

  def RemoteCall( self, method, request, timeout ):
    self.donesucces_ = False
    self.message_ = None
    self.channel_.timeout = timeout
    method( self, request, self.Callback )
    return self.message_

  def Channel(self):
    return self.channel_

  def Connect( self, host, port ):
    res = self.channel_.Connect(host, port)
    if res:
      self.failed_ = False
    return res

  def Reset(self):
    self.failed_ = False
    self.reason_ = 'No errors'
    self.channel_.Disconnect()

  def ErrorText(self):
    return self.reason_

  def Failed(self):
    return self.failed_

  def SetFailed( self, reason ):
    self.reason_ = reason
    self.failed_ = True
