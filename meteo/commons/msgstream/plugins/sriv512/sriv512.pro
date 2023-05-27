TEMPLATE  = lib
TARGET    = meteo.msgstream.sriv512

LOG_MODULE_NAME = recgmi

SOURCES =           sriv512.cpp \
                    sriv512base.cpp \
    sriv512streamserver.cpp \
    sriv512streamclient.cpp \
    sriv512sender.cpp \
    sriv512receiver.cpp \
    sriv512server.cpp

HEADERS =           sriv512.h \
                    sriv512base.h \
    sriv512streamserver.h \
    sriv512streamclient.h \
    sriv512sender.h \
    sriv512receiver.h \
    sriv512server.h

LIBS +=             -lprotobuf          \

LIBS +=             -ltapp              \
                    -ltdebug

LIBS +=             -lmeteo.msgstream       \
                    -lmeteo.compress        \
                    -lmeteo.msgparser       \
                    -lmeteo.msgcenter

include( $$(SRC)/include.pro )
