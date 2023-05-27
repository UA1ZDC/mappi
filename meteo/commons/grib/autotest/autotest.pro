
TEMPLATE = autotest
TARGET   = test
QT += widgets

SOURCES = main.cpp  ctest.cpp

HEADERS =  ctest.h

LIBS += -lcppunit \
        -ltdebug  \
        -ltapp    \
        -lmnmathtools  \
        -ltgrib   \
        -ltgribiface   \
        -lprotobuf      \
        -lmeteo.proto   \
        -ltrpc          \

include( $$(SRC)/include.pro )

