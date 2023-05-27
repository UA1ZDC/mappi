TARGET   = satelliteimage
TEMPLATE = lib

QT       += widgets

HEADERS = satelliteimage.h \
          georastr.h \
    satlegend.h

SOURCES = satelliteimage.cpp \
          georastr.cpp \
    satlegend.cpp

LIBS    = -lmeteo.global      \
          -lmappi.settings

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )
