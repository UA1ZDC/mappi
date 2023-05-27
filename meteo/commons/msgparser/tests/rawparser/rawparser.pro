TEMPLATE = app test
TARGET   = test

SOURCES                 = main.cpp                    \

PROTOS                  = test.proto                  \

LIBS                   += -lprotobuf                  \

LIBS                   += -ltapp                      \
                          -ltdebug                    \
                          -lmeteo.msgparser           \
                          -lnovost.msgstream.core     \

include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )

QMAKE_CXXFLAGS += -Wno-unused-parameter
