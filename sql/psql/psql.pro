TEMPLATE = lib
TARGET   = meteo.sql

LOG_MODULE_NAME = dbconn

PUB_HEADERS =   psql.h                  \
                tpgsettings.h           \
                psqlquery.h             \
                psqlentry.h             \
                psqlarray.h             \
                connectiondict.h        \
                localfs.h               \
                localfile.h             \
                localfilelist.h

SOURCES =       psql.cpp                \
                tpgsettings.cpp         \
                psqlquery.cpp           \
                psqlentry.cpp           \
                psqlarray.cpp           \
                connectiondict.cpp      \
                localfs.cpp             \
                localfile.cpp           \
                localfilelist.cpp

LIBS +=         -lpq                    \
                -lmeteo.dbi             \
                -ltincludes             \
                -lmeteo.nosql

win32:DEFINES+=HAVE_STRUCT_TIMESPEC
                
include( $$(SRC)/include.pro )
