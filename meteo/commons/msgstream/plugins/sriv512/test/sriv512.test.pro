TEMPLATE = test
TARGET   = test

LOG_MODULE_NAME = recgmi

SOURCES =     main.cpp                \

LIBS +=       -lprotobuf              \

LIBS +=       -ltapp                  \
              -ltdebug                \
              -ltsingleton            \
              -ltrpc                  \
              -lmeteo.sql              \

LIBS +=       -lmeteo.msgparser               \
              -lmeteo.proto                   \
              -lmeteo.global                  \
              -lmeteo.msgstream.sriv512       \
              -lmeteo.settings                \

include( $$(SRC)/include.pro )

