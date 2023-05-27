TEMPLATE              = lib
TARGET                = prognoz.global

QT                   += network

HEADERS               = global.h

SOURCES               = global.cpp

LIBS                 += -ltdebug            \
                        -ltapp

LIBS                 += -lprognoz.settings

include( $$(SRC)/include.pro )
