TEMPLATE = app test
TARGET   = test

HEADERS =         \
              main.h          \
    dbthread.h

SOURCES =     main.cpp        \


LIBS +=       -ltapp          \
              -ltdebug        \
              -lmeteo.sql      \

include( $$(SRC)/include.pro )
