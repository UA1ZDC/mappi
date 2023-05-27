TEMPLATE = lib
TARGET   = meteo.map.graphcoordaction

QT += widgets

LOG_MODULE_NAME = vizdoc

HEADERS =       graphcoordaction.h              \

SOURCES =       graphcoordaction.cpp            \

LIBS +=         -ltapp                          \
                -ltdebug                        \
                -lmnmathtools                   \

LIBS +=         -lmeteo.etc                     \
                -lmeteo.global                  \
                -lmeteo.proto                   \
                -lmeteo.map                     \
                -lmeteo.map.view                \
                -lmeteo.graph                   \
                -lmeteo.geobasis                \
                -lmeteo.graphitems              \

include( $$(SRC)/include.pro )
