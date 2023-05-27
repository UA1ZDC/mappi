TEMPLATE  = lib
TARGET    = meteo.tcpstream

LOG_MODULE_NAME = recgmi

SOURCES =       tcpstreamin.cpp         \
                rawdatadriver.cpp       \
                bcmdriver.cpp           \
                lingdriver.cpp          \
                ipvdriver.cpp           \

HEADERS =       tcpstreamin.h           \
                rawdatadriver.h         \
                bcmdriver.h             \
                lingdriver.h            \
                ipvdriver.h             \

RESOURCES     = tcpstream.qrc           \

PROTOS =        tcpstream.proto         \

PROTOPATH  += $(SRC)/meteo/commons/proto          \
              $(SRC)/meteo/commons/msgstream      \

LIBS +=         -lprotobuf              \

LIBS +=         -ltapp                  \
                -ltdebug                \

LIBS +=         -lmnmathtools           \

LIBS +=         -lmeteo.msgstream       \
                -lmeteo.msgcenter        \
                -lmeteo.msgparser

include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/msgstream/
