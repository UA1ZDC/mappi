TEMPLATE = lib
TARGET = stationedit.plugin
QT += widgets

LOG_MODULE_NAME = stationedit
HEADERS  += stationeditplugin.h \
            stationeditwidget.h \
    stationdetailsdialog.h

SOURCES += stationeditplugin.cpp \
           stationeditwidget.cpp \
    stationdetailsdialog.cpp

FORMS    += stationeditwidget.ui \
    stationdetailsdialog.ui


LIBS += -lprotobuf            \
        -ltapp                \
        -ltdebug              \
        -ltrpc                \
        -ltcustomui           \
        -lmnmathtools         \
        -lmeteo.etc           \
        -lmeteo.textproto     \
        -lmeteo.geobasis      \
        -lmeteo.mainwindow    \
        -lmeteo.proto         \
        -lmeteo.global        \

include( $$(SRC)/include.pro )

DESTDIR = $(BUILD_DIR)/lib/plugins/meteo.app.widgets
