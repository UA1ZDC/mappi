TEMPLATE = lib
TARGET   = meteo.renamedoc.plugin
QT += widgets
FORMS           =       renamedoc.ui

PRIVATE_HEADERS =       plugin.h

SOURCES =               plugin.cpp

LIBS +=                 -ltapp                          \
                        -ltdebug                        \
                        -ltrpc                          \
                        -lprotobuf                      \
                        -lmeteo.proto                   \
                        -lmeteo.map                     \
                        -lmeteo.map.view                \
                        -lmeteo.global                  \
                        -lmeteo.geobasis                \
                        -lmeteo.mainwindow              \
                        -ltcustomui                     \
                        -lmnmathtools

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.common
