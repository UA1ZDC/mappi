TEMPLATE = lib
TARGET   = mappi.pos

QT += widgets

LOG_MODULE_NAME = map
PUB_HEADERS =           posdocument.h           
                        
HEADERS =     $$PUB_HEADERS  $$PRIVATE_HEADERS \
  posgrid.h

SOURCES =               posdocument.cpp          \
  posgrid.cpp

LIBS +=                 -lmeteo.map \
                        -lmnsatellite\
                        -lmappi.satprojection

include( $$(SRC)/include.pro )
