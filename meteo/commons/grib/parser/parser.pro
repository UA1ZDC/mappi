TEMPLATE    =   lib
TARGET      =   tgribparser

LOG_MODULE_NAME=codecontrol

PRIVATE_HHEADERS = 

PUB_HEADERS =   tgribparser.h  \
                treprgrib.h    \
                tproductgrib.h \
                tgridgrib.h   \
                tunpack.h     \
                tgribformat.h                

HEADERS     =   $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES     =   tgribparser.cpp \
                treprgrib.cpp   \
                tproductgrib.cpp \
                tgridgrib.cpp \
                tunpack.cpp   \
                tgribformat.cpp

LIBS       +=   -ltdebug          \
                -lprotobuf        \
                -lpng             \
                -lopenjp2         \
                -ltgrib           \
                -lmnmathtools     \
                
LIBS       +=   -lmeteo.proto     \

PROTOS = trepresent.proto 


include( $$(SRC)/protobuf.pri)
include( $$(SRC)/include.pro )

QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += `pkg-config --cflags libopenjp2`
