TEMPLATE = lib
TARGET = oceandiag.plugin

QT += widgets

SOURCES += oceanplugin.cpp \
           oceanaction.cpp

HEADERS += oceanplugin.h \
           oceanaction.h

LIBS +=                 -lmeteo.etc                     \
                        -lmeteo.global                  \
                        -lmeteo.proto                   \
                        -lmnfuncs                       \
                        -ltapp                          \
                        -ltdebug                        \
                        -lprotobuf                      \
                        -lmeteo.map.view                \
                        -lmeteo.mainwindow              \
                        -lmeteo.oceandiag.core          \
                        -lmeteo.map.graphcoordaction    \
                        -lmeteo.map.graphvalueaction

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.app.widgets

DISTFILES += \
    oceanplugin.json

RESOURCES +=
