TARGET = test 
TEMPLATE = test
QT += xml

SOURCES = test.cpp
HEADERS = 

LIBS += -L$(BUILD_DIR)/lib      -ltapp                  \
                                -ltdebug                \
                                -lmeteodata             \
                                -ltbufr             \
                                -ltmeteocontrol         \
                                -lmeteo.decodeserv      \
                                -lmeteo.geobasis    \
                                -lprimarydb      \
                                -lprotobuf        \
                                -lmeteo.sql          \
                                -lmeteo.proto \
                                -lmeteo.textproto \
                                -lmeteo.global\
                                -lmeteo.settings \
                                -lmeteo.dbi
                                
#                                -lsbor.settings


include( $$(SRC)/include.pro )
