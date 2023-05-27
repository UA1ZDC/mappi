TEMPLATE = lib
TARGET   = meteo.map.customui

LOG_MODULE_NAME = vizdoc

QT      += gui printsupport

PUB_HEADERS = faxviewer.h

HEADERS =   faxscene.h \
            printoptionstab.h

SOURCES  =  faxviewer.cpp \
            faxscene.cpp \
            printoptionstab.cpp

FORMS    =  faxviewer.ui \
            printoptionstab.ui

LIBS    += -lmeteo.etc              \
           -ltdebug                 \
           -ltapp                   \
           -ltrpc                   \
           -lmeteo.proto            \
           -lmeteo.global           \
           -ltcustomui              \
           -lmeteo.mainwindow    

include ($$(SRC)/include.pro)
