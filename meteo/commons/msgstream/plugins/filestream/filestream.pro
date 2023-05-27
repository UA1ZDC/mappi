TEMPLATE  = lib
TARGET    = meteo.filestream

LOG_MODULE_NAME = recgmi

SOURCES =       filestreamin.cpp        \
                filestreamout.cpp       \

HEADERS =       filestreamin.h          \
                filestreamout.h         \

LIBS +=         -lprotobuf              \

LIBS +=         -ltapp                  \
                -ltdebug                \

LIBS +=         -ltgribdecode           \

LIBS +=         -lmeteo.msgstream       \
                -lmeteo.msgcenter       \
                -lmeteo.msgparser

include( $$(SRC)/include.pro )
