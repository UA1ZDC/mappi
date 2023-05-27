TEMPLATE = app
TARGET   = mappi.srcdata

LOG_MODULE_NAME = dbconn

SOURCES += main.cpp

LIBS +=         -lmeteo.srcdata

include( $$(SRC)/include.pro )
