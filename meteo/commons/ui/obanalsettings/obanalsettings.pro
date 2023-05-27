TEMPLATE = lib
TARGET   = meteo.obanal.settings

QT += widgets

HEADERS = obanalsettings.h    \
          obanaloptions.h     \
          obanalresults.h     \
          selectvalues.h      \
          selectlevels.h      \
          selectcenters.h     \
          selectdatatypes.h   \
          appmain.h

SOURCES = obanalsettings.cpp  \
          obanaloptions.cpp   \
          obanalresults.cpp   \
          selectvalues.cpp    \
          selectlevels.cpp    \
          selectcenters.cpp   \
          selectdatatypes.cpp \
          appmain.cpp

FORMS =   obanalsettings.ui   \
          obanaloptions.ui    \
          obanalresults.ui    \
          selectvalues.ui

LIBS += -ltapp                \
        -ltdebug              \
        -ltrpc                \
        -lprotobuf            \
        -lmeteo.sql

LIBS += -lmeteo.planner            \
        -lmeteo.global             \
        -lmeteo.settings           \
        -lmeteo.proto              \
        -lmeteo.textproto          \
        -lmeteo.map                \
        -lmeteo.tileimage          \
        -lmeteo.map.dataexchange   \
        -lmeteo.mainwindow         \
        -lmeteo.map.view           \
        -ltcustomui

LIBS += \
        -lmeteo.settings           \
        -lprognoz.timesheeteditor



include( $$(SRC)/include.pro )
