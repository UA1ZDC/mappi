TEMPLATE = app test
TARGET   = test

SOURCES = test.cpp

LIBS += -ltapp -ltdebug

include( $$(SRC)/include.pro )
