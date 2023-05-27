TEMPLATE = test
CONFIG += debug
SOURCES = main.cpp


INCLUDEPATH += $(BUILD_DIR)/include

LIBS += -L$(BUILD_DIR)/lib -lmappi.schedule \
                           -lmnmathtools \
                           -ltdebug \
                           -lmnsatellite \
                           -lmappi.global \
                           -ltapp


include( $$(SRC)/include.pro )

