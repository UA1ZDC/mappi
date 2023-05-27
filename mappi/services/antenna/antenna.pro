TARGET = mappi.antenna.service
TEMPLATE = app

QT += \
  serialport

HEADERS = \
  context.h \
  syncstate.h \
  idlingstate.h \
  warmingstate.h \
  activestate.h \
  skippingstate.h \
  servicestate.h \
  fsm.h \
  service.h \
  serverstub.h

SOURCES = \
  context.cpp \
  syncstate.cpp \
  idlingstate.cpp \
  warmingstate.cpp \
  activestate.cpp \
  skippingstate.cpp \
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
  -lmnsatellite \
  -lmnmathtools \
  -lmnprocread \
  -lmeteo.global \
  -lmeteo.proto \
  -lmeteo.dbi \
  -lmeteo.settings \
  -lmeteo.geobasis \
  -lmappi.global \
  -lmappi.proto \
  -lmappi.settings \
  -lmappi.fsm \
  -lmappi.schedule \
  -lmappi.antenna

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )
