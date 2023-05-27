TEMPLATE = lib
TARGET   = meteo.stationlist

QT      += widgets


SOURCES =      stationlist.cpp

HEADERS =      stationlist.h

LIBS +=         -lprotobuf              \
                -lmnmathtools           \
                -ltrpc                  \
                -ltcustomui             \
                -ltdebug                \
                

LIBS +=         -lmeteo.global          \
                -lmeteo.geobasis        

FORMS =  stationlist.ui
                
include( $$(SRC)/include.pro )
