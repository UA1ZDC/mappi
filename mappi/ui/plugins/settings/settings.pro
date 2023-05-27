TARGET     = settings.plugin
TEMPLATE   = lib
QT        += widgets

DESTDIR    = $(BUILD_DIR)/lib/plugins/mappi.app.widgets

SOURCES   +=  settingsplugin.cpp

HEADERS   +=  settingsplugin.h

DISTFILES += settings.json

LIBS      += -lmeteo.mainwindow   \
             -lsettings.base

include($$(SRC)/include.pro)
