TEMPLATE = lib
TARGET   = meteo.wrb.map

QT += widgets

LOG_MODULE_NAME = vizdoc

HEADERS =       coordrequestaction.h            \

SOURCES =       coordrequestaction.cpp          \

LIBS +=         -ltapp                          \
                -ltdebug                        \
                
LIBS +=         -lmeteo.map.view                \
                -lmeteo.map                     \
                -lmeteo.geobasis                \

include( $$(SRC)/include.pro )
