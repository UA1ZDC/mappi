TEMPLATE = autotest
TARGET   = test

HEADERS =     terror_test.h

SOURCES =     main.cpp                \
              terror_test.cpp         \
              targ_test.cpp

LIBS +=       -lcppunit

LIBS +=       -ltapp      \
              -ltdebug    \

include( $$(SRC)/include.pro )
