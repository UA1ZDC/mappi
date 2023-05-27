TEMPLATE  = app
TARGET    = mappi.bufr

QT       += widgets

LOG_MODULE_NAME = codecontrol

SOURCES = main.cpp

LIBS += -ltbufr

include( $$(SRC)/include.pro )
