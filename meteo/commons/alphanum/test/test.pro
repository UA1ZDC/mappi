TARGET = test 
TEMPLATE = test
QT += xml

SOURCES = test.cpp

LIBS += -L$(BUILD_DIR)/lib      -ltapp                  \
                                -ltdebug                \
                                -lmeteodata             \
                                -ltalphanum             \
                                -ltrpc                  \
                                -lmeteo.global          \
                                -lmeteo.settings        \
                                -lprimarydb \
                                -lmeteo.dbi
                                

include( $$(SRC)/include.pro )
