TEMPLATE = app
TARGET = meteo.map.runjob

QT += widgets

LOG_MODULE_NAME = docmaker

SOURCES =               main.cpp


LIBS += -lmeteo.map.runjob

include( $$(SRC)/include.pro )
