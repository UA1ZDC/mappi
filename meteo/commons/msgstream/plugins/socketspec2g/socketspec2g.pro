TEMPLATE  = lib
TARGET    = meteo.msgstream.socksp2g

LOG_MODULE_NAME = recgmi

SOURCES =           socksp2g.cpp \
                    socksp2gbase.cpp \
                    socksp2gstreamserver.cpp \
                    socksp2gstreamclient.cpp \
                    socksp2gsender.cpp \
                    socksp2greceiver.cpp \
                    socksp2gserver.cpp

HEADERS =           socksp2g.h \
                    socksp2gbase.h \
                    socksp2gstreamserver.h \
                    socksp2gstreamclient.h \
                    socksp2gsender.h \
                    socksp2greceiver.h \
                    socksp2gserver.h

LIBS +=             -lprotobuf          \

LIBS +=             -ltapp              \
                    -ltdebug

LIBS +=             -lmeteo.msgstream       \
                    -lmeteo.compress        \
                    -lmeteo.msgparser       \
                    -lmeteo.msgcenter

include( $$(SRC)/include.pro )
