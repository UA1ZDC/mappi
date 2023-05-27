TEMPLATE = app
TARGET   = meteo.map.client

QT += widgets

LOG_MODULE_NAME = dbconn

SOURCES =       main.cpp

LIBS += -lmeteo.map.client

include( $$(SRC)/include.pro )
