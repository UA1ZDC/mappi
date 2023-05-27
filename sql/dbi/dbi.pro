TEMPLATE     = lib
TARGET       = meteo.dbi

LOG_MODULE_NAME = dbconn

PUB_HEADERS  =          dbi.h           \
                        dbiquery.h      \
                        dbientry.h      \
                        dbiarray.h      \
                        gridfs.h        \
                        gridfile.h      \
                        gridfilelist.h

SOURCES      =          dbi.cpp         \
                        dbiquery.cpp    \
                        dbientry.cpp    \
                        dbiarray.cpp    \
                        gridfs.cpp      \
                        gridfile.cpp    \
                        gridfilelist.cpp

LIBS        = -lsql.proto \
              -ltdebug \
              -ltapp \
              -lbson

include( $$(SRC)/include.pro )
