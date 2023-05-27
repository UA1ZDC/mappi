TEMPLATE = lib
TARGET   = mappi.global

LOG_MODULE_NAME = global

QT += network

PUB_HEADERS = global.h                  \
              streamheader.h


HEADERS =     $$PUB_HEADERS             \

SOURCES =     global.cpp                \
              streamheader.cpp


LIBS +=       -lprotobuf

LIBS +=       -ltapp                    \
              -ltdebug                  \
              -lmnfuncs                 \
              -ltrpc                    \
              -lmnsatellite

LIBS +=       -lmeteo.global            \
              -lmeteo.proto


LIBS +=       -lmappi.proto              \
              -lmappi.settings


include( $$(SRC)/include.pro )
