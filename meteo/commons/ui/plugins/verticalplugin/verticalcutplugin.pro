TEMPLATE = lib
TARGET = verticalcut.plugin
QT += widgets
LOG_MODULE_NAME = applied

SOURCES = verticalcutplugin.cpp                         \
          createcutaction.cpp

HEADERS = verticalcutplugin.h                           \
          createcutaction.h

LIBS +=                 -lmeteo.etc                     \
                        -lmeteo.global                  \
                        -lmeteo.proto                   \
                        -lmnfuncs                       \
                        -lprotobuf                      \
                        -ltapp                          \
                        -lmeteo.mainwindow              \
                        -lmeteo.map                     \
                        -lmeteo.map.view                \
                        -lmeteo.verticalcut.core         \
                        -lmeteo.map.axisaction

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.app.widgets

DISTFILES += \
    verticalcutplugin.json \
    menu.conf

RESOURCES +=
