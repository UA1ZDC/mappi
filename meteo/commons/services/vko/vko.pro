TEMPLATE = lib
TARGET   = vkocalc

QT += widgets printsupport

SOURCES = vkocalc.cpp

PUB_HEADERS = vkocalc.h

HEADERS =       $$PUB_HEADERS

linux:LIBS   += -lmnprocread

LIBS         += -lmeteo.proto           \
                -lmeteo.textproto       \
                -lmeteo.global          \
                -lprotobuf              \
                -ltdebug                \
                -ltapp                  \
                -ltrpc                  \
                -ltcustomplot           \
                -lmeteo.settings


include( $$(SRC)/include.pro )                
LIBS += $$L_BOOST_PYTHON




