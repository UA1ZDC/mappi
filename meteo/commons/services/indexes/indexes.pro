TEMPLATE  = lib
TARGET    = meteo.indexes

SOURCES = indexessaver.cpp

HEADERS = indexessaver.h

LIBS += -ltapp              \
        -ltdebug            \
        -ltrpc              \
        -lmeteodata         \
        -lmnprocread        \
        -lprimarydb         \
        -lprotobuf          \
        -lzond              \
        -lmnmathtools

LIBS += -lmeteo.global      \
        -lmeteo.proto       \
        -lmeteo.settings    \
        -lmeteo.nosql       \
        -lmeteo.dbi         \
        -lmeteo.geobasis

LIBS += -lprotobuf

include( $$(SRC)/include.pro )

