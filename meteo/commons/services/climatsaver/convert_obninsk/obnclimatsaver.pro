TEMPLATE = app
TARGET   = meteo.obnclimatsaver

SOURCES  = main.cpp \
           obnclimatsaver.cpp \
           obnsurfsaver.cpp   \
           obnaerosaver.cpp   
           
           
HEADERS  = obnclimatsaver.h \
           obnsurfsaver.h   \
           obnaerosaver.h 

           
           
LIBS    += -ltdebug           \
           -ltrpc             \
           -lmnmathtools      \
           -lmeteodata        \
           -lmeteo.sql        \
           -lmeteo.geobasis   \
           -ltapp             \
           -lmeteo.dbi

LIBS    += -lmeteo.proto      \
           -lmeteo.global     \
           -lzond             \
           -lmeteo.textproto  \
           -lmeteo.settings   \
           -lmeteo.climatsaver
           
LIBS    += -lprotobuf
           
INCLUDEPATH +=  $(SRC)/meteo/commons/rpc



include( $$(SRC)/include.pro )
