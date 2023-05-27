TEMPLATE = lib
TARGET = meteo.station.prognoz
QT += widgets

HEADERS += punktwidget.h

SOURCES += punktwidget.cpp

FORMS    += punktwidget.ui

LIBS += -lprotobuf            \
        -ltapp                \
        -ltdebug              \
        -ltrpc                \
        -ltcustomui           \
        -lmnmathtools         \
        -lmeteo.etc           \
        -lmeteo.textproto     \
        -lmeteo.geobasis      \
        -lmeteo.mainwindow    \
        -lmeteo.proto         \
        -lmeteo.global        \

include( $$(SRC)/include.pro )
