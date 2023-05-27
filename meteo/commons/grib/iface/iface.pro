TEMPLATE    =   lib
TARGET      =   tgribiface

PRIVATE_HHEADERS = 

PUB_HEADERS =   tgribiface.h \
                tgribcalc.h

HEADERS     =   $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES     =   tgribiface.cpp \
                tgribcalc.cpp

LIBS       +=   -ltdebug      \
                -lprotobuf    \
                -ltgrib       \
                -ltgribparser \
                -lmnmathtools \

LIBS       +=   -lmeteo.proto \

include( $$(SRC)/include.pro )

