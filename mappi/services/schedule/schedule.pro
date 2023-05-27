TARGET = mappi.schedule.service
TEMPLATE = app

QT += \
  core

HEADERS = \
  context.h \
  service.h \
  serverstub.h \
  tlemonitor.h

SOURCES = \
  context.cpp \
  service.cpp \
  serverstub.cpp \
  tlemonitor.cpp \
  main.cpp

LIBS = \
  -ltapp \
  -ltrpc \
  -ltdebug \
  -lprotobuf \
  -lmnprocread \
  -lmnsatellite \
  -lmeteo.dbi \
  -lmeteo.proto \
  -lmeteo.global \
  -lmeteo.settings \
  -lmappi.proto \
  -lmappi.global \
  -lmappi.settings \
  -lmappi.schedule

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )
