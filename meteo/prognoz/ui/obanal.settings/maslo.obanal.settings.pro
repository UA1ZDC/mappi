TEMPLATE = app
TARGET   = mappi.obanal.settings

SOURCES = main.cpp

LIBS += -lmeteo.obanal.settings

include( $$(SRC)/include.pro )
