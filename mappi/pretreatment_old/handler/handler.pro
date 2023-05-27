TEMPLATE    =   lib
TARGET      =   mappi.po.handler

QMAKE_LFLAGS += -Wl,--no-as-needed 

PRIVATE_HHEADERS = 

PUB_HEADERS =   handler.h 

HEADERS     =   $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES     =   handler.cpp 


LIBS       +=   -ltdebug \
                -lmappi.global \
                -lmappi.po.frame \
                -lmappi.po.formats \
                -lmappi.po.instruments \
                

include( $$(SRC)/include.pro )
