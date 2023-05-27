TEMPLATE = lib
TARGET   = meteo.journalplugin

QT += widgets

LOG_MODULE_NAME = global


LIBS +=   -ltapp                    \
          -ltdebug                  \
          -lprotobuf                \
          -lmeteo.customviewer


include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.app.widgets

HEADERS +=  journalplugin.h           \
            journalwidgetdatamodel.h \
    journalleftpanel.h

SOURCES +=  journalplugin.cpp           \
            journalwidgetdatamodel.cpp \
    journalleftpanel.cpp

FORMS += \
    journalleftpanel.ui
