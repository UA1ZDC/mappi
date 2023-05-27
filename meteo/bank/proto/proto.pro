TEMPLATE = lib
TARGET   = bank.proto

QT -= gui core xml


PROTOS =      #msgstream.proto           \
              #meteosettings.proto       


LIBS +=         -lprotobuf              \
                -ltrpc

PROTOPATH +=    $(SRC)/meteo/commons/grib
PROTOPATH +=    $(SRC)/meteo/commons/rpc
PROTOPATH +=    $(SRC)/meteo/commons/proto

include( $$(SRC)/include.pro )
include( $$(SRC)/protobuf.pri )
