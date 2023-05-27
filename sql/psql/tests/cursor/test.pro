TEMPLATE = test
TARGET   = test

QT    += sql

SOURCES =     \
  test.cpp

unix: exists( /usr/bin/pg_config ):INCLUDEPATH += $$system( pg_config --includedir )

LIBS += -lmeteo.sql             \
        -lmeteo.dbi             \
        -ltincludes             \
        -ltdebug                \
        -lmeteo.global          \
        -lmeteo.settings        \
        -ltapp

include( $$(SRC)/include.pro )
