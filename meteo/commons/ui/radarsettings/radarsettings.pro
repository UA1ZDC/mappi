TEMPLATE = lib
TARGET = meteo.radarsettings

QT += widgets

LOG_MODULE_NAME = vizdoc

SOURCES = \
                radarsettings.cpp

HEADERS = \
                radarsettings.h

FORMS =         addradardescr.ui                        \
                radarsettings.ui

LIBS +=         -lprotobuf                      \

LIBS +=         -lmeteo.etc                     \
                -lmeteo.global                  \
                -lmeteo.proto                   \
                -lmnfuncs                       \
                -lmeteo.map.view

LIBS +=         -ltapp                          \
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
                -lmeteo.font

include( $$(SRC)/include.pro )

#DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.app.widgets

DISTFILES += \
    radarparams.json

RESOURCES +=




