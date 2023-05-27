TEMPLATE = lib
TARGET = meteo.decodeserv

QT += network

LOG_MODULE_NAME = codecontrol
                
HEADERS = decservice.h    \
          tservicestat.h  \
          meteo_convert.h
              
SOURCES =   decservice.cpp \
            tservicestat.cpp \
            meteo_convert.cpp

LIBS   +=   -ltdebug        \
            -ltapp          \
            -lprotobuf      \
            -lmeteo.global  \
            -lmeteo.proto   \
            -lmeteo.tlgpool \
            -ltrpc          \
            -lmeteo.sql      \
            -lmeteodata

PROTOPATH +=  $(SRC)/meteo/commons/rpc    \
              $(SRC)/meteo/commons/proto

include( $$(SRC)/include.pro )
