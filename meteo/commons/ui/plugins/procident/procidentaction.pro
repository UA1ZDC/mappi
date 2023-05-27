TEMPLATE = lib
TARGET   = meteo.map.procidentaction
QT += widgets

LOG_MODULE_NAME = procident

PUB_HEADERS =           procidentaction.h

PRIVATE_HEADERS =

HEADERS =               procidentplugin.h \
                        procidentwidget.h \
                        fielddescrwidget.h \
                        object_trajection.h\ 
                        prepeartrajwidget.h 

SOURCES =               procidentaction.cpp \
                        procidentplugin.cpp \
                        procidentwidget.cpp  \
                        fielddescrwidget.cpp \
                        object_trajection.cpp \
                        prepeartrajwidget.cpp 

FORMS = procidentwidget.ui fielddescrwidget.ui prepeartrajwidget.ui


LIBS +=                 -lprotobuf

LIBS +=                 -lmeteo.etc             \
                        -lmeteo.global          \
                        -lmeteo.map.view

                        
LIBS +=                 -ltapp                          \
                        -ltdebug                        \
                        -ltrpc                          \
                        -lmeteo.proto                   \
                        -lmeteo.map                     \
                        -lmeteo.map.view                \
                        -lmeteo.global                  \
                        -lmeteo.geobasis                \
                        -lmeteodata                     \
                        -lmeteo.sql                      \
                        -lmeteo.mainwindow              \
                        -lobanal                        \
                        -ltcustomui                     \
                        -lmnmathtools

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.weather
