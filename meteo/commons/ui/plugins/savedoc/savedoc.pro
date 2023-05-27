TEMPLATE = lib
TARGET   = meteo.savedoc.plugin
LOG_MODULE_NAME = savedoc
FORMS =         savedoc.ui
QT += widgets

PRIVATE_HEADERS =       savedoc.h               \
                        plugin.h

SOURCES =               savedoc.cpp             \
                        plugin.cpp

LIBS +=

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.common
