TEMPLATE = lib
TARGET = meteo.viewheader
QT += printsupport
QT += widgets

HEADERS += viewheader.h

SOURCES += viewheader.cpp

LIBS +=   -ltapp               \
          -lmeteo.etc          \
          -ltdebug

include( $$(SRC)/include.pro )

