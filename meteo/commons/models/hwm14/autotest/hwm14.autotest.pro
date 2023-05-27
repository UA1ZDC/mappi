TEMPLATE = autotest
TARGET   = test
QT      += widgets

HEADERS =     autotest.h            \

SOURCES =     main.cpp              \
              test_hwm14.cpp        \

RESOURCES =   files.qrc             \

LIBS +=       -lcppunit             \

LIBS +=       -ltapp                \
              -ltdebug              \
              -lttesthelper         \

LIBS +=       -lmeteo.hwm14         \

include( $$(SRC)/include.pro )

QMAKE_CXXFLAGS += -Wno-unused-variable
QMAKE_CXXFLAGS += -Wno-unused-but-set-variable

DISTFILES += \
    files/local_time_profile_cpp.txt


