TARGET = test
TEMPLATE = autotest
QT      += widgets

SOURCES += main.cpp  ctest.cpp
HEADERS = ctest.h


include( $$(SRC)/include.pro )


LIBS += -lcppunit \
        -ltapp          \
        -ltdebug        \         
        -ltmeteocontrol \
        -lmeteodata
