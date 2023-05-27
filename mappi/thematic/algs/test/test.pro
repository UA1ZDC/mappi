TARGET = test 
TEMPLATE = test
QT += xml

SOURCES = test.cpp

LIBS += -L$(BUILD_DIR)/lib      -ltapp             \
                                -ltdebug           \
                                -lmappi.them.algs_calc  \
                                -lmeteo.global     \
                                -lmeteo.settings   \
                                -lmeteo.dbi        \
                                -lmappi.settings


PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
