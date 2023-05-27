TEMPLATE = test
TARGET = test

HEADERS =               thread.h

SOURCES =               test.cpp                \
                        thread.cpp
QT += widgets

LIBS +=                 -lprotobuf              \
                        -ltdebug                \
                        -lmnfuncs               \
                        -lmeteo.dbi             \
                        -lspcoordsys            \
                        -lmnsatellite           \
                        -lmeteo.geobasis        \
                        -lmeteo.global          \
                        -lmeteo.proto           \
                        -lmeteo.map             \
                        -lmeteo.map.oldgeo      \
                        -lmeteo.map.ptkpp       \
                        -lmeteo.map.dataexchange \
                        -lmeteo.map.view        \
                        -lmeteo.settings        \
                        -lmeteo.mainwindow      \
                        -lmnmathtools           \
                        -ltapp                  \
                        -ltrpc                  \
                        -lmappi.global          \
                        -lmappi.proto           \
                        -lmappi.pos             \
                        -lmappi.settings        \
                        -lcrossfuncs            \
                        -ltsingleton

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
