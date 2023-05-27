TEMPLATE = lib
TARGET   = meteo.radarmap.plugin
LOG_MODULE_NAME = radar
QT += widgets
FORMS =         radarmap.ui

PRIVATE_HEADERS =       radarmap.h            \
                        plugin.h

SOURCES =               radarmap.cpp          \
                        plugin.cpp

LIBS +=                 -lprotobuf

LIBS +=                 -ltapp                          \
                        -ltdebug                        \
                        -ltrpc                          \
                        -lmeteo.proto                   \
                        -lmeteo.map                     \
                        -lmeteo.map.view                \
                        -lmeteo.global                  \
                        -lmeteo.geobasis                \
                        -lmeteodata                     \
                        -lmeteo.sql                      \
                        -lmeteo.mainwindow              \
                        -lobanal                        \
                        -ltcustomui                     \
                        -lmnmathtools

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.weather
