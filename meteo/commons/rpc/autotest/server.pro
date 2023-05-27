TEMPLATE = autotest
TARGET   = test

QT += testlib

PROTOS =  testserver.proto

HEADERS = server.h \
          testmain.h \
          serverthread.h

SOURCES = server.cpp \
          testmain.cpp \
          serverthread.cpp

LIBS += -ltdebug        \
        -ltapp          \
        -ltrpc          \
        -lmeteo.global  \
        -lmeteo.settings\
        -lprotobuf

include( $$(SRC)/include.pro )
include( $$(SRC)/protobuf.pri )
