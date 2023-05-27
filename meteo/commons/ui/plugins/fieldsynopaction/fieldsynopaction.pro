TEMPLATE = lib
TARGET   = meteo.map.fieldsynopaction

LOG_MODULE_NAME = vizdoc

PUB_HEADERS =           fieldsynopaction.h

PRIVATE_HEADERS =

HEADERS += fieldsynoplugin.h
#    fieldsynopwidg.h \
#    fieldsynoproxy.h

SOURCES +=  fieldsynopaction.cpp  \
            fieldsynoplugin.cpp   \
            fieldsynopwidg.cpp    \
            fieldsynoproxy.cpp

LIBS +=                 -lmeteo.etc                     \
                        -lmeteo.global                  \
                        -lmeteo.map.view

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.common
