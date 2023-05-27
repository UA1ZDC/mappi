TEMPLATE = lib
TARGET = aerodiag.plugin

QT += widgets

SOURCES += aeroplugin.cpp 

HEADERS += aeroplugin.h 

LIBS +=                 -lmeteo.etc                     \
                        -lmeteo.global                  \
                        -lmeteo.proto                   \
                        -lmnfuncs                       \
                        -lprotobuf                      \
                        -lmeteo.map                     \
                        -lmeteo.map.view                \
                        -lmeteo.mainwindow              \
                        -lmeteo.aerodiag.core        \
                        -ltapp                          

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.app.widgets

DISTFILES += \
    aeroplugin.json

RESOURCES +=
