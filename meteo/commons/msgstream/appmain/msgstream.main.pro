TEMPLATE = lib
TARGET   = meteo.msgstream.appmain

LOG_MODULE_NAME = recgmi

HEADERS =     appmain.h

SOURCES = \
    appmain.cpp

LIBS +=       -lprotobuf              \

LIBS +=       -ltapp                  \
              -ltdebug                \
              -ltsingleton            \
              -ltrpc                  \

LIBS +=       -lmeteo.msgparser               \
              -lmeteo.proto                   \
              -lmeteo.global                  \
              -lmeteo.msgcenter               \
              -lmeteo.textproto               \
              -lmeteo.msgstream               \
              -lmeteo.ftpstream               \
              -lmeteo.filestream              \
              -lmeteo.msgstream.sriv512       \
              -lmeteo.msgstream.socksp2g      \
              -lmeteo.msgstream.socketspecial \
              -lmeteo.msgstream.udpstream     \
              -lmeteo.settings                \
              -ltcustomui                     \
              -lmeteo.msgstream.ptkpp         \
              -lmeteo.mainwindow              \
              -lmeteo.map.view                \
              -lmeteo.cliwarestream           \
              -lmeteo.esimostream             \
              -lmeteo.map.dataexchange
include( $$(SRC)/include.pro )

