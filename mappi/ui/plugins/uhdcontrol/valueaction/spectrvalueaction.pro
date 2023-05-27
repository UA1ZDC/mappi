TEMPLATE = lib
TARGET   = meteo.map.spectrvalueaction

QT += widgets

LOG_MODULE_NAME = vizdoc

HEADERS =       combolabelitem.h \
                spectrvalueaction.h

SOURCES =       combolabelitem.cpp \
                spectrvalueaction.cpp

LIBS +=         -ltapp                          \
                -ltdebug                        \
                -lmnmathtools                   \

LIBS +=         -lmeteo.etc                     \
                -lmeteo.global                  \
                -lmeteo.proto                   \
                -lmeteo.map                     \
                -lmeteo.map.view                \
#                -lmeteo.graph                   \
                -lmeteo.geobasis                \
#                -lmeteo.graphitems              \
                -lmeteo.map.spectrcoordaction

include( $$(SRC)/include.pro )
