TEMPLATE = lib
TARGET   = meteo.punchrules

LOG_MODULE_NAME = map
PUB_HEADERS =           punchrules.h


SOURCES =               punchrules.cpp


LIBS +=                 -lprotobuf              \
                        -lmeteodata             \
                        -lmeteo.proto           \
                        -lmeteo.textproto       \
                        -ltdebug                \
                        -ltcontainer            \
                        -lmnfuncs
                        

include( $$(SRC)/include.pro )
