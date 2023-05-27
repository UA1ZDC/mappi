TEMPLATE = app
TARGET = meteo.ml.testcase
LOG_MODULE_NAME = testcaseapp

SOURCES =       main.cpp 

LIBS    =       -lprotobuf                  \
                -ltapp                      \
                -ltdebug                    \
                -ltrpc                      \
                -lmeteo.global              \
                -lmeteo.settings            \
                -lobanal                    \
                -lprotobuf                  \
                -ltcustomui                 \
                -lmeteo.proto               \
                -lmeteodata                 \
                -lmeteo.fieldata            \
                -lmnmathtools               \
                -lmeteo.obanal              \
                -lmeteo.map                 \
                -lmeteo.map.dataexchange    \
                -lmeteo.fieldata            \
                -lmeteo.ml.analyser   \
                -lmeteo.ml.methods         \
                -lmeteo.punchrules          \
		-lmeteo.dbi                 \
		-lmeteo.sql                 \
		-lmeteo.nosql               \
                -lmeteo.geobasis            \
                -ldlib
                

QMAKE_CXXFLAGS += -Wno-error=unused-function
QMAKE_CXXFLAGS += -Wno-error=cast-qual
QMAKE_CXXFLAGS += -Wno-deprecated-declarations
QMAKE_CXXFLAGS += -Wno-misleading-indentation
QMAKE_CXXFLAGS += -Wno-error=terminate

include( $$(SRC)/include.pro )
QMAKE_CXXFLAGS -= -std=c++17
QMAKE_CXXFLAGS += -std=c++14