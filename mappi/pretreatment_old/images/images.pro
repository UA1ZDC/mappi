TEMPLATE    = lib
TARGET      = mappi.po.images

HEADERS     = colors.h   \
              geomcorr.h \
              filters.h  \
              image.h

SOURCES     = colors.cpp   \
              geomcorr.cpp \
              filters.cpp  \
              image.cpp


LIBS       +=   -ltdebug \
                -lmappi.global\
                -lmnmathtools \
                -lmnsatellite \
                -lgeotiff \
                -ltiff    \
                -lgdal

                

include( $$(SRC)/include.pro )

