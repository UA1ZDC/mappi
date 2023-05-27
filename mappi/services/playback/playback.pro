TARGET = mappi.playback.service
TEMPLATE = app

HEADERS = \
  context.h \
  findstate.h \
  prewaitstate.h \
  warmingstate.h \
  activestate.h \
  servicestate.h \
  fsm.h \
  service.h \
  serverstub.h

SOURCES = \
  context.cpp \
  findstate.cpp \
  prewaitstate.cpp \
  warmingstate.cpp \
  activestate.cpp \
  servicestate.cpp \
  fsm.cpp \
  service.cpp \
  serverstub.cpp \
  main.cpp

LIBS = \
  -ltdebug \
  -ltapp \
  -ltrpc \
  -lprotobuf \
  -lmnprocread \
  -lmnsatellite \
  -lmeteo.global \
  -lmeteo.proto \
  -lmeteo.dbi \
  -lmeteo.settings \
  -lmeteo.textproto \
  -lmappi.global \
  -lmappi.proto \
  -lmappi.settings \
  -lmappi.fsm \
  -lmappi.schedule

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )
