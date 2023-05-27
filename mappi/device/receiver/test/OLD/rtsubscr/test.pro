TEMPLATE = test
TARGET = test
QT += widgets

SOURCES += main.cpp \
           recvimg.cpp

HEADERS = recvimg.h



LIBS += -L$(BUILD_DIR)/lib -ltapp \
                         -ltdebug \
                         -lmnsatellite \
                         -lmappi.receiver \
                         -lmappi.global \
                         -ltrpc            \ 
                         -lmappi.settings \
                         -lprotobuf \
                         -lmeteo.global \
                         -lmeteo.dbi \
                         -lmeteo.settings \
                         -lmeteo.proto \
                         -lmappi.proto


PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )


