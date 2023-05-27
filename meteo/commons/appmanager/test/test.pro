TEMPLATE = app test
TARGET   = test

HEADERS =

SOURCES =     test.cpp

LIBS +=       -ltdebug              \
              -ltapp                \
              -lprotobuf            \
              -lmeteo.appconf       \
              -lnovost.proto

PRO_PATH = $$system( pwd )
include( $$(SRC)/include.pro )

