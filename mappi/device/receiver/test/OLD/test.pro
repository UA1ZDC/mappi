TEMPLATE = test
TARGET = test

QT += widgets

SOURCES = \
  main.cpp

LIBS = \
  -ltdebug \
  -lmnsatellite \
  -lmappi.global \
  -lmappi.receiver

#LIBS = \-L$(BUILD_DIR)/lib -ltapp \
#                         -ltdebug \
#                         -lmnsatellite \

#                         -lmeteo.global \
#                         -lmappi.global \
#                         -lmeteo.dbi \
#                         -lmappi.settings \
#                         -lmeteo.settings

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
