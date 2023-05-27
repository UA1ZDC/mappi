TEMPLATE    =   lib
TARGET      =   mappi.po.handler

CONFIG += c++17

QMAKE_LFLAGS += -Wl,--no-as-needed

INCLUDEPATH += $$(SRC)/satdump/src-core

PRIVATE_HHEADERS = 

PUB_HEADERS =   handler.h 

HEADERS     =   $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES     =   handler.cpp 


LIBS       +=   -ltdebug \
                -lmappi.global \
                -lmappi.po.formats \
                -lsatdump_core
                

include( $$(SRC)/include.pro )
