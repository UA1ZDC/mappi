TEMPLATE = lib
TARGET   = trpc

LOG_MODULE_NAME = rpc

QT += network

PUB_HEADERS =           transport.h             \
                        transport_v1.h          \
                        tcpsocket.h             \
                        tcpsocket_v1.h          \
                        channel.h               \
                        controller.h            \
                        server.h                \
                        parser.h                \
                        parser_v1.h             \
                        methodpack.h            \
                        clienthandler.h

PRIVATE_HEADERS =

SOURCES =               transport.cpp           \
                        transport_v1.cpp        \
                        tcpsocket.cpp           \
                        tcpsocket_v1.cpp        \
                        channel.cpp             \
                        controller.cpp          \
                        server.cpp              \
                        parser.cpp              \
                        parser_v1.cpp           \
                        methodpack.cpp          \
                        clienthandler.cpp

PYMODULES =           client.py/*.py

LIBS =                -lmnfuncs                 \
                      -lprotobuf                \
                      -ltdebug

include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
