TARGET   = mappi.schedule.service
TEMPLATE = app
CONFIG  += thread

HEADERS += \
    scheduleservice.h

SOURCES += \
    main.cpp            \
    scheduleservice.cpp

LIBS    += \
    -ltdebug            \
    -ltapp              \
    -lprotobuf          \
    -ltrpc              \
    -lmnprocread        \
    -lmnsatellite       \
    -lmeteo.dbi         \
    -lmeteo.global      \
    -lmeteo.proto       \
    -lmeteo.settings    \
    -lmappi.global      \
    -lmappi.proto       \
    -lmappi.settings    \
    -lmappi.schedule

    
PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
             
include( $$(SRC)/include.pro )
