TEMPLATE = autotest
TARGET   = test
QT      += widgets

HEADERS =     autotest.h                  \

SOURCES =     main.cpp                    \
              test_labellayout.cpp        \

LIBS +=       -lcppunit                   \

LIBS +=       -ltapp                      \
              -ltdebug                    \

LIBS +=       -lmeteo.global         \
              -lmeteo.graph               \

include( $$(SRC)/include.pro )
