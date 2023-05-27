TEMPLATE = test
TARGET   = test

QT += testlib

SOURCES = \
    test.cpp

HEADERS = \
    test.h

LIBS += -lprotobuf        \
        -ltdebug          \
        -ltapp            \
        -ltrpc            \
        -lmeteo.proto     \
        -lmeteo.nosql     \
        -lmeteo.global    \
        -lmeteo.settings  \
        -lmeteo.textproto

INCLUDEPATH +=      $$(SRC)/meteo/commons/proto     \

include( $$(SRC)/include.pro )
