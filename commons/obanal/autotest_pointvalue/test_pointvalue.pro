TEMPLATE = test
TARGET = test

QT += testlib

SOURCES += test_pointvalue.cpp

INCLUDEPATH += ../

LIBS += \
    -lobanal \
    -lmnmathtools \
    -lmeteo.geobasis \
    -ltdebug

include($$(SRC)/include.pro)
