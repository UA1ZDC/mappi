TEMPLATE = lib
TARGET   = novost.mapviewer
QT += widgets
LOG_MODULE_NAME = vizdoc

QT      += gui

HEADERS  = mapviewer.h

SOURCES  = mapviewer.cpp

LIBS    += -lmeteo.map             \
           -lmeteo.map.view        \
           -lmeteo.rpc.selfchecked \
           -lmeteo.navigator

include ($$(SRC)/include.pro)
