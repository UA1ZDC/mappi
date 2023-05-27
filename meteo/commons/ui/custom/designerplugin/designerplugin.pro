TARGET   = tcustomuidesignerplugin
TEMPLATE = lib
CONFIG += plugin debug_and_release
QT +=  designer


SOURCES   =      tplugin.cpp                     \
                 tplugins.cpp


PRIVATE_HEADERS =       tplugin.h               \
                        tplugins.h

LIBS +=   -ltcustomui     \
          -ltdebug        \
          -ltapp          \
          -lmeteo.proto   \
          -lmeteo.mainwindow       \
          -lmeteo.map.view         \
          -lmeteo.map.dataexchange

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target

include( $$(SRC)/include.pro )
