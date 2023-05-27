TEMPLATE  = lib
TARGET    = meteo.msgstream.socketspecial

LOG_MODULE_NAME = recgmi

SOURCES =           unimas.cpp                \
                    unimassender.cpp          \
                    unimasserver.cpp          \
                    unimasstreamclient.cpp \
                    unimasstreamserver.cpp \
                    unimasreceiver.cpp \
                    unimasbase.cpp

HEADERS =           unimas.h                  \
                    unimassender.h            \
                    unimasserver.h            \
                    unimasstreamclient.h \
                    unimasstreamserver.h \
                    unimasreceiver.h \
                    unimasbase.h

LIBS +=             -lprotobuf                \
                     -ltapp                    \
                    -ltdebug                  \
                    -lmeteo.msgstream         \
                    -lmeteo.msgcenter         \
                    -lmeteo.compress           \
                    -lmeteo.msgparser

include( $$(SRC)/include.pro )
