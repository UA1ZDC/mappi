TEMPLATE = lib
TARGET   = meteo.mainwindow

QT += widgets

LOG_MODULE_NAME = vizdoc

PUB_HEADERS =           mainwindow.h                    \
                        mdisubwindow.h                  \
                        widgethandler.h                 \
                        mainwindowplugin.h

SOURCES =               mainwindow.cpp                  \
                        widgethandler.cpp               \
                        mdisubwindow.cpp                \
                        mainwindowplugin.cpp

LIBS +=                 -ltapp                          \
                        -ltdebug                        \
                        -ltsingleton                    \
                        -ltcustomui                     \
                        -lprotobuf

LIBS +=                 -lmeteo.proto                   \
                        -lmeteo.global                  \
                        -lmeteo.formsh                  \
                        -lmeteo.textproto               \
                        -lmeteo.embedwindow

include( $$(SRC)/include.pro )
