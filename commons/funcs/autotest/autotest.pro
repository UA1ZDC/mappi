TEMPLATE = autotest
TARGET   = test
QT      += widgets

HEADERS = \
    ../tcolorgrad.h \
    tcolorgrad_test.h

SOURCES =     main.cpp \
    ../tcolorgrad.cpp \
    tcolorgrad_test.cpp

LIBS +=       -lcppunit

LIBS +=       -ltapp      \
              -ltdebug

include( $$(SRC)/include.pro )

