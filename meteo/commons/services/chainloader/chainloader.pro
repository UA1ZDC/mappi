TEMPLATE  = lib
TARGET    = meteo.chainloader

LOG_MODULE_NAME = sortgmi

SOURCES = \
    sateliteloaderparamswatcher.cpp \
    sateliteimagebuilder.cpp \
    genericchainbuilder.cpp \
    faxbuilder.cpp \
    appmain.cpp

HEADERS = \
    sateliteloaderparamswatcher.h \
    sateliteimagebuilder.h \
    genericchainbuilder.h \
    appmain.h \
    faxbuilder.h


LIBS += -ltapp              \
        -ltdebug            \
#        -lcrossfuncs        \
        -lprotobuf          \
        -ltrpc              \
        -lmeteo.global      \
        -lmeteo.proto       \
        -lmeteo.settings    \
        -ltsingleton        \
        -lmeteo.nosql       \
        -lmeteo.faxes       \
        -lmeteo.msgparser
include( $$(SRC)/include.pro )
