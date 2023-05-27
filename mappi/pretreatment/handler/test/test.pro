TARGET = test 
TEMPLATE = test
QT += xml

SOURCES = test.cpp

LIBS += -L$(BUILD_DIR)/lib      -ltapp                  \
                                -ltdebug                \
                                -lmnsatellite          \
                                -lmappi.po.handler

include( $$(SRC)/include.pro )
