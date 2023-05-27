TARGET = test 
TEMPLATE = app test
QT += xml

SOURCES = test.cpp
HEADERS = 

LIBS += -L$(BUILD_DIR)/lib      -ltapp                  \
                                -ltdebug                \
                                -lmeteodata             \
                                -ltbufr                 \
                                -ltmeteocontrol         \
                                -lmeteo.decodeserv      \
                                -lnovost.parserserv     \
                                -lmsgparser             \
                                -lnovost.global         \
                                -lnovost.settings       \
                                -lmeteo.proto           \
                                -lmeteo.punchrules      \
                                -ltrpc                  \
                                -lprotobuf

include( $$(SRC)/include.pro )
