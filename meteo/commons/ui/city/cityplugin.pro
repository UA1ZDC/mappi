TEMPLATE = lib
TARGET = meteo.ui.city
QT += widgets

LOG_MODULE_NAME = vizdoc

SOURCES += \
           citywidget.cpp \
           displaysettingseditdialog.cpp \
    citypluginhandler.cpp

HEADERS += \
           citywidget.h \
           displaysettingseditdialog.h \
    citypluginhandler.h

FORMS +=  city.ui \
          servicewindow.ui \
          displaysettingseditdialog.ui

LIBS +=                 -lmeteo.etc                     \
                        -lmeteo.global                  \
                        -lmeteo.proto                   \
                        -lmnfuncs                       \
                        -lprotobuf                      \
                        -lmeteo.map.view                \
                        -lmeteo.settings

include( $$(SRC)/include.pro )

RESOURCES += \
  ../../rc/meteo.icons.qrc

