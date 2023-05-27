TEMPLATE     = test

QT          -= xml network

SOURCES      = client.cpp

LIBS        += -lmeteo.sql         \
               -ltapp             \
               -ltdebug           \

LIBS        += -lmeteo.geobasis   \
               -lmeteo.proto      \
               -lmeteo.sprinf     \
               -lmeteo.global     \
               -lmeteo.settings  \
               -ltrpc                \
               -lmeteo.dbi
LIBS        += -lprotobuf

include( $$(SRC)/include.pro )

HEADERS += client.h
