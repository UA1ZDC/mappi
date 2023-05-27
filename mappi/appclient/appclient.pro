TEMPLATE = app
TARGET   = mappi.app.client


SOURCES =     main.cpp

LIBS =  -lmeteo.app.client     \
        -lmeteo.punchrules     \
        -lmeteo.proto 

#LIBS +=       -ltdebug              \
#              -ltapp                \
#              -ltrpc                \
#              -lprotobuf            \
#              -lnspgbase            \

#LIBS +=       -lmeteo.appclient     \
#              -lmeteo.proto         \
#              -lmeteo.global        \

#LIBS +=       -lmappi.proto          \
#              -lmappi.global         \
#              -lmappi.settings       \

include( $$(SRC)/include.pro )
