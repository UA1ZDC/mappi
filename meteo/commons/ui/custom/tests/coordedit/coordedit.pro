TEMPLATE = app test
TARGET   = test

SOURCES =       main.cpp              \

LIBS +=         -ltcustomui           \

LIBS +=         -ltapp                \
                -ltdebug              \

LIBS +=         -lmeteo.geobasis      \
                -lmeteo.proto         \
                -lmeteo.global   \

include( $$(SRC)/include.pro )

DESTDIR = .
