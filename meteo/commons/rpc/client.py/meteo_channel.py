#!/usr/bin/python3
# coding: utf-8


from google.protobuf.service import RpcChannel
from google.protobuf.service import RpcController

import struct
import socket
import uuid
import time
import sys

import librpcpywrap

class ClientHandler():
  def __init__( self, channel ):
    self.done = None
    self.parser = librpcpywrap.RpcParser()
    self.response_creator  = None
    self.received = False
    self.channel = channel
    self.currentmethod = None

  def readMessage(self):
    self.received = False
    timeout = float(self.channel.timeout)/1000.0
    cdt = time.time()
    timeout_save = timeout

    recv_size = 0
    while ( self.parser.msgsize() <= 0 and 0 < timeout ):
      arr = self.channel.recvMessage( self.channel.recvsocket, 8, timeout )
      curlen = len(arr)
      if ( 0 == curlen ):
        sys.stderr.write('Ошибка при чтении собщения\n')
        break
      self.parser.appendData( arr, curlen )
      recv_size += curlen
      pack = librpcpywrap.MethodPack()
      self.parser.parseMessage(pack)
      timeout -= ( time.time() - cdt )
  
    msgreceived = False
    while ( False == self.received and 0 < timeout and 0 < self.parser.msgsize() ):
      arr = self.channel.recvMessage( self.channel.recvsocket,( self.parser.msgsize() - recv_size), timeout )
      curlen = len(arr)
      if ( 0 == curlen ):
        sys.stderr.write('Ошибка при чтении собщения\n')
        break
      self.parser.appendData( arr, curlen )
      recv_size += curlen
      pack = librpcpywrap.MethodPack()
      timeout -= ( time.time() - cdt )
      if ( True == self.parser.parseMessage(pack) ):
        self.received = True
        response = self.response_creator()
        try:
          response.ParseFromString( pack.message() )
        except:
          self.channel.controller.SetFailed('Ошибка извлечения ответа от сервера')
          response = None
        self.done(response)
        return

    if ( False == self.received and 0 > timeout ):
      sys.stderr.write('Вызов ' + self.currentmethod + ' не успел завершиться за ' + str(timeout_save) + ' секунд\n')
    elif ( False == self.received ):
      sys.stderr.write('Обрыв соединения во время выполнения метода\n')
      

class ClientChannel(RpcChannel):

  def __init__( self, ctrl ):
    self.controller = ctrl
    self.sendsocket = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
    self.recvsocket = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
    self.uuid = None
    self.host = None
    self.port = 0
    self.blocked = False
    self.timeout = 100000
    self.client = ClientHandler(self)

  def close(self):
    try:
      self.sendsocket.shutdown( socket.SHUT_RDWR )
      self.sendsocket.close()
    except:
      pass
    try:
      self.recvsocket.shutdown( socket.SHUT_RDWR )
      self.recvsocket.close()
    except:
      pass

  def Connect( self, address, port ):
    if ( 0 != self.sendsocket.connect_ex( ( address, port ) ) ):
      sys.stderr.write('Не удалось установить соединение\n')
      return False

    uid = str('{' + str(uuid.uuid1()) + '}' )
    if ( False == self.ClientHandshake( self.sendsocket, uid ) ):
      return False
    
    if ( 0 != self.recvsocket.connect_ex( ( address, port ) ) ):
      return False
    if ( False == self.ClientHandshake( self.recvsocket, uid ) ):
      return False
    
    self.uuid = uid
    self.host = address
    self.port = port

    return True

  def Disconnect(self):
    self.sendsocket.close()
    self.recvsocket.close()

  def CallMethod(self, method, controller, request, response, done):
    pack = librpcpywrap.MethodPack()
    pack.setId( '{' + str(uuid.uuid1()) + '}' )
    pack.setService(method.containing_service.full_name )
    pack.setMethod( method.name )
    pack.setStub(True)
    pack.setFailed(False)
    pack.setMessage( request.SerializeToString() )
    self.client.done = done
    self.client.response_creator = response
    self.client.currentmethod = method.name
    self.SendRequest(pack)
    self.WaitForAnswer()


  def WaitForAnswer(self):
    self.client.readMessage()

  def SendRequest( self, pack ):
    packstr = pack.data()
    sendstr = 'rpcv2msgstart'.encode('utf-8')
    self.writeMessage( self.sendsocket, sendstr )
    sz = int(pack.size())
    arr = struct.pack('<q', sz )
    self.writeMessage( self.sendsocket, arr )
    self.writeMessage( self.sendsocket, packstr )
    self.writeMessage( self.sendsocket, 'rpcmsgend'.encode('utf-8') )

  def ClientHandshake( self, socket, uuid ):
    shakestr = 'qwertypod' + uuid
    sz = len(shakestr)
    shakestr = struct.pack('<i', sz ) + shakestr.encode('utf-8')
    self.writeMessage( socket, shakestr )
#    shakestr = 'qwertypod' + uuid
    length = len(shakestr)

    arr = self.recvMessage( socket, length, 3 )
    if ( arr != shakestr ):
      return False
    return True

  def writeMessage( self, socket, arr ):
    length = len(arr)
    totalsend = 0
    while totalsend < length:
      sent = socket.send( arr[totalsend:] )
      if 0 == sent:
        sys.stderr.write('Ошибка передачи данных сервису\n')
        return False
      totalsend += sent
    return True

  def recvMessage( self, socket, length, timeout ):
    bytes_recd = 0
    allrecv = bytes()
    socket.settimeout(timeout)
    while bytes_recd < length:
      try:
        chunk = socket.recv( min( length - bytes_recd, 2048 ) )
        if ( 0 == len(chunk) ):
          return allrecv
          break
      except:
        return allrecv
      allrecv += chunk
      bytes_recd += len(chunk)
    return allrecv
