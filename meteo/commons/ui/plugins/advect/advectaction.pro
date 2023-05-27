TEMPLATE = lib
TARGET   = meteo.map.advectaction

QT += widgets

LOG_MODULE_NAME = vizdoc

PUB_HEADERS =           advectaction.h

PRIVATE_HEADERS =

HEADERS =               advectplugin.h \
    prepeartransferwidget.h

SOURCES =               advectaction.cpp \
                        advectplugin.cpp \
    prepeartransferwidget.cpp

FORMS = \
    prepeartransferwidget.ui

LIBS +=                 -lmeteo.etc             \
                        -lmeteo.global          \
                        -lmeteo.map.view        \
                        -lmeteo.map.view.drawtools


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
                        -ltcustomui
                        
include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.weather
