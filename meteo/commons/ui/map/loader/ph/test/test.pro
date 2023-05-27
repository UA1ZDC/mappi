TEMPLATE = app test
TARGET   = test

SOURCES = test.cpp

LIBS +=                 -ltdebug                \
                        -ltapp                  \
                        -ltsingleton            \
                        -lmnmathtools           \
                        -lmeteo.geobasis        \
                        -lmeteo.map             \
                        -lmeteo.map.loader      \
                        -lmeteo.map.phload      \
                        -lmeteo.textproto\
                         -lmeteo.verticalcut.core  
                         

PROTOPATH +=  $$(BUILD_DIR)/include /usr/include

include( $$(SRC)/include.pro )
