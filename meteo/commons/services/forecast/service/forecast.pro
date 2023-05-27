TEMPLATE = lib
TARGET = meteo.forecast.service
QT += script

LOG_MODULE_NAME = forecast

SOURCES = tforecastdb.cpp \
          fizmethodbag.cpp \
          methodbag.cpp \
          tforecastservice.cpp \
          punktdb.cpp \
          appmain.cpp \
          tgradaciidb.cpp

HEADERS = tforecastdb.h \
          fizmethodbag.h \
          methodbag.h \
          tforecastservice.h \
          punktdb.h \
          appmain.h \
          tgradaciidb.h

LIBS +=       -ltrpc            \
              -ltapp            \
              -ltdebug          \
              -lprotobuf        \
              -lobanal          \
              -lmeteo.geobasis  \
              -lmeteo.proto     \
              -lmeteo.global    \
              -lmeteo.forecast  \
              -lzond            \
              -lmeteo.settings  \
               -lmnprocread


include( $$(SRC)/include.pro )
