TEMPLATE            = test
TARGET              = test

SOURCES             = main.cpp

LIBS               += -ltapp                                                                                           \
                      -ltdebug

LIBS               += -lobanal                                                                                         \
                      -lmeteo.geobasis

include( $$(SRC)/include.pro )

QMAKE_CXXFLAGS += -Wno-error=unused-variable
