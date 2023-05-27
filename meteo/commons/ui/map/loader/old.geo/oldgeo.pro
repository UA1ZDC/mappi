TEMPLATE = lib
TARGET   = meteo.map.oldgeo

QT += widgets

LOG_MODULE_NAME = vizdoc

PUB_HEADERS =           oldgeo.h

PRIVATE_HEADERS =

HEADERS =     $$PUB_HEADERS  $$PRIVATE_HEADERS

SOURCES =               oldgeo.cpp

LIBS +=                 -lmeteo.font    \
                        -lmeteo.sql      \
                        -lprotobuf      \
                        -ltapp          \
                        -ltdebug        \
                        -lmeteo.proto   \
                        -lmeteo.map     \
                        -lmeteo.geobasis\
                        -lmnmathtools   \
                        -lmeteo.textproto \
                        -lmeteo.global

include( $$(SRC)/include.pro )

GEO_FILES = files/*.geo

share.files = $$GEO_FILES
share.path  = $$(BUILD_DIR)/share/meteo/geo

INSTALLS += share
