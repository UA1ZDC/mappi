TEMPLATE = app test
TARGET = test
CONFIG -= moc
CONFIG += debug

#INCLUDEPATH += $(BUILD_DIR)/include

SOURCES += test.cpp

LIBS += -L$(BUILD_DIR)/lib -lmnmathtools \
                           -ltdebug \
                           -ltapp

include( $$(SRC)/include.pro )
