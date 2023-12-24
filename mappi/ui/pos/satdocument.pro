TEMPLATE = lib
TARGET   = mappi.pos

QT += widgets

LOG_MODULE_NAME = map
PUB_HEADERS =           pos.h                   \
                        posdocument.h           \
                        posgrid.h

HEADERS =     $$PUB_HEADERS  $$PRIVATE_HEADERS

SOURCES =               pos.cpp                 \
                        posdocument.cpp         \
                        posgrid.cpp

LIBS +=                 -lmeteo.map \
                        -lmnsatellite\
                        -lmappi.satprojection

include( $$(SRC)/include.pro )
