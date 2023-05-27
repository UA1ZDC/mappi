TEMPLATE = app 
TARGET   = mappi.fieldata

LOG_MODULE_NAME = dbconn

SOURCES =       main.cpp

LIBS +=         -lmeteo.fieldata

include( $$(SRC)/include.pro )
