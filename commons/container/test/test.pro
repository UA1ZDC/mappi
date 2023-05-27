#TEMPLATE = app test
TARGET = test

#QT += widgets
SOURCES = test.cpp

LIBS -= -L$(BUILD_DIR)/lib

#INCLUDEPATH += $$(SRC)/release_build/include

#include( $$(SRC)/include.pro )
