TEMPLATE = lib
TARGET = mappi.schedule

SOURCES = sat_schedule.cpp  \
          session.cpp       \
          scheduleinfo.cpp  \
          schedule.cpp      \
          scheduler.cpp


HEADERS = sat_schedule.h    \
          session.h         \
          scheduleinfo.h    \
          schedule.h        \
          scheduler.h
    

LIBS += -lspcoordsys        \
        -lmnsatellite       \
        -lmnmathtools       \
        -lmeteo.textproto   \
        -lmappi.proto

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
      
include( $$(SRC)/include.pro )
