TEMPLATE = lib
TARGET   = meteo.map.phload

LOG_MODULE_NAME = vizdoc

HEADERS =     ph.h

SOURCES =     ph.cpp

LIBS +=       -lmeteo.font              \
              -lmeteo.verticalcut.core  \

include( $$(SRC)/include.pro )
