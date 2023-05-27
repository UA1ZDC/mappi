TEMPLATE = lib
TARGET   = meteo.sprinf
LOG_MODULE_NAME = sprinf
QT      -= xml network concurrent

HEADERS  = sprinfservice.h \
           appmain.h \
           clienthandler.h

SOURCES  = sprinfservice.cpp \
           appmain.cpp \
           clienthandler.cpp

LIBS              +=  -lprotobuf        \
                      -ltrpc            \
                      -lmeteo.nosql     \
                      -lmeteo.sql       \
                      -ltapp            \
                      -ltdebug          \
                      -ltsingleton      \
                      -lmeteo.proto     \
                      -lmeteo.global    \
                      -lmeteo.geobasis  \
                      -lmeteodata       \
                      -lmnmathtools     \
                      -lmnprocread      \
                      -lmeteo.geobasis  \
                      -lmeteo.settings

include( $$(SRC)/include.pro )

QMAKE_CXXFLAGS += -Wno-pmf-conversions
