TEMPLATE = lib
TARGET   = meteo.faxes

LOG_MODULE_NAME = sortgmi

QT      -= xml network
QT += gui

HEADERS  =  extract.h           \            
            tiffconvert.h       \
            packer.h            \
            datachain.h


SOURCES  =  extract.cpp         \            
            tiffconvert.cpp     \
            packer.cpp          \
            datachain.cpp

LIBS    += -ltapp             \
           -ltdebug           \
           -ltrpc          

LIBS    += -lmeteo.proto           \
           -lmeteo.global          \
           -lprotobuf              \
           -lmeteo.compress        \
           -ltiff                 \
            -ltiffxx

include( $$(SRC)/include.pro )
