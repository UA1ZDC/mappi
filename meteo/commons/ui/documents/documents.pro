TARGET   = documentparams
TEMPLATE = lib
QT      += widgets

# Input
SOURCES  = documentparams.cpp \
           gmiform.cpp

FORMS    = documentparams.ui  \
           gmiform.ui

HEADERS  = documentparams.h \
           gmiform.h

LIBS    += -lmeteo.global              \
           -lmeteo.mainwindow          \
           -lmeteo.map                 \
           -ltdebug                    \
           -ltapp                      \
           -ltcustomui

include( $$(SRC)/include.pro )
