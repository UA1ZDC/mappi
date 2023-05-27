TEMPLATE = lib
TARGET   = zond

LOG_MODULE_NAME = prepare

PRIVATE_HEADERS =

PUB_HEADERS =  zond.h           \
               zondfunc.h       \
               turoven.h        \
               urovenlist.h     \
               placedata.h      \
               clouddata.h      \
               diagn_func.h     \
               indexes.h


HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES =  zond.cpp             \
           zondfunc.cpp         \
           turoven.cpp          \
           urovenlist.cpp       \
           placedata.cpp        \
           clouddata.cpp        \
           diagn_func.cpp       \
           indexes.cpp


LIBS += -lmeteo.geobasis        \
        -lmnmathtools           \
        -lprotobuf              \
        -lmeteo.proto           \
        -ltdebug                \
        -lmeteodata

include( $$(SRC)/include.pro )
