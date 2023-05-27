TEMPLATE = lib
TARGET   = ocean

LOG_MODULE_NAME = prepare

#QT += dbus

PRIVATE_HEADERS = turoven.h \
                  ocean.h   \
                  placedata.h

PUB_HEADERS = 


HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES =   \
           ocean.cpp             \
           turoven.cpp          \
           placedata.cpp  

LIBS += -lmeteo.geobasis        \
        -lmnmathtools           \
        -ltdebug                \
        -lmeteo.proto           \
        -lmeteodata

include( $$(SRC)/include.pro )
