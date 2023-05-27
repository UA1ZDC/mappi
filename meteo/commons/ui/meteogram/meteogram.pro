TEMPLATE = lib
TARGET   = meteo.meteogram

QT      += widgets

LOG_MODULE_NAME = applied

SOURCES =      meteogramwindow.cpp

HEADERS =      meteogramwindow.h

LIBS +=         -lprotobuf              \

LIBS +=         -lmnmathtools           \
                -ltapp                  \
                -ltdebug                \
                -ltrpc                  \
                -lmeteodata             \
                -ltcustomui             

LIBS +=         -lmeteo.global          \
                -lmeteo.geobasis        \
                -lmeteo.map             \
                -lmeteo.map.view        \
                -lmeteo.proto           \
                -lmeteo.mainwindow      \
                -lmeteo.graph           \
                -lmeteo.map.graphcoordaction    \
                -lmeteo.map.graphvalueaction

include( $$(SRC)/include.pro )
