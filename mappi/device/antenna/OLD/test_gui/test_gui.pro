TEMPLATE = app
CONFIG  += thread

QT       += widgets core
QT += serialport

TARGET = mappi.test.antenna


SOURCES = \
    smallantwidget.cpp \
    main.cpp 
    

PUB_HEADERS = \
    smallantwidget.h


PRIVATE_HEADERS =



FORMS = smallant.ui 

LIBS += -L$(BUILD_DIR)/lib \
    -lmappi.antenna   \
    -lmeteo.textproto \
    -lmappi.proto     \
    -lmappi.settings  \
    -lprotobuf        \
    -ltdebug          \
    -ltapp            \
    -ltsingleton      \
    -lmnprocread      \
    -lmnmathtools     \
    -lmnsatellite     \
    -lmappi.etc       \
    -lmeteo.dbi \
    -lmeteo.sql \
    -lmeteo.global



include( $$(SRC)/include.pro )
    
pict.files = antcircle.png
pict.path  = $$(BUILD_DIR)/share/mappi/
    
INSTALLS += pict

