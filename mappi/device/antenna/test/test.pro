TEMPLATE = test
TARGET = mappi.antenna.test

QT += \
  core \
  serialport

SOURCES = \
  main.cpp

LIBS = \
  -ltapp \
  -ltdebug \
  -lmnsatellite \
  -lmnmathtools \
  -lmeteo.dbi \
  -lmeteo.global \
  -lmappi.global \
  -lmappi.schedule \
  -lmappi.antenna

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )
