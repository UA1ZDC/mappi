TARGET = test 
TEMPLATE = test

SOURCES = test.cpp

LIBS += -L$(BUILD_DIR)/lib      -ltapp                  \
                                -ltdebug                \
                                -lmeteo.proto           \
                                -lprotobuf
                              

include( $$(SRC)/include.pro )
