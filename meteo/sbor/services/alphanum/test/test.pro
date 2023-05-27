TARGET = test 
TEMPLATE = test
QT += xml

SOURCES = test.cpp
HEADERS = 

LIBS += -L$(BUILD_DIR)/lib      -ltapp                  \
                                -ltdebug                \
                                -lmeteodata             \
                                -ltalphanum             \
                                -ltmeteocontrol         \
                                -lmeteo.decodeserv      \
                                -lprimarydb      \
                                -lmeteo.sql       \
#                                -lobanal        \
                                -lmeteo.proto    \
                                -lprotobuf     \
                                -lmeteo.global \
                                -lmeteo.settings \
                                -lmeteo.geobasis \
                                -lmeteo.dbi
                                

#LIBS += -lsbor.settings     \
                                
include( $$(SRC)/include.pro )
