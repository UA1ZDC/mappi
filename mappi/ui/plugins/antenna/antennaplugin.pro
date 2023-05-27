TEMPLATE = lib
TARGET = antenna.plugin

QT = \
  widgets \
  serialport

DESTDIR = $(BUILD_DIR)/lib/plugins/mappi.app.widgets
LOG_MODULE_NAME = antennaplugin

HEADERS = \
  plugin.h

SOURCES = \
  plugin.cpp

LIBS = \
  -ltapp \
  -ltdebug \
  -lmeteo.mainwindow \
  -lmappi.antenna.widget \
  -lmappi.settings

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )
