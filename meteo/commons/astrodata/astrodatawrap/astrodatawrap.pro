TEMPLATE = lib
TARGET = astrowrap

QT +=  core

LIBS += -ltrpc              \
        -ltcustomui         \
        -lmeteo.astrodata   \
        -ltdebug

SOURCES = astrowrap.cpp

HEADERS += astrowrap.h

include( $$(SRC)/include.pro )

LIBS += $$L_BOOST_PYTHON
