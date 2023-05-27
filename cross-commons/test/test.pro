TEMPLATE = app test

INCLUDEPATH += $(BUILD_DIR)/include

LIBS += -L$(BUILD_DIR)/lib -ltincludes

SOURCES = test.cpp

PRO_PATH = $$system( cd )
include( $$(SRC)/include.pro )
