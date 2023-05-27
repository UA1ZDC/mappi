TEMPLATE  = lib
TARGET    = meteo.cliwarestream

LOG_MODULE_NAME = recgmi

SOURCES = \
  cliwarerequest.cpp \
  cliwarestream.cpp

HEADERS = \
  cliwarerequest.h \
  cliwarestream.h

LIBS +=         -lprotobuf              \

LIBS +=         -ltapp                  \
                -ltdebug                \
                -lqftp                  \

LIBS +=         -lmeteo.msgstream       \
                -lmeteo.msgcenter       \
                -lmeteo.msgparser       \
                -lmeteo.httpstream

include( $$(SRC)/include.pro )
