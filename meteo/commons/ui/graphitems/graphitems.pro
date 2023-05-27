TEMPLATE = lib
TARGET   = meteo.graphitems

QT += widgets

LOG_MODULE_NAME = vizdoc

HEADERS =   markeritem.h                  \
            puansonitem.h                 \
            geoaxisitem.h

SOURCES =   markeritem.cpp                \
            puansonitem.cpp               \
            geoaxisitem.cpp

LIBS              += -ltdebug                  \
                     -lmeteo.map               \
                     -lmeteo.map.view          \
                     -lmeteo.geobasis          \

include( $$(SRC)/include.pro )
