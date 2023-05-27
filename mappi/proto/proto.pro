TEMPLATE = lib
TARGET = mappi.proto

QT -= \
  gui \
  core \
  xml

LIBS += \
  -ltrpc \
  -lprotobuf \
  -lmeteo.proto

PUB_PROTOS = \
  common.proto \
  satellite.proto \
  antenna.proto \
  receiver.proto \
  schedule.proto \
  reception.proto \
  sessiondataservice.proto \
  satelliteimage.proto \
  fileservice.proto \
  thematic.proto \
  session.proto

PROTOS = $$PUB_PROTOS

PROTOPATH += $(SRC)/meteo/commons/rpc
PROTOPATH += $(SRC)/meteo/commons/proto

include( $$(SRC)/include.pro )
include( $$(SRC)/protobuf.pri )

QMAKE_CXXFLAGS += -Wno-unused-parameter
