TEMPLATE = lib
TARGET = meteo.obanal.indexes
CONFIG += debug

SOURCES = obanalindexes.cpp 
   

HEADERS = obanalindexes.cpp 


LIBS    =       -lprotobuf              \
                -ltapp                  \
                -ltdebug                \
                -ltrpc                  \
                -ltsingleton            \
                -lmeteodata             \
                -lmeteo.nosql           \
                -lmeteo.dbi             \
                -lmnprocread            \
                -lmeteo.punchrules      \
                -lobanal                \
                -lmeteo.font            \
                -lmeteo.obanal          \
                -lmeteo.proto           \
                -lmeteo.global          \
                -lmeteo.settings        \
                -lmeteo.textproto

INCLUDEPATH +=  $(SRC)/meteo/commons/rpc

QMAKE_CXXFLAGS += -Wno-error=unused-function

include( $$(SRC)/include.pro )
