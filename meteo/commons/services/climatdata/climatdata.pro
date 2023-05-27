TEMPLATE = lib
TARGET   = meteo.climatdata

SOURCES = climatservice.cpp \
          climatsurf.cpp \
          climataero.cpp

PUB_HEADERS = climatservice.h \
              climatsurf.h \
              climataero.h

linux:LIBS   += -lmnprocread

LIBS         += -lmeteo.proto           \
                -lmeteo.textproto       \
                -lmeteo.global          \
                -lprotobuf              \
                -lmeteo.sql              \
                -ltdebug                \
                -ltapp                  \
                -ltrpc                  \
                -lmeteodata             \
                -lzond                   \
                -lmeteo.sql

include( $$(SRC)/include.pro )
