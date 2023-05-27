TEMPLATE = app
TARGET   = meteo.toolchain

QT -= gl

LOG_MODULE_NAME = toolchain

HEADERS =       toolchain.h
SOURCES =       main.cpp \
                toolchain.cpp

LIBS +=         -lprotobuf              \
                -ltapp                  \
                -ltrpc                  \
                -ltdebug                \
                -lmeteo.global          \
                -lmeteo.settings        \
                -lmeteo.proto           \
                -lmeteo.textproto       \
                -lmeteodata             \
                -lmeteo.punchrules      \
                -lmnmathtools           \
                -lmeteo.dbi

QMAKE_CXXFLAGS += -Wno-error=unused-function

include( $$(SRC)/include.pro )

