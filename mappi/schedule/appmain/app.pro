TEMPLATE = app
TARGET = mappi.schedule

SOURCES = \
  main.cpp

LIBS += \
  -lmappi.schedule \
  -lmappi.global \
  -ltapp \
  -ltdebug

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )
