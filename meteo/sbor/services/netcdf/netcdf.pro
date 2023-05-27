TEMPLATE = app
TARGET   = netcdf.service

SOURCES  = main.cpp

LIBS    =       -ltapp                  \
                -ltdebug                \
                -ltrpc                  \
                -lmnprocread            \
                -lmeteo.proto           \
                -lmeteo.settings        \
                -lmeteo.global          \
                -lmeteo.dbi             \
                -lmeteo.esimonc.converter
                


INCLUDEPATH +=  $(SRC)/meteo/commons/rpc

LIBS    += -lprotobuf

include( $$(SRC)/include.pro )
