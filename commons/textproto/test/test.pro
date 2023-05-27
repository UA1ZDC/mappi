TEMPLATE = test
TARGET   = test
QT += widgets

PROTOS   = test.proto

SOURCES  = test.cpp

LIBS    += -lprotobuf              \
           -ltapp                  \
           -ltdebug                \
           -lmeteo.textproto       \
           -lmeteo.proto

#PROTOPATH +=  $$(BUILD_DIR)/include /usr/include

include( $$(SRC)/include.pro )
include( $$(SRC)/protobuf.pri )

QMAKE_CXXFLAGS += -Wno-unused-parameter
