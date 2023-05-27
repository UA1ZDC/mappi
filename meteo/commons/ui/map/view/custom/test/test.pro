TEMPLATE = app test
TARGET   = test

SOURCES = test.cpp
FORMS   = form.ui

LIBS += -ltcustomui


INCLUDEPATH += ../
INCLUDEPATH += $(BUILD_DIR)/include

include( $$(SRC)/include.pro )

