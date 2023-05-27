TEMPLATE = lib
TARGET   = meteo.map.synopaction

QT += widgets

LOG_MODULE_NAME = vizdoc

PUB_HEADERS =           synopaction.h

PRIVATE_HEADERS =

HEADERS = synopwidget.h \
    synoplugin.h


SOURCES =                             \
    synopwidget.cpp \
    synoplugin.cpp \
    synopaction.cpp

LIBS +=                 -ltapp                          \
                        -ltdebug                        \
                        -lprotobuf                      \
                        -ltrpc                          \
                        -lmeteo.proto                   \
                        -lmeteo.map                     \
                        -lmeteo.geobasis                \
                        -lmeteodata                     \
                        -lmeteo.sql                      \
                        -lmeteo.mainwindow              \
                        -lobanal

LIBS +=                 -lmeteo.etc                     \
                        -lmeteo.global                  \
                        -lmeteo.map.view

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.weather

