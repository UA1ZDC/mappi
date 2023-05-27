GOOGLETEST_DIR=$$(GOOGLETEST_DIR)
isEmpty(GOOGLETEST_DIR){
    GOOGLETEST_DIR="/usr/local/include"
}

!isEmpty(GOOGLETEST_DIR): {
    GTEST_SRCDIR = $$GOOGLETEST_DIR/gtest
    GMOCK_SRCDIR = $$GOOGLETEST_DIR/gmock
}

requires(exists($$GTEST_SRCDIR):exists($$GMOCK_SRCDIR))

!exists($$GOOGLETEST_DIR):message("No googletest src dir found - set GOOGLETEST_DIR to enable.")

INCLUDEPATH *= \
    $$GTEST_SRCDIR \
    $$GMOCK_SRCDIR

SOURCES += \
    $$GTEST_SRCDIR/src/gtest-all.cc \
    $$GMOCK_SRCDIR/src/gmock-all.cc

QMAKE_CXXFLAGS -= -W-error
QMAKE_CXXFLAGS += -Wno-error
QMAKE_CFLAGS -= -W-error
QMAKE_CFLAGS += -Wno-error