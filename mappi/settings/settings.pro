TEMPLATE = lib
TARGET   = mappi.settings

#QT      += network

PUB_HEADERS =   mappisettings.h



SOURCES =       mappisettings.cpp

LIBS +=         -lprotobuf              \
                -ltsingleton            \
                -lmeteo.geobasis        \
                -lmeteo.proto           \
                -lmappi.proto

INCLUDEPATH += $$(SRC)/meteo/commons/proto \
               $$(SRC)/meteo/commons/rpc

include( $$(SRC)/include.pro )
