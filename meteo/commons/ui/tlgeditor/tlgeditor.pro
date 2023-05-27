TEMPLATE  = lib
TARGET    = meteo.tlgeditor
QT += widgets
LOG_MODULE_NAME = tlgeditor

HEADERS =   tlgeditorwindow.h                     \
            codecdlg.h                            \
            senddlg.h                             \

SOURCES =   tlgeditorwindow.cpp                   \
            codecdlg.cpp                          \
            senddlg.cpp                           \

FORMS =     tlgeditorwindow.ui                    \
            codecdlg.ui                           \
            senddlg.ui                            \

RESOURCES += tlgeditor.qrc                        \

LIBS +=     -lmeteodata                           \
            -ltcustomui                           \

LIBS +=     -lmeteo.msgparser                     \
            -lmeteo.etc                           \
            -lmeteo.decodeviewer

include( $$(SRC)/include.pro )


