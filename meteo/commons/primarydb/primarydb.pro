TEMPLATE = lib
TARGET   = primarydb
QT += network

LOG_MODULE_NAME = codecontrol

SOURCES = gribmongo.cpp   \
          ancdb.cpp \
          dbmeta.cpp

PUB_HEADERS = gribmongo.h \
              ancdb.h \
              dbmeta.h

HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS


LIBS += -lmeteodata         \
        -lprotobuf          \
        -lmeteo.sql          \
        -ltapp              \
        -ltdebug            \
        -ltgrib             \
        -ltgribiface        \
        -lmeteo.nosql       \
        -lmeteo.proto       \
        -lmeteo.global      \
        -lmeteo.textproto   \
        -lmeteo.geobasis    \
        -lmeteo.dbi    \
        -lobanal

include( $$(SRC)/include.pro )


