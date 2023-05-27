TEMPLATE = app
CONFIG -= moc
CONFIG += debug

QT+=qt3support

INCLUDEPATH += $(BUILD_DIR)/include

LIBS += -L$(BUILD_DIR)/lib -lspcoordsys -lmnmathtools -lmnfuncs -lmngeobasis


SOURCES += test.cpp

