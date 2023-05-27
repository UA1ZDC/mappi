TEMPLATE = lib
TARGET = product.station.plugin
QT += widgets

LOG_MODULE_NAME = settings
HEADERS  += stationplugin.h

SOURCES += stationplugin.cpp

LIBS += -lprotobuf            \
        -ltapp                \
        -ltdebug              \
        -lmeteo.mainwindow    \
        -lmeteo.global        \
        -lmeteo.station.prognoz

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.app.widgets
