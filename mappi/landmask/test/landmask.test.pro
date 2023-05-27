TEMPLATE      = test
TARGET        = test

QT           += widgets

HEADERS =

SOURCES =           main.cpp

LIBS +=             -ltapp                \
                    -ltdebug              \
                    -lmnfuncs             \
                    -lmeteo.landmask      \
                    -lmappi.landmask      \
                    -lmeteo.geobasis      \
                    -lmeteo.astrodata     \

include( $$(SRC)/include.pro )
