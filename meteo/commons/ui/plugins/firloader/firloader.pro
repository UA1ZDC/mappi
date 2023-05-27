TEMPLATE = lib
TARGET   = meteo.firloader.plugin
LOG_MODULE_NAME = fir
FORMS =         firloader.ui

PRIVATE_HEADERS =       firloader.h            \
                        plugin.h

SOURCES =               firloader.cpp          \
                        plugin.cpp

HEADERS = $$PUB_HEADERS  $$PRIVATE_HEADERS

LIBS +=

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.weather
