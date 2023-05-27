TEMPLATE    = lib
TARGET      = mappi.them.algs_calc
  
CONFIG += no_keywords #AsmJit use emit keyword

HEADERS     = channel.h     \
              cloudmask.h \
              rgb.h \
              seatemperature.h \
              themformat.h  \
              themalg.h     \
              datastore.h \
              dataservstore.h \
              datafsstore.h \
              index.h \
              total_precip.h
  

SOURCES     = channel.cpp     \
              cloudmask.cpp \
              rgb.cpp \
              seatemperature.cpp \
              themformat.cpp  \
              themalg.cpp     \
              dataservstore.cpp  \
              datafsstore.cpp  \
              index.cpp \
              total_precip.cpp
                

LIBS       +=   -ltdebug            \
                -lmappi.global      \
                -lmnmathtools       \
                -lthematiccalc      \
                -lasmjit            \
                -lmappi.po.images   \
                -lmappi.satprojection \
                -lmappi.landmask      \
                -lzond


PROTOPATH +=    $(SRC)/meteo/commons/proto

include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
QMAKE_CXXFLAGS += -Wno-error=class-memaccess
