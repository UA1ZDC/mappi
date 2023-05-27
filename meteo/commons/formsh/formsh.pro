TEMPLATE = lib
TARGET   = meteo.formsh

SOURCES =               tformadaptor.cpp                \
                        taction.cpp                     \
                        thandler.cpp

PUB_HEADERS =           tformadaptor.h                  \
                        taction.h                       \
                        thandler.h

LIBS +=                 -ltdebug                        \
                        -lmeteo.proto
							
include( $$(SRC)/include.pro )

QT += widgets
