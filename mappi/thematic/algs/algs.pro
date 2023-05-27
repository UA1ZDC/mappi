TEMPLATE    = lib
TARGET      = mappi.them.algs
  
CONFIG += no_keywords #AsmJit use emit keyword

HEADERS     = channel.h     \
              cloudaltitude.h \
              cloudtempr.h \
              cloudtype.h \
              seatempr.h \
              surfacetempr.h \
              themformat.h  \
              themalg.h     \
              grayscale.h \
              ndvi.h      \
              rgbimg.h    \
              cloudmask.h     \
              datastore.h \
              dataservstore.h \
              datafsstore.h

#              falsecolor.h  \
#              index.h
  

SOURCES     = channel.cpp     \
              cloudaltitude.cpp \
              cloudtempr.cpp \
              cloudtype.cpp \
              seatempr.cpp \
              surfacetempr.cpp \
              themformat.cpp  \
              themalg.cpp     \
              grayscale.cpp  \
              ndvi.cpp       \
              rgbimg.cpp     \
              cloudmask.cpp     \
              dataservstore.cpp  \
              datafsstore.cpp

#              falsecolor.cpp  \
#              index.cpp
                

LIBS       +=   -ltdebug            \
                -lmappi.global      \
                -lmnmathtools       \
                -lthematiccalc      \
                -lasmjit            \
                -lmappi.po.images

PROTOPATH +=    $(SRC)/meteo/commons/proto

include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
QMAKE_CXXFLAGS += -Wno-error=class-memaccess
