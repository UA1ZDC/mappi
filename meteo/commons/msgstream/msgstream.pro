TEMPLATE = lib
TARGET   = meteo.msgstream

LOG_MODULE_NAME = recgmi

QT      += network
CONFIG  += threads

HEADERS =         streamapp.h                 \
                  processedfiles.h            \
                  streammodule.h

SOURCES =         streamapp.cpp               \
                  processedfiles.cpp          \
                  streammodule.cpp

LIBS +=       -lprotobuf                    \

LIBS +=       -ltapp                        \
              -ltdebug                      \
              -ltrpc                        \
              
LIBS +=       -lmeteo.textproto             \
              -lmeteo.proto                 \
              -lmeteo.global                \
              -lmeteo.msgparser             \
              -lmeteo.msgcenter             \
              -lmeteo.settings

include( $$(SRC)/include.pro )
