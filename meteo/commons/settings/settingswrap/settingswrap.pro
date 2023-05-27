TEMPLATE = lib
TARGET = settingswrap
DEPENDPATH += .
INCLUDEPATH += .

LIBS += -lmeteo.settings            \
        -ltrpc                      \
        -lmeteo.textproto


# Input
HEADERS += .
SOURCES += tadminsettingswrap.cpp

include( $$(SRC)/include.pro )

LIBS += $$L_BOOST_PYTHON
