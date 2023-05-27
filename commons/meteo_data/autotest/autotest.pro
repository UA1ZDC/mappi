TEMPLATE = autotest
TARGET   = test
QT      += widgets

SOURCES = main.cpp  ctest.cpp

HEADERS =  ctest.h

LIBS += -lcppunit   \
        -lmeteodata \
        -ltdebug \
        -ltapp

include( $$(SRC)/include.pro )

