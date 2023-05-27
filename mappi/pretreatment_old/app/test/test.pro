TARGET = test 
TEMPLATE = test


SOURCES = test.cpp

LIBS += -L$(BUILD_DIR)/lib      -ltapp             \
                                -ltdebug           \
                                -lmnsatellite      \
                                -lmnmathtools      \
                                -lmappi.po.formats \
                                -lmeteo.geobasis
                                

include( $$(SRC)/include.pro )
