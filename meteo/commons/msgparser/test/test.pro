TARGET = test
TEMPLATE = app test
QT += xml

SOURCES = test.cpp

LIBS += -L$(BUILD_DIR)/lib      -ltapp                  \
                                -lmeteo.msgparser       \
                                -ltdebug                \
                                -lmeteodata             \
                                -lprotobuf              \
                                -lmeteo.proto           \
                                -lmeteo.textproto       \
                                -ltrpc

include( $$(SRC)/include.pro )
