TEMPLATE   = lib
TARGET     = prognoz.proto

QT        -= gui core xml

PROTOS     = obanal.proto              

PROTOPATH +=    $(SRC)/meteo/commons/grib
PROTOPATH +=    $(SRC)/meteo/commons/rpc
PROTOPATH +=    $(SRC)/meteo/commons/proto

LIBS      += -lprotobuf                 \
             -ltrpc                     \
             -lmeteo.proto

include( $$(SRC)/include.pro )
include( $$(SRC)/protobuf.pri )
