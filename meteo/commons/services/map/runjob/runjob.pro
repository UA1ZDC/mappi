TEMPLATE = lib
TARGET = meteo.map.runjob

QT += widgets

LOG_MODULE_NAME = docmaker

SOURCES =               appmain.cpp        \
                        stdinreader.cpp

HEADERS =               stdinreader.h \
                        appmain.h

LIBS += -lprotobuf

LIBS += -ltrpc                  \
        -ltapp                  \
        -ltdebug                \
        -lmeteo.sql

LIBS += -lmeteo.proto           \
        -lmeteo.global          \
        -lmeteo.textproto       \
        -lmeteo.global          \
        -lmeteo.settings        \
        -lmeteo.map             \
        -lmeteo.map.dataexchange\
        -lmeteo.tileimage       \
        -lmeteo.map.oldgeo      \
        -lmeteo.map.ptkpp       \
        -lmeteo.planner         \
        -lmeteo.geobasis        \
        -ltcustomui             \
        -lmeteo.mainwindow      \
        -lmeteo.map.view

include( $$(SRC)/include.pro )
