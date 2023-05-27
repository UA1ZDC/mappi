TEMPLATE = test
QT += network

SOURCES =       test.cpp


HEADERS =       test.h



LIBS += -L$(BUILD_DIR)/lib      -ltdebug        \
                                -ltapp          \
                                -ltrpc

LIBS +=                         -lprotobuf

include( $$(SRC)/include.pro )
