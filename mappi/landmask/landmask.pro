TEMPLATE      = lib
TARGET        = mappi.landmask

SOURCES       = landmask.cpp

HEADERS       = landmask.h

LIBS         += -ltapp                              \
                -lmnmathtools                       \
                -lmeteo.landmask                    \
                -lmeteo.geobasis                    \
                -lmeteo.astrodata

include( $$(SRC)/include.pro )
