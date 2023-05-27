TEMPLATE = lib
TARGET   = meteo.decodeviewer
LOG_MODULE_NAME = decodeviewer
QT      += gui
QT += widgets
HEADERS  = decodeviewer.h      \
           tlgdecode.h

SOURCES  = decodeviewer.cpp    \
           tlgdecode.cpp

FORMS    = decodeviewer.ui

LIBS    += -lmeteo.proto            \
           -lmeteo.sql               \
           -ltalphanum              \
           -ltmeteocontrol          \
           -lmeteo.msgparser        \
           -ltbufr                  \
           -ltgribdecode\
           -ltgribiface


include ($$(SRC)/include.pro)

