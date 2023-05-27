TEMPLATE = app test
TARGET   = test

SOURCES = test.cpp

LIBS +=                 -ltdebug                \
                        -ltapp                  \
                        -ltsingleton            \
                        -lmeteo.geobasis        \
                        -lmeteo.map             \
                        -lmeteo.map.loader      \
                        -lmeteo.map.oldgeo

PROTOPATH +=  $$(BUILD_DIR)/include /usr/include

include( $$(SRC)/include.pro )
