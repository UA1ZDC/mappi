TEMPLATE = lib
TARGET   = meteo.msgdata

LOG_MODULE_NAME = sortgmi

HEADERS =     types.h                   \
              msgqueue.h                \
              msgreceiver.h             \
              routetable.h              \
              msgupdater.h              \
              msggetter.h               \
              msgprocessor.h            \
              msgsaver.h                \
              msgfilter.h               \
              msgcreater.h

SOURCES =     msgqueue.cpp              \
              msgreceiver.cpp           \
              routetable.cpp            \
              msgupdater.cpp            \
              msggetter.cpp             \
              msgprocessor.cpp          \
              msgsaver.cpp              \
              msgfilter.cpp             \
              msgcreater.cpp

RESOURCES =  

LIBS +=       -lprotobuf -lmeteo.msgparser

LIBS +=       -lmeteo.proto

include( $$(SRC)/include.pro )
