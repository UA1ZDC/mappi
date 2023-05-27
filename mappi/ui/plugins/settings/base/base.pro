TARGET    = settings.base
QT       += widgets
TEMPLATE  = lib

DESTDIR   = $(BUILD_DIR)/lib/

SOURCES  += settingswidget.cpp \
            settingwidget.cpp  \
            settingplugin.cpp

HEADERS  += settingswidget.h   \
            settingwidget.h    \
            settingplugin.h

FORMS    += settingswidget.ui

include( $$(SRC)/include.pro )
