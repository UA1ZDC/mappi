TEMPLATE = lib
TARGET = meteo.ml.methods
LOG_MODULE_NAME = forecastml

SOURCES =   ml.cpp    

HEADERS = ml.h 

INCLUDEPATH += $$(SRC)/dlib/include/

LIBS    =       -L$$(SRC)/dlib/lib
LIBS    +=      -ldlib

LIBS    +=      `pkg-config --libs blas-netlib`

LIBS    +=      -lprotobuf                      \
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
                -lmeteo.ml.analyser       \
                -lmeteo.punchrules              \
                -lmeteo.geobasis

QMAKE_CXXFLAGS += -Wno-error=unused-function
QMAKE_CXXFLAGS += -Wno-error=cast-qual
QMAKE_CXXFLAGS += -Wno-deprecated-declarations
QMAKE_CXXFLAGS += -Wno-misleading-indentation
QMAKE_CXXFLAGS += -Wno-error=terminate


include( $$(SRC)/include.pro )

QMAKE_CXXFLAGS -= -std=c++17
QMAKE_CXXFLAGS += -std=c++14