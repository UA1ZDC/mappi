TEMPLATE     = app test

QT          -= xml network

INCLUDEPATH += ../

HEADERS      = ../extract.h       \
               ../packer.h        \
               ../tiffconvert.h

SOURCES      = ../extract.cpp     \
               ../packer.cpp     \
               ../tiffconvert.cpp \
               main.cpp

LIBS +=     -lmeteo.faxes   \
            -ltapp          \
            -ltdebug        \
            -lmeteo.global  \
            -lmeteo.settings\
            -ltrpc          \
            -lmeteo.proto   \
            -lprotobuf      \
            -lmeteo.nosql\
            -ltiff


include( $$(SRC)/include.pro )
