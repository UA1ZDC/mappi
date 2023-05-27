TEMPLATE = autotest
TARGET   = test
QT      += widgets

HEADERS =     autotest.h            \

SOURCES =     main.cpp              \
              test_msis.cpp         \

RESOURCES =   files.qrc             \

LIBS +=       -lcppunit             \

LIBS +=       -ltapp                \
              -ltdebug              \
              -lttesthelper         \

LIBS +=       -lmeteo.msis          \

include( $$(SRC)/include.pro )
