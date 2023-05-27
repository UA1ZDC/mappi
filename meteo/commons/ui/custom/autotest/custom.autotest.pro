TEMPLATE = autotest
TARGET   = test
QT      += widgets

HEADERS =     autotest.h                  \

SOURCES =     main.cpp                    \
              test_coorddata.cpp          \


LIBS +=       -lcppunit                   \

LIBS +=       -ltapp                      \
              -ltdebug                    \
              -ltcustomui                 \
              -lmeteo.global         \


include( $$(SRC)/include.pro )

