TEMPLATE = lib
CONFIG += thread

QT       += widgets
QT       += core
QT       += serialport

TARGET = mappi.app.antenna


SOURCES = antctrl.cpp

#    main.cpp 
    

PUB_HEADERS = antctrl.h


PRIVATE_HEADERS = \

#HEADERS = $$PUB_HEADERS $$PRIVATE_HEADERS

FORMS = antctrl.ui 


LIBS += -L$(BUILD_DIR)/lib \
    -lmappi.antenna   \
    -lmeteo.textproto \
    -lmappi.proto     \
    -lprotobuf        \
    -ltdebug          \
    -ltapp            \
    -ltsingleton            \
    -lmnprocread            \
    -lmappi.scheduletable   \
    -lmnmathtools


include( $$(SRC)/include.pro )
