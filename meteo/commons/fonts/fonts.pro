TEMPLATE = lib
TARGET   = meteo.font

PUB_HEADERS =           weatherfont.h

HEADERS =     $$PUB_HEADERS  $$PRIVATE_HEADERS

SOURCES =             weatherfont.cpp
                  
LIBS =                  -lmnfuncs       \
                        -ltsingleton    \
                        -ltapp          \
                        -ltdebug
                      
include( $$(SRC)/include.pro )
