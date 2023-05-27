TEMPLATE = lib
TARGET   = meteo.map.ptkpp

QT += widgets

LOG_MODULE_NAME = vizdoc

PUB_HEADERS =           ptkppgeo.h

PRIVATE_HEADERS =

HEADERS =     $$PUB_HEADERS  $$PRIVATE_HEADERS

SOURCES =               ptkppgeo.cpp

LIBS +=                 -lmeteo.font      \
                        -lprotobuf        \
                        -lmeteo.sql        \
                        -ltapp            \
                        -ltdebug          \
                        -lmeteo.proto     \
                        -lmeteo.global    \
                        -lmeteo.textproto \
                        -ltsingleton      \
                        -lmeteo.map

include( $$(SRC)/include.pro )

GEO_FILES = files/*.geo

share.files = $$GEO_FILES
share.path  = $$(BUILD_DIR)/share/meteo/geo

INSTALLS += share
