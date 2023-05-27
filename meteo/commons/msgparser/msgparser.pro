TEMPLATE = lib
TARGET   = meteo.msgparser

LOG_MODULE_NAME = recgmi

HEADERS     = tlgfuncs.h                \
              msgmetainfo.h             \
              tmsgformat.h              \
              tlgparser.h               \
              tlgfileparser.h           \
              tmsgrecv.h                \
              common.h                  \
              tlgmsg.h                  \
              rawparser.h               \

SOURCES     = tlgfuncs.cpp              \
              msgmetainfo.cpp           \
              tmsgformat.cpp            \
              tlgparser.cpp             \
              tlgmsg.cpp                \
              tlgfileparser.cpp         \
              tmsgrecv.cpp              \
              common.cpp                \
              rawparser.cpp             \

PROTOS      = rawparser.proto           \


LIBS +=       -lprotobuf                \

LIBS +=       -ltapp                    \
              -ltdebug                  \

LIBS +=       -ltxmlproto               \
              -lmeteo.proto             \
              -lmeteo.textproto         \

msgformat.files = regex/*.xml
msgformat.path  = $$(BUILD_DIR)/etc/meteo/parserules
INSTALLS += msgformat

include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )

QMAKE_CXXFLAGS += -Wno-unused-parameter
