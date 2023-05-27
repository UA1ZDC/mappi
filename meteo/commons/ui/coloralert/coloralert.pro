TEMPLATE = lib
TARGET   = meteo.coloralert
QT += widgets

LOG_MODULE_NAME = vizdoc

FORMS =     coloralertdlg.ui      \
            coloralertwidget.ui   \
            conditionedit.ui      \

HEADERS =   alertcheck.h          \
            coloralertdlg.h       \
            coloralertwidget.h    \
            conditionedit.h       \
            conditionitem.h       \
            delegate.h            \

SOURCES =   alertcheck.cpp        \
            coloralertdlg.cpp     \
            coloralertwidget.cpp  \
            conditionedit.cpp     \
            conditionitem.cpp     \
            delegate.cpp          \

LIBS +=     -lmeteo.textproto     \
            -lmeteo.proto         \

include( $$(SRC)/include.pro )

RESOURCES += \
    ../../rc/meteo.icons.qrc
