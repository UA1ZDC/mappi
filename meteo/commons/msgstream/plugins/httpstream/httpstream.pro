TEMPLATE  = lib
TARGET    = meteo.httpstream
QT += network

LOG_MODULE_NAME = recgmi

SOURCES = \
  crontimer.cpp \
  httprequest.cpp \
  httpstream.cpp

HEADERS = \
  crontimer.h \
  httprequest.h \
  httpstream.h

LIBS +=         -lprotobuf              \

LIBS +=         -ltapp                  \
                -ltdebug                \
                -lqftp                  \

LIBS +=         -lmeteo.msgstream       \
                -lmeteo.msgcenter       \
                -lmeteo.msgparser

include( $$(SRC)/include.pro )
