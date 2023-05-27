#!/usr/bin/python
# coding: utf-8

import os
import sys

from google.protobuf import message
from google.protobuf import text_format

class ProtoText(object):
  @staticmethod
  def ToText(msg):
    return text_format.MessageToString(msg, True).decode('string-escape')

  @staticmethod
  def ToProto( txt, cls ):
    if ( sys.version_info > (3,0) ):
      text = txt.decode('unicode_escape')
      return text_format.Parse(text, cls() )

    msg = cls()
    text_format.Merge( txt, msg )
    return msg
#    return text_format.Parse( txt, cls() )

  @staticmethod
  def ToFile( msg, filename ):
    f = open( filename, "wb" )
    f.write( ProtoText.ToText(msg) )
    f.close()

  @staticmethod
  def FromFile( filename, cls ):
    f = open( filename, "rb" )
    txt = f.read()
    return ProtoText.ToProto( txt, cls )

  @staticmethod
  def ProtosFromDirectory( path, cls ):
    dirs = os.listdir(path)
    protos = dict()
    for f in dirs:
      filename = path + "/" + f
      if ( True == os.path.isfile(filename) ):
        protos[filename] = ProtoText.FromFile( filename, cls )
    return protos

