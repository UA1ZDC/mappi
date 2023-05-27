TEMPLATE = lib
TARGET   = meteo.verticalcut.core

QT += widgets

LOG_MODULE_NAME = verticalcut

SOURCES =       createcutwidget.cpp     \
                verticalcutaction.cpp

HEADERS =       createcutwidget.h       \
                verticalcutaction.h

FORMS =         createcutwidgeet.ui

RESOURCES =     vc.qrc

LIBS +=         -lprotobuf              \
                -lmnmathtools           \
                -ltdebug                \
                -ltrpc                  \
                -lzond                  \
                -lmeteodata             \
                -lmeteo.global          \
                -lmeteo.geobasis        \
                -lmeteo.map.view        \
                -lmeteo.map             \
                -lmeteo.proto           \
                -ltapp                  \
                -lmeteodata             \
                -lmeteo.sql              \
                -lmeteo.mainwindow      \
                -ltcustomui             \
                -lmnfuncs               \
                -lmeteo.textproto

include( $$(SRC)/include.pro )
