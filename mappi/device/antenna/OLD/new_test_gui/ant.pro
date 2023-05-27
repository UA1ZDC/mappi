TEMPLATE = app
TARGET = mappi.test.antenna.new

QT += \
  widgets \
  serialport

SOURCES = \
  main.cpp

LIBS += \
  -L$(BUILD_DIR)/lib \
  -ltapp \
  -ltdebug \
  -lmnsatellite \
  -lmeteo.dbi \
  -lmeteo.global \
  -lmeteo.settings \
  -lmappi.settings \
  -lmappi.app.antenna_new

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
