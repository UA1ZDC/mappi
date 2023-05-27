TARGET = mappi.fsm
TEMPLATE = lib

HEADERS += \
  state.h \
  primitive.h \
  statemachine.h

SOURCES += \
  state.cpp \
  statemachine.cpp

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )

include( $$(SRC)/include.pro )
