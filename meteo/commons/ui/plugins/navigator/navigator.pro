TEMPLATE = lib
TARGET   = meteo.map.navigator

QT += widgets

LOG_MODULE_NAME = vizdoc

HEADERS = \
    navigatoraction.h \
    navigatorplugin.h \
    navigatorwidget.h

SOURCES = \
    navigatoraction.cpp \
    navigatorplugin.cpp \
    navigatorwidget.cpp

LIBS += -lmeteo.etc \
        -lmeteo.global \
        -lmeteo.map.view
        
LIBS +=                 -ltapp                          \
                        -ltdebug                        \
                        -lprotobuf                      \
                        -ltrpc                          \
                        -lmeteo.proto                   \
                        -lmeteo.map                     \
                        -lmeteo.geobasis                \
                        -lmeteodata                     \
                        -lmeteo.sql                      \
                        -lmeteo.mainwindow              \
                        -lobanal                        \
#                        -lmeteo.rpc.selfchecked         \
                        -lmnmathtools                   \

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.map.weather
