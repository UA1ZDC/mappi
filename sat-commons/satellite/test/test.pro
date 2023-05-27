TARGET = test
TEMPLATE = test

SOURCES +=  test_sgp.cpp

INCLUDEPATH += $(BUILD_DIR)/include

LIBS += -L$(BUILD_DIR)/lib -lmnmathtools \
                           -lmnsatellite \
                           -lspcoordsys  \ 
                           -lmnfuncs     \
                           -ltdebug      \
                           -lmeteo.geobasis \
                           -ltapp

include( $$(SRC)/include.pro )
