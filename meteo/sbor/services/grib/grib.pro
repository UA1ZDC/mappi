TEMPLATE  = app
TARGET    = mappi.grib

QT       += widgets

LOG_MODULE_NAME = codecontrol

SOURCES = main.cpp

LIBS += -ltgribdecode

include( $$(SRC)/include.pro )
