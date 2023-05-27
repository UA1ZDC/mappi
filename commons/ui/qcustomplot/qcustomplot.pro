TEMPLATE = lib
TARGET   = tcustomplot

DEFINES += QCUSTOMPLOT_COMPILE_LIBRARY

PUB_HEADERS =   qcustomplot.h \
                axisdiag.h    \
                axiswindrose.h

SOURCES =       qcustomplot.cpp \
                axisdiag.cpp \
                axiswindrose.cpp

include( $$(SRC)/include.pro )

CONFIG += warn_off

QT += widgets printsupport
