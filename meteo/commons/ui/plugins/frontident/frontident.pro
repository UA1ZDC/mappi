TEMPLATE = lib
TARGET   = meteo.map.frontidentaction

QT += widgets

LOG_MODULE_NAME = frontident

PUB_HEADERS =           

PRIVATE_HEADERS =

HEADERS =               frontidentaction.h \
                        frontidentplugin.h \
                        frontidentwidget.h 

SOURCES =               frontidentaction.cpp \
                        frontidentplugin.cpp \
                        frontidentwidget.cpp 

FORMS = frontidentwdgt.ui 

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
        
LIBS +=                 -lmeteo.etc          \
                        -lmeteo.global       \
                        -lmeteo.map          \
                        -lmeteo.map.view

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.weather
