TEMPLATE = app
TARGET = mappi.pretreatment.service

CONFIG += c++17

INCLUDEPATH += $$(SRC)/satdump/src-core

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
  -lmappi.po.savenotify\
  -lmappi.po.formats\
  -lmappi.po.images \
  -lsatdump_core

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
