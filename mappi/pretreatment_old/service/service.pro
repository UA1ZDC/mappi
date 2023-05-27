TEMPLATE = app
TARGET = mappi.pretreatment.service

HEADERS = \
  pretrservice.h

SOURCES = \
  pretrservice.cpp \
  main.cpp

LIBS = \
  -ltapp \
  -ltrpc \
  -ltdebug \
  -lprotobuf \
  -lmnprocread \
  -ltsingleton \
  -lmnmathtools \
  -lmnsatellite \
  -lmeteo.textproto \
  -lmeteo.settings \
  -lmeteo.geobasis \
  -lmeteo.global \
  -lmeteo.proto \
  -lmeteo.dbi \
  -lmappi.po.handler \
  -lmappi.settings \
  -lmappi.schedule \
  -lmappi.global \
  -lmappi.proto  \
  -lmappi.po.savenotify

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
