TEMPLATE = app test
TARGET = test.obanal

SOURCES =               main.cpp                \
                        ../tobanaldb.cpp           \
                        ../tobanal.cpp             \
                        ../options.cpp\
                        ../forecast_accuracy.cpp\
                        ../diagn_func.cpp\
                        ../field_analyse.cpp

PUB_HEADERS =           ../tobanaldb.h             \
                        ../tobanal.h               \
                        ../options.h\
                        ../forecast_accuracy.h\
                        ../diagn_func.h\
                        ../field_analyse.h
                  

HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS


LIBS += -lmnprocread            \
        -ltrpc                  \
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
        -lnovost.global         \
        -lnovost.settings       \
        -lnovost.proto\
        -ltgribsave             \
                        -ltgribiface


LIBS += -lprotobuf

PRO_PATH = $$system( pwd )
include( $$(SRC)/include.pro )
