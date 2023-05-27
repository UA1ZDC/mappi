TEMPLATE  = lib

TARGET    = meteo.prewiewwidget
QT       += widgets printsupport

LOG_MODULE_NAME = documentviewlib


LIBS +=       -ltdebug              \
              -lpoppler-qt5

include( $$(SRC)/include.pro )

HEADERS += viewimagepreview.h

SOURCES += viewimagepreview.cpp
