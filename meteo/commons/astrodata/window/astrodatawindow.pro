TEMPLATE = lib
TARGET   = meteo.astrodata.window

INCLUDEPATH += ./ ./extlib

QT += core gui printsupport


HEADERS =       astrodatacolorproxymodel.h      \
                astrodatawidget.h               \                
                headerview.h

SOURCES =       astrodatacolorproxymodel.cpp    \
                astrodatawidget.cpp             \                
                headerview.cpp

FORMS =         astrodatawidget.ui

RESOURCES =     astro.qrc

LIBS +=         -lmeteo.etc                     \
                -lmeteo.global                  \
                -ltdebug                        \
                -ltcustomui                     \
                -lmeteo.geobasis                \
                -lmeteo.sql                      \
                -ltapp

include( $$(SRC)/include.pro )


