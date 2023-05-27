TARGET = test 
TEMPLATE = test
QT += xml

SOURCES = test.cpp

LIBS += -L$(BUILD_DIR)/lib      -ltapp                  \
                                -ltdebug                \
                                -lmappi.po.formats      \
                                -lmappi.po.images     \
                                -lmappi.global          \
                                -lmappi.po.instruments  \
                                -lmappi.po.frame  \
                                -lmeteo.geobasis \
                                -lgeotiff \
                                -ltiff    \
                                -lgdal
                              
include( $$(SRC)/include.pro )
