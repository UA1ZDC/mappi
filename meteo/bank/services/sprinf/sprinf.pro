TEMPLATE = app
TARGET   = mappi.sprinf

SOURCES += main.cpp

LIBS +=    -lmeteo.sprinf

include( $$(SRC)/include.pro )
