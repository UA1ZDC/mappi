TEMPLATE  = app
TARGET    = mappi.postcontrol

QT       += widgets

LOG_MODULE_NAME = codecontrol

SOURCES = main.cpp   \
          postcontrol.cpp

HEADERS = postcontrol.h


LIBS += -ltapp              \
        -ltdebug            \
        -lcrossfuncs        \
        -lmeteodata         \
        -lmnprocread        \
        -lprimarydb         \
        -lprotobuf          \
        -lmeteo.sql          \
        -ltalphanum         \
        -ltsingleton        \
        -ltmeteocontrol     \
        -lmeteo.dbi


LIBS += -lmeteo.global      \
        -lmeteo.proto       \
        -lmeteo.settings    \
        -lmeteo.geobasis

include( $$(SRC)/include.pro )

