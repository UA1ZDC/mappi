TEMPLATE = lib
TARGET   = meteo.av12core
QT       += widgets core

#DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.app.widgets

LOG_MODULE_NAME = vizdoc

HEADERS = \
          av12widget.h        \
          factwindstations.h  \
          availablestations.h

SOURCES = \
          av12widget.cpp        \
          factwindstations.cpp  \
          availablestations.cpp


LIBS    = -lmeteo.global              \
          -lmeteo.mainwindow          \
          -ltdebug                    \
          -ltapp                      \
          -ltcustomui                 \
          -lmeteo.stationlist

FORMS += \
    av12widget.ui        \
    factwindstations.ui  \
    availablestations.ui

include( $$(SRC)/include.pro )
