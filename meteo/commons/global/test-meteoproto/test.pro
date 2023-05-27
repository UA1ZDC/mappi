TEMPLATE              = test
TARGET                = maslo.global.getmeteodataproto.test

QT += testlib

HEADERS = test.h
SOURCES = test.cpp

LIBS                 += -ltdebug          \
                        -ltapp            \
                        -lmeteo.settings  \
                        -lmnprocread      \
                        -ltrpc            \
                        -lmeteo.proto     \
                        -lmeteo.nosql     \
                        -lprotobuf        \
                        -lmeteo.global    \
                        -lmeteo.textproto


include( $$(SRC)/include.pro )

RESOURCES += \
    test.qrc
