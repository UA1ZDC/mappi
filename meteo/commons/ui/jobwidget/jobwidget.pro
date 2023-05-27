TARGET   =  jobwidget
TEMPLATE =  lib

QT += widgets

FORMS   = job.ui           \
          select.ui        \
          timesheet.ui

HEADERS = jobwidget.h      \
          selectwidget.h   \
          timesheet.h

SOURCES = jobwidget.cpp    \
          selectwidget.cpp \
          timesheet.cpp

include( $$(SRC)/include.pro )
