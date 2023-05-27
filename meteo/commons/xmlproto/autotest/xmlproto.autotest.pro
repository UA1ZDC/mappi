TEMPLATE = autotest
TARGET   = test
QT      += widgets

HEADERS =     txmlproto_test.h

SOURCES =     main.cpp            \
              txmlproto_test.cpp

PROTOS =      test.proto

OTHER_FILES +=  $$PROTOS          \
                expected/*        \
                input/*           \

LIBS +=       -lcppunit   \
              -lprotobuf

LIBS +=       -ltapp      \
              -ltdebug    \
              -ltxmlproto

include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
