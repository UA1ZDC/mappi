TARGET = test 
TEMPLATE = test
QT += widgets

SOURCES = test.cpp
HEADERS = 

LIBS += -L$(BUILD_DIR)/lib      -ltapp        \
                                -ltdebug      \
                                -lmeteo.proto \
                                -lprotobuf    \
                                -lmeteo.global \
                                -lmeteo.settings \
                                -lmeteo.climatdata \
                                -lmeteo.nosql \
                                -lvkocalc \
                                -ltcustomplot

include( $$(SRC)/include.pro )

LIBS += $$L_BOOST_PYTHON

exists( /usr/bin/python3.5-config ):LIBS += $$system( python3.5-config --libs )
exists( /usr/bin/python3.7-config ):LIBS += $$system( python3.7-config --libs )
exists( /usr/bin/python3.8-config ):LIBS += $$system( python3.8-config --libs )
