TEMPLATE    =   lib
TARGET      =   mappi.po.frame

PRIVATE_HHEADERS = frametypes.h

PUB_HEADERS = $$system(ls *.h)


HEADERS     = $$PUB_HEADERS $$PRIVATE_HEADERS simpledeframer.h

SOURCES     = $$system(ls *.cpp)




LIBS       +=   -ltdebug \
                -lmnsatellite     \
                -lmappi.po.formats
                

include( $$(SRC)/include.pro )

