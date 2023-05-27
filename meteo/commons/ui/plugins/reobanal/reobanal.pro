TEMPLATE = lib
TARGET   = meteo.map.reobanalaction

QT += widgets

LOG_MODULE_NAME = reobanal

PUB_HEADERS =

PRIVATE_HEADERS =

HEADERS =               reobanalaction.h \
                        reobanalplugin.h \
                        reobanalwidget.h

SOURCES =               reobanalaction.cpp \
                        reobanalplugin.cpp \
                        reobanalwidget.cpp

FORMS = reobanalwdgt.ui

LIBS += -lprotobuf            \
        -ltapp                \
        -ltdebug              \
        -ltrpc                \
        -lobanal              \
        -lmnfuncs             \
        -lmnmathtools         \
        -lmeteo.textproto     \
        -lmeteo.mainwindow    \
        -lmeteo.proto         \
        -lmeteo.geobasis      \
        -lmeteo.global        \
        -lmeteo.obanal        \

LIBS +=                 -lmeteo.etc          \
                        -lmeteo.global       \
                        -lmeteo.map          \
                        -lmeteo.map.view  \
                    #    -lmeteo.cgal      \
                    #    -lCGAL               \
                   #     -lgmp

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.weather
