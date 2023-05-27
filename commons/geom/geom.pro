TEMPLATE = lib
TARGET   = meteo.geom

SOURCES =       point.cpp              \
                edge.cpp               \
                node.cpp               \
                polygon.cpp            \
                line.cpp               \
                geom.cpp

PUB_HEADERS =   point.h                \
                edge.h                 \
                node.h                 \
                polygon.h              \
                line.h                 \
                geom.h

HEADERS =       $$PUB_HEADERS

LIBS +=         -ltdebug               \

include( $$(SRC)/include.pro )
