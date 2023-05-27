TEMPLATE = lib
TARGET   = meteo.cron

LOG_MODULE_NAME = cron

HEADERS = runapp.h  \
          cronapp.h \
          appmain.h

SOURCES = runapp.cpp  \
          cronapp.cpp \
          appmain.cpp

LIBS += -lprotobuf        \
        -ltapp            \
        -ltdebug          \
        -ltrpc            \
        -ltsingleton      \
        -lmeteo.planner   \
        -lmeteo.textproto \
        -lmeteo.proto     \
        -lmeteo.global    \
        -lmeteo.settings

include( $$(SRC)/include.pro )
