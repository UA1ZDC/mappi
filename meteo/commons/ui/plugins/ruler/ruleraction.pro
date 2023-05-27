TEMPLATE = lib
TARGET   = meteo.map.ruleraction

QT += widgets

LOG_MODULE_NAME = vizdoc

PUB_HEADERS =           ruleraction.h

PRIVATE_HEADERS =

HEADERS =               rulerplugin.h

SOURCES =               ruleraction.cpp \
                        rulerplugin.cpp

LIBS +=                 -ltapp                          \
                        -ltdebug                        \
                        -lprotobuf                      \
                        -ltrpc                          \
                        -lmeteo.proto                   \
                        -lmeteo.map                     \
                        -lmeteo.geobasis

LIBS +=                 -lmeteo.etc          \
                        -lmeteo.global       \
                        -lmeteo.map.view

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.weather
