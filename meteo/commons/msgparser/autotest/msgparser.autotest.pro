TEMPLATE = autotest
TARGET   = test
QT      += widgets

HEADERS =     autotest.h            \

SOURCES =     main.cpp              \
              test_tlgfuncs.cpp     \
              test_metaparser.cpp   \
              test_msgfilter.cpp    \
              test_common.cpp       \
              test_rawparser.cpp    \

RESOURCES   = autotest.qrc          \

PROTOS     += test_rawparser.proto  \

LIBS +=       -lcppunit             \
              -lprotobuf            \

LIBS +=       -ltapp                \
              -ltdebug              \
              -lttesthelper         \
              -lmeteo.textproto     \

LIBS +=       -lmeteo.msgparser     \
              -lmeteo.proto         \

include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )

QMAKE_CXXFLAGS += -Wno-unused-parameter


