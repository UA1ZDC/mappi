TEMPLATE = lib
TARGET = mappi.schedule

HEADERS = \
  config.h \
  sessiondata.h \
  session.h \
  schedule.h \
  schedulehelper.hpp

SOURCES = \
  config.cpp \
  sessiondata.cpp \
  session.cpp \
  schedule.cpp \

LIBS += \
  -lspcoordsys \
  -lmnsatellite \
  -lmnmathtools \
  -lmeteo.textproto \
  -lmappi.global \
  -lmappi.proto

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )
