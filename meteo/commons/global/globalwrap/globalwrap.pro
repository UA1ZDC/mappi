TEMPLATE = lib
TARGET = globalwrap

LIBS += -lmeteo.settings                \
        -lmeteo.global                  \
        -lmeteo.sql                     \
        -lmeteo.nosql                   \
        -lsql.proto

SOURCES = globalwrap.cpp

include( $$(SRC)/include.pro )

LIBS += $$L_BOOST_PYTHON
