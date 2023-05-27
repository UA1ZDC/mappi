TEMPLATE = lib
TARGET   = meteo.map.graphvalueaction

QT += widgets

LOG_MODULE_NAME = vizdoc

HEADERS =       combolabelitem.h                \
                graphvalueaction.h              \

SOURCES =       combolabelitem.cpp              \
                graphvalueaction.cpp            \

LIBS +=         -lmeteo.etc                     \
                -lmeteo.global                  \
                -lmeteo.map.view                \
                -lmeteo.graph                   \

include( $$(SRC)/include.pro )
