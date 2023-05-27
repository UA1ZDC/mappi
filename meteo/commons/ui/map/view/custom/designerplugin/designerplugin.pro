TARGET   = meteo.mapcustomui.plugin
TEMPLATE = lib
CONFIG += designer plugin debug_and_release


SOURCES   =      tplugin.cpp                     \
                 tplugins.cpp


PUB_HEADERS     =
PRIVATE_HEADERS =       tplugin.h               \
                        tplugins.h
                        

HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS

LOG_MODULE_NAME = vizdoc

LIBS += -lmeteo.map.customui            \
        -lnovost.global

PRO_PATH = $$system( pwd )
include( $$(SRC)/include.pro )

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target
