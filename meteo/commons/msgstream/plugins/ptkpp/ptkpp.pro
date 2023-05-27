TEMPLATE  = lib
TARGET    = meteo.msgstream.ptkpp

LOG_MODULE_NAME = recgmi

HEADERS =       ptkppstreamrecv.h \
    ptkppstreamservice.h

SOURCES =       ptkppstreamrecv.cpp \
    ptkppstreamservice.cpp

LIBS +=         -lprotobuf              \

LIBS +=         -ltapp                  \
                -ltdebug

LIBS +=         -lmeteo.msgstream       \
                -lmeteo.msgparser

include( $$(SRC)/include.pro )
