TEMPLATE = app test
TARGET   = test

SOURCES = test.cpp

LIBS +=                 -lmeteo.font    \
                        -ltapp

PROTOPATH +=  $$(BUILD_DIR)/include /usr/include

include( $$(SRC)/include.pro )
