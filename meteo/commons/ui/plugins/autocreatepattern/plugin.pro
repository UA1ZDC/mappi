TEMPLATE = lib
TARGET   = meteo.autocreatepattern.plugin
QT += widgets
FORMS           =       autocreatepattern.ui

PRIVATE_HEADERS =       plugin.h

SOURCES =               plugin.cpp

LIBS +=                 -ltapp                          \
                        -ltdebug                        \
                        -ltrpc                          \
                        -lprotobuf                      \
                        -lmeteo.proto                   \
                        -lmeteo.textproto               \
                        -lmeteo.map                     \
                        -lmeteo.map.view                \
                        -lmeteo.global                  \
                        -lmeteo.geobasis                \
                        -lmeteo.mainwindow              \
                        -ltcustomui                     \
                        -lmnmathtools                   \
                        -ljobwidget


include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.weather
