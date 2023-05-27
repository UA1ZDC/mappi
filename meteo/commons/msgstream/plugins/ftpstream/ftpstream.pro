TEMPLATE  = lib
TARGET    = meteo.ftpstream

LOG_MODULE_NAME = recgmi

SOURCES =       ftpstreamin.cpp         \
                ftpstreamout.cpp

HEADERS =       ftpstreamin.h           \
                ftpstreamout.h

LIBS +=         -lprotobuf              \

LIBS +=         -ltapp                  \
                -ltdebug                \
                -lmeteo.faxes           \
                -lqftp                  \

LIBS +=         -lmeteo.msgstream       \
                -lmeteo.msgcenter       \
                -lmeteo.msgparser

include( $$(SRC)/include.pro )
