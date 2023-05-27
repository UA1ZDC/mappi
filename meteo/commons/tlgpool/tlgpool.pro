TEMPLATE = lib
TARGET = meteo.tlgpool

                
HEADERS = telegramspool.h 
              
SOURCES = telegramspool.cpp


LIBS   += -ltdebug \
          -lmeteo.proto \


PROTOPATH += /usr/include \
            $(SRC)/meteo/commons/rpc    \
            $(SRC)/meteo/commons/proto


include( $$(SRC)/include.pro )
