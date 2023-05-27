TEMPLATE = test
TARGET = test

SOURCES = test.cpp

LIBS = -L$(BUILD_DIR)/lib      -lmeteo.geobasis        \
                                -ltapp                  \
                                -ltdebug

INCLUDEPATH += $$(SRC)/release_build/include

include( $$(SRC)/include.pro )
