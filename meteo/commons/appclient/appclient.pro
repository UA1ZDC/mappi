TARGET   = meteo.app.client
TEMPLATE = lib
QT      += widgets

LOG_MODULE_NAME = contrdiag

HEADERS =     appmain.h

SOURCES =     appmain.cpp

LIBS +=       -ltdebug              \
              -ltapp                \
              -ltrpc                \
              -lprotobuf            \              


LIBS +=       -lmeteo.appclient     \
              -lmeteo.proto         \
              -lcommons.proto       \
              -lmeteo.settings      \
              -lmeteo.global        \


include( $$(SRC)/include.pro )
