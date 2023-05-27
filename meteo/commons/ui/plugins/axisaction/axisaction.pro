TEMPLATE = lib
TARGET   = meteo.map.axisaction

QT += widgets

LOG_MODULE_NAME = vizdoc

HEADERS =       axisaction.h                    \
                pointaction.h

SOURCES =       axisaction.cpp                  \
                pointaction.cpp

LIBS +=         -ltapp                          \
                -ltdebug                        \
                -lmnmathtools

LIBS +=         -lmeteo.etc                     \
                -lmeteo.global                  \
                -lmeteo.proto                   \
                -lmeteo.map                     \
                -lmeteo.graphitems              \
                -lmeteo.map.view

include( $$(SRC)/include.pro )
