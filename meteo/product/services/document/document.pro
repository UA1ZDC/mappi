TEMPLATE = app
TARGET   = mappi.document.service

QT += widgets

LOG_MODULE_NAME = dbconn

SOURCES =       main.cpp

LIBS += -lmeteo.map.service

include( $$(SRC)/include.pro )
