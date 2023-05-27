TEMPLATE = app test
TARGET = test

SOURCES = test.cpp

LIBS += -lnovost.timesheeteditor \
        -lmeteo.planner          \
        -ltapp                   \
        -ltdebug

include( $$(SRC)/include.pro )
