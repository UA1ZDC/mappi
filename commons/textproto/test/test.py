#!/usr/bin/python
# coding: utf-8

import filecmp
import unittest
import os

from prototext import *
from test_pb2 import *
from map_isoline_pb2 import *

class TestProtoText( unittest.TestCase ):
  def test_prototext(self):
    msg = ProtoText.FromFile( "proto.test", Applications )
    ProtoText.ToFile( msg, "result.test" )
    self.assertEqual( filecmp.cmp( "proto.test", "result.test" ), True )
    os.unlink("result.test")
    protos = protos = ProtoText.ProtosFromDirectory( os.getenv("BUILD_DIR")+"/etc/meteo/isoline.conf.d", FieldColors )
    #TODO test for load from dirs

if __name__ == "__main__":
  unittest.main()
