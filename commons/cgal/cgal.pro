TEMPLATE = lib
TARGET   = meteo.cgal

SOURCES =       cgal_iso.cpp

PRIVATE_HEADERS =

PUB_HEADERS =   cgal_iso.h


HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS

LIBS +=  -lmeteo.geobasis \
         -lCGAL               \
         -lgmp \
         -lmpfr
include( $$(SRC)/include.pro )

