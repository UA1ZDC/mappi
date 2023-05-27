TEMPLATE = app
TARGET   = maslo.su
QT += network
QT += widgets
HEADERS =       appsu.h \
                loginform.h

SOURCES =  appsu.cpp  \
           loginform.cpp

FORMS =         loginform.ui

LIBS +=         -ltapp                  \
                -ltdebug                \
                -ltrpc                  \
                -lmeteo.proto           \
                -lmeteo.global          \
                -lmeteo.global         \
                -lmeteo.settings       \
                -lutil

include( $$(SRC)/include.pro )
