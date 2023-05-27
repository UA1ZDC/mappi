TEMPLATE = lib
TARGET = servicestate

QT += network

LOG_MODULE_NAME = contrdiag

PRIVATE_HEADERS = 

PUB_HEADERS = rpctimer.h

SOURCES     =   rpctimer.cpp

LIBS       +=   -ltdebug            \
                -ltapp              \
                -lmeteo.proto       \
                -lmeteo.global      \
                -lmeteo.decodeserv  \
                -lprotobuf          \
                -ltrpc           

include( $$(SRC)/include.pro )

