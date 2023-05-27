TEMPLATE = lib
TARGET   = meteo.meteogram.plugin

QT      += widgets

LOG_MODULE_NAME = applied

SOURCES =       meteogramplugin.cpp     \
                meteogramaction.cpp     \

HEADERS =       meteogramplugin.h       \
                meteogramaction.h       \


LIBS +=       -lmeteo.meteogram

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.app.widgets

DISTFILES += \
    meteogram.json
