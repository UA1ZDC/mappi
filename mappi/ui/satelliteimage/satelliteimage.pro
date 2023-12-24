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
          -lmappi.settings    \
          -lgeotiff           \
          -ltiff              \
          -lgdal

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )

QMAKE_CXXFLAGS += -Wno-error=unused-result
QMAKE_CXXFLAGS += -Wno-error=unused-variable