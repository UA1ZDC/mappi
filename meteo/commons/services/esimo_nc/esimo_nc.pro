TARGET      = meteo.esimonc.converter
TEMPLATE    = lib


HEADERS     = convert_nc.h \
              esimonc.h    \
              esimoparam.h

SOURCES     = convert_nc.cpp \
              esimonc.cpp \
              esimoparam.cpp

              
LIBS       += -ltapp              \
              -ltdebug            \
              -ltrpc              \
              -lobanal            


LIBS       += -lmeteo.proto       \
              -lmeteo.global      \
              -lmeteo.geobasis    \
              -lmeteo.obanal      \
              -lmeteo.dbi         \
              -lprimarydb

LIBS       += -lnetcdf            


include( $$(SRC)/include.pro )
