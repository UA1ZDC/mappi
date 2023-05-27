TEMPLATE = lib
TARGET = meteo.ml.analyser

LOG_MODULE_NAME = ml

SOURCES =       analyser.cpp   \
                processes.cpp

HEADERS =       analyser.h     \
                processes.h

LIBS    =       -lprotobuf              \
                -ltapp                  \
                -ltdebug                \
                -ltrpc                  \
                -lmeteo.global          \
                -lmeteo.settings        \
                -lobanal                \
                -lmeteo.proto           \
                -lmeteodata             \
                -lmeteo.fieldata        \
                -lmnmathtools           \
                -lmeteo.obanal          \
                -lmeteo.fieldata        \
                -lmeteo.punchrules      \
                -lmeteo.geobasis


QMAKE_CXXFLAGS += -Wno-error=unused-function 

include( $$(SRC)/include.pro )
