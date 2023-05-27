TEMPLATE = lib 
TARGET = meteo.obanal

LOG_MODULE_NAME = obanal

SOURCES =               tobanaldb.cpp           \
                        tobanalservice.cpp      \
                        tobanal.cpp             \
                        tfield_calc.cpp         \
                        tcalcfhi.cpp            \
                        obanalmaker.cpp         \
                        gribobanalmaker.cpp 


HEADERS =               tobanalservice.h        \
                        tobanaldb.h             \
                        tobanal.h               \
                        tfield_calc.h           \
                        tcalcfhi.h              \
                        obanalmaker.h           \
                        gribobanalmaker.h

linux:LIBS += -lmnprocread      \

LIBS += -ltrpc                  \
        -ltapp                  \
        -ltdebug                \
        -lmeteo.sql              \
        -lprotobuf              \
        -lobanal                \
        -ltgrib                 \
        -lmeteo.geobasis        \
        -lmeteo.proto           \
        -lmeteo.global          \
        -lmeteo.landmask        \
        -lmeteodata             \
        -lmnmathtools           \
        -lprimarydb             \
        -ltsingleton            \
        -lmeteo.nosql           \
        -lmeteo.font            \
        -lmeteo.textproto       \
        -lmeteo.settings

include( $$(SRC)/include.pro )
