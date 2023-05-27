TEMPLATE = lib
TARGET   = meteo.meteotablo

QT += widgets

LOG_MODULE_NAME = vizdoc

HEADERS =     tablowindow.h           \
              tablowidget.h           \
              stationdlg.h

SOURCES =     tablowindow.cpp         \
              tablowidget.cpp         \
              stationdlg.cpp

FORMS +=      tablowindow.ui          \
              stationdlg.ui           \
    archive.ui \
    savepreset.ui

LIBS +=       -lmeteodata             \
              -lmeteo.textproto       \
              -lmeteo.coloralert    \

include( $$(SRC)/include.pro )
