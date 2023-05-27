
TEMPLATE = app
TARGET = restore_data
INCLUDEPATH += .

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += restore_aero.h
SOURCES += main.cpp restore_aero.cpp

LIBS    =       -lprotobuf              \
                -ltapp                  \
                -ltdebug                \
                -ltrpc                  \
                -ltsingleton            \
                -lmeteodata             \
                -lmeteo.nosql           \
                -lmeteo.dbi             \
                -lmnprocread            \
                -lmeteo.punchrules      \
                -lobanal                \
                -lmeteo.font            \
                -lmeteo.obanal          \
                -lmeteo.proto           \
                -lmeteo.global          \
                -lmeteo.settings        \
                -lmeteo.textproto       \
                -lzond

include( $$(SRC)/include.pro )
