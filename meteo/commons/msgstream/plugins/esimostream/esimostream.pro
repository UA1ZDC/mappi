TEMPLATE  = lib
TARGET    = meteo.esimostream

QT += xml widgets network

LOG_MODULE_NAME = recgmi

SOURCES = \
  esimostream.cpp \
  dwnldrequest.cpp \
  esimorequest.cpp 

HEADERS = \
  esimostream.h \
  dwnldrequest.h \
  esimorequest.h

LIBS +=         -lprotobuf              \

LIBS +=         -ltapp                  \
                -ltdebug                \
                -lqftp                  \

LIBS +=         -lmeteo.msgstream       \
                -lmeteo.msgcenter       \
                -lmeteo.msgparser

include( $$(SRC)/include.pro )
