TEMPLATE = app test
TARGET = test

SOURCES =               test.cpp

PUB_HEADERS =           
                  

HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS


LIBS += -ltrpc                  \
        -ltapp                  \
        -ltdebug                \
        -lmeteo.sql              \
        -lmnmathtools           \
        -lobanal                \
        -lmeteodata             \
        -ltgrib                 \
        -lmeteo.geobasis        \
        -lmeteo.proto           \
        -lmeteo.global          \
        -lmeteo.obanal          \
        -lnovost.global         \
        -lnovost.settings       \
        -lnovost.proto


LIBS += -lprotobuf

PRO_PATH = $$system( pwd )
include( $$(SRC)/include.pro )
