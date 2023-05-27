TEMPLATE = autotest
TARGET   = test
QT      += widgets

HEADERS =     autotest.h                  \

SOURCES =     main.cpp                    \
              test_pbhelper.cpp           \

PROTOS =      test_pbhelper.proto         \

LIBS +=       -lcppunit                   \
              -lprotobuf                  \

LIBS +=       -ltapp                      \
              -ltdebug                    \

LIBS +=       -lmeteo.textproto           \

include( $$(SRC)/include.pro )
include( $$(SRC)/protobuf.pri )

