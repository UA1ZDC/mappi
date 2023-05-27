TEMPLATE = lib
TARGET   = meteo.radar

SOURCES =       radar.cpp

PUB_HEADERS =   radar.h

HEADERS =       $$PUB_HEADERS

LIBS +=			-lmeteo.sql				  \
        		-lprotobuf      		\
        		-ltdebug        		\
        		-lmeteo.geobasis		\
        		-lmeteo.proto
        		

include( $$(SRC)/include.pro )
