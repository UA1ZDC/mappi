TEMPLATE = lib
TARGET   = meteo.map.sxfloader

LOG_MODULE_NAME = vizdoc

PUB_HEADERS =           

PRIVATE_HEADERS = sxf.h

HEADERS =     $$PUB_HEADERS  $$PRIVATE_HEADERS

SOURCES =               sxf.cpp

LIBS +=                 -lmeteo.map.dataexchange

include( $$(SRC)/include.pro )



