TEMPLATE              = app
TARGET                = mappi.customviewer

LOG_MODULE_NAME = inter

SOURCES = main.cpp  

LIBS                 += -lmeteo.customviewer.service

include( $$(SRC)/include.pro )

