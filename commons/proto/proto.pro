TEMPLATE = lib
TARGET   = commons.proto

#QT -= gui core xml

PROTOS   =      tfield.proto    \
                mongo.proto

LIBS +=         -lprotobuf

include( $$(SRC)/include.pro )
include( $$(SRC)/protobuf.pri )
