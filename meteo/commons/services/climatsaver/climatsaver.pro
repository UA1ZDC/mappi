TEMPLATE = lib
TARGET   = meteo.climatsaver

SOURCES  = climatsaver.cpp \
           surfsaver.cpp   \
           aerosaver.cpp
           
           
HEADERS  = climatsaver.h \
           surfsaver.h   \
           aerosaver.h
           
           
LIBS    += -ltdebug           \
           -ltrpc             \
           -lmnmathtools      \
           -lmeteodata        \
           -lmeteo.sql        \
           -lmeteo.geobasis

LIBS    += -lmeteo.proto      \
           -lmeteo.global     \
           -lzond             \
           -lmeteo.textproto  \
           -lmeteo.settings
           
LIBS    += -lprotobuf
           
INCLUDEPATH +=  $(SRC)/meteo/commons/rpc



include( $$(SRC)/include.pro )
