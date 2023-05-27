TEMPLATE = test
TARGET   = test

QT       = widgets

SOURCES = test.cpp

LIBS +=  -ltapp  \
         -ltdebug \
         -lobanal \
         -lmnmathtools \
         -lmngeobasis
       

include( $$(SRC)/include.pro )

