TEMPLATE = test
TARGET   = test

SOURCES =   test.cpp

LIBS =  -lmeteo.sql             \
        -lmeteo.dbi             \
        -lmeteo.global          \
        -lmeteo.nosql           \
        -lmeteo.settings        \
        -ltincludes             \
        -ltdebug                \
        -ltapp                  \
        -lprotobuf              \
        -lmeteo.proto

include( $$(SRC)/include.pro )
