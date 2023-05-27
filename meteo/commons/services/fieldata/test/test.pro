TEMPLATE = app test
TARGET = test.fielddata

SOURCES = main.cpp

HEADERS = headers.h       \
          getfielddata.h \
    getfieldsmdata.h \
    getfieldsdata.h

LIBS += -lmeteo.sql              \
        -lmeteodata             \
        -lmnprocread            \
        -ltrpc                  \
        -ltapp                  \
        -ltdebug                \
        -lmeteo.proto           \
        -lmeteo.global          \
        -lbank.global           \
        -lbank.settings         \
        -lbank.proto

LIBS += -lprotobuf              \
        -lgtest

PRO_PATH = $$system( pwd )
include( $$(SRC)/include.pro )
