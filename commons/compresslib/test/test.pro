TARGET = test
TEMPLATE = test

SOURCES = test.cpp

LIBS += -L$(BUILD_DIR)/lib      -lmeteo.compress

include( $$(SRC)/include.pro )
