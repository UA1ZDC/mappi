TARGET = test 
TEMPLATE = test
QT += xml

SOURCES = test.cpp
HEADERS = test.h

LIBS += -L$(BUILD_DIR)/lib      -ltapp                  \
                                -ltdebug                \
                                -lmeteodata
                              

include( $$(SRC)/include.pro )
