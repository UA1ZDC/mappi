TARGET = test 
TEMPLATE = test


SOURCES = test.cpp
HEADERS = 

LIBS += -L$(BUILD_DIR)/lib      -ltapp                  \
                                -ltdebug                \
                                -lmeteo.esimonc.converter      \
                                -lobanal \
                                -lmeteo.global \
                                -lmeteo.settings \
                                -lmeteo.dbi
                              

                                
include( $$(SRC)/include.pro )
