TEMPLATE = lib
TARGET = isoline.settings.plugin

QT += widgets

LOG_MODULE_NAME = vizdoc

SOURCES += settingsplugin.cpp

HEADERS += settingsplugin.h

LIBS +=                 -lmeteo.etc                     \
                        -lmeteo.global                  \
                        -ltapp                          \
                        -ltdebug                        \
                        -lisoline.settings

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.app.widgets

DISTFILES += \
    settingsplugin.json

RESOURCES +=




