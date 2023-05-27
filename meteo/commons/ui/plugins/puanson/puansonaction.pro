TEMPLATE = lib
TARGET   = meteo.map.puansonview

QT += widgets

LOG_MODULE_NAME = vizdoc

PRIVATE_HEADERS =       puansonaction.h         \
                        puansonview.h           \
                        dlgvalue.h              \
                        puansonitem.h

SOURCES =               puansonaction.cpp       \
                        puansonview.cpp         \
                        dlgvalue.cpp            \
                        puansonitem.cpp

FORMS =                 puansonview.ui                  \
                        dlgvalue.ui
                        
LIBS +=                 -ltapp                          \
                        -ltdebug                        \
                        -lprotobuf                      \
                        -ltrpc                          \
                        -lmeteo.proto                   \
                        -lmeteo.map                     \
                        -lmeteo.geobasis                \
                        -lmeteodata                     \
                        -lmeteo.sql

LIBS +=                 -lmeteo.etc                     \
                        -lmeteo.global                  \
                        -lmeteo.map.view                \
                        -lmeteo.meteogram

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.common
