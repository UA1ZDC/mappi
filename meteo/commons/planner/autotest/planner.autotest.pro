TEMPLATE = autotest
TARGET   = test
QT      += widgets

HEADERS =     autotest.h              \

SOURCES =     main.cpp                \
              test_timesheet.cpp      \

LIBS +=       -lcppunit               \

LIBS +=       -ltapp                  \
              -ltdebug                \
              -lttesthelper           \

LIBS +=       -lmeteo.planner         \

include( $$(SRC)/include.pro )
