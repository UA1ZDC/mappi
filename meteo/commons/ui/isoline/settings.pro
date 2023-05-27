TEMPLATE = lib
TARGET = isoline.settings

QT += widgets

SOURCES += settingswidget.cpp

HEADERS += settingswidget.h

FORMS += settings.ui \
         addisolinewidget.ui

LIBS +=                 -lmeteo.etc                     \
                        -lmeteo.global                  \
                        -lmeteo.proto                   \
                        -lmnfuncs                       \
                        -lmeteo.map.view                \
                        -lprotobuf                      \
                        -ltapp                          \
                        -ltrpc                          \
                        -ltcustomui                     \
                        -lmeteo.mainwindow              \
                        -ltdebug                        \
                        -lmeteo.map                     \
                        -lmeteodata                     \

include( $$(SRC)/include.pro )



