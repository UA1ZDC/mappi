QT += widgets
TEMPLATE = lib
TARGET = puansonwrap
DEPENDPATH += .
INCLUDEPATH += .

LIBS += -lmeteo.map                 \
        -lmeteo.map.dataexchange    \
        -lmeteo.global              \
	-lmeteo.tileimage           \
        -ltcustomui                 \
        -lmeteo.mainwindow          \
        -lmeteo.punchrules          \
        -lmeteo.map.view 



# Input
HEADERS += .
SOURCES += puansonwrap.cpp

include( $$(SRC)/include.pro )

LIBS += $$L_BOOST_PYTHON
