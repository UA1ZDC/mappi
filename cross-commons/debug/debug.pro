TEMPLATE = lib
TARGET   = tdebug

PUB_HEADERS =   tdebug.h      \
                tlog_debug.h  \
                tlog.h        \
                threads.h     \
                tmap.h

HEADERS =       $$PUB_HEADERS   \

SOURCES =       tlog.cpp        \
                threads.cpp     \

LIBS +=         -ltapp          \

QT += widgets

include( $$(SRC)/include.pro )
