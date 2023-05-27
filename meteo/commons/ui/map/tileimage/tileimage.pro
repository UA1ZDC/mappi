TEMPLATE = lib
TARGET   = meteo.tileimage

QT += widgets

LOG_MODULE_NAME = map
PUB_HEADERS =           basiscache.h

HEADERS =     $$PUB_HEADERS  $$PRIVATE_HEADERS



SOURCES =               basiscache.cpp


include( $$(SRC)/include.pro )
