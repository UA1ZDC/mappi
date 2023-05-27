TEMPLATE = test
CONFIG += debug

SOURCES = main.cpp

INCLUDEPATH += $(BUILD_DIR)/include

LIBS += -L$(BUILD_DIR)/lib -lmappi.schedule\
         -lmeteo.global \
         -lmeteo.settings \
         -lmeteo.dbi \
         -lmappi.global \
         -ltapp

         
include( $$(SRC)/include.pro )

