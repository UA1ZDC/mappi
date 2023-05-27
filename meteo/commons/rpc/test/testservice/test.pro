TEMPLATE = lib
TARGET   = rpctest

QT += network

PROTOS =      test.proto

PROTOPATH +=  $$(BUILD_DIR)/include

include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )

proto.files = $$PUB_PROTOS
proto.path  = $$(BUILD_DIR)/include
INSTALLS += proto
