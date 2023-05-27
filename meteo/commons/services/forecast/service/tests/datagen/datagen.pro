TEMPLATE = app test
TARGET   = test

SOURCES =     main.cpp                  \

LIBS +=       -lprotobuf                \

LIBS += -ltapp                      \
        -ltdebug                    \
        -ltrpc                      \
        -lmeteo.proto               \
        -lbank.settings             \
        -ltrpc                      \
        -lmeteo.geobasis            \
        -lmeteo.proto               \
        -lmeteo.global              \
        -lmeteo.forecast.service    \
        -lzond                      \
        -lprotobuf                  \
        -lmeteo.settings            \
        -lbank.settings


include( $$(SRC)/include.pro )
