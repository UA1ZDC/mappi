TEMPLATE = lib
TARGET   = meteo.fieldata

LOG_MODULE_NAME = dbconn

SOURCES =               fieldservice.cpp        \
                        methodwrap.cpp          \
                        fieldanalyse.cpp        \
                        fieldanalysewrap.cpp    \
                        appmain.cpp

PUB_HEADERS =           fieldservice.h          \
                        methodwrap.h            \
                        fieldanalyse.h          \
                        fieldanalysewrap.h      \
                        appmain.h

#HEADERS =               $$PUB_HEADERS

linux:LIBS +=           -lmnprocread

LIBS +=                 -lmeteo.proto           \
                        -lmeteo.textproto       \
                        -lmeteo.global          \
                        -lprotobuf              \
                        -lmeteo.sql              \
                        -ltdebug                \
                        -ltapp                  \
                        -ltrpc                  \
                        -lmeteodata             \
                        -lzond                  \
                        -lmeteo.geobasis        \
                        -lobanal                \
                        -lmeteo.obanal          \
                        -lmeteo.settings        \
                        -lmeteo.map             \
                        -ltcustomui             \
                        -lmeteo.mainwindow      \
                        -lmeteo.map.view        \
                        -lmeteo.map.dataexchange\
                        -lgdal

include( $$(SRC)/include.pro )
