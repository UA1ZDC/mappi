TEMPLATE = app
TARGET   = mappi.mainwindow
  
QT += widgets

LOG_MODULE_NAME = mappi_mainwindow

SOURCES =     main.cpp

LIBS +=       -lmeteo.mainwindow.main



LIBS += -ltdebug \
        -ltapp   \
        -lmeteo.global     \
        -lmeteo.settings   \
        -lmeteo.dbi    \
        -lmeteo.mainwindow.main \       
        -lmeteo.mainwindow \
        -lmeteo.map \
        -lmeteo.map.dataexchange  \
        -lmeteo.map.view          \
        -lmeteo.map.dataexchange \
        -ltcustomui \        
        -lmeteo.punchrules


        

LIBS += -lmappi.global \
        -lmappi.settings \


              
PROTOPATH += $(SRC)/meteo/commons/proto
include( $$(SRC)/protobuf.pri )
include( $$(SRC)/include.pro )
