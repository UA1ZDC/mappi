TEMPLATE = lib
TARGET   = meteo.oceantable.core
QT += widgets
LOG_MODULE_NAME = oceantable

SOURCES =       oceantablewidget.cpp 


HEADERS =       oceantablewidget.h 
                
FORMS = oceantablewidget.ui      \
        taboceantablewidget.ui

LIBS +=                 -lmeteo.proto          \
                        -lmeteo.global         \
                        -lmeteodata            \
                        -lmeteo.etc

include( $$(SRC)/include.pro )
