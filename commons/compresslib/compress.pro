TEMPLATE = lib
TARGET   = meteo.compress

SOURCES  = \
    mcompress.cpp
HEADERS  = \
    mcompress.h

LIBS +=             -ltdebug

include( $$(SRC)/include.pro )

DEFINES += DEBUG
CONFIG += warn_off
