TEMPLATE = lib
TARGET = plotting

QT += widgets


SOURCES = plottingwidget.cpp                            \
          isosettingswidget.cpp

HEADERS = plottingwidget.h                              \
          isosettingswidget.h

FORMS += plotting.ui

LIBS +=                 -lprotobuf

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
                        -lmeteo.geobasis                \
                        -lmeteodata                     \
                        -lmeteo.sql                      \
                        -lmeteo.mainwindow              \
                        -lobanal                        \
                        -ltcustomui                     \
                        -lmnmathtools                   

include( $$(SRC)/include.pro )




