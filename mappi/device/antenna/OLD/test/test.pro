TEMPLATE = test
CONFIG += debug
CONFIG += thread

QT += serialport

HEADERS += \

SOURCES += \
    main.cpp


LIBS += -L$(BUILD_DIR)/lib \
                     -ltdebug \
                     -lmappi.antenna \
		     -lmnmathtools \
                     -lmnsatellite     \
                     -lmeteo.geobasis  \
                     -lmeteo.dbi \
                     -lmeteo.sql

PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )                    

include( $$(SRC)/include.pro )
