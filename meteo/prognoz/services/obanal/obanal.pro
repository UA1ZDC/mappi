TEMPLATE = app 
TARGET = mappi.obanal
LOG_MODULE_NAME = obanal

SOURCES = main.cpp

LIBS    =       -lmeteo.obanal          \
                -lmeteo.obanal.appmain  \
                -lmeteo.punchrules
                                
include( $$(SRC)/include.pro )  
