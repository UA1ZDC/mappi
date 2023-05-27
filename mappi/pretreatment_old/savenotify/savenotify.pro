TEMPLATE    = lib
TARGET      = mappi.po.savenotify

HEADERS     = savenotify.h


SOURCES     = savenotify.cpp


LIBS       +=   -ltdebug \
                -lmnmathtools \
                -lmnsatellite \
#               -lmeteo.proto \
                -lmappi.global     \
                -lmappi.settings   \
                -lmappi.proto


PROTOPATH += $(SRC)/meteo/commons/proto              
include( $$(SRC)/protobuf.pri )          
include( $$(SRC)/include.pro )

