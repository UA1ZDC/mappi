TEMPLATE = app
TARGET = meteo.ml.app

LOG_MODULE_NAME = ml.app

SOURCES =       main.cpp 

LIBS    =       -lprotobuf                      \
                -ltapp                          \
                -ltdebug                        \
                -ltrpc                          \
                -lmeteo.global                  \
                -lmeteo.settings                \
                -lobanal                        \
                -lprotobuf                      \
                -lmeteo.proto                   \
                -lmeteodata                     \
                -lmeteo.fieldata                \
                -lmnmathtools                   \
                -lmeteo.obanal                  \
                -lmeteo.fieldata                \
                -lmeteo.ml.analyser       \
                -lmeteo.ml.methods             \
                -lmeteo.geobasis                \
                -lmeteo.map                     \
                -ltcustomui                     \
                -lmeteo.map.dataexchange        \
                -lmeteo.punchrules              \
                -lmeteo.dbi			\
                -lmeteo.sql			\
                -lmeteo.nosql			\
                -ldlib

LIBS += `pkg-config --libs blas-netlib`

QMAKE_CXXFLAGS += -Wno-error=unused-function
QMAKE_CXXFLAGS += -Wno-error=cast-qual
QMAKE_CXXFLAGS += -Wno-deprecated-declarations
QMAKE_CXXFLAGS += -Wno-misleading-indentation
QMAKE_CXXFLAGS += -Wno-error=terminate

include( $$(SRC)/include.pro )

QMAKE_CXXFLAGS -= -std=c++17
QMAKE_CXXFLAGS += -std=c++14
