QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mappi.thematic.import
TEMPLATE = app


HEADERS += import.h mainwindow.h

SOURCES += import.cpp main.cpp mainwindow.cpp

FORMS +=  mainwindow.ui

LIBS +=           -ltapp                     \
                  -ltdebug                   \
                  -lprotobuf                 \
                  -ltrpc                     \
                  -ltsingleton               \
                  -lmnprocread               \
                  -lmeteo.global             \
                  -lmeteo.proto              \
                  -lmeteo.settings           \
                  -lmeteo.textproto          \
                  -lmeteo.geobasis           \
                  -lmeteo.dbi                \
                  -lmappi.global             \
                  -lmappi.proto              \
                  -lmappi.settings           \
                  -lmappi.them.algs_calc     \
                  -lmappi.po.formats \
                  -lmappi.po.savenotify      \
                  -lmnsatellite              \
                  -lgeotiff                  \
                  -ltiff                     \
                  -lgdal



PROTOPATH +=    $(SRC)/meteo/commons/proto

include( $$(SRC)/protobuf.pri )
include($$(SRC)/include.pro)

QMAKE_CXXFLAGS += -Wno-error=class-memaccess
QMAKE_CXXFLAGS += -Wno-error=unused-result
