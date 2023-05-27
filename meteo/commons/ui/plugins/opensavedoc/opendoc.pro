TEMPLATE = lib
TARGET = meteo.opendoc.plugin

QT += widgets

LOG_MODULE_NAME = vizdoc

SOURCES =       plugin.cpp                              \
                opendoc.cpp                             \
                savedoc.cpp

HEADERS =       plugin.h                                \
                opendoc.h                               \
                savedoc.h

FORMS =         opendoc.ui                              \
                savedoc.ui

LIBS +=                 -lprotobuf                      \

LIBS +=                 -lmeteo.etc                     \
                        -lmeteo.global                  \
                        -lmeteo.proto                   \
                        -lmnfuncs                       \
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

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.common
