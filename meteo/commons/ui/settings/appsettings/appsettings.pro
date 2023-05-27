TEMPLATE = lib
TARGET = meteo.appsettings
QT += widgets
HEADERS = editapp.h            \
          chooseicon.h         \
          argedit.h            \
          appstartargsview.h   \
          appstarteditor.h

SOURCES = \
          editapp.cpp          \
          chooseicon.cpp       \
          argedit.cpp          \
          appstartargsview.cpp \
          appstarteditor.cpp

FORMS = editapp.ui             \
        chooseicon.ui          \
        argedit.ui             \
        appstartargsview.ui    \
        appstarteditor.ui

include( $$(SRC)/include.pro )
