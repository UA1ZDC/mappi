TEMPLATE = lib
TARGET   = meteo.aerotable.core
QT += widgets
LOG_MODULE_NAME = aerotable

SOURCES =       aerotablewidget.cpp 


HEADERS =       aerotablewidget.h 
                
FORMS = aerotablewidget.ui tabaerotablewidget.ui 

LIBS +=                 -lmeteo.proto           \
                        -lmeteo.global          \
                        -lmeteo.etc

include( $$(SRC)/include.pro )
