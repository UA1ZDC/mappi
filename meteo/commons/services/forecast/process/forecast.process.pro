TEMPLATE = lib
TARGET = meteo.forecast.process
QT += script  widgets

LOG_MODULE_NAME = forecast

SOURCES =  appmain.cpp
HEADERS = appmain.h

LIBS += -lprotobuf              \
        -lmeteo.obanal          \
        -ltrpc                  \
        -ltapp                  \
        -ltdebug                \
        -lmeteo.proto           \
        -lmeteo.forecast        \
        -lmeteo.geobasis        \
        -lmeteo.font            \
        -lmeteo.global          \
        -lmeteo.settings

include( $$(SRC)/include.pro )
