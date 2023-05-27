TEMPLATE = lib
TARGET = meteo.forecast.opravd


LOG_MODULE_NAME = forecast

SOURCES =  appmain.cpp
HEADERS = appmain.h

LIBS += -lprotobuf              \
        -ltrpc                  \
        -ltapp                  \
        -ltdebug                \
        -lmeteo.proto           \
        -lmeteo.global          \
        -lmeteo.sql             \
        -ltsingleton            \
        -lmeteo.settings


include( $$(SRC)/include.pro )
