TEMPLATE = lib
TARGET = meteo.geobasis

PRIVATE_HEADERS =

PUB_HEADERS =   geopoint.h              \
                geovector.h             \
                geodata.h               \
                projectionfunc.h        \
                projection.h            \
                stereoproj.h            \
                merkatproj.h            \
                generalproj.h           \
                conichproj.h            \
                obliquemerkator.h       \
                coords.h

HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS

SOURCES =       geopoint.cpp            \
                geodata.cpp             \
                geovector.cpp           \
                projection.cpp          \
                projectionfunc.cpp      \
                stereoproj.cpp          \
                merkatproj.cpp          \
                generalproj.cpp         \
                conichproj.cpp          \
                obliquemerkator.cpp     \
                coords.cpp

LIBS += -lmnmathtools                   \
        -ltdebug                        \
        -lspcoordsys

include( $$(SRC)/include.pro )
