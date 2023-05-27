TEMPLATE = app test
TARGET   = test

SOURCES  = test.cpp

LIBS    += -ltdebug             \
           -ltapp               \
           -lmeteo.geom

PROTOPATH +=  $$(BUILD_DIR)/include /usr/include

include( $$(SRC)/include.pro )
