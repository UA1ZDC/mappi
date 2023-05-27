TEMPLATE = test
QT += widgets

SOURCES  = main.cpp

LIBS    += -ltdebug                \
           -ltapp                  \
           -ltrpc                  \
           -lmeteo.geobasis        \
           -lmeteo.map             \
           -lmeteo.map.oldgeo      \
           -lmeteo.proto           \
           -lmeteo.textproto       \
           -lmeteo.map.dataexchange \
           -lmeteo.global

include( $$(SRC)/include.pro )
