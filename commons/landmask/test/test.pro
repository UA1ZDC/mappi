TEMPLATE = app test
CONFIG += debug


INCLUDEPATH += $(BUILD_DIR)/include

# Input
SOURCES = test.cpp


LIBS += -lmeteo.landmask -ltdebug -ltapp

include( $$(SRC)/include.pro )
