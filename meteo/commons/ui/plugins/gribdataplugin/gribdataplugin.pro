TEMPLATE = lib
TARGET = gribdata.plugin

LOG_MODULE_NAME = vizdoc

SOURCES += gribdataplugin.cpp \
           gribdatawidget.cpp

HEADERS += gribdataplugin.h \
           gribdatawidget.h

FORMS += gribdata.ui

LIBS +=                 -lmeteo.etc                     \
                        -lmeteo.global                  \
                        -lmeteo.proto                   \
                        -lmnfuncs                       \
                        -lprotobuf                      \
                        -lmeteo.map.view

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.widgets




