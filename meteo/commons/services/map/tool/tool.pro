TEMPLATE = lib
TARGET = meteo.map.client

QT += widgets

LOG_MODULE_NAME = docmaker

SOURCES =               appmain.cpp        \
                        mapclient.cpp

HEADERS =               appmain.h   \
                        mapclient.h

LIBS += -lprotobuf

LIBS += -ltrpc                  \
        -ltapp                  \
        -ltdebug                \
        -lmeteo.sql              \

LIBS += -lmeteo.proto           \
        -lmeteo.global          \
        -lmeteo.textproto       \
        -lmeteo.global          \
        -lmeteo.settings        \
        -lmeteo.map             \
        -lmeteo.map.dataexchange\
        -lmeteo.map.oldgeo      \
        -lmeteo.planner         \
        -lmeteo.geobasis        \
        -ltcustomui             \
        -lmeteo.mainwindow      \
        -lmeteo.map.view        \
        -lmeteo.map.dataexchange
	-lmeteo.tileimage


include( $$(SRC)/include.pro )
