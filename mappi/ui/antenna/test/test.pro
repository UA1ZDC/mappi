TEMPLATE = test
TARGET = mappi.antenna.widget.test

QT += \
  gui \
  widgets \
  serialport

SOURCES = \
  main.cpp

LIBS = \
  -ltdebug \
  -ltapp \
  -ltrpc \
  -lprotobuf \
  -lmeteo.dbi \
  -lmeteo.global \
  -lmeteo.proto \
  -lmeteo.dbi \
  -lmeteo.settings \
  -lmeteo.geobasis \
  -lmappi.global \
  -lmappi.proto \
  -lmappi.settings \
  -lmappi.antenna.widget

#  -lmappi.global \
#  -lmappi.schedule \
#  -lmappi.antenna

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )
