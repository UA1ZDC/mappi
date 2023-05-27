TEMPLATE = lib 
TARGET = meteo.obanal.appmain

LOG_MODULE_NAME = obanal.appmain

SOURCES =               maker.cpp   \
                        appmain.cpp

HEADERS =               maker.h     \
                        appmain.h

LIBS += -ltrpc                  \
        -ltapp                  \
        -ltdebug                \
        -lobanal                

include( $$(SRC)/include.pro )
