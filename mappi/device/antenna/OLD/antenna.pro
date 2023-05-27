TEMPLATE = lib
TARGET = mappi.antenna
  
CONFIG += thread
QT += serialport



SOURCES = prbl-ant.cpp  \
          suzhetant.cpp \
          smallant.cpp  \
          antprogon.cpp \
          mantcoord.cpp \
          antfollow.cpp \
          cafant.cpp


PUB_HEADERS = prbl-ant.h \
              suzhetant.h \
              smallant.h \
              antfollow.h \
              cafant.h

PRIVATE_HEADERS = antprogon.h \
                  mantcoord.h



LIBS +=         -lmappi.proto \
                -lmeteo.textproto
                
#    -ltcomport \
#    -lmnangle\
#    -lmndevice\
#    -lmnsatellite


include( $$(SRC)/include.pro )

