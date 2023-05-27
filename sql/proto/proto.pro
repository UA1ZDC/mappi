TEMPLATE = lib
TARGET   = sql.proto

QT -= gui core xml

PROTOS =  dbsettings.proto

LIBS +=         -lprotobuf

include( $$(SRC)/include.pro )
include( $$(SRC)/protobuf.pri )
