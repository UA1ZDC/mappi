TEMPLATE = app test
TARGET   = test

SOURCES =   main.cpp


LIBS +=     -lprotobuf

LIBS +=     -ltapp                  \
            -ltdebug                \
            -lmeteo.verticalcut.core\
            -lmeteo.textproto       \
            -lmeteo.global          \
            -lmeteo.proto           \
            -lnovost.proto          \
            -lmeteo.sql              \
            -lnovost.global         \
            -lnovost.settings

include( $$(SRC)/include.pro )
