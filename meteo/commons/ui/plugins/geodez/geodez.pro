TEMPLATE = lib
TARGET   = meteo.map.geodez

#SOURCES +=\
#        geodez.cpp

#HEADERS  += geodez.h

#FORMS    += geodez.ui


PUB_HEADERS =           geodez.h

PRIVATE_HEADERS =

HEADERS =               $$PUB_HEADERS \
    geodez.h \
    geodezplugin.h


SOURCES =                             \
    geodez.cpp \
    geodezplugin.cpp



LIBS +=                 -lmeteo.etc                     \
                        -lmeteo.global                  \
                        -lmeteo.map.view

FORMS +=                 geodez.ui

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.widgets
