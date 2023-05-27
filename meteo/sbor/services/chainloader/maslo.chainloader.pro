TEMPLATE  = app
TARGET    = mappi.chainloader

LOG_MODULE_NAME = sortgmi

SOURCES = main.cpp

LIBS += -lmeteo.chainloader

include( $$(SRC)/include.pro )
