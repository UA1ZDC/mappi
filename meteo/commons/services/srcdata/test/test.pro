TARGET = test 
TEMPLATE = test

QT += xml core widgets

SOURCES = test.cpp

LIBS += -L$(BUILD_DIR)/lib      -ltapp                  \
                                -ltdebug                \
                                -lmeteodata             \
                                -lmeteo.srcdata         \
                                -lmeteo.proto           \
                                -ltrpc                  \
                                -lmeteo.proto           \
                                -lzond                  \
                                -lmeteo.etc             \
                                -lprotobuf              \
                                -lmeteo.global          \
                                -lmeteo.dbi          \
                                -lmeteo.font            \
                                -lmeteo.map             \
                                -ltsingleton            \
                                -lmeteo.settings
 
include( $$(SRC)/include.pro )
