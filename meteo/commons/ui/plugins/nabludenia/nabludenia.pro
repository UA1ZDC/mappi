TEMPLATE = lib
TARGET   = meteo.nabludenia.plugin
QT += widgets

LOG_MODULE_NAME = vizdoc

FORMS =         nabludenia.ui

PRIVATE_HEADERS =       nabludenia.h            \
                        plugin.h

SOURCES =               nabludenia.cpp          \
                        plugin.cpp

LIBS +=                 -ltapp                          \
                        -ltdebug                        \
                        -ltrpc                          \
                        -lprotobuf                      \
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

RESOURCES += style.qrc
