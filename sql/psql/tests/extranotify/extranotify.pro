TEMPLATE = app test
TARGET   = test

SOURCES =   test.cpp

LIBS +=   -ltapp      \
          -ltdebug    \
          -lmeteo.sql

include( $$(SRC)/include.pro )

HEADERS += test.h

