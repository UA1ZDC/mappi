TEMPLATE = lib
TARGET = product.plotting.plugin

QT += widgets

LOG_MODULE_NAME = vizdoc

SOURCES = plottingplugin.cpp

HEADERS = plottingplugin.h

LIBS +=                 -lmeteo.etc                     \
                        -lmeteo.global                  \
                        -lmeteo.proto
                        
LIBS +=                 -ltapp                          \
                        -ltdebug                        \
                        -lmeteo.map                     \
                        -lplotting

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.weather




