TARGET   = vko
TEMPLATE = lib
QT      += widgets

LIBS    += -lmeteo.global              \
           -lmeteo.mainwindow          \
           -lmeteo.map                 \
           -ltdebug                    \
           -ltapp                      \
           -ltcustomui                 \
           -ltrpc

include( $$(SRC)/include.pro )

HEADERS += \
    vkowidget.h \
    vkostations.h

SOURCES += \
    vkowidget.cpp \
    vkostations.cpp

FORMS += \
    vkowidget.ui \
    vkostations.ui
