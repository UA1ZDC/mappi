TEMPLATE = app test
TARGET   = test

SOURCES = main.cpp
FORMS   = form.ui

LIBS += -ltapp                  \
              -ltdebug                \
            -ltcustomui   \

#INCLUDEPATH += ../
#INCLUDEPATH += $(BUILD_DIR)/include

include( $$(SRC)/include.pro )

