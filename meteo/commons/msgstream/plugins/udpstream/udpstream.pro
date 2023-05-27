TEMPLATE  = lib
TARGET    = meteo.msgstream.udpstream

LOG_MODULE_NAME = recgmi

SOURCES =       udp_packet.cpp        \
                reeds.cpp \
                udpstreamin.cpp        \
                udpstreamout.cpp        \
                udpreader.cpp  \
                rs_base.cpp \

HEADERS =       udp_packet.h           \
                reeds.h \
                rs.h \
                udpstreamin.h          \
                udpstreamout.h         \
                udpreader.h

LIBS +=         -lprotobuf              \
                -ltdebug                \
                -lmeteo.compress         \
                -lmeteo.msgstream       \
                -lmeteo.msgcenter   \
                -lmeteo.msgparser       \
                -lmnfuncs

include( $$(SRC)/include.pro )
