TEMPLATE = app test
TARGET   = test

QT    += sql

SOURCES =     \
  test.cpp

unix: exists( /usr/bin/pg_config ):INCLUDEPATH += $$system( pg_config --includedir )

LIBS += -lmeteo.sql      \
        -ltincludes     \
        -ltdebug        \
        -ltapp

include( $$(SRC)/include.pro )
