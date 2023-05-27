TEMPLATE = app
TARGET   = mappi.msgcenter

LOG_MODULE_NAME = recgmi

SOURCES =       main.cpp

LIBS    =       -lmeteo.msgcenter       \
                -lmeteo.punchrules       \
                -ltalphanum
                                 
include( $$(SRC)/include.pro )   
                                 
                                 
                                 
                                 
                                 
                                 
