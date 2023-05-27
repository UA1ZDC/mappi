TEMPLATE =  lib
TARGET =    meteo.tlglog
QT +=       widgets
LOG_MODULE_NAME = applied

SOURCES =   lgmonitor.cpp            \
            checkpanel.cpp           \
            regexpconditiondialog.cpp\
            choicecondition.cpp      \
            entergroupname.cpp       \
            receivemessthread.cpp

HEADERS =   lgmonitor.h              \
            checkpanel.h             \
            regexpconditiondialog.h  \
            choicecondition.h        \
            entergroupname.h         \
            receivemessthread.h

LIBS =      -lmeteo.global           \
            -lmeteo.proto            \
            -lmeteo.nosql            \
            -lmeteo.viewheader       \
            -lmeteo.settings         \
            -lmeteo.customviewer

include( $$(SRC)/include.pro )

#DESTDIR =   $(BUILD_DIR)/lib/plugins/meteo.app.widgets

FORMS  +=   tlgmonitorform.ui        \
            regexpconditiondialog.ui \
            choicecondition.ui       \
            entergroupname.ui

RESOURCES +=
