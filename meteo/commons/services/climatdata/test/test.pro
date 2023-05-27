TARGET = test 
TEMPLATE = test
QT += xml

SOURCES = test.cpp
HEADERS = 

LIBS += -L$(BUILD_DIR)/lib      -ltapp        \
                                -ltdebug      \
                                -lmeteo.proto \
                                -lprotobuf    \
                                -lmeteo.global \
                                -lmeteo.settings \
                                -lmeteo.climatdata \
                                -lmeteo.nosql \
                                -lmeteo.dbi

include( $$(SRC)/include.pro )
