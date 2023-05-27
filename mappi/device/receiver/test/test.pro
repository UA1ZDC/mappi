TEMPLATE = test
TARGET = mappi.receiver.test

QT += core

SOURCES = \
  main.cpp

LIBS = \
  -ltapp \
  -ltdebug \
  -lmnsatellite \
  -lmeteo.dbi \
  -lmeteo.global \
  -lmappi.global \
  -lmappi.receiver \

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )
