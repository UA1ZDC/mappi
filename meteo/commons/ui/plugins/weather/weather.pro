TEMPLATE = lib
TARGET   = meteo.weather.plugin

QT += widgets

LOG_MODULE_NAME = docmaker

PRIVATE_HEADERS =       termselect.h            \
                        weatheritem.h           \
                        docdlg.h                \
                        plugin.h

SOURCES =               termselect.cpp          \
                        weatheritem.cpp         \
                        docdlg.cpp              \
                        plugin.cpp

FORMS =                 termselect.ui           \
                        blankparams.ui

LIBS +=                 -lprotobuf

LIBS +=                 -ltapp                          \
                        -ltdebug                        \
                        -ltrpc                          \
                        -lmeteo.proto                   \
                        -lmeteo.map                     \
                        -lmeteo.map.view                \
                        -lmeteo.global                  \
                        -lmeteo.geobasis                \
                        -lmeteodata                     \
                        -lmeteo.sql                      \
                        -lmeteo.mainwindow              \
                        -lobanal                        \
                        -ltcustomui                     \
                        -lmnmathtools                   \
                        -lmeteo.map.ptkpp               \
                        -lmeteo.map.oldgeo

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.app.widgets
