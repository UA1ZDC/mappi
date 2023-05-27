TEMPLATE = lib
TARGET   = txmlproto

PUB_HEADERS =   txmlproto.h

HEADERS =       $$PUB_HEADERS

SOURCES =       txmlproto.cpp

LIBS +=         -ltdebug               \
                -lprotobuf             \

include( $$(SRC)/include.pro )
