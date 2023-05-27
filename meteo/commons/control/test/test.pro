TARGET = test 
TEMPLATE = test
QT += xml

SOURCES = test.cpp

LIBS += -L$(BUILD_DIR)/lib      -ltapp                  \
                                -ltdebug                \
                                -lmeteodata             \
                                -ltalphanum             \
                                -ltmeteocontrol         \
                                -ltbufr                 \
                                -lmnmathtools           \
                                -lmeteo.proto           \
                                -lprotobuf              \
                                -lmeteo.global   \
                                -lprimarydb      \
                                -lmeteo.settings \
                                -lmeteo.geobasis \
                                -lmeteo.dbi


include( $$(SRC)/include.pro )
