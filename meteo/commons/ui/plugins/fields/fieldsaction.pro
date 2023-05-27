TEMPLATE = lib
TARGET   = meteo.map.fieldsaction

QT += widgets

LOG_MODULE_NAME = vizdoc

PUB_HEADERS =           fieldsaction.h

PRIVATE_HEADERS =

HEADERS = fieldsplugin.h \
    fieldswidget.h 

SOURCES =               fieldsaction.cpp                \
                        fieldsplugin.cpp                \
                        fieldswidget.cpp

LIBS +=                 -ltapp                          \
                        -ltdebug                        \
                        -lprotobuf                      \
                        -ltrpc                          \
                        -lmeteo.proto                   \
                        -lmeteo.map                     \
                        -lmeteo.geobasis                \
                        -lmeteodata                     \
                        -lmeteo.sql                      \
                        -lmeteo.mainwindow

LIBS +=                 -lmeteo.etc                     \
                        -lmeteo.global                  \
                        -lmeteo.map.view

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.common
