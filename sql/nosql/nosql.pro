TEMPLATE     = lib
TARGET       = meteo.nosql

LOG_MODULE_NAME = dbconn

PUB_HEADERS  = document.h               \
               array.h                  \
               nosqlquery.h

SOURCES      = document.cpp             \
               array.cpp                \
               nosqlquery.cpp

HEADERS +=     $$PUB_HEADERS

LIBS        += -lbson                   \
               -lmeteo.dbi


include( $$(SRC)/include.pro )

QMAKE_CXXFLAGS += -Wno-aligned-new
QMAKE_CXXFLAGS += -Wno-deprecated-declarations


