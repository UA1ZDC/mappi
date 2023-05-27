TEMPLATE = lib
TARGET = rpcpywrap

QT -= gui

LIBS += -ltrpc                 \
        -lmeteo.proto          

SOURCES = rpcpywrap.cpp

include( $$(SRC)/include.pro )

LIBS += $$L_BOOST_PYTHON

