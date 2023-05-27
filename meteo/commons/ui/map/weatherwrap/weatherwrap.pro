TEMPLATE = lib
TARGET = weatherwrap

QT += widgets core

LIBS += -ltrpc              \
        -ltcustomui         \
        -lmeteo.map

SOURCES = weatherwrap.cpp

include( $$(SRC)/include.pro )

HEADERS += \
    weatherwrap.h

LIBS += $$L_BOOST_PYTHON
