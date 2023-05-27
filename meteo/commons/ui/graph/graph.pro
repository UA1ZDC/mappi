TEMPLATE = lib
TARGET   = meteo.graph

QT      += widgets

SOURCES =       lineitem.cpp            \
                labelitem.cpp           \
                labellayout.cpp         \

HEADERS =       lineitem.h              \
                labelitem.h             \
                labellayout.h           \

LIBS +=         -lprotobuf              \

LIBS +=         -lmnmathtools           \
                -ltdebug                \
                -ltapp                  \
                -ltrpc                  \
                -lmeteodata             \

LIBS +=         -lmeteo.global          \
#                -lmeteo.geom            \
                -lmeteo.geobasis        \
                -lmeteo.map             \
                -lmeteo.map.view        \
                -lmeteo.proto           \
                -lmeteo.mainwindow      \

include( $$(SRC)/include.pro )
