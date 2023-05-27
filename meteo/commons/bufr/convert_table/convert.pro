TEMPLATE = app test
TARGET   = convert

SOURCES += main.cpp

LIBS +=  -ltdebug \
         -ltapp
         

include( $$(SRC)/include.pro )


