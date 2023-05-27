TEMPLATE    =   lib
TARGET      =   mappi.po.formats

QMAKE_LFLAGS += -Wl,--no-as-needed

PRIVATE_HHEADERS = 
                

PUB_HEADERS =   satformat.h \
                channel.h   \
                instrument.h \
                frame.h \
                stream_utility.h

HEADERS     =   $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES     =   satformat.cpp \
#                channel.cpp   \
                instrument.cpp \
                frame.cpp \
                stream_utility.cpp

LIBS       +=   -ltdebug \
                -ltapp   \
                -lmeteo.textproto \
                -lmappi.proto \
                -lmeteo.geobasis \
                -lmnsatellite \
                -lmappi.global \
                -lmappi.po.images


include( $$(SRC)/include.pro )

