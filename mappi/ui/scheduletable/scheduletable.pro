TARGET    = mappi.scheduletable
TEMPLATE  = lib
QT       += widgets
CONFIG   += thread

HEADERS  += \
    scheduletable.h      \
    checksessiondata.h

SOURCES  += \
    scheduletable.cpp    \
    checksessiondata.cpp

FORMS    += \
    scheduletable.ui

LIBS     += \
    -lmappi.schedule     \

RESOURCES  = $$(SRC)/mappi/etc/mappi.qrc

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )             
include( $$(SRC)/include.pro )
